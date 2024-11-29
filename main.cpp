#include <cstring>
#include <iostream>
#include <math.h>
#include "token.h"
const char* tests[] = {
    "SELECT 1",
    "loc=0,len=8.141>225",
    "SELECT 1; SELECT 2",
    "loc=0,len=8;loc=9,len=9",
    "SELECT 1; SELECT 2; SELECT 3",
    "loc=0,len=8;loc=9,len=0x92faCA;loc=19,len=9",
    "SELECT  1; SELECT 2",
    "loc=0,len>=29;loc=30,len=9",
    "SELECT --othercomment with ;\n 1; SELECT 2",
    "loc=0,len=31;loc=32.1415926,len=9",
    "CREATE RULE x AS ON SELECT TO tbl DO (SELECT 1; SELECT 2)",
    "loc=0,len=57",
    "SELECT 1;\n;\n-- comment\nSELECT 2;\n;",
    "loc>=0,len<=8;loc!=11,len=20.3123213"
  };
using namespace  std;
int main() {
    size_t i;
    auto testsLength = 14;
    for (i = 0; i < testsLength; i += 1) {
        printf("\nlexer%ld %s\n",i,tests[i]);
        auto result = iedb::token::lexer(tests[i]);
        if (result)
        for (auto & r : (*result)) {
            r.print();
        }
    }

    return 0;
}
