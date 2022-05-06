FROM ubuntu:trusty-20191217

RUN apt update &&\
  apt install -y gcc-4.7-arm-linux-gnueabi libgtk2.0-dev &&\
  cd /usr/bin &&\
  ln -s arm-linux-gnueabi-gcc-4.7 arm-linux-gnueabi-gcc &&\
  ln -s arm-linux-gnueabi-cpp-4.7 arm-linux-gnueabi-cpp &&\
  ln -s arm-linux-gnueabi-gcc-ar-4.7 arm-linux-gnueabi-gcc-ar &&\
  ln -s arm-linux-gnueabi-gcc-nm-4.7 arm-linux-gnueabi-gcc-nm &&\
  ln -s arm-linux-gnueabi-gcc-ranlib-4.7 arm-linux-gnueabi-gcc-ranlib &&\
  ln -s arm-linux-gnueabi-gcov-4.7 arm-linux-gnueabi-gcov

WORKDIR /src
