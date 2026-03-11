#ifndef DATABASECLASS_H
#define DATABASECLASS_H

#include <sqlite3.h>
#include "BookClasses.h"

class Database {
    
    private:

        const char* Table_Name;
        const char* Table_Path;

    public:

        sqlite3* connection = nullptr;
        NewBook_Buffer Book_Buffer;

        Database(const char* _Table_Name, const char* _Table_Path) {

            Table_Name = _Table_Name;
            Table_Path = _Table_Path;

            int rc = sqlite3_open(
                Table_Path,
                &connection
            );

            RETURN_CODE_CHECK(rc, "Error initializing/opening database.");
            return;

        };

        ~Database() {};

};

#endif