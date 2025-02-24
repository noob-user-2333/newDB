cmake_minimum_required(VERSION 3.22)
project(newDB)
set(CMAKE_CXX_STANDARD 17)

# Google Test 设置
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

link_directories(lib)
include_directories(include)
aux_source_directory(src SOURCES)
add_compile_options(-mavx2)
file(GLOB INCLUDE "include/*")

add_executable(newDB  ${SOURCES} ${INCLUDE} main.cpp
        #roaring_bitmap
        third-part/roaring_bitmap/roaring.hh
        third-part/roaring_bitmap/roaring.h
        third-part/roaring_bitmap/roaring.c
        #sqlite
        third-part/sqlite/sqlite3.c
        third-part/sqlite/sqlite3.h
        #cJson
        third-part/cjson/cJSON.c
        third-part/cjson/cJSON.h

        #test
        test/test.h
        test/function.cpp
#        test/test_pager.cpp
        test/test_btree_page.cpp
        src/btree-cursor.cpp
        test/test_btree.cpp
)

target_link_libraries(newDB ${GTEST_LIBRARIES} pthread)