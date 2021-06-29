#!/usr/bin/bash -e

# build the gizmo compiler, gizmoc
gcc -rdynamic -o gizmoc -O1 -g -fsanitize=address -fno-omit-frame-pointer -Isrc/common/include src/*.c src/front-end/*.c src/back-end/*.c src/common/*.c

# make sure it has exe permissions
chmod +x gizmoc

# run gizmo against the supplied gizmo source file, and generate LLVM IR output file gizmo_llvm_ir.ll
./gizmoc $1 -o gizmo_llvm_ir.ll

#if ! ./gizmoc $1 gizmo_llvm_ir.ll; then 
#    exit -1
#fi

# compile the gizmo_llvm_ir.ll file to native x86 assembly file native.s
llc --relocation-model=pic -o native.s gizmo_llvm_ir.ll

if [ $? -ne 0 ]; then
    echo "gizmo: Compiler failed to generate llvm ir code"
    exit -1
fi

# assemble and link the native x86 assembly to create a native executable
gcc -o $2 native.s

echo "Generated native executable as: $2"
