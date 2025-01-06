#include "db_data_manager.h"

using namespace iedb;
constexpr char path[] = "/dev/shm/iedb.db";
int main() {
    auto db = db_data_manager::open(path);
    db->insert_record("test","ok",3);
    return 0;
}
