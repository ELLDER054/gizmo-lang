#!/usr/bin/bash -e
clang -o gizmoc -O1 -g -fsanitize=address -fno-omit-frame-pointer src/*.c src/front-end/*.c src/back-end/*.c
chmod +x gizmoc
./gizmoc $1 gizmo_llvm_ir.ll
llc -o $2 gizmo_llvm_ir.ll
chmod +x $2
