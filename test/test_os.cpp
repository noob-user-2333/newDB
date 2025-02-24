//
// Created by user on 25-2-23.
//
#include "test.h"
#include "os.h"


namespace iedb
{
TEST(os,checksum)
{
    auto random = test::get_random();
    auto checksum = os::calculate_checksum(random,test::random_buffer_size);
    auto normal = 0UL;
    auto array = static_cast<const uint64*>(random);
    for (auto i = 0; i < test::random_buffer_size / sizeof(uint64); i++)
        normal += array[i];
    ASSERT_EQ(checksum, normal);

}

}