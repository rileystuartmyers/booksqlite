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

const char *TABLE_NAME = "BOOKS";
const char *TABLE_PATH = "../db/books.db";
const char *WINDOW_NAME = "BookDB";
const bool ENABLE_CLOSURE = true;

int monitorX; int monitorY;
const int windowX = 400; const int windowY = 500;

class WINDOW {

    public:
        
        int x;
        int y;
        bool is_active = true;
        char *name;
        WINDOW(int _x, int _y, char *_name) {
            x = _x;
            y = _y;
            name = _name;
        }


};

WINDOW USER_WINDOW(windowX, windowY, (char *)WINDOW_NAME);

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

    RETURN_CODE_CHECK(returnCode, sqlString);
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


    std::vector<std::string> book_names;
    static int current_book_ind = 0;

    statement fetch_names_stmt = create_statement(
        db,
        "SELECT name FROM BOOKS;"
    );

    while (sqlite3_step(fetch_names_stmt.get()) == SQLITE_ROW) {
        const unsigned char * row_text = sqlite3_column_text(fetch_names_stmt.get(), 0);
        book_names.emplace_back(reinterpret_cast<const char *>(row_text));
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

                    ImGui::SetCursorPos(ImVec2(43.999878,29));
                    ImGui::PushItemWidth(285);
                    if (ListBoxWrapper("##Names", &current_book_ind, book_names)) {
                        // do something else
                    }

                    ImGui::SetCursorPos(ImVec2(168.5,10));
                    ImGui::Text("Books");

                    ImGui::SetCursorPos(ImVec2(154,170));
                    ImGui::Button("Download", ImVec2(64,19)); //remove size argument (ImVec2) to auto-resize

                    ImGui::SetCursorPos(ImVec2(154,200));
                    ImGui::Button("New Book", ImVec2(64,19));

                }
                ImGui::EndChild();



                ImGui::SetCursorPos(ImVec2(204,277));
                ImGui::BeginChild(10, ImVec2(181,-12), true);
                {

                    ImGui::SetCursorPos(ImVec2(11,30));
                    ImGui::Text("Title: ");

                    ImGui::SetCursorPos(ImVec2(11,50));
                    ImGui::Text("File Size:");

                    ImGui::SetCursorPos(ImVec2(11,70));
                    ImGui::Text("File Type:");

                    ImGui::SetCursorPos(ImVec2(11,90));
                    ImGui::Text("Date Added:");

                    ImGui::SetCursorPos(ImVec2(60,30));
                    ImGui::Text((book_names[current_book_ind]).c_str());

                    ImGui::SetCursorPos(ImVec2(86.5,50));
                    ImGui::Text("Blank Size");

                    ImGui::SetCursorPos(ImVec2(88,70));
                    ImGui::Text("Blank Type");

                    ImGui::SetCursorPos(ImVec2(91.5,90));
                    ImGui::Text("Blank Date");

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