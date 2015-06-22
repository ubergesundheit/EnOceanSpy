FROM armhfbuild/alpine:3.1

RUN apk add --update curl-dev gcc g++ && rm -rf /var/cache/apk/*

COPY EnOceanSpy.c EnOceanSpy.c

CMD sh -c 'gcc -c EnOceanSpy.c && gcc -lcurl -o EnOceanSpy EnOceanSpy.o && ./EnOceanSpy'

