cmake_minimum_required(VERSION 3.0)

project(gizmo VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)

include_directories(src/common/include)

add_executable(gizmo src/front-end/scanner.c src/front-end/parser.c src/front-end/ast.c src/back-end/codegen.c src/back-end/heap.c src/compiler.c src/common/log.c src/front-end/symbols.c)

