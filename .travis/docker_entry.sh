#!/bin/bash

include_env() {
    INCLUDE_ENV=""
    for e in $($1 -E -x c++ -v /dev/null 2>&1 | awk '/#include <...> search starts here:/,/End of search list./' | sed -e '1d' | sed -e '$d' | sed 's/([^)]*)//g' | sed 's/ //g'); do 
        INCLUDE_ENV="$INCLUDE_ENV -I $e";
    done
    echo $INCLUDE_ENV
}

uname -a
echo "deb http://ftp.us.debian.org/debian testing main contrib non-free" >> /etc/apt/sources.list 
apt-get update
apt-get -y -qq upgrade
apt-get -y -qq autoremove
apt-get install -y -qq -t testing g++
apt-get install -f -y -qq
apt-get install -y -qq aptitude
aptitude -yq install\
    build-essential\
    zlib1g-dev\
    g++-multilib\
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
    libsdl2-ttf-dev > /dev/null

g++ --version
clang++ --version

echo "$(include_env g++) -I /usr/include/SDL2" > /src/gcc_include_path
echo "$(include_env clang++)" > /src/emcc_include_path
