cmake_minimum_required(VERSION 3.22)
project(newDB)
set(CMAKE_CXX_STANDARD 17)

# Google Test 设置
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
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
        src/newDB.cpp

        #test
        test/test.h
        test/function.cpp
        #        test/test_pager.cpp
#        test/test_btree_page.cpp
#        test/test_btree.cpp
#        test/test_btree_internal_page.cpp
)
target_link_libraries(newDB ${GTEST_LIBRARIES} pthread xxhash)

add_library(newIEDB STATIC ${SOURCES})
target_include_directories(newIEDB PUBLIC include third-part/roaring_bitmap third-part/sqlite third-part/cjson)
target_link_libraries(newIEDB pthread)
target_include_directories(newIEDB PUBLIC include third-part/roaring_bitmap third-part/sqlite third-part/cjson)
install(TARGETS newIEDB
        EXPORT IEDB
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include)
target_link_libraries(newDB newIEDB ${GTEST_LIBRARIES} pthread)