/*
 * EnOceanSpy.c
 * Use this tool to spy for EnOcean telegrams
 *
 *  Created on: 13.09.2013
 *      Author: hfunke
 *
 */

#include <stdio.h>
#include <unistd.h>	// Used for UART
#include <fcntl.h>	// Used for UART
#include <termios.h>	// Used for UART
#include "time.h"
#include <stdlib.h>
#include <curl/curl.h>

//#define USB300 "/dev/ttyAMA0"  // default EnOcean device

main( int argc, char *argv[] )
{
	printf("Starting EnOceanSpy...\n");

	// Check number of args
	if (argc > 2 || argc <=1 )
	{
		printf("Usage: EnOceanSpy <port_name>\n");
		printf("       e.g. EnOceanSpy /dev/ttyUSB0 \n");
		return -1;
	}


	// Check content of args
	if ((strcmp(argv[1], "/dev/ttyUSB0") != 0)
		&& (strcmp(argv[1], "/dev/ttyAMA0") !=0) )
	{
		printf("Error: %s is not a valid port name!\n", argv[1]);
		return -1;
	}


	// If pipe is used to write output in file set buffer to null
	setbuf(stdout, NULL);

	int uart0_filestream = -1;

	uart0_filestream = open(argv[1], O_RDWR | O_NDELAY);
	if (uart0_filestream == -1)
	{
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

	while (1)
	{
		// Check for incoming bytes
		if (uart0_filestream != -1)
		{
			// Give Raspberry a chance and wait before read :)
						sleep(1);

						// Read up to 255 characters from comport if they are there
						unsigned char rx_buffer[256];

						int rx_length = read(uart0_filestream, (void*)rx_buffer, 255);

						if (rx_length < 0)
						{
							// An error occurs if there are no bytes
							//printf("No bytes received");
						}
						else if (rx_length == 0)
						{
									// If there are no data then ignore and wait
						}
						else if (rx_length == 24)
						{
							// Some bytes received
									rx_buffer[rx_length] = '\0';
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

									char json[115];

									sprintf(json, "{\"timestamp\":\"%s\",\"temp\":%.1f,\"hum\":%.1f,\"batt\":%.1f,\"raw_bytes\":\"%02X%02X%02X\"}", dt, temp, hum, volt, rx_buffer[7],rx_buffer[8], rx_buffer[9]);
									printf("%s\n", json);
									CURL *curl;
									CURLcode res;

									curl_global_init(CURL_GLOBAL_DEFAULT);

									curl = curl_easy_init();
									if(curl) {
									struct curl_slist *headers = NULL;
									headers = curl_slist_append(headers, "Accept: application/json");
									headers = curl_slist_append(headers, "Content-Type: application/json");
									headers = curl_slist_append(headers, "charsets: utf-8");
									curl_easy_setopt(curl, CURLOPT_URL, "<KEEN IO POST URL>");
									curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
									curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
									res = curl_easy_perform(curl);
									/* Check for errors */
									if(res != CURLE_OK)
										 fprintf(stderr, "curl_easy_perform() failed: %s\n",
										 curl_easy_strerror(res));

									/* always cleanup */
									curl_easy_cleanup(curl);
					}

					curl_global_cleanup();

			}
		}
	}
}

