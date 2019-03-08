#!/bin/bash

uname -a
echo "deb http://ftp.us.debian.org/debian testing main contrib non-free" >> /etc/apt/sources.list
sudo apt-get update
sudo apt-get -y -qq upgrade
sudo apt-get -y autoremove
sudo apt-get install -y -qq -t testing g++
sudo apt-get install -y 
sudo apt-get install -f -y
sudo apt-get install -y -qq aptitude
sudo aptitude -y install\
    build-essential\
    zlib1g-dev\
    gcc-7\
    g++-7\
    g++-multilib\
    clang\
    subversion\
    make\
    cmake\
    python\
    libtool\
    libsdl2-2.0-0\
    libsdl2-dev\
    libsdl2-gfx-dev\
    libsdl2-image-2.0-0\
    libsdl2-image-dev\
    libsdl2-ttf-2.0-0\
    libsdl2-ttf-dev
