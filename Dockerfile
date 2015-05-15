FROM alpine

RUN apk add --update curl-dev && rm -rf /var/cache/apk/*

COPY EnOceanSpy /usr/local/bin/EnOceanSpy

CMD ["EnOceanSpy"]

