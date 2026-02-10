#include <iostream>
#include <sqlite3.h>

int main(int argc, char** argv) {

    if (argc < 3) {
        std::cout << "Usage: db_file sql_statement" << std::endl;
        return 1;
    }

    sqlite3* db;
    sqlite3_stmt* stmt;
    int nCol;
    int rc;

    rc = sqlite3_open(
        argv[1],
        &db
    );
    if (rc != SQLITE_OK) {std::cerr << "Error initializing table." << std::endl; return 1;}

    rc = sqlite3_prepare(
        db, 
        argv[2], 
        -1, 
        &stmt, 
        nullptr
    );
    if (rc != SQLITE_OK) {std::cerr << "Error issuing statement." << std::endl; return 1;}

    nCol = sqlite3_column_count(stmt);
    int g = 1;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("Row %d:\n", g++);
        
        for (int i = 0; i < nCol; ++i) {

            printf("    %s = %s\n",
                sqlite3_column_name(stmt, i),
                sqlite3_column_text(stmt, i)
            );

        }
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return 0;

}