cmake_minimum_required(VERSION 3.22)
project(newDB)

set(CMAKE_CXX_STANDARD 17)
link_directories(lib)
include_directories(include)
aux_source_directory(src SOURCES)
#file(GLOB HEADER_FILES "${CMAKE_SOURCE_DIR}/include/*" )


add_executable(newDB
        #${HEADER_FILES}
        ${SOURCES}
        main.cpp
        #roaring_bitmap
        third-part/roaring_bitmap/roaring.hh
        third-part/roaring_bitmap/roaring.h
        third-part/roaring_bitmap/roaring.c

)
# 添加头文件目录
target_include_directories(newDB PRIVATE include)

