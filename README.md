### Download from SQLite website ###
_(https://sqlite.org/download.html)_
* Download and extract the amalgamation package

* Take the sqlite3.c, sqlite3.h, and sqlite3ext.h files,
drag and drop them into your project

* In terminal, compile your sqlite3.c file into an object with the command
    _gcc -c sqlite3.c -o sqlite3.o_

    This compiles into the _sqlite3.o_ file.

* Now, whenever you compile your cpp projcect, be sure to use the command
    _g++ main.cpp sqlite3.o -o app_
