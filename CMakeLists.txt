cmake_minimum_required(VERSION 3.0)

project(gizmo VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)

add_executable(gizmo compiler.c lexer.c parser.c tools.c symbol.c ast.c generator.c)

