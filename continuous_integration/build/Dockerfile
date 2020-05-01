FROM ubuntu:18.04

RUN echo "Acquire::http::Proxy \"http://10.0.0.1:3142\";" > /etc/apt/apt.conf.d/02proxy

RUN apt-get update && apt-get install -y wget rsync libc6-i386 libusb-0.1-4 libwebkitgtk-3.0-0 libncurses5

RUN apt-get update && apt-get install -y build-essential git cmake python python-serial

RUN groupadd buildbot && useradd --no-log-init --create-home --home-dir /home/buildbot -g buildbot buildbot

USER buildbot
WORKDIR /home/buildbot

RUN wget http://download.atollic.com/TrueSTUDIO/installers/Atollic_TrueSTUDIO_for_STM32_linux_x86_64_v9.3.0_20190212-0734.tar.gz
RUN tar -xzf Atollic_TrueSTUDIO_for_STM32_linux_x86_64_v9.3.0_20190212-0734.tar.gz
WORKDIR /home/buildbot/Atollic_TrueSTUDIO_for_STM32_9.3.0_installer

USER root
RUN mkdir -p /opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ && tar -xzf install.data -C /opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/

USER buildbot
WORKDIR /home/buildbot
RUN git clone --depth 1 --branch master --single-branch https://github.com/suburbanembedded/hadoucan-fw.git
WORKDIR /home/buildbot/hadoucan-fw
RUN git clone submodule update --init
RUN ./generate_cmake.sh
RUN make -j`nproc` -C build/ram/release
