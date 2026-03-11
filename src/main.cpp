#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <memory>
#include <filesystem>
#include <ctime>
#include <algorithm>

#include <zlib.h>

#include <sqlite3.h>
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <nfd.h>

#include "SQLiteClasses.h"
#include "BookClasses.h"
#include "DatabaseClass.h"

GLFWwindow* GLFW_WINDOW;

class WINDOW {

    public:
        
        int x;
        int y;
        bool is_active = true;
        const char *name;

        WINDOW(int _x, int _y, const char *_name, bool _is_active = true) {
            x = _x;
            y = _y;
            name = _name;
            is_active = _is_active;
        }

}; 

WINDOW USER_WINDOW(
    400, 500, 
    (const char *)"BookDB", 
    true
);
WINDOW NEWBOOK_WINDOW(
    380, 225, 
    (const char *)"New Book",
    false
);

bool ListBoxWrapper(const char* label, int* current_item, std::vector<std::string>& values) {

    auto getter = [](void* data, int idx, const char** out_text) -> bool {

        auto& vec = *static_cast<std::vector<std::string>*>(data);

        if (idx < 0 || idx >= static_cast<int>(vec.size())){
            return false;
        }

        *out_text = vec[idx].c_str();
        return true;

    };

    return ImGui::ListBox(label, current_item, getter, &values, values.size());

}

std::vector<unsigned char> CompressBlob(std::vector<unsigned char>& blob) {

    uLong src_size = blob.size();
    ulong dest_size = compressBound(src_size);
    std::vector<unsigned char> compressed_blob(dest_size);
    
    int res = compress2(
        compressed_blob.data(),
        &dest_size,
        blob.data(),
        src_size,
        Z_BEST_COMPRESSION
    );
    
    if (res != Z_OK){
        throw std::runtime_error("Compression failed.");
    }

    compressed_blob.resize(dest_size);
    return compressed_blob;
    
}

std::vector<unsigned char> DecompressBlob(std::vector<unsigned char>& compressed_blob, size_t original_size) {

    std::vector<unsigned char> decompressed_blob(original_size);
    uLongf dest_size = original_size;

    int res = uncompress(
        decompressed_blob.data(),
        &dest_size,
        compressed_blob.data(),
        compressed_blob.size()
    );

    if (res != Z_OK) {
        throw std::runtime_error("Decompression failed.");
    }
    
    return decompressed_blob;

}

std::vector<unsigned char> ReadVoidBlobIntoVector(const void* blob, int size) {

    std::vector<unsigned char> blob_vector;
    const unsigned char* blob_bytes = static_cast<const unsigned char*>(blob);

    blob_vector.assign(blob_bytes, blob_bytes + size);
    return blob_vector;

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
        
        std::vector<unsigned char> compressed_blob = ReadVoidBlobIntoVector(
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

        std::cout << "File downloaded successfully." << std::endl;
        
    } else {

        std::cerr << "Book not found." << std::endl;   

    }
    
    sqlite3_reset(stmt.get()); 
    sqlite3_clear_bindings(stmt.get());
    return;

}

std::vector<unsigned char> readFile(std::string& path) {
    
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

GLFWwindow* GLFWSetup(WINDOW window, const char* window_name) {

    if (!glfwInit()) {
        throw std::runtime_error("GLFW Lib Initialization Error. Exiting...");
    }

    GLFWwindow* GLFWWINDOW = glfwCreateWindow(window.x, window.y, "BookDB", nullptr, nullptr);
    if (GLFWWINDOW == nullptr) {
        throw std::runtime_error("Failed to initialize GLFWwindow. Exiting...");
    }

    glfwSetWindowSizeLimits(GLFWWINDOW, USER_WINDOW.x, USER_WINDOW.y, USER_WINDOW.x, USER_WINDOW.y);
    glfwMakeContextCurrent(GLFWWINDOW);
    glfwSwapInterval(1); // Enable vsync

    return GLFWWINDOW;

}

void ImGuiSetup(GLFWwindow* GLFWWINDOW) {

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    ImGui_ImplGlfw_InitForOpenGL(GLFWWINDOW, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();

    IMGUI_CHECKVERSION();

    return;

}

void sqlite3Setup() {

    int returnCode = sqlite3_initialize();
    if (returnCode != SQLITE_OK) {
        throw std::runtime_error("sqlite3 Lib Initialization Error. Exiting...");
    }

    return;

}

void InitialSetup(GLFWwindow*& GLFWWINDOW, WINDOW window, const char* window_name) {

    sqlite3Setup();
    GLFWWINDOW = GLFWSetup(window, window_name);
    ImGuiSetup(GLFWWINDOW);

    return;

}

void ImGuiNewBookWindowLayout(NewBook_Buffer& Book_Buffer) {

    ImGui::SetCursorPos(ImVec2(5,25));
    ImGui::BeginChild(2, ImVec2(370,170), true);

    ImGui::SetCursorPos(ImVec2(20,20));
    ImGui::Text("Title:");

    ImGui::SetCursorPos(ImVec2(20,50));
    ImGui::Text("Author:");

    ImGui::SetCursorPos(ImVec2(20,80));
    ImGui::Text("Size (Bytes):");

    ImGui::SetCursorPos(ImVec2(20,110));
    ImGui::Text("File Type:");

    ImGui::SetCursorPos(ImVec2(20,140));
    ImGui::Text("Date Added:");

    ImGui::SetCursorPos(ImVec2(120,20));
    ImGui::PushItemWidth(210); //NOTE: (Push/Pop)ItemWidth is optional
    ImGui::InputText("##NewBook_Title", Book_Buffer.file_name, IM_ARRAYSIZE(Book_Buffer.file_name));
    ImGui::PopItemWidth();

    ImGui::SetCursorPos(ImVec2(120,50));
    ImGui::PushItemWidth(210); //NOTE: (Push/Pop)ItemWidth is optional
    ImGui::InputText("##NewBook_Author", Book_Buffer.author_name, IM_ARRAYSIZE(Book_Buffer.author_name));
    ImGui::PopItemWidth();

    ImGui::SetCursorPos(ImVec2(120,80));
    ImGui::PushItemWidth(210); //NOTE: (Push/Pop)ItemWidth is optional
    ImGui::InputText("##NewBook_Size", Book_Buffer.file_size, IM_ARRAYSIZE(Book_Buffer.file_size), ImGuiInputTextFlags_ReadOnly);
    ImGui::PopItemWidth();

    
    ImGui::SetCursorPos(ImVec2(120,110));
    ImGui::PushItemWidth(210); //NOTE: (Push/Pop)ItemWidth is optional
    ImGui::InputText("##NewBook_Type", Book_Buffer.extension, IM_ARRAYSIZE(Book_Buffer.extension));
    ImGui::PopItemWidth();
    
    ImGui::SetCursorPos(ImVec2(120,140));
    ImGui::PushItemWidth(210); //NOTE: (Push/Pop)ItemWidth is optional
    ImGui::InputText("##NewBook_Date", Book_Buffer.date_time, IM_ARRAYSIZE(Book_Buffer.date_time), ImGuiInputTextFlags_ReadOnly);
    ImGui::PopItemWidth();

    ImGui::EndChild();

}

void ImGuiDisplayBookInfoSectionLayout(Book_Collection& Collection) {

    ImGui::SetCursorPos(ImVec2(204,277));
    ImGui::BeginChild(10, ImVec2(181,-12), true);
    {

        ImGui::SetCursorPos(ImVec2(8,30));
        ImGui::Text("Title:");

        ImGui::SetCursorPos(ImVec2(8,50));
        ImGui::Text("Author:");

        ImGui::SetCursorPos(ImVec2(8,90));
        ImGui::Text("Size (Bytes):");

        ImGui::SetCursorPos(ImVec2(8,70));
        ImGui::Text("File Type:");
        
        ImGui::SetCursorPos(ImVec2(8,110));
        ImGui::Text("Date Added:");

        if (!Collection.is_empty()) {

            // Title Box
            ImGui::SetCursorPos(ImVec2(53,30));
            ImGui::Text("%s", Collection.Display_Book.gettitle());
            
            // Author Box
            ImGui::SetCursorPos(ImVec2(60,50));
            ImGui::Text("%s", Collection.Display_Book.getauthor());
            
            // File Type Box
            ImGui::SetCursorPos(ImVec2(82,70));
            ImGui::Text("%s", Collection.Display_Book.gettype());

            // File Size Box
            ImGui::SetCursorPos(ImVec2(102,90));
            ImGui::Text("%ld", Collection.Display_Book.getsize());

            // Date Modified Box
            ImGui::SetCursorPos(ImVec2(92,110));
            ImGui::Text("%s", Collection.Display_Book.getdate());

        }
        
    }            
    ImGui::EndChild();
}

void ImGuiDisplayBookCoverImageLayout() {

    ImGui::SetCursorPos(ImVec2(21,281));
    ImGui::BeginChild(20, ImVec2(171,-16), true);
    {
        // logic for inserting the epub cover image
    }
    ImGui::EndChild();

}

void ImGuiMainBookDisplayLayout(Book_Collection& Collection, precompiled_sqliteStatements& statements) {

    ImGui::SetCursorPos(ImVec2(168.5,10));
    ImGui::Text("Books");

    ImGui::SetCursorPos(ImVec2(43.999878,29));
    ImGui::PushItemWidth(285);
    if (ListBoxWrapper("##Names", &Collection.CURRENT_INDEX, Collection.Book_Names)) {
        
        ImGui::SetItemDefaultFocus();

    }

    ImGui::SetCursorPos(ImVec2(230, 170));
    if ( (ImGui::Button("Delete", ImVec2(56, 19))) && (!Collection.is_empty()) ) {

        DeleteEpubById(statements.delete_stmt, Collection.Display_Book.getid());
        Collection.RefreshBooks();

    }

    ImGui::SetCursorPos(ImVec2(154,170));
    if ( (ImGui::Button("Download", ImVec2(64,19))) && (!Collection.is_empty()) ){

        DownloadEpubById(statements.download_stmt, Collection.Display_Book.getid());

    }
    
}

int main(int argc, char **argv) {
    
    InitialSetup(
        GLFW_WINDOW, 
        USER_WINDOW, 
        "BookDB"
    );
    
    Database sqlite_db("BOOKS", "../db/books.db");
    precompiled_sqliteStatements sqliteStatements(sqlite_db.connection);
    Book_Collection Collection(sqlite_db.connection);
    Collection.RefreshBooks();

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        
    while (!glfwWindowShouldClose(GLFW_WINDOW)) {
        
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        
        ImGui::SetNextWindowSize(ImVec2(USER_WINDOW.x, USER_WINDOW.y));
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        
        if (!Collection.is_empty()) {
            Collection.SetDisplayBookToCurrentIndex();
        }

        {

            if (ImGui::Begin("Books", &USER_WINDOW.is_active, 
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
            {

                ImGui::SetCursorPos(ImVec2(15,25));
                ImGui::BeginChild(5, ImVec2(372,230), true);
                {

                    ImGuiMainBookDisplayLayout(Collection, sqliteStatements);

                    
                    ImGui::SetCursorPos(ImVec2(154,200));
                    if (ImGui::Button("New Book", ImVec2(64,19))) {

                        nfdchar_t *path = nullptr;
                        nfdresult_t result = NFD_OpenDialog(NULL, NULL, &path);

                        if (result == NFD_OKAY) {

                            std::string temp_path = path;
                            sqlite_db.Book_Buffer.blob_data = readFile(temp_path);

                            if (sqlite_db.Book_Buffer.blob_data.empty()) {

                                std::cerr << "File is empty..." << std::endl;
                            
                            } else {
                                
                                sqlite_db.Book_Buffer.Selected_Path = path;
                                NEWBOOK_WINDOW.is_active = true;

                                std::filesystem::path p(sqlite_db.Book_Buffer.Selected_Path);
                                strcpy(sqlite_db.Book_Buffer.file_name, p.stem().string().c_str());
                                strcpy(sqlite_db.Book_Buffer.extension, p.extension().string().c_str());
                               
                                sqlite_db.Book_Buffer.file_size_int = std::filesystem::file_size(p);
                                std::string file_size_str = std::to_string(sqlite_db.Book_Buffer.file_size_int);
                                strcpy(sqlite_db.Book_Buffer.file_size, file_size_str.c_str());
                                strcat(sqlite_db.Book_Buffer.file_size, " B");
                                
                                time_t now = time(0);
                                strftime(sqlite_db.Book_Buffer.date_time, sizeof(sqlite_db.Book_Buffer.date_time), "%m/%d/%Y", localtime(&now));

                            }
                            
                            free(path);

                        } else if (result == NFD_CANCEL) {

                            puts("User pressed cancel.");

                        } else {
                            
                            printf("Error: %s\n", NFD_GetError() );

                        }

                    }

                }
                ImGui::EndChild();


                ImGuiDisplayBookInfoSectionLayout(Collection);
                

                ImGuiDisplayBookCoverImageLayout();


                if (NEWBOOK_WINDOW.is_active) {

                    ImGui::SetNextWindowSize(ImVec2(NEWBOOK_WINDOW.x, NEWBOOK_WINDOW.y));
                    ImGui::SetNextWindowPos(ImVec2(10, 150));
                    ImGui::SetNextWindowFocus();

                    if (ImGui::Begin("Add A New Book", &NEWBOOK_WINDOW.is_active, 
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar)) {
                        
                        ImGuiNewBookWindowLayout(sqlite_db.Book_Buffer);

                        ImGui::SetCursorPos(ImVec2(290, 199));
                        if (ImGui::Button("Add Book", ImVec2(70, 20))) {
                            
                            std::vector<unsigned char> compressed_blob = CompressBlob(sqlite_db.Book_Buffer.blob_data);

                            sqlite3_bind_text(sqliteStatements.insert_stmt.get(), 1, sqlite_db.Book_Buffer.file_name, -1, SQLITE_TRANSIENT);
                            sqlite3_bind_text(sqliteStatements.insert_stmt.get(), 2, sqlite_db.Book_Buffer.author_name, -1, SQLITE_TRANSIENT);
                            sqlite3_bind_text(sqliteStatements.insert_stmt.get(), 3, sqlite_db.Book_Buffer.extension, -1, SQLITE_TRANSIENT);
                            sqlite3_bind_int64(sqliteStatements.insert_stmt.get(), 4, sqlite_db.Book_Buffer.file_size_int);
                            sqlite3_bind_text(sqliteStatements.insert_stmt.get(), 5, sqlite_db.Book_Buffer.date_time, -1, SQLITE_TRANSIENT);
                            sqlite3_bind_blob(sqliteStatements.insert_stmt.get(), 6, compressed_blob.data(), sqlite_db.Book_Buffer.blob_data.size(), SQLITE_TRANSIENT);

                            int rc = sqlite3_step(sqliteStatements.insert_stmt.get());
                            if (rc != SQLITE_DONE) {
                                std::cerr << "Insert failed :::> " << sqlite3_errmsg(sqlite_db.connection) << std::endl;
                            }

                            sqlite3_reset(sqliteStatements.insert_stmt.get());
                            sqlite3_clear_bindings(sqliteStatements.insert_stmt.get());
                            
                            sqlite_db.Book_Buffer.Clear();
                            Collection.RefreshBooks();
    
                            NEWBOOK_WINDOW.is_active = false;
                            
                        }


                    }
                    ImGui::End();

                }

            }

            ImGui::End();

        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(GLFW_WINDOW, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(GLFW_WINDOW);

    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;

}