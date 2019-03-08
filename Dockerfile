FROM trzeci/emscripten:sdk-incoming-64bit
ADD docker_entry.sh /
ENTRYPOINT /docker_entry.sh
