#include <any>

#include "parser.h"
#include "test/test.h"
#include "test/timer.h"
#include <iostream>
#include <ostream>
#include <gtest/gtest.h>

#include "initializer.h"
#include "os.h"
using namespace iedb;
char path[1024] = "/dev/shm/iedb.db";
char original_buffer[0x1000];
char buffer[0x2000];
int main() {
    // memcpy(path + 5,path,16);
    // printf("%s",path);
    test::run();
    return 0;
}
