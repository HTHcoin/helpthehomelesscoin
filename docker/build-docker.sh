#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR/..

DOCKER_IMAGE=${DOCKER_IMAGE:-HelpTheHomelessCoin/HelpTheHomelessd-develop}
DOCKER_TAG=${DOCKER_TAG:-latest}

BUILD_DIR=${BUILD_DIR:-.}

rm docker/bin/*
mkdir docker/bin
cp $BUILD_DIR/src/HelpTheHomelessd docker/bin/
cp $BUILD_DIR/src/HelpTheHomeless-cli docker/bin/
cp $BUILD_DIR/src/HelpTheHomeless-tx docker/bin/
strip docker/bin/HelpTheHomelessd
strip docker/bin/HelpTheHomeless-cli
strip docker/bin/HelpTheHomeless-tx

docker build --pull -t $DOCKER_IMAGE:$DOCKER_TAG -f docker/Dockerfile docker
