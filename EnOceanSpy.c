/*
 * EnOceanSpy.c
 * Use this tool to spy for EnOcean telegrams
 *
 *  Created on: 13.09.2013
 *      Author: hfunke
 *
 */

#include <stdio.h>
#include <unistd.h> // Used for UART
#include <fcntl.h>  // Used for UART
#include <termios.h>    // Used for UART
#include "time.h"
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define ENV_DEVICE "ENOCEAN_SPY_DEVICE"
#define ENV_URL "ENOCEAN_SPY_URL"
#define ENV_CREDENTIALS "ENOCEAN_SPY_CREDENTIALS"

#define FAFT60_ADDR "01005170"
#define FIFT63_ADDR "0088271E"

#define FAFT60_COLLECTION "tmps_out"
#define FIFT63_COLLECTION "tmps_in1"

main(int argc, char *argv[]) {
  // check for presence of environment vars

  if (!getenv(ENV_DEVICE) || !getenv(ENV_URL)) {
    printf("Necessary environment vars not found, please set %s and %s\n", ENV_DEVICE, ENV_URL);
    return -1;
  }

  int has_credentials = 0;
  char credentials[200] = "Authorization: Basic ";

  if (getenv(ENV_CREDENTIALS)) {
    has_credentials = 1;
    strcat(credentials, getenv(ENV_CREDENTIALS));
    //credentials = getenv(ENV_CREDENTIALS);
  }

  char* device;
  device = getenv(ENV_DEVICE);
  char* url;
  url = getenv(ENV_URL);

  // Check content of args
  if ((strcmp(device, "/dev/ttyUSB0") != 0) && (strcmp(device, "/dev/ttyAMA0") !=0) ) {
    printf("Error: %s is not a valid port name!\n", device);
    return -1;
  }

  printf("Starting EnOceanSpy listening on %s...\n", device);

  // If pipe is used to write output in file set buffer to null
  setbuf(stdout, NULL);

  int uart0_filestream = -1;

  uart0_filestream = open(device, O_RDWR | O_NDELAY);
  if (uart0_filestream == -1) {
    printf("Error! Unable to open UART. Maybe UART in use by another application or device is not ready.\n");
    return -1;
  }

  struct termios options;
  tcgetattr(uart0_filestream, &options);
  options.c_cflag = B57600 | CS8 | CLOCAL | CREAD;
  options.c_iflag = IGNPAR | ICRNL | PARENB;
  options.c_oflag = 0;
  options.c_lflag = 0;
  tcflush(uart0_filestream, TCIFLUSH);
  tcsetattr(uart0_filestream, TCSANOW, &options);

  while (1) {
    // Check for incoming bytes
    if (uart0_filestream != -1) {
      // Give Raspberry a chance and wait before read :)
      sleep(1);

      // Read up to 255 characters from comport if they are there
      unsigned char rx_buffer[256];

      int rx_length = read(uart0_filestream, (void*)rx_buffer, 255);

      if (rx_length < 0) {
        // An error occurs if there are no bytes
        //printf("No bytes received");
      } else if (rx_length == 0) {
        // If there are no data then ignore and wait
      } else if (rx_length == 24) {
        // Some bytes received
        rx_buffer[rx_length] = '\0';

        char address[9];
        char collection[9];
        int is_my_device = 0;
        sprintf(address, "%02X%02X%02X%02X", rx_buffer[11],rx_buffer[12], rx_buffer[13], rx_buffer[14]);

        if (strcmp(FAFT60_ADDR, address) == 0) {
          strcpy(collection, FAFT60_COLLECTION);
          is_my_device = 1;
        }
        if (strcmp(FIFT63_ADDR, address) == 0) {
          strcpy(collection, FIFT63_COLLECTION);
          is_my_device = 1;
        }

        if(is_my_device == 1) {
          struct tm *utc;
          time_t t;
          t = time(NULL);
          utc = localtime(&t);

          char dt[80];
          strftime(dt, 80, "%Y-%m-%dT%H:%M:%S%z", utc);

          double volt, hum, temp, volt_multiply_by;

          // volt
          volt_multiply_by = 0.0258039;
          volt = volt_multiply_by * rx_buffer[7];

          // humidity
          hum = (double)rx_buffer[8] / 2.5;

          // temperature
          temp = 0.32 * (double)rx_buffer[9] - 20.0;

          char json[200];

          sprintf(json, "{\"collection\":\"%s\",\"timestamp\":\"%s\",\"data\":{\"temp\":%.1f,\"hum\":%.1f,\"batt\":%.1f,\"raw_bytes\":\"%02X%02X%02X\",\"addr\":\"%s\"}}", collection, dt, temp, hum, volt, rx_buffer[7],rx_buffer[8], rx_buffer[9], address);
          printf("%s ", json);

          // init curl
          CURL *curl;
          CURLcode res;

          curl_global_init(CURL_GLOBAL_DEFAULT);

          curl = curl_easy_init();
          if(curl) {
            struct curl_slist *headers = NULL;
            if (has_credentials == 1) {
              headers = curl_slist_append(headers, credentials);
            }
            headers = curl_slist_append(headers, "Accept: application/json");
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, "charsets: utf-8");
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
            res = curl_easy_perform(curl);
            /* Check for errors */
            if (res != CURLE_OK) {
              fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
            }

            /* always cleanup */
            curl_easy_cleanup(curl);
          }

          curl_global_cleanup();
          printf("\n");
        }
      }
    }
  }
}

