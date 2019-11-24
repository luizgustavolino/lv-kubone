FROM alpine:3.10
RUN apk add --update alpine-sdk

RUN mkdir /tmp/src
COPY src /tmp/src/
RUN g++ /tmp/src/main.cpp -o /snake

EXPOSE 1996
CMD ["/snake"]