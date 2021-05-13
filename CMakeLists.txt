cmake_minimum_required(VERSION 3.0)

project(Gizmo VERSION 1.0)

add_executable(gizmo src/compiler.c src/front-end/parser.c src/front-end/scanner.c src/front-end/ast.c src/back-end/codegen.c)
