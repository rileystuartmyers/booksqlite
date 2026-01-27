### Download from SQLite website ###
_(https://sqlite.org/download.html)_
* Download and extract the amalgamation package

* Take the sqlite3.c, sqlite3.h, and sqlite3ext.h files,
drag and drop them into your project

* In terminal, compile your sqlite3.c file into an object with 
    gcc -c sqlite3.c -o sqlite3.o

    This gives you sqlite3.o object file

* You're basically done. Now, whenever you compile your cpp file you use
    g++ main.cpp sqlite3.o -o app
