#!/usr/bin/env bash

trap 'exit -1' err

set -v

CONTAINER_ID=$(docker create -v $GITHUB_WORKSPACE:/tmp/workspace -it docker.pkg.github.com/suburbanembedded/hadoucan-fw/hadoucan-fw-tools:${GITHUB_REF##*/}  /bin/bash)
docker start $CONTAINER_ID
docker exec -u root -w /tmp/workspace/ $CONTAINER_ID ./generate_cmake.sh
docker exec -u root -w /tmp/workspace/ $CONTAINER_ID make -j`nproc` -C build/ram/debug/
docker exec -u root -w /tmp/workspace/ $CONTAINER_ID make -j`nproc` -C build/ram/release/
docker stop $CONTAINER_ID

tar -czf $GITHUB_WORKSPACE/canusbfdiso-debug.tar.gz   -C $GITHUB_WORKSPACE/build/ram/debug   canusbfdiso.elf canusbfdiso.hex canusbfdiso.bin
tar -czf $GITHUB_WORKSPACE/canusbfdiso-release.tar.gz -C $GITHUB_WORKSPACE/build/ram/release canusbfdiso.elf canusbfdiso.hex canusbfdiso.bin
