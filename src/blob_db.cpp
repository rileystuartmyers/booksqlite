#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <memory>

#include <sqlite3.h>
#include "../include/debug_printing_funcs.h"

const char *TABLE_NAME = "BOOKS";
const char *TABLE_PATH = "../db/blobs.db";
std::string EPUB_PATH = "/home/str/Downloads/books/Ana Karênina -- Tolstói, Liev -- Europa-América -- 0b8d377a680cf38fab9f98cdf594df83 -- Anna’s Archive.epub";

std::vector<std::string> ColNames;
std::vector<std::vector<std::string>> RowValues;

int DropTable(sqlite3* db, const char* tableName = TABLE_NAME) {

    std::stringstream query;
    query << " DROP TABLE " << tableName << ";";

    int return_code = sqlite3_exec(
        db, 
        query.str().c_str(),
        callback,
        nullptr,
        &pErrorMessage
    );

    return ERROR_CHECK(return_code, query);

}

int ExecuteQuery(sqlite3* db, std::stringstream& query, bool closure = false) {

    int return_code;

    if (closure == true) {
        
        return_code = sqlite3_exec(
            db,
            query.str().c_str(),
            callback_vector,
            nullptr,
            &pErrorMessage
        );
        
    } else {

        return_code = sqlite3_exec(
            db,
            query.str().c_str(),
            callback,
            nullptr,
            &pErrorMessage
        );

    }

    return ERROR_CHECK(return_code, query);

}

int ExecuteQuery(sqlite3* db, std::string query, bool closure = false) {

    int return_code;

    if (closure == true) {
        
        return_code = sqlite3_exec(
            db,
            query.c_str(),
            callback_vector,
            nullptr,
            &pErrorMessage
        );
        
    } else {

        return_code = sqlite3_exec(
            db,
            query.c_str(),
            callback,
            nullptr,
            &pErrorMessage
        );

    }

    return ERROR_CHECK(return_code, query);

}

std::vector<unsigned char> readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<unsigned char> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);

    return buffer;
}


int main(int argc, char** argv) {

    sqlite3* db;
    sqlite3_stmt* stmt;

    if (sqlite3_open(TABLE_PATH, &db) != SQLITE_OK) {
        std::cerr << "Error opening database." << std::endl;
        return -1;
    }

    const char* sql = "INSERT INTO BOOKS (id, name, epub) VALUES (?, ?, ?)";

    std::vector<unsigned char> epub = readFile(EPUB_PATH);
    if (epub.empty()) {
        std::cerr << "EPUB empty." << std::endl;
        return -1;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed." << std::endl;
        return -1;
    }

    sqlite3_bind_text(stmt, 2, "Ana Karenina", -1, SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 3, epub.data(), static_cast<int>(epub.size()), SQLITE_STATIC);


    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Insert failed." << std::endl;
    } else {
        std::cout << "Insert success!" << std::endl;
    }

    sqlite3_finalize(stmt);

    sqlite3_stmt* stmt2;
    const char* sql2 = "SELECT epub FROM BOOKS WHERE id = 3";

    if (sqlite3_prepare_v2(db, sql2, -1, &stmt2, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return 1;
    }

    if (sqlite3_step(stmt2) == SQLITE_ROW) {
        const void* blob = sqlite3_column_blob(stmt2, 0);
        int size = sqlite3_column_bytes(stmt2, 0);

        std::ofstream out("Ana Karenina.epub", std::ios::binary);
        out.write(static_cast<const char*>(blob), size);
        out.close();

        std::cout << "EPUB downloaded successfully\n";

    } else {
        
        std::cerr << "Book not found\n";
    
    }

    sqlite3_finalize(stmt2);
    sqlite3_close(db);

    return 0;

}