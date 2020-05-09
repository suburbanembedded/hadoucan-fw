#!/usr/bin/env bash

trap 'exit -1' err

set -v

CONTAINER_ID=$(docker create -v $GITHUB_WORKSPACE:/tmp/workspace -it docker.pkg.github.com/suburbanembedded/hadoucan-fw/hadoucan-fw-tools:${GITHUB_REF##*/}  /bin/bash)
docker start $CONTAINER_ID
docker exec -u $(id -u):$(id -g) -w /tmp/workspace/ $CONTAINER_ID ./generate_cmake.sh
docker exec -u $(id -u):$(id -g) -w /tmp/workspace/ $CONTAINER_ID make -j`nproc` -C build/ram/debug/
docker exec -u $(id -u):$(id -g) -w /tmp/workspace/ $CONTAINER_ID make -j`nproc` -C build/ram/release/
docker stop $CONTAINER_ID

pushd $GITHUB_WORKSPACE/build/ram/debug
ls -la
id
sha256sum -b hadoucan-fw.elf hadoucan-fw.hex hadoucan-fw.bin | tee sha256.txt
tar -czf $GITHUB_WORKSPACE/hadoucan-fw-debug-$GITHUB_SHA.tar.gz    hadoucan-fw.elf hadoucan-fw.hex hadoucan-fw.bin sha256.txt
popd

pushd $GITHUB_WORKSPACE/build/ram/release
sha256sum -b hadoucan-fw.elf hadoucan-fw.hex hadoucan-fw.bin | tee sha256.txt
tar -czf $GITHUB_WORKSPACE/hadoucan-fw-release-$GITHUB_SHA.tar.gz  hadoucan-fw.elf hadoucan-fw.hex hadoucan-fw.bin sha256.txt
popd
