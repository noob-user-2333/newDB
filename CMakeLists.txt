cmake_minimum_required(VERSION 3.22)
project(newDB)

set(CMAKE_CXX_STANDARD 17)
link_directories(lib)
include_directories(include)
aux_source_directory(src SOURCES)
add_compile_options(-mavx2)

add_executable(newDB  ${SOURCES} main.cpp
        include/journal.h
        include/file.h
        #roaring_bitmap
        third-part/roaring_bitmap/roaring.hh
        third-part/roaring_bitmap/roaring.h
        third-part/roaring_bitmap/roaring.c
)
# 添加头文件目录
target_include_directories(newDB PRIVATE include)

