# BookDB

A lightweight desktop application for managing a personal collection of books. It provides a graphical interface for importing, storing, and browsing book files (such as EPUBs) using an SQLite database backend.

The application is built in **C++** with **OpenGL + ImGui** for the user interface and **SQLite3** for persistent storage.

---

## Features

* 📚 Import books directly from your filesystem
* 🗂 Store book files as compressed binary data in an SQLite database
    * 💾 Efficient storage using compression (zlib)
* 🖼 Display book metadata and cover images
* ➕ Add new books through a built-in file picker
* 🧹 Automatic database cleanup via SQLite VACUUM query

---

## Building

This project requires the following libraries:

* SQLite3
* GLFW
* Dear ImGui
* zlib
* Native File Dialog (NFD)

Build and Compile (preferrably using Makefile) by navigating into /src and using the command...

```
make
./BookDB
```

---

## Project Structure

```

/include
      /imgui
      /nativefiledialog
      debug_printing_funcs.h
      sqlite3.c
      sqlite3.h
      sqlite3ext.h

/sqlcli
      sql_cli.cpp (tool used to edit sqlite database in terminal)

/src
      main.cpp
      BookClasses.h
      DatabaseClass.h
      FileManip.h
      GLFWAssets.h
      ImGuiAssets.h
      SQLiteAssets.h

/db
      books.db

```

---

### SQLite Download ###
_(https://sqlite.org/download.html)_
* Download the amalgamation package here

---

## This project is open source.
