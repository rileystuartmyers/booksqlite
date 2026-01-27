#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <memory>
#include <sqlite3.h>

const char *TABLE_NAME = "BOOKS";
const char *TABLE_PATH = "../db/books.db";

char *pErrorMessage = nullptr;
static std::vector<std::string> ColNames;
static std::vector<std::vector<std::string>> RowValues;

const bool ENABLE_CLOSURE = true;
static unsigned short print_title_flag = 0;
static unsigned long id_counter = 1;

using statement = std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>;

statement create_statement(sqlite3* db, std::string sql) {

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(
        db,
        sql.c_str(),
        sql.length(),
        &stmt,
        nullptr
    );

    std::cout << rc << std::endl;

    if (rc != SQLITE_OK) {

        std::cerr << "Unable to create statement '" << sql << std::endl;
        std::exit(EXIT_FAILURE);

    }

    return statement(stmt, sqlite3_finalize);

}

void ColumnCallback(sqlite3_stmt* stmt) {

    for (int i = 0; i < sqlite3_column_count(stmt); ++i) {

        continue;
    }

    return;
    
}

static int callback(void *NotUsed, int argc, char **argv, char **ColName) {

    std::stringstream ss;

    if (print_title_flag == 0) {
        std::stringstream ssCol;
        for (int i = 0; i < argc; ++i) {
            ssCol << ColName[i] << " ";            
        }
        std::cout << ssCol.str() << std::endl;
        print_title_flag = 1;
    }

    for (int i = 0; i < argc; ++i) {
        std::string rstStr = argv[i] ? argv[i] : "NULL";
        ss << rstStr << " ";
    }
    std::cout << ss.str() << std::endl;

    return 0;

}

static int callback_vector(void *NotUsed, int argc, char **argv, char **ColName) {
    
    std::vector<std::string> row;
    if (print_title_flag == 0) {

        for (int i = 0; i < argc; ++i) {
            ColNames.push_back((ColName[i]));
        }    

        print_title_flag = 1;

    } 
    
    for (int i = 0; i < argc; ++i) {
        std::string rowValue = argv[i] ? argv[i] : "NULL";
        row.push_back(rowValue);
    }

    RowValues.push_back(row);

    return 0;

}

void PRAGMA_ENABLE_WAL(sqlite3* db) {
    sqlite3_exec(db, "PRAGMA journal_mode = WAL", nullptr, nullptr, nullptr);
    return;
}

void PRAGMA_DISABLE_WAL(sqlite3* db) {
    sqlite3_exec(db, "PRAGMA journal_mode = DELETE", nullptr, nullptr, nullptr);
    return;
}

void CLEAR_CALLBACK_VALUES() {

    ColNames.clear();
    RowValues.clear();
    print_title_flag = 0;

    return;

}

int ERROR_CHECK(int return_code, std::stringstream& ss) {

    if (return_code != SQLITE_OK) {
        std::cerr << "Error with statement: { " << ss.str() << " }.  RETURN CODE " << return_code << ".    ERR " << pErrorMessage << std::endl;
        return -1;
    }

    return 0;

}

int ERROR_CHECK(int return_code, std::string query) {

    if (return_code != SQLITE_OK) {
        std::cerr << "Error with statement: { " << query << " }.  RETURN CODE " << return_code << ".    ERR " << pErrorMessage << std::endl;
        return -1;
    }

    return 0;

}

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


int main(int argc, char **argv) {

    sqlite3* db;
    int rc = sqlite3_open("../db/temp.db", &db);

    if (rc != SQLITE_OK) {
        std::cerr << "Error initializing table... " << std::endl;
        return -1;
    }


    auto statement = create_statement(
        db,
        "INSERT INTO BOOKS VALUES(@id, @name);"
    );


    



    int create_code = ExecuteQuery(db, " CREATE TABLE BOOKS (id INTEGER PRIMARY KEY, name VARCHAR(50));");
    if (create_code == -1) return 0;

    std::ifstream ifs("../db/names.txt");
    std::string line;
    while (std::getline(ifs, line)) {

        std::stringstream iss(line);
        std::string id;
        std::string name;
        iss >> id >> name;

        std::string query = "INSERT INTO BOOKS (id, name) VALUES (" + id + ", '" + name + "');";
        int insert_code = ExecuteQuery(db, query);

    }

    return 0;

}