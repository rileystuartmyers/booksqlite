#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <memory>

#include <sqlite3.h>
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <nfd.h>

const char *TABLE_NAME = "BOOKS";
const char *TABLE_PATH = "../db/books.db";
const char *WINDOW_NAME = "BookDB";
const bool ENABLE_CLOSURE = true;

int monitorX; int monitorY;
const int windowX = 400; const int windowY = 500;
const int VARCHAR_LENGTH = 50;

class Book {

    private:

        int id;
        std::string title;
        std::string author;
        std::string file_type;
        long int file_size;
        std::string date_modified;

    public:

        Book(int _id, const unsigned char *_title, const unsigned char *_author, const unsigned char *_file_type, long int _file_size, const unsigned char *_date_modified) {
            
            if (!_title || !_author || !_file_type || !_date_modified) {
                std::cerr << "Failed to load book: Invalid or null entry. :::>    id=" << _id << std::endl;
                return;
            }


            id = _id;
            title = reinterpret_cast<const char *>(_title);
            author = reinterpret_cast<const char *>(_author);
            file_type = reinterpret_cast<const char *>(_file_type);
            file_size = _file_size;
            date_modified = reinterpret_cast<const char *>(_date_modified);

        }

        int getid() const { return id; }
        const char *gettitle() { return title.c_str(); }
        const char *getauthor() { return author.c_str(); }
        const char *gettype() { return file_type.c_str(); }
        long int getsize() const { return file_size; }
        const char *getdate() { return date_modified.c_str(); }

};

class WINDOW {

    public:
        
        int x;
        int y;
        bool is_active = true;
        const char *name;

        WINDOW(int _x, int _y, char *_name) {
            x = _x;
            y = _y;
            name = _name;
        }

}; WINDOW USER_WINDOW(windowX, windowY, (char *)WINDOW_NAME);

void file_iter_count(std::string path) {

    int count;

    std::ifstream iff(path);
    iff >> count;
    iff.close();

    printf("{%d}_ \n\n", count);

    std::ofstream ofs(path);
    ofs << count + 1;
    ofs.close();

    return;

}

void RETURN_CODE_CHECK(int returnCode, std::string cerrMessage = "Unable to create statement.", std::string sqlString = "") {

    if (returnCode != SQLITE_OK) {
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
        sqlString.length(),
        &stmt,
        nullptr
    );

    RETURN_CODE_CHECK(returnCode, "Unable to issue SQL statement", sqlString);
    return statement(stmt, sqlite3_finalize);

}

bool ListBoxWrapper(const char* label, int* current_item, std::vector<std::string>& values)
{
    auto getter = [](void* data, int idx, const char** out_text) -> bool
    {
        auto& vec = *static_cast<std::vector<std::string>*>(data);
        if (idx < 0 || idx >= static_cast<int>(vec.size()))
            return false;

        *out_text = vec[idx].c_str();
        return true;
    };

    return ImGui::ListBox(label, current_item, getter, &values, values.size());
    
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

int main(int argc, char **argv) {

    if (!glfwInit()) {
        return 1;
    }

    sqlite3* db;
    int rc = sqlite3_open(
        TABLE_PATH,
        &db
    );
    RETURN_CODE_CHECK(rc, "Error initializing/opening database.");

    statement fetch_books_statement = create_statement(
        db,
        "SELECT * FROM BOOKS;"
    );
    

    std::vector<Book> books;
    std::vector<std::string> names;
    static int current_book_ind = 0;

    while (sqlite3_step(fetch_books_statement.get()) == SQLITE_ROW) {

        names.emplace_back(
            reinterpret_cast<const char *>(sqlite3_column_text(fetch_books_statement.get(), 1))
        );
        
        Book new_book(
            sqlite3_column_int(fetch_books_statement.get(), 0),
            sqlite3_column_text(fetch_books_statement.get(), 1),
            sqlite3_column_text(fetch_books_statement.get(), 2),
            sqlite3_column_text(fetch_books_statement.get(), 3),
            sqlite3_column_int64(fetch_books_statement.get(), 4),
            sqlite3_column_text(fetch_books_statement.get(), 5)
        );

        books.push_back(new_book);
        
    }

    GLFWwindow* GLFW_WINDOW = glfwCreateWindow(USER_WINDOW.x, USER_WINDOW.y, "BookDB", nullptr, nullptr);
    if (GLFW_WINDOW == nullptr) return 1;

    glfwSetWindowSizeLimits(GLFW_WINDOW, USER_WINDOW.x, USER_WINDOW.y, USER_WINDOW.x, USER_WINDOW.y);
    glfwMakeContextCurrent(GLFW_WINDOW);
    glfwSwapInterval(1); // Enable vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    ImGui_ImplGlfw_InitForOpenGL(GLFW_WINDOW, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        
    while (!glfwWindowShouldClose(GLFW_WINDOW)) {
        
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        ImGui::SetNextWindowSize(ImVec2(USER_WINDOW.x, USER_WINDOW.y));
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        {

            if (ImGui::Begin("Books", &USER_WINDOW.is_active, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
            {

                ImGui::SetCursorPos(ImVec2(15,25));
                ImGui::BeginChild(5, ImVec2(372,230), true);
                {

                    ImGui::SetCursorPos(ImVec2(168.5,10));
                    ImGui::Text("Books");

                    ImGui::SetCursorPos(ImVec2(43.999878,29));
                    ImGui::PushItemWidth(285);
                    if (ListBoxWrapper("##Names", &current_book_ind, names)) {
                        printf("List Box clicked!\n");
                    }

                    ImGui::SetCursorPos(ImVec2(154,170));
                    if ( (ImGui::Button("Download", ImVec2(64,19))) && (!books.empty()) ){
                        printf("Download button clicked!\n");
                    }

                    ImGui::SetCursorPos(ImVec2(154,200));
                    if ( ImGui::Button("New Book", ImVec2(64,19)) && (!books.empty()) ) {

                        nfdchar_t *path = nullptr;
                        nfdresult_t result = NFD_OpenDialog(NULL, NULL, &path);

                        if (result == NFD_OKAY) {

                            std::string path2 = path;
                            std::vector<unsigned char> epub = readFile(path2);
                            if (epub.empty()) {
                                std::cerr << "File is empty..." << std::endl;
                                return -1;
                            }
                            
                            free(path);
                        }
                        else if (result == NFD_CANCEL) {
                            puts("User pressed cancel.");
                        }
                        else {
                            printf("Error: %s\n", NFD_GetError() );
                        }

                    }

                }
                
                ImGui::EndChild();



                ImGui::SetCursorPos(ImVec2(204,277));
                ImGui::BeginChild(10, ImVec2(181,-12), true);
                {

                    ImGui::SetCursorPos(ImVec2(8,30));
                    ImGui::Text("Title:");

                    ImGui::SetCursorPos(ImVec2(8,50));
                    ImGui::Text("Author:");

                    ImGui::SetCursorPos(ImVec2(8,90));
                    ImGui::Text("File Type:");

                    ImGui::SetCursorPos(ImVec2(8,70));
                    ImGui::Text("File Size:");
                    
                    ImGui::SetCursorPos(ImVec2(8,110));
                    ImGui::Text("Date Added:");


                    if (!books.empty()) {

                        // Title Box
                        ImGui::SetCursorPos(ImVec2(53,30));
                        ImGui::Text("%s", books[current_book_ind].gettitle());
                        
                        // Author Box
                        ImGui::SetCursorPos(ImVec2(60,50));
                        ImGui::Text("%s", books[current_book_ind].getauthor());
                        
                        // File Type Box
                        ImGui::SetCursorPos(ImVec2(82,70));
                        ImGui::Text("%s", books[current_book_ind].gettype());

                        // File Size Box
                        ImGui::SetCursorPos(ImVec2(82,90));
                        ImGui::Text("%ld", books[current_book_ind].getsize());

                        // Date Modified Box
                        ImGui::SetCursorPos(ImVec2(92,110));
                        ImGui::Text("%s", books[current_book_ind].getdate());

                    }
                    
                }
                
                ImGui::EndChild();
                

                ImGui::SetCursorPos(ImVec2(21,281));
                ImGui::BeginChild(20, ImVec2(171,-16), true);
                {

                }
                ImGui::EndChild();


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