FROM ubuntu:18.04

RUN apt-get update && apt-get install -y wget

RUN apt-get update && apt-get install -y build-essential git cmake gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf

RUN groupadd buildbot && useradd --no-log-init --create-home --home-dir /home/buildbot -g buildbot buildbot

USER buildbot
WORKDIR /home/buildbot

#ENV FILEID=1mm5Jp-ce4sFJX6crsmJZU6g_iDZJ1Cht
#RUN wget --load-cookies /tmp/cookies.txt "https://docs.google.com/uc?export=download&confirm=$(wget --quiet --save-cookies /tmp/cookies.txt --keep-session-cookies "https://docs.google.com/uc?export=download&id=${FILEID}" -O- | sed -rn 's/.*confirm=([0-9A-Za-z_]+).*/\1\n/p')&id=${FILEID}" -O Atollic_TrueSTUDIO_for_STM32_linux_x86_64_v9.3.0_20190212-0734.tar.gz && rm -rf /tmp/cookies.txt

RUN wget http://download.atollic.com/TrueSTUDIO/installers/Atollic_TrueSTUDIO_for_STM32_linux_x86_64_v9.3.0_20190212-0734.tar.gz

RUN tar -xzf Atollic_TrueSTUDIO_for_STM32_linux_x86_64_v9.3.0_20190212-0734.tar.gz

WORKDIR /home/buildbot/Atollic_TrueSTUDIO_for_STM32_9.3.0_installer
USER root
RUN yes | ./install.sh --help