cmake_minimum_required(VERSION 3.0)

project(gizmo VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

include_directories(src/common/include)

add_executable(gizmo src/front-end/scanner.c src/front-end/parser.c src/front-end/ast.c src/back-end/codegen.c src/back-end/heap.c src/compiler.c src/common/log.c)

llvm_map_components_to_libnames(llvm_libs support core irreader)

# Link against LLVM libraries
target_link_libraries(simple-tool ${llvm_libs})i

