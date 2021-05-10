cmake_minimum_required (VERSION 3.0)
project(gizmo-lang)
add_executable(gizmo src/front-end/scanner.c src/front-end/parser.c src/front-end/ast.c src/back-end/codegen.c src/compiler.c)
