#include <sqlite3.h>
#include <iostream>
#include <sstream>
#include <fstream>

static short int print_title_flag = 0;
char* pErrorMessage = nullptr;

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

int ERROR_CHECK(int return_code, std::string query) {

    if (return_code != SQLITE_OK) {
        std::cerr << "Error with statement: { " << query << " }.  RETURN CODE " << return_code << ".    ERR " << pErrorMessage << std::endl;
        return -1;
    }

    return 0;

}

int ExecuteQuery(sqlite3* db, std::string query, bool closure = false) {

    int return_code = sqlite3_exec(
        db,
        query.c_str(),
        callback,
        nullptr,
        &pErrorMessage

    );

    return ERROR_CHECK(return_code, query);

}

int main(int argc, char** argv) {

    if (argc < 4) {
        std::cout << "Usage: {cpp file} {path_to_db_file} {path_to_txt_file} {table_name}" << std::endl;
        return -1;
    }

    const char* TABLE_PATH = argv[1];
    std::string TEXT_PATH = argv[2];
    std::string TABLE_NAME = argv[3];
    sqlite3* db;

    int rc = sqlite3_open(TABLE_PATH, &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Error initializing table." << std::endl;
        return -1;
    }

    std::ifstream ifs(TEXT_PATH);
    std::string line;
    while (std::getline(ifs, line)) {

        std::stringstream iss(line);
        std::string id;
        std::string name;
        iss >> id >> name;

        std::string query = "INSERT INTO " + TABLE_NAME + " (id, name) VALUES (" + id + ", '" + name + "');";
        int insert_code = ExecuteQuery(db, query);

    }

    return 0;
    
}