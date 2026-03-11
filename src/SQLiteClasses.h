#ifndef SQLITE_CLASSES_H
#define SQLITE_CLASSES_H

#include <memory>
#include <string>
#include <cstring>
#include <iostream>

#include <sqlite3.h>

const unsigned char* InsertWithNullCheck(const unsigned char* str) {
    return str ? str : reinterpret_cast<const unsigned char*>("");
}

void RETURN_CODE_CHECK(int returnCode, std::string cerrMessage = "Unable to create statement.", std::string sqlString = "") {

    if (returnCode != 0) {
        std::cerr << cerrMessage << " :::> " << sqlString << std::endl;
        std::exit(EXIT_FAILURE);
    }

}

using statement = std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>;
statement create_statement(sqlite3* db, std::string sqlString) {

    sqlite3_stmt* stmt;
    int returnCode = sqlite3_prepare_v2(
        db,
        sqlString.c_str(),
        -1,
        &stmt,
        nullptr
    );

    RETURN_CODE_CHECK(returnCode, "Unable to issue SQL statement", sqlString);
    return statement(stmt, sqlite3_finalize);

}

struct precompiled_sqliteStatements {
    
    sqlite3* db;
    statement fetch_stmt;
    statement insert_stmt;
    statement delete_stmt;
    statement download_stmt;
    
    precompiled_sqliteStatements(sqlite3* db_ptr) :
    
        db(db_ptr),
        
        fetch_stmt(
            create_statement(
                db,
                "SELECT id, title, author, file_type, file_size, date_modified FROM BOOKS WHERE title IS NOT NULL;"
            )
        ),

        insert_stmt(
            create_statement(
                db,
                "INSERT INTO BOOKS(title, author, file_type, file_size, date_modified, binary) VALUES (?, ?, ?, ?, ?, ?);"
            )
        ),
        
        delete_stmt(
            create_statement(
                db,
                "DELETE FROM BOOKS WHERE id = ?;"
            )
        ),
        
        download_stmt(
            create_statement(
                db,
                "SELECT title, file_type, binary FROM BOOKS WHERE id = ?"
            )
        )
        
    {}
        
    ~precompiled_sqliteStatements() = default;
        
};

#endif