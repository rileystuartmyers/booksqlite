#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <memory>

#include <sqlite3.h>

#include "../include/imgui/imgui.h"
#include "../include/imgui/backends/imgui_impl_glfw.h"
#include "../include/imgui/backends/imgui_impl_opengl3.h"

#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>

static bool window = true;
const char *TABLE_NAME = "BOOKS";
const char *TABLE_PATH = "../db/books.db";

char *pErrorMessage = nullptr;
static std::vector<std::string> ColNames;
static std::vector<std::vector<std::string>> RowValues;

const bool ENABLE_CLOSURE = true;
static unsigned short print_title_flag = 0;

using statement = std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>;
/*
    auto statement = create_statement(
        db,
        "INSERT INTO BOOKS VALUES(@id, @name);"
    );
*/
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

int main(int argc, char **argv) {

    if (!glfwInit()) {
        return 1;
    }
    
    GLFWwindow* WINDOW = glfwCreateWindow((int)(1280), (int)(800), "BookDB", nullptr, nullptr);
    if (WINDOW == nullptr) return 1;

    glfwMakeContextCurrent(WINDOW);
    glfwSwapInterval(1); // Enable vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    ImGui_ImplGlfw_InitForOpenGL(WINDOW, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    
    // opening db
    sqlite3* db;
    int rc = sqlite3_open(
        TABLE_PATH,
        &db
    );
    if (rc != SQLITE_OK) {
        std::cerr << "Error initializing/opening database." << std::endl;
        return 1;
    }

    // statement pulling count of books

    int book_count = 0;
    sqlite3_stmt* count_stmt;
    const char* count_sql = "SELECT COUNT(*) FROM BOOKS;";
    rc = sqlite3_prepare_v2(
        db,
        count_sql,
        -1,
        &count_stmt,
        nullptr
    );
    if (rc != SQLITE_OK) {
        std::cerr << "Error issuing statement." << std::endl << std::endl;
    } else {
        rc = sqlite3_step(count_stmt);
        if (rc == SQLITE_ROW) {
            book_count = sqlite3_column_int(count_stmt, 0);
            std::cout << "BOOK_COUNT == " << book_count << std::endl;
        }
    }   
    sqlite3_finalize(count_stmt);

    // statement pulling book names from db
    const char* items2[book_count];
    static int item_current2 = 0;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT * FROM BOOKS;";
    
    rc = sqlite3_prepare_v2(
        db,
        sql,
        -1,
        &stmt,
        nullptr
    );
    if (rc != SQLITE_OK) {
        std::cerr << "Error issuing statement." << std::endl << std::endl; 
    } else {
        std::cout << "Successfully issued query." << std::endl << std::endl;
    }


    // adding names to listbox array
    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        items2[count] = (const unsignedchar*)sqlite3_column_text(stmt, 1);
        std::cout << items2[count] << std::endl;
        count++;
    }
    sqlite3_finalize(stmt);

    while (!glfwWindowShouldClose(WINDOW)) {

        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        {
            if (ImGui::Begin("window_name", &window))
            {

                ImGui::SetCursorPos(ImVec2(52,87));
                ImGui::PushItemWidth(200);
                static int item_current2 = 0;
                ImGui::ListBox("##", &item_current2, items2, IM_ARRAYSIZE(items2));
                ImGui::PopItemWidth();

                ImGui::SetCursorPos(ImVec2(62.5,63.5));
                ImGui::Text("Book List");

                ImGui::SetCursorPos(ImVec2(198,183.5));
                ImGui::Button("Select", ImVec2(50,19)); //remove size argument (ImVec2) to auto-resize

            }
            ImGui::End();
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(WINDOW, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(WINDOW);

    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;

}