#!/usr/bin/bash -e
#gcc -o gizmoc *.c
clang -o gizmoc -O1 -g -fsanitize=address -fno-omit-frame-pointer *.c
chmod +x gizmoc
./gizmoc
