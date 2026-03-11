#ifndef SQLITE_ASSETS_H
#define SQLITE_ASSETS_H

#include <memory>
#include <string>
#include <cstring>
#include <iostream>

#include <sqlite3.h>

#include "FileManip.h"


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
    statement vacuum_stmt;

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
        ),

        vacuum_stmt(
            create_statement(
                db,
                "VACUUM;"
            )
        )
        
    {}
        
    ~precompiled_sqliteStatements() = default;
    
};

void ShrinkDatabaseFile(statement& stmt) {

    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
        std::cerr << "Error shrinking database file." << std::endl;
    }

    sqlite3_reset(stmt.get());

    return;

}

void DeleteEpubById(statement& stmt, int id) {

    sqlite3_bind_int64(stmt.get(), 1, id);

    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
        std::cerr << "Error deleting book." << std::endl;
    }
    
    sqlite3_reset(stmt.get()); 
    sqlite3_clear_bindings(stmt.get());

    return;

}

void DownloadEpubById(statement& stmt, int id) {

    sqlite3_bind_int64(stmt.get(), 1, id);

    if (sqlite3_step(stmt.get()) == SQLITE_ROW) {

        std::string book_title = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 0));
        std::string book_file_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 1));
        
        std::vector<unsigned char> compressed_blob = ReadVoidBlobIntoUnsignedCharVector(
            (void*) sqlite3_column_blob(stmt.get(), 2), 
            (int) sqlite3_column_bytes(stmt.get(), 2)
        );

        std::vector<unsigned char> decompressed_blob = DecompressBlob(
            compressed_blob, 
            (size_t) sqlite3_column_bytes(stmt.get(), 2)
        );

        std::ofstream out(book_title + book_file_type, std::ios::binary);
        out.write(reinterpret_cast<const char*>(decompressed_blob.data()), decompressed_blob.size());
        out.close();
        
    } else {

        std::cerr << "Book not found." << std::endl;   

    }
    
    sqlite3_reset(stmt.get()); 
    sqlite3_clear_bindings(stmt.get());
    return;

}


void sqlite3Setup() {

    int returnCode = sqlite3_initialize();
    if (returnCode != SQLITE_OK) {
        throw std::runtime_error("sqlite3 Lib Initialization Error. Exiting...");
    }

    return;

}

#endif