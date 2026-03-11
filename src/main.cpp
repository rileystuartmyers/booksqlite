#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <memory>
#include <filesystem>
#include <ctime>
#include <algorithm>

#include <zlib.h>
#include <nfd.h>

#include "SQLiteAssets.h"
#include "BookClasses.h"
#include "DatabaseClass.h"
#include "GLFWAssets.h"
#include "ImGuiAssets.h"


void InitialSetup(GLFWwindow*& GLFWWINDOW, WINDOW window, const char* window_name) {

    sqlite3Setup();
    GLFWWINDOW = GLFWSetup(window, window_name);
    ImGuiSetup(GLFWWINDOW);

    return;

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

    while (!glfwWindowShouldClose(GLFW_WINDOW)) {
        
        glfwPollEvents();
        ImGuiNewFrameSetup();
        
        ImGui::SetNextWindowSize(ImVec2(USER_WINDOW.X, USER_WINDOW.Y));
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        
        if (!Collection.is_empty()) {
            Collection.SetDisplayBookToCurrentIndex();
        }

        {

            if (ImGui::Begin("Books", &USER_WINDOW.IS_ACTIVE, 
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
            {

                ImGui::SetCursorPos(ImVec2(15,25));
                ImGui::BeginChild(5, ImVec2(372,230), true);
                {

                    ImGuiMainBookDisplayLayout(sqlite_db, Collection, sqliteStatements);

                    
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
                                
                                sqlite_db.Populate_Buffer(path);
                                NEWBOOK_WINDOW.Activate();

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


                if (NEWBOOK_WINDOW.Is_Active()) {

                    ImGui::SetNextWindowSize(ImVec2(NEWBOOK_WINDOW.X, NEWBOOK_WINDOW.Y));
                    ImGui::SetNextWindowPos(ImVec2(10, 150));
                    ImGui::SetNextWindowFocus();

                    if (ImGui::Begin("Add A New Book", &NEWBOOK_WINDOW.IS_ACTIVE, 
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar)) {
                        
                        ImGuiNewBookWindowLayout(sqlite_db.Book_Buffer);
                        ImGuiNewBookWindowAddBookButtonLayout(Collection, sqlite_db, sqliteStatements, NEWBOOK_WINDOW);

                    }
                    ImGui::End();

                }

            }

            ImGui::End();

        }

        ImGui::Render();
        GLFWRenderFrameProcess(GLFW_WINDOW, USER_WINDOW);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(GLFW_WINDOW);

    }

    sqlite_db.ShrinkDatabaseFile(sqliteStatements.vacuum_stmt);
    ImGuiCleanupProcess();

    return 0;

}