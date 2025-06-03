FROM ubuntu:22.04

USER root

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
     wget               \
     rsync              \
     && apt-get clean

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
     build-essential \
     git             \
     cmake           \
     python          \
     python-serial   \
     doxygen         \
     graphviz        \
     mscgen          \
     && apt-get clean

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
     gcc-arm-none-eabi \
     && apt-get clean

RUN groupadd buildbot && useradd --no-log-init --create-home --home-dir /home/buildbot -g buildbot buildbot

USER buildbot
WORKDIR /home/buildbot

RUN mkdir ~/.ssh && echo 'github.com ssh-rsa AAAAB3NzaC1yc2EAAAABIwAAAQEAq2A7hRGmdnm9tUDbO9IDSwBK6TbQa+PXYPCPy6rbTrTtw7PHkccKrpp0yVhp5HdEIcKr6pLlVDBfOLX9QUsyCOV0wzfjIJNlGEYsdlLJizHhbn2mUjvSAHQqZETYP81eFzLQNnPHt4EVVUh7VfDESU84KezmD5QlWpXLmvU31/yMf+Se8xhHTvKSCZIFImWwoG6mbUoWf9nzpIoaSjB+weqqUUmpaaasXVal72J+UX2B+2RPW3RcT0eOzQgqlJL3RKrTJvdsjE3JEAvGq3lGHSZXy28G3skua2SmVi/w4yCE6gbODqnTWlg7+wC604ydGXA8VJiS5ap43JXiUFFAaQ==' >> ~/.ssh/known_hosts
