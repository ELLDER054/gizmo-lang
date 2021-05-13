cmake_minimum_required(VERSION 3.0)

project(gizmo VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)

add_executable(gizmo src/front-end/scanner.c src/front-end/parser.c src/front-end/ast.c src/back-end/codegen.c src/compiler.c)
