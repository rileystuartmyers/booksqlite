#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <memory>

#include <sqlite3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>

const char *TABLE_NAME = "BOOKS";
const char *TABLE_PATH = "../db/books.db";

char *pErrorMessage = nullptr;
static std::vector<std::string> ColNames;
static std::vector<std::vector<std::string>> RowValues;

const bool ENABLE_CLOSURE = true;
static unsigned short print_title_flag = 0;

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

    file_iter_count("iter2.txt");

    if (!glfwInit()) {
        return 1;
    }
    
    GLFWwindow* WINDOW = glfwCreateWindow((int)(1280), (int)(800), "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (WINDOW == nullptr) return 1;
    glfwMakeContextCurrent(WINDOW);
    glfwSwapInterval(1); // Enable vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui_ImplGlfw_InitForOpenGL(WINDOW, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    while (!glfwWindowShouldClose(WINDOW)) {

        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }
        //ImGui::ShowDemoWindow(); // Show demo window! :)
        //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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