#!/usr/bin/env bash

trap 'exit -1' err

set -v

CONTAINER_ID=$(docker create -v $GITHUB_WORKSPACE:/tmp/workspace -it docker.pkg.github.com/suburbanembedded/hadoucan-fw/hadoucan-fw-tools:latest  /bin/bash)
docker start $CONTAINER_ID
docker exec -u root -w /tmp/workspace/ $CONTAINER_ID ./generate_cmake.sh
docker exec -u root -w /tmp/workspace/ $CONTAINER_ID make -j`nproc` -C build/ram/debug/
docker exec -u root -w /tmp/workspace/ $CONTAINER_ID make -j`nproc` -C build/ram/release/
docker stop $CONTAINER_ID

tar -czf $GITHUB_WORKSPACE/canusbfdiso-debug.tar.gz $GITHUB_WORKSPACE/build/ram/debug/canusbfdiso.elf $GITHUB_WORKSPACE/build/ram/debug/canusbfdiso.hex $GITHUB_WORKSPACE/build/ram/debug/canusbfdiso.bin
tar -czf $GITHUB_WORKSPACE/canusbfdiso-release.tar.gz $GITHUB_WORKSPACE/build/ram/release/canusbfdiso.elf $GITHUB_WORKSPACE/build/ram/release/canusbfdiso.hex $GITHUB_WORKSPACE/build/ram/release/canusbfdiso.bin