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

    bool processEvents();         // Process Win32 messages
    bool shouldClose() const;     // True if window should close
    HWND getHWND() const;         // Access native handle if needed

    // Add more windows features as you expand (resize, input, etc.
    void GLwinSetWindowIcon(const std::wstring& iconPath); // Optional: set window icon
	bool GLwinSetScreenMaximized(bool maximize); // Optional: check if window is maximized

    // Swap OpenGL buffers
    void swapBuffers();
    
    // New: query current screen size
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

    // Optional: user can set this to get notified when resized
    std::function<void(int, int)> onResize;
private:
    HWND hwnd_;
	HDC hdc_ = nullptr; // Device context OpenGL
	HGLRC hglrc_ = nullptr; // OpenGL rendering context
    bool closed_;
    int width_ = 0, height_ = 0;
    // Static message procedure
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	bool setupOpenGL(); // Setup OpenGL context

};