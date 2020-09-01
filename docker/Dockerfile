FROM debian:stretch
LABEL maintainer="HTH Developers <hthcoin@gmail.com>"
LABEL description="Dockerised HTHCore, built from Travis"

RUN apt-get update && apt-get -y upgrade && apt-get clean && rm -fr /var/cache/apt/*

COPY bin/* /usr/bin/
