#!/usr/bin/env bash

trap 'exit -1' err

set -v

CONTAINER_ID=$(docker create -v $GITHUB_WORKSPACE:/tmp/workspace -it docker.pkg.github.com/suburbanembedded/hadoucan-fw/hadoucan-fw-tools:latest  /bin/bash)
docker start $CONTAINER_ID
docker exec -u root -w /tmp/workspace/ $CONTAINER_ID ./generate_cmake.sh
docker exec -u root -w /tmp/workspace/ $CONTAINER_ID make -C build/ram/debug/
docker exec -u root -w /tmp/workspace/ $CONTAINER_ID make -C build/ram/release/
docker stop $CONTAINER_ID
