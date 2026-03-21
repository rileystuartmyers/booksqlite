#ifndef GLFWASSETS_H
#define GLFWASSETS_H

#include <stdexcept>

#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>

#include <imgui.h>

const int COVER_PIXEL_WIDTH = 110;
const int COVER_PIXEL_HEIGHT = 160;

GLFWwindow* GLFW_WINDOW;
GLuint BLANK_TEXTURE = 0;
GLuint COVER_TEXTURE;

GLuint CreateTextureFromRGBA(const std::vector<unsigned char>& pixels)
{

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, COVER_PIXEL_WIDTH, COVER_PIXEL_HEIGHT, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    glBindTexture(GL_TEXTURE_2D, 0);
    return texID;

}

void BLANK_TEXTURE_Setup(GLuint& blank) {

    if (blank == 0) {

        std::vector<unsigned char> pixels(COVER_PIXEL_WIDTH * COVER_PIXEL_HEIGHT * 4);
        
        for (int i = 0; i < COVER_PIXEL_WIDTH * COVER_PIXEL_HEIGHT; ++i) {
            pixels[i * 4 + 0] = 200;
            pixels[i * 4 + 1] = 200;
            pixels[i * 4 + 2] = 200;
            pixels[i * 4 + 3] = 255;
        }
        
        BLANK_TEXTURE = CreateTextureFromRGBA(pixels);
        
    }

    return;

}

struct WINDOW {
    
    int X;
    int Y;
    int DISPLAY_W;
    int DISPLAY_H;
    ImVec4 COLOR = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    bool IS_ACTIVE = true;
    const char *name;

    WINDOW(int _X, int _Y, const char *_name, bool _IS_ACTIVE = true) {

        X = _X;
        Y = _Y;
        name = _name;
        IS_ACTIVE = _IS_ACTIVE;
        
    };
    
    void Activate() {
        
        IS_ACTIVE = true;
        
    };
    
    void Deactivate() {
        
        IS_ACTIVE = false;
        
    };
    
    bool Is_Active() {
        
        return IS_ACTIVE;
        
    };
            
        
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


void GLFWRenderFrameProcess(GLFWwindow* GLFWWINDOW, WINDOW& USERWINDOW) {

    glfwGetFramebufferSize(GLFWWINDOW, &USERWINDOW.DISPLAY_W, &USERWINDOW.DISPLAY_H);
    glViewport(0, 0, USERWINDOW.DISPLAY_W, USERWINDOW.DISPLAY_H);
    glClearColor(USERWINDOW.COLOR.x * USERWINDOW.COLOR.w, USERWINDOW.COLOR.y * USERWINDOW.COLOR.w, USERWINDOW.COLOR.z * USERWINDOW.COLOR.w, USERWINDOW.COLOR.w);
    glClear(GL_COLOR_BUFFER_BIT);

}

GLFWwindow* GLFWSetup(WINDOW window, const char* window_name) {

    if (!glfwInit()) {
        throw std::runtime_error("GLFW Lib Initialization Error. Exiting...");
    }

    GLFWwindow* GLFWWINDOW = glfwCreateWindow(window.X, window.Y, "BookDB", nullptr, nullptr);
    if (GLFWWINDOW == nullptr) {
        throw std::runtime_error("Failed to initialize GLFWwindow. Exiting...");
    }

    BLANK_TEXTURE_Setup(BLANK_TEXTURE);

    glfwSetWindowSizeLimits(GLFWWINDOW, USER_WINDOW.X, USER_WINDOW.Y, USER_WINDOW.X, USER_WINDOW.Y);
    glfwMakeContextCurrent(GLFWWINDOW);
    glfwSwapInterval(1); // Enable vsync

    return GLFWWINDOW;

}

#endif