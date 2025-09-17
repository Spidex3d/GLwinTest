#pragma once
#include <windows.h>
#include <string>
#include <functional>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// My Window library functions for messages at startup
void GLwinHello();

class GLwinCreateWindow
{
public:
    GLwinCreateWindow(int width, int height, const std::wstring& title);
    ~GLwinCreateWindow();


    // Add more windows features as you expand (resize, input, etc.
    void GLwinSetWindowIcon(const std::wstring& iconPath); // Optional: set window icon
	bool GLwinSetScreenMaximized(bool maximize); // Optional: check if window is maximized
	void GLwinWindowHint(int hint, int value); // Optional: set window hints (not implemented)
	void GLwinGetFramebufferSize(int* width, int* height); // Get framebuffer size
	void GLwinMakeContextCurrent(); // Make OpenGL context current
    void GLwinTerminate(); // Terminate and clean up

    bool GLwinPollEvents();         // Process Win32 messages

    bool shouldClose() const;     // True if window should close
    HWND getHWND() const;         // Access native handle if needed

    // Swap OpenGL buffers
    void swapBuffers();
    
    // New: query current screen size
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

    // Optional: user can set this to get notified when resized
    std::function<void(int, int)> onResize;

    static LRESULT CALLBACK GLwinGetProcAddress(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HWND hwnd_;
	HDC hdc_ = nullptr; // Device context OpenGL
	HGLRC hglrc_ = nullptr; // OpenGL rendering context
    bool closed_;
    int width_ = 0, height_ = 0;
    // Static message procedure
    //static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	bool setupOpenGL(); // Setup OpenGL context


};

//#define WM_KEYFIRST                     0x0100
//#define WM_KEYDOWN                      0x0100
//#define WM_KEYUP                        0x0101
// Key codes
#define GLWIN_KEYDOWN        256 //0x0100
#define WM_KEYUP             257 //0x0101

#define GLWIN_ESCAPE         27 //0x1B
#define GLWIN_RETURN         13 //0x0D

#define GLWIN_LEFT           37 //0x25
#define GLWIN_UP             38 //0x26
#define GLWIN_RIGHT          39 //0x27
#define GLWIN_DOWN           40 //0x28

#define GLWIN_KEY_A          65 //0x41
#define GLWIN_KEY_D          68 //0x44
#define GLWIN_KEY_S          83 //0x53
#define GLWIN_KEY_W          87 //0x57

#define GLWIN_NO_ERROR       0


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