#pragma once
#include <windows.h>
#include <string>
#include "GLwinDefs.h"

#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers

#ifdef __cplusplus
extern "C" {
#endif

    // My Window library functions for messages at startup
    void GLwinHello(void);

    // Opaque window structure
    typedef struct GLWIN_window GLWIN_window;

    // Window creation & destruction
    GLWIN_window* GLwin_CreateWindow(int width, int height, const wchar_t* title);
    void GLwin_DestroyWindow(GLWIN_window* window);

    // Window/context management
    void GLwinMakeContextCurrent(GLWIN_window* window);
    void GLwinSwapBuffers(GLWIN_window* window);
    void GLwinPollEvents(void);
    int  GLwinWindowShouldClose(GLWIN_window* window);

    // Framebuffer size and window state
    void GLwinGetFramebufferSize(GLWIN_window* window, int* width, int* height);
    int  GLwinGetWidth(GLWIN_window* window);
    int  GLwinGetHeight(GLWIN_window* window);

    // Window icon and maximize
    void GLwinSetWindowIcon(GLWIN_window* window, const wchar_t* iconPath);
   // bool GLwinSetScreenMaximized(GLWIN_window* window, bool maximize);

    // Window hints (stub, for API compatibility)
    void GLwinWindowHint(int hint, int value);

	// Keyboard input API
    int GLwinGetKey(GLWIN_window* window, int keycode);


    // Mouse input API
    int GLwinGetMouseButton(GLWIN_window* window, int button);
    void GLwinGetCursorPos(GLWIN_window* window, double* xpos, double* ypos);

    // Optional: callback for resize
    typedef void(*GLwinResizeCallback)(int width, int height);

    // Terminate and cleanup library (optional, for symmetry with GLFW)
    void GLwinTerminate(void);

#ifdef __cplusplus
}
#endif


// My Window library functions for messages at startup
//void GLwinHello();
//
//typedef struct GLWIN_window GLWIN_window; // Opaque window structure
//
//class GLwinCreateWindow
//{
//public:
//    GLwinCreateWindow(int width, int height, const std::wstring& title);
//    ~GLwinCreateWindow();
//    // ###########################  Test Code  ###########################
//
//    GLWIN_window* GLwin_CreateWindow(int width, int height, const std::wstring& title);
//
//    void Initialize(GLWIN_window* window); // Initialize with window pointer
//
//    /*int glfwGetMouseButton(GLFWwindow* window, int button);
//    void glfwGetCursorPos(GLFWwindow* window, double* xpos, double* ypos);
//    void glfwSetCursorPos(GLFWwindow* window, double xpos, double ypos);
//    GLFWcursor* glfwCreateCursor(const GLFWimage* image, int xhot, int yhot);
//    GLFWcursor* glfwCreateStandardCursor(int shape);
//    void glfwDestroyCursor(GLFWcursor* cursor);
//    void glfwSetCursor(GLFWwindow* window, GLFWcursor* cursor);
//    GLFWmousebuttonfun glfwSetMouseButtonCallback(GLFWwindow* window, GLFWmousebuttonfun callback);
//    GLFWscrollfun glfwSetScrollCallback(GLFWwindow* window, GLFWscrollfun callback);
//    GLFWcursorenterfun glfwSetCursorEnterCallback(GLFWwindow* window, GLFWcursorenterfun callback);*/
//
//
//    GLWIN_window* window;
//    // ###########################  End Test Code  ########################
//
//    // Add more windows features as you expand (resize, input, etc.
//    void GLwinSetWindowIcon(const std::wstring& iconPath); // Optional: set window icon
//    bool GLwinSetScreenMaximized(bool maximize); // Optional: check if window is maximized
//    void GLwinWindowHint(int hint, int value); // Optional: set window hints (not implemented)
//    void GLwinGetFramebufferSize(int* width, int* height); // Get framebuffer size
//    void GLwinMakeContextCurrent(); // Make OpenGL context current
//    void GLwinTerminate(); // Terminate and clean up
//
//    bool GLwinPollEvents();         // Process Win32 messages
//
//    bool shouldClose() const;     // True if window should close
//    HWND getHWND() const;         // Access native handle if needed
//
//    // Swap OpenGL buffers
//    void swapBuffers();
//
//    // New: query current screen size
//    int getWidth() const { return width_; }
//    int getHeight() const { return height_; }
//
//    // Optional: user can set this to get notified when resized
//    std::function<void(int, int)> onResize;
//
//    void updatePrevKeyStates();
//    bool isKeyPressed(int keycode) const; // True if key was just pressed
//    bool isKeyReleased(int keycode) const;
//    bool isKeyDown(int keycode) const;
//    bool isKeyUp(int keycode) const;
//private:
//    HWND hwnd_;
//    HDC hdc_ = nullptr; // Device context OpenGL
//    HGLRC hglrc_ = nullptr; // OpenGL rendering context
//    bool closed_;
//    int width_ = 0, height_ = 0;
//    // Static message procedure
//    std::unordered_map<int, bool> keyState_;
//    std::unordered_map<int, bool> prevKeyState_;
//    static LRESULT CALLBACK GLwinGetProcAddress(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//
//    bool setupOpenGL(); // Setup OpenGL context
//
//
//};





/*
if (!glfwInit())
        return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window) {
        glfwTerminate();
        return false;
    }
    else {
        LogInternals::Instance()->Info("Window created successfully");
    }

    int bufferWidth, bufferHeight;
    glfwGetFramebufferSize(window, &bufferWidth, &bufferHeight);
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1); // Enable vsync



    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        LogInternals::Instance()->Error("Glad Failed!");
    }
    else {
        LogInternals::Instance()->Info("Glad Loaded Correctly");
    }

*/