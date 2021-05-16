#!/usr/bin/bash -e
clang -o gizmoc -O1 -g -fsanitize=address -fno-omit-frame-pointer src/*.c src/front-end/*.c src/back-end/*.c
chmod +x gizmoc
./gizmoc $1 gizmo_llvm_ir.ll
llc -o asmbly gizmo_llvm_ir.ll
llvm-as -o $2 asmbly
chmod +x $2
