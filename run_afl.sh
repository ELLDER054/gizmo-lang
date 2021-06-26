#!/usr/bin/bash -e

# make sure afl is installed
sudo apt install -y afl++

# rebuild gizmo with afl
rm -rf afl-build
mkdir afl-build
cd afl-build
CC=`which afl-clang` cmake ..
make
cd ..

# run afl
mkdir -p afl-output
afl-fuzz -i afl-input -o afl-output -- ./afl-build/gizmo @@
