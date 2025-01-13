#include "db.h"
#include "db_data_manager.h"
#include "parser.h"

using namespace iedb;
constexpr char path[] = "/dev/shm/iedb.db";
constexpr char create[] = "CREATE TABLE test("
                          "id int,"
                          "score float,"
                          "name text);";
constexpr char insert[] = "INSERT INTO test VALUES (0x12,4.345,'gooddafsafsdfsdf');";
constexpr char query[] = "SELECT * FROM test where  (id * (3 -2))  >  (2 * 4) /(5 - 3)% 2 *(2 + (2 - 4))  ;";
int main() {
    auto db = db::open(path);
    // db->sql_execute(create);
    // for (int i = 0 ;i < 16; i++)
    // db->sql_execute_without_reader(create);
        db->sql_execute(query);
    // db->insert_record("test","ok",3);
    // auto it = db->get_record_iterator_write_transaction("test");
    // while (it.next())
    // {
        // it.update_record(data,10);
    // }
    return 0;
}
