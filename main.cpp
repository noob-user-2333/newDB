#include <cstring>
#include <iostream>
#include <math.h>
#include "parser.h"
const char* tests[] = {
    "select * from test  where id > 3 order by id  ;",
  };
using namespace  std;
int main() {
    size_t i;
    auto testsLength = 1;
    for (i = 0; i < testsLength; i += 1) {
        printf("\nlexer%ld %s\n",i,tests[i]);
        auto res = iedb::parse(tests[i]);
        res->print();
    }

    return 0;
}
