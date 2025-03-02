#include <any>

#include "parser.h"
#include "test/test.h"
#include "test/timer.h"
#include <iostream>
#include <ostream>
#include <gtest/gtest.h>
#include <sys/time.h>

#include "initializer.h"
#include "os.h"
using namespace iedb;
char path[1024] = "/dev/shm/test/test.db";
char original_buffer[0x1000];
char buffer[0x2000];
int64 a = -1;
uint64 b = 2;
std::array<uint8,1024 * 1024> test1;
int main() {


    assert(test::run() == 0);
    // test::run();
    return 0;
}
