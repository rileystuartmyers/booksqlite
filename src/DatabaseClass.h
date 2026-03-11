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

        void Populate_Buffer(std::string Path) {

            Book_Buffer.Selected_Path = Path;

            std::filesystem::path ParsedPath(Path);
            strcpy(Book_Buffer.file_name, ParsedPath.stem().string().c_str());
            strcpy(Book_Buffer.extension, ParsedPath.extension().string().c_str());
            
            Book_Buffer.file_size_int = std::filesystem::file_size(ParsedPath);
            std::string file_size_str = std::to_string(Book_Buffer.file_size_int);
            strcpy(Book_Buffer.file_size, file_size_str.c_str());
            strcat(Book_Buffer.file_size, " B");
            
            time_t now = time(0);
            strftime(Book_Buffer.date_time, sizeof(Book_Buffer.date_time), "%m/%d/%Y", localtime(&now));

            return;
            
        };

        ~Database() {};

};

#endif