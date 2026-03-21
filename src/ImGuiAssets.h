#ifndef IMGUI_LAYOUTS_H
#define IMGUI_LAYOUTS_H

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "BookClasses.h"
#include "DatabaseClass.h"
#include "GLFWAssets.h"

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
        
        if (!Collection.empty()) {

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

void ImGuiMainBookDisplayLayout(Database& sqlite_db, Book_Collection& Collection, precompiled_sqliteStatements& statements) {

    ImGui::SetCursorPos(ImVec2(168.5,10));
    ImGui::Text("Books");
    
    ImGui::SetCursorPos(ImVec2(43.999878,29));
    ImGui::PushItemWidth(285);
    if (ListBoxWrapper("##Names", &Collection.CURRENT_INDEX, Collection.Book_Names)) {
        
        ImGui::SetItemDefaultFocus();
        
    }

    ImGui::SetCursorPos(ImVec2(230, 170));
    if ( (ImGui::Button("Delete", ImVec2(56, 19))) && (!Collection.empty()) ) {
    
        Collection.DeleteCurrentBook(statements);

    }

    ImGui::SetCursorPos(ImVec2(154,170));
    if ( (ImGui::Button("Download", ImVec2(64,19))) && (!Collection.empty()) ){

        DownloadEpubById(statements.download_stmt, Collection.Display_Book.getid());
        
    }
    
}


void ImGuiNewBookWindowAddBookButtonLayout(Book_Collection& Collection, Database& db, precompiled_sqliteStatements& statements, WINDOW& window) {

    ImGui::SetCursorPos(ImVec2(290, 199));
    if (ImGui::Button("Add Book", ImVec2(70, 20))) {
        
        std::vector<unsigned char> compressed_blob = CompressBlob(db.Book_Buffer.blob_data);

        sqlite3_bind_text(statements.insert_stmt.get(), 1, db.Book_Buffer.file_name, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(statements.insert_stmt.get(), 2, db.Book_Buffer.author_name, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(statements.insert_stmt.get(), 3, db.Book_Buffer.extension, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(statements.insert_stmt.get(), 4, db.Book_Buffer.file_size_int);
        sqlite3_bind_text(statements.insert_stmt.get(), 5, db.Book_Buffer.date_time, -1, SQLITE_TRANSIENT);
        sqlite3_bind_blob(statements.insert_stmt.get(), 6, compressed_blob.data(), db.Book_Buffer.blob_data.size(), SQLITE_TRANSIENT);
        sqlite3_bind_blob(statements.insert_stmt.get(), 7, db.Book_Buffer.cover_data.data(), db.Book_Buffer.cover_data.size(), SQLITE_TRANSIENT);

        int rc = sqlite3_step(statements.insert_stmt.get());
        if (rc != SQLITE_DONE) {
            std::cerr << "Insert failed :::> " << sqlite3_errmsg(db.connection) << std::endl;
        }

        sqlite3_reset(statements.insert_stmt.get());
        sqlite3_clear_bindings(statements.insert_stmt.get());
        
        COVER_TEXTURE = CreateTextureFromRGBA(db.Book_Buffer.cover_data);

        db.Book_Buffer.Clear();
        Collection.RefreshBooks();
        Collection.SetDisplayBookToLastIndex();

        window.IS_ACTIVE = false;
        
    }

}

void ImGuiDisplayBookCoverImageLayout(Book_Collection& Collection, std::vector<unsigned char>& cover_data) {

    if (cover_data.empty()) {
        COVER_TEXTURE = BLANK_TEXTURE;
    } else if (Collection.GetIdOfCurrentBook() != Collection.GetIdOfDisplayBook()) {
        COVER_TEXTURE = CreateTextureFromRGBA(cover_data); // prevents retexturing of the same cover
    }
    
    ImGui::SetCursorPos(ImVec2(21,281));
    ImGui::BeginChild(20, ImVec2(171,-16), true);
    {
        ImGui::Image(

            (void*)(intptr_t)COVER_TEXTURE, 

            ImVec2(
                (float)COVER_PIXEL_WIDTH,
                (float)COVER_PIXEL_HEIGHT
            )

        );

    }
    ImGui::EndChild();

}

void ImGuiNewFrameSetup() {

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

}

void ImGuiCleanupProcess() {

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
}

void ImGuiSetup(GLFWwindow* GLFWWINDOW) {

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = "../build/imgui.ini";
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    
    ImGui_ImplGlfw_InitForOpenGL(GLFWWINDOW, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
    
    IMGUI_CHECKVERSION();

    return;

}


#endif