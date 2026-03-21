#ifndef BOOK_CLASSES_H
#define BOOK_CLASSES_H

#include <vector>
#include <string>
#include <memory>
#include <cstring>

#include "SQLiteAssets.h"

const int BUFFER_SIZE = 128;

struct NewBook_Buffer {

    std::vector<unsigned char> blob_data;   
    std::string Selected_Path;
    char file_name[BUFFER_SIZE];
    char author_name[BUFFER_SIZE];
    char extension[BUFFER_SIZE];
    char file_size[BUFFER_SIZE];
    int file_size_int;
    char date_time[BUFFER_SIZE];
    std::vector<unsigned char> cover_data;

    NewBook_Buffer() :
        Selected_Path(""),
        file_name(""),
        author_name(""),
        extension(""),
        file_size(""),
        file_size_int(0),
        date_time("")
    {};

    ~NewBook_Buffer() {};

    void Clear() {

        blob_data.clear();
        Selected_Path.clear();

        memset(file_name, ' ', BUFFER_SIZE);
        file_name[0] = '\0';

        memset(author_name, ' ', BUFFER_SIZE);
        author_name[0] = '\0';

        memset(file_size, ' ', BUFFER_SIZE);
        file_size[0] = '\0';

        memset(extension, ' ', BUFFER_SIZE);
        extension[0] = '\0';
        
        memset(date_time, ' ', BUFFER_SIZE);
        date_time[0] = '\0';

        file_size_int = 0;

        cover_data.clear();
        
    }

};

class Book {

    private:

        int id;
        std::string title;
        std::string author;
        std::string file_type;
        long int file_size;
        std::string date_modified;
        
    public:
        
        std::vector<unsigned char> cover_data;

        Book(int _id, const unsigned char *_title, const unsigned char *_author, const unsigned char *_file_type, long int _file_size, const unsigned char *_date_modified, std::vector<unsigned char> _cover_data) {

            id = _id;
            title = _title ? reinterpret_cast<const char *>(_title) : "";
            author = _author ? reinterpret_cast<const char *>(_author) : "";
            file_type = _file_type ? reinterpret_cast<const char *>(_file_type) : "";
            file_size = _file_size;
            date_modified = _date_modified ? reinterpret_cast<const char *>(_date_modified) : "";
            cover_data = _cover_data;

        }

        Book() : 

            id(0), 
            title(""), 
            author(""), 
            file_type(""), 
            file_size(0), 
            date_modified("")
            
        {};

        ~Book() {};

        int getid() const { return id; }
        const char *gettitle() { return title.c_str(); }
        const char *getauthor() { return author.c_str(); }
        const char *gettype() { return file_type.c_str(); }
        long int getsize() const { return file_size; }
        const char *getdate() { return date_modified.c_str(); }

        void Clear() {
            id = 0;
            title = "";
            author = "";
            file_type = "";
            file_size = 0;
            date_modified = "";
            cover_data.clear();
        }

};

class Book_Collection {

    private:

        std::vector<Book> Books;
        statement fetch_stmt;

        int size = 0;
        
    public:
    
        std::vector<std::string> Book_Names;
        Book Display_Book;

        int CURRENT_INDEX = 0;

        void RefreshBooks() {

            size = 0;
            CURRENT_INDEX = 0;
            Books.clear();
            Book_Names.clear();
            sqlite3_reset(fetch_stmt.get());

            while (sqlite3_step(fetch_stmt.get()) == SQLITE_ROW) {

                const unsigned char* title = sqlite3_column_text(fetch_stmt.get(), 1);
                std::vector<unsigned char> cover_data = ReadVoidBlobIntoUnsignedCharVector(
                    (void*)sqlite3_column_blob(fetch_stmt.get(), 6),
                    (int)sqlite3_column_bytes(fetch_stmt.get(), 6)
                );

                Book New_Book(
                    sqlite3_column_int(fetch_stmt.get(), 0),
                    InsertWithNullCheck(title),
                    InsertWithNullCheck(sqlite3_column_text(fetch_stmt.get(), 2)),
                    InsertWithNullCheck(sqlite3_column_text(fetch_stmt.get(), 3)),
                    sqlite3_column_int64(fetch_stmt.get(), 4),
                    InsertWithNullCheck(sqlite3_column_text(fetch_stmt.get(), 5)),
                    cover_data
                );

                Books.push_back(New_Book);
                Book_Names.emplace_back(reinterpret_cast<const char *>(InsertWithNullCheck(title)));
                
                size++;

            }

            return;

        };

        Book_Collection(sqlite3* db_ptr) :
            
            fetch_stmt(
                create_statement(
                    db_ptr,
                    "SELECT id, title, author, file_type, file_size, date_modified, cover FROM BOOKS WHERE title IS NOT NULL;"
                )
            )

        {
            RefreshBooks();
        };

        ~Book_Collection() {};


        Book GetBookAtIndex(int index) {
            return Books[index];
        }

        
        void SetDisplayBookToCurrentIndex() {
            Display_Book = Books[CURRENT_INDEX];
        }

        void SetDisplayBookWithIndex(int index) {
            Display_Book = Books[index];
        }
        
        void SetDisplayBookToLastIndex() {
            CURRENT_INDEX = std::max(0, size - 1);
            Display_Book = Books[CURRENT_INDEX];
        }
        
        void SetIndex(int index) {
            CURRENT_INDEX = index;
        }
        
        int GetIdOfCurrentBook() {
            return Books[CURRENT_INDEX].getid();
        }
        
        int GetIdOfDisplayBook() {
            return Display_Book.getid();
        }
        
        void DeleteCurrentBook(precompiled_sqliteStatements& statements) {

            DeleteEpubById(statements.delete_stmt, Display_Book.getid());

            SetDefaultCoverTextureAfterDelete(COVER_TEXTURE);
            Display_Book.Clear();
            RefreshBooks();

        }

        int count() {
            return size;
        }

        bool empty() {
            return Books.empty();
        }

};

void RefreshBookList(std::vector<Book> &books, std::vector<std::string> &names, statement &stmt) {

    books.clear();
    names.clear();
    sqlite3_reset(stmt.get());

    while (sqlite3_step(stmt.get()) == SQLITE_ROW) {

        names.emplace_back(
            reinterpret_cast<const char *>(InsertWithNullCheck(sqlite3_column_text(stmt.get(), 1)))
        );

        std::vector<unsigned char> cover_data = ReadVoidBlobIntoUnsignedCharVector(
            (void*)sqlite3_column_blob(stmt.get(), 6),
            (int)sqlite3_column_bytes(stmt.get(), 6)
        );        

        Book new_book(
            sqlite3_column_int(stmt.get(), 0),
            InsertWithNullCheck(sqlite3_column_text(stmt.get(), 1)),
            InsertWithNullCheck(sqlite3_column_text(stmt.get(), 2)),
            InsertWithNullCheck(sqlite3_column_text(stmt.get(), 3)),
            sqlite3_column_int64(stmt.get(), 4),
            InsertWithNullCheck(sqlite3_column_text(stmt.get(), 5)),
            cover_data
        );

        books.push_back(new_book);

    }

    return;

}

#endif