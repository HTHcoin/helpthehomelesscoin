#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR/..

DOCKER_IMAGE=${DOCKER_IMAGE:-helpthehomeless/helpthehomelessd-develop}
DOCKER_TAG=${DOCKER_TAG:-latest}

BUILD_DIR=${BUILD_DIR:-.}

rm docker/bin/*
mkdir docker/bin
cp $BUILD_DIR/src/helpthehomelessd docker/bin/
cp $BUILD_DIR/src/helpthehomeless-cli docker/bin/
cp $BUILD_DIR/src/helpthehomeless-tx docker/bin/
strip docker/bin/helpthehomelessd
strip docker/bin/helpthehomeless-cli
strip docker/bin/helpthehomeless-tx

docker build --pull -t $DOCKER_IMAGE:$DOCKER_TAG -f docker/Dockerfile docker
