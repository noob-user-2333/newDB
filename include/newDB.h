//
// Created by user on 24-11-26.
//

#ifndef NEWDB_H
#define NEWDB_H
#include <cstdint>

extern "C"{
    struct DBreader;
    typedef struct DBreader DBreader;
    int IEDB_execute_sql_without_reader(const char *sql);
    int IEDB_execute_query_sql(const char *sql,DBreader *reader);



    DBreader* IEDB_reader_malloc();
    void IEDB_reader_free(const DBreader* reader);
    int IEDB_reader_next(DBreader* reader);
    int IEDB_reader_get_column_bytes(DBreader* reader,int column);
    int64_t IEDB_reader_get_column_int(DBreader* reader,int column);
    double IEDB_reader_get_column_double(DBreader* reader,int column);
    const char* IEDB_reader_get_column_string(DBreader* reader,int column);
}




#endif //NEWDB_H
