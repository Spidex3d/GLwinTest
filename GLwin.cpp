#include "pch.h"
#include <iostream>
#include "../GLwin.h" // In This Order
#include <GL\gl.h> // OpenGL header
#include <direct.h> // _getcwd
#include <stdexcept>

void GLwinHello()
{
	std::cout << "Hello, GLwin first test!" << std::endl;
}

// Helper: Set pixel format for OpenGL
static bool SetPixelFormatForGL(HDC hdc) {
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pf = ChoosePixelFormat(hdc, &pfd);
    if (pf == 0) return false;
    if (!SetPixelFormat(hdc, pf, &pfd)) return false;
    return true;
}

GLwinCreateWindow::GLwinCreateWindow(int width, int height, const std::wstring& title)
    : width_(width), height_(height)
{
    // Register window class (once per app)
    static const wchar_t CLASS_NAME[] = L"GLwinWindowClass";
    static bool classRegistered = false;

    if (!classRegistered) {
        WNDCLASS wc = {};
        wc.lpfnWndProc = GLwinCreateWindow::WindowProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = CLASS_NAME;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        RegisterClass(&wc);
        classRegistered = true;
    }

    // Create window
    hwnd_ = CreateWindowEx(
        0,                          // Optional window styles.
        CLASS_NAME,                 // Window class
        title.c_str(),              // Window text
        WS_OVERLAPPEDWINDOW,        // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        nullptr, nullptr, GetModuleHandle(nullptr), this);

    if (!hwnd_) {
        throw std::runtime_error("Failed to create window");
    }

    ShowWindow(hwnd_, SW_SHOW);
    UpdateWindow(hwnd_);

    // Setup OpenGL
    if (!setupOpenGL()) {
        throw std::runtime_error("Failed to initialize OpenGL context");
    }
}



GLwinCreateWindow::~GLwinCreateWindow()
{
    if (hglrc_) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(hglrc_);
        hglrc_ = nullptr;
    }
    if (hdc_ && hwnd_) {
        ReleaseDC(hwnd_, hdc_);
        hdc_ = nullptr;
    }

    if (hwnd_) {
        DestroyWindow(hwnd_);
    }
}

bool GLwinCreateWindow::processEvents()
{
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            closed_ = true;
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return !closed_;
}

bool GLwinCreateWindow::shouldClose() const
{
    return closed_;
}

HWND GLwinCreateWindow::getHWND() const
{
    return hwnd_;
}

void GLwinCreateWindow::GLwinSetWindowIcon(const std::wstring& iconPath)
{
    HICON hIcon = (HICON)LoadImage(
        nullptr, iconPath.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE
    );
    if (hIcon) {
        // Set both big and small icons
        SendMessage(hwnd_, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage(hwnd_, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        // Don't destroy hIcon immediately; Windows may use it as long as window exists
    }
    else {
        // Optionally handle error (icon not found, bad format, etc.)
        std::wcout << "Failed to load icon: " << std::wstring(iconPath) << std::endl;
    }
}

bool GLwinCreateWindow::GLwinSetScreenMaximized(bool maximize)
{
    if (!hwnd_) return false;
    if (maximize) {
        ShowWindow(hwnd_, SW_MAXIMIZE);
    }
    else {
        ShowWindow(hwnd_, SW_RESTORE);
    }
    // Optionally, verify state:
    WINDOWPLACEMENT wp;
    wp.length = sizeof(WINDOWPLACEMENT);
    if (GetWindowPlacement(hwnd_, &wp)) {
        return (maximize ? wp.showCmd == SW_MAXIMIZE : wp.showCmd == SW_SHOWNORMAL || wp.showCmd == SW_RESTORE);
    }
    return false;
}

void GLwinCreateWindow::swapBuffers()
{
	
    if (hdc_) SwapBuffers(hdc_);
}




LRESULT GLwinCreateWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    GLwinCreateWindow* self = nullptr;
    

    if (uMsg == WM_NCCREATE) {
        // Store pointer to class in window user data
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        self = static_cast<GLwinCreateWindow*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->hwnd_ = hwnd;
    }
    else {
        self = reinterpret_cast<GLwinCreateWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (self) {
        switch (uMsg) {
            case WM_CLOSE:
                self->closed_ = true;
                PostQuitMessage(0);
                return 0;
			case WM_KEYDOWN: // close on Escape key
                if (wParam == VK_ESCAPE) {
                    self->closed_ = true;
                    PostQuitMessage(0);
                    return 0;
                }
                break;
            case WM_SIZE: {
                int w = LOWORD(lParam);
                int h = HIWORD(lParam);
                self->width_ = w;
                self->height_ = h;
                if (self->onResize) self->onResize(w, h);
                return 0;
            
            }
        
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool GLwinCreateWindow::setupOpenGL()
{
    hdc_ = GetDC(hwnd_);
    if (!hdc_) return false;
    if (!SetPixelFormatForGL(hdc_)) return false;

    hglrc_ = wglCreateContext(hdc_);
    if (!hglrc_) return false;
    if (!wglMakeCurrent(hdc_, hglrc_)) return false;
    return true;
}
