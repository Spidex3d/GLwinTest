#include "pch.h"
#include "include/GLwin.h"

#include <windows.h>
//#include <GL/gl.h>
#include <iostream>
#include <unordered_map>

// windows hints
static int g_GLwinMaximizedHint = 0;
static int g_GLwinResizableHint = 1; // Default to resizable

// Internal struct definition
struct GLWIN_window {
    HWND hwnd = nullptr;
    HDC hdc = nullptr;
    HGLRC hglrc = nullptr;
    int width = 0, height = 0;
    bool closed = false;
    std::unordered_map<int, bool> keyState;
    std::unordered_map<int, bool> prevKeyState;
    GLwinResizeCallback resizeCallback = nullptr;
	// mouse state
    double mouseX = 0.0, mouseY = 0.0;
    bool mouseButtons[3] = { false, false, false };
};

// Internal static
static const wchar_t* GLWIN_WINDOW_CLASS = L"GLWIN_WindowClass";
static bool classRegistered = false;

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

// Forward declaration
static LRESULT CALLBACK GLwin_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Implementation

void GLwinHello(void) {
    std::cout << "Hello, GLwin API Test!" << std::endl;
}

GLWIN_window* GLwin_CreateWindow(int width, int height, const wchar_t* title) {
    if (!classRegistered) {
        WNDCLASS wc = {};
        wc.lpfnWndProc = GLwin_WndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = GLWIN_WINDOW_CLASS;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        if (!RegisterClass(&wc)) return nullptr;
        classRegistered = true;
    }
    
    GLWIN_window* win = new GLWIN_window();
    win->width = width;
    win->height = height;

    DWORD style = WS_OVERLAPPEDWINDOW;
    if (!g_GLwinResizableHint) {
        style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX); // Remove resizing and maximize
    }

    HWND hwnd = CreateWindowEx(
        0,
        GLWIN_WINDOW_CLASS,
        title,
		style, //
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        nullptr, nullptr, GetModuleHandle(nullptr), win);

    

    if (!hwnd) {
        delete win;
        return nullptr;
    }
    win->hwnd = hwnd;

    // windows hints
    // Apply maximized hint BEFORE showing window
    if (g_GLwinMaximizedHint) {
        ShowWindow(hwnd, SW_MAXIMIZE);
    }
    else {
        ShowWindow(hwnd, SW_SHOW);
    }
    

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Setup OpenGL
    win->hdc = GetDC(hwnd);
    if (!win->hdc || !SetPixelFormatForGL(win->hdc)) {
        DestroyWindow(hwnd);
        delete win;
        return nullptr;
    }

    win->hglrc = wglCreateContext(win->hdc);
    if (!win->hglrc) {
        ReleaseDC(hwnd, win->hdc);
        DestroyWindow(hwnd);
        delete win;
        return nullptr;
    }
    if (!wglMakeCurrent(win->hdc, win->hglrc)) {
        wglDeleteContext(win->hglrc);
        ReleaseDC(hwnd, win->hdc);
        DestroyWindow(hwnd);
        delete win;
        return nullptr;
    }

    return win;
}

void GLwin_DestroyWindow(GLWIN_window* window) {
    if (!window) return;
    if (window->hglrc) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(window->hglrc);
        window->hglrc = nullptr;
    }
    if (window->hdc && window->hwnd) {
        ReleaseDC(window->hwnd, window->hdc);
        window->hdc = nullptr;
    }
    if (window->hwnd) {
        DestroyWindow(window->hwnd);
        window->hwnd = nullptr;
    }
    delete window;
}

void GLwinMakeContextCurrent(GLWIN_window* window) {
    if (!window || !window->hdc || !window->hglrc) return;
    wglMakeCurrent(window->hdc, window->hglrc);
}

void GLwinSwapBuffers(GLWIN_window* window) {
    if (window && window->hdc) {
        ::SwapBuffers(window->hdc);
    }
}

void GLwinPollEvents(void) {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

int GLwinWindowShouldClose(GLWIN_window* window) {
    return window ? window->closed : 1;
}

void GLwinGetFramebufferSize(GLWIN_window* window, int* width, int* height) {
    if (!window || !window->hwnd) {
        if (width) *width = 0;
        if (height) *height = 0;
        return;
    }
    RECT rect;
    if (GetClientRect(window->hwnd, &rect)) {
        if (width) *width = rect.right - rect.left;
        if (height) *height = rect.bottom - rect.top;
    }
    else {
        if (width) *width = 0;
        if (height) *height = 0;
    }
}

int GLwinGetWidth(GLWIN_window* window) {
    int w = 0;
    GLwinGetFramebufferSize(window, &w, nullptr);
    return w;
}

int GLwinGetHeight(GLWIN_window* window) {
    int h = 0;
    GLwinGetFramebufferSize(window, nullptr, &h);
    return h;
}

void GLwinSetWindowIcon(GLWIN_window* window, const wchar_t* iconPath) {
    if (!window || !window->hwnd) return;
    HICON hIcon = (HICON)LoadImage(
        nullptr, iconPath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE
    );
    if (hIcon) {
        SendMessage(window->hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage(window->hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    }
}

//bool GLwinSetScreenMaximized(GLWIN_window* window, bool maximize) {
//    if (!window || !window->hwnd) return false;
//    ShowWindow(window->hwnd, maximize ? SW_MAXIMIZE : SW_RESTORE);
//    WINDOWPLACEMENT wp;
//    wp.length = sizeof(WINDOWPLACEMENT);
//    if (GetWindowPlacement(window->hwnd, &wp)) {
//        return (maximize ? wp.showCmd == SW_MAXIMIZE
//            : wp.showCmd == SW_SHOWNORMAL || wp.showCmd == SW_RESTORE);
//    }
//    return false;
//}

void GLwinWindowHint(int hint, int value) {

    switch (hint)
    {
    case GLWIN_MAXIMIZED:
		std::cout << "GLwinWindowHint: GLWIN_MAXIMIZED hint set to " << value << " (implemented)\n";
        g_GLwinMaximizedHint = value;
		break;
	case GLWIN_RESIZABLE:
		std::cout << "GLwinWindowHint: GLWIN_RESIZABLE hint set to " << value << " (implemented)\n";
		g_GLwinResizableHint = value;
		break;
    default:
        break;
    }
}


// Keyboard state 
int GLwinGetKey(GLWIN_window* window, int keycode) {
    if (!window) return GLWIN_RELEASE;
    auto it = window->keyState.find(keycode);
    return (it != window->keyState.end() && it->second) ? GLWIN_PRESS : GLWIN_RELEASE;
}
// Mouse state
int GLwinGetMouseButton(GLWIN_window* window, int button)
{
    if (!window || button < 0 || button > 2) return GLWIN_RELEASE;
    return window->mouseButtons[button] ? GLWIN_PRESS : GLWIN_RELEASE;
}
// Get cursor position
void GLwinGetCursorPos(GLWIN_window* window, double* xpos, double* ypos)
{
    if (!window) {
        if (xpos) *xpos = 0;
        if (ypos) *ypos = 0;
        return;
    }
    if (xpos) *xpos = window->mouseX;
    if (ypos) *ypos = window->mouseY;
}

// Optional: terminate function
void GLwinTerminate(void) {
    // For now, nothing (all handled per-window)
}

// Window procedure (handles messages and input)
static LRESULT CALLBACK GLwin_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    GLWIN_window* window = nullptr;
    if (msg == WM_NCCREATE) {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        window = static_cast<GLWIN_window*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        if (window) window->hwnd = hwnd;
    }
    else {
        window = reinterpret_cast<GLWIN_window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    switch (msg) {
    case WM_CLOSE:
        if (window) window->closed = true;
        PostQuitMessage(0);
        return 0;
    case WM_SIZE:
        if (window) {
            window->width = LOWORD(lParam);
            window->height = HIWORD(lParam);
            if (window->resizeCallback) {
                window->resizeCallback(window->width, window->height);
            }
        }
        return 0;
    case WM_KEYDOWN:
        if (window) window->keyState[(int)wParam] = true;
        return 0;
    case WM_KEYUP:
        if (window) window->keyState[(int)wParam] = false;
        return 0;

        // Mouse events can be added here
    case WM_MOUSEMOVE:
        if (window) {
            window->mouseX = GET_X_LPARAM(lParam);
            window->mouseY = GET_Y_LPARAM(lParam);
        }
        break;
    case WM_LBUTTONDOWN:
        if (window) window->mouseButtons[GLWIN_MOUSE_BUTTON_LEFT] = true;
        break;
    case WM_LBUTTONUP:
        if (window) window->mouseButtons[GLWIN_MOUSE_BUTTON_LEFT] = false;
        break;
    case WM_RBUTTONDOWN:
        if (window) window->mouseButtons[GLWIN_MOUSE_BUTTON_RIGHT] = true;
        break;
    case WM_RBUTTONUP:
        if (window) window->mouseButtons[GLWIN_MOUSE_BUTTON_RIGHT] = false;
        break;
    case WM_MBUTTONDOWN:
        if (window) window->mouseButtons[GLWIN_MOUSE_BUTTON_MIDDLE] = true;
        break;
    case WM_MBUTTONUP:
        if (window) window->mouseButtons[GLWIN_MOUSE_BUTTON_MIDDLE] = false;
        break;
    }
	
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

//void GLwinHello()
//{
//	std::cout << "Hello, GLwin first test!" << std::endl;
//}
//
//// Helper: Set pixel format for OpenGL
//static bool SetPixelFormatForGL(HDC hdc) {
//    PIXELFORMATDESCRIPTOR pfd = {};
//    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
//    pfd.nVersion = 1;
//    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
//    pfd.iPixelType = PFD_TYPE_RGBA;
//    pfd.cColorBits = 24;
//    pfd.cDepthBits = 24;
//    pfd.iLayerType = PFD_MAIN_PLANE;
//
//    int pf = ChoosePixelFormat(hdc, &pfd);
//    if (pf == 0) return false;
//    if (!SetPixelFormat(hdc, pf, &pfd)) return false;
//    return true;
//}
//
//GLwinCreateWindow::GLwinCreateWindow(int width, int height, const std::wstring& title)
//    : width_(width), height_(height)
//{
//    // Register window class (once per app)
//    static const wchar_t CLASS_NAME[] = L"GLwinWindowClass";
//    static bool classRegistered = false;
//
//    if (!classRegistered) {
//        WNDCLASS wc = {};
//       // wc.lpfnWndProc = GLwinCreateWindow::WindowProc;
//        wc.lpfnWndProc = GLwinCreateWindow::GLwinGetProcAddress;
//        wc.hInstance = GetModuleHandle(nullptr);
//        wc.lpszClassName = CLASS_NAME;
//        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
//        RegisterClass(&wc);
//        classRegistered = true;
//    }
//
//    // Create window
//    hwnd_ = CreateWindowEx(
//        0,                          // Optional window styles.
//        CLASS_NAME,                 // Window class
//        title.c_str(),              // Window text
//        WS_OVERLAPPEDWINDOW,        // Window style
//        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
//        nullptr, nullptr, GetModuleHandle(nullptr), this);
//
//    if (!hwnd_) {
//        throw std::runtime_error("Failed to create window");
//    }
//
//    ShowWindow(hwnd_, SW_SHOW);
//    UpdateWindow(hwnd_);
//
//    // Setup OpenGL
//    if (!setupOpenGL()) {
//        throw std::runtime_error("Failed to initialize OpenGL context");
//    }
//}
//
//GLwinCreateWindow::~GLwinCreateWindow()
//{
//	if (hglrc_) {  // opengl context
//        wglMakeCurrent(nullptr, nullptr);
//        wglDeleteContext(hglrc_);
//        hglrc_ = nullptr;
//    }
//	if (hdc_ && hwnd_) {  // device context
//        ReleaseDC(hwnd_, hdc_);
//        hdc_ = nullptr;
//    }
//
//	if (hwnd_) { // window handle
//        DestroyWindow(hwnd_);
//    }
//}
//
//
//
//// Remamed
//bool GLwinCreateWindow::GLwinPollEvents()
//{
//    MSG msg;
//    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
//        if (msg.message == WM_QUIT) {
//            closed_ = true;
//            return false;
//        }
//        TranslateMessage(&msg);
//        DispatchMessage(&msg);
//    }
//    return !closed_;
//}
//
//bool GLwinCreateWindow::shouldClose() const
//{
//    return closed_;
//}
//
//HWND GLwinCreateWindow::getHWND() const
//{
//    return hwnd_;
//}
//// Set window icon from file path
//void GLwinCreateWindow::GLwinSetWindowIcon(const std::wstring& iconPath)
//{
//    HICON hIcon = (HICON)LoadImage(
//        nullptr, iconPath.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE
//    );
//    if (hIcon) {
//        // Set both big and small icons
//        SendMessage(hwnd_, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
//        SendMessage(hwnd_, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
//        // Don't destroy hIcon immediately; Windows may use it as long as window exists
//    }
//    else {
//        // Optionally handle error (icon not found, bad format, etc.)
//        std::wcout << "Failed to load icon: " << std::wstring(iconPath) << std::endl;
//    }
//}
//
//bool GLwinCreateWindow::GLwinSetScreenMaximized(bool maximize)
//{
//    if (!hwnd_) return false;
//    if (maximize) {
//        ShowWindow(hwnd_, SW_MAXIMIZE);
//    }
//    else {
//        ShowWindow(hwnd_, SW_RESTORE);
//    }
//    // Optionally, verify state:
//    WINDOWPLACEMENT wp;
//    wp.length = sizeof(WINDOWPLACEMENT);
//    if (GetWindowPlacement(hwnd_, &wp)) {
//        return (maximize ? wp.showCmd == SW_MAXIMIZE : wp.showCmd == SW_SHOWNORMAL || wp.showCmd == SW_RESTORE);
//    }
//    return false;
//}
//
//void GLwinCreateWindow::GLwinWindowHint(int hint, int value)
//{
//    std::cout << "GLwinWindowHint: (hint=" << hint << ", value=" << value << ") -- not implemented.\n";
//
//}
//void GLwinCreateWindow::GLwinGetFramebufferSize(int* width, int* height)
//{
//    if (!hwnd_) {
//        if (width) *width = 0;
//        if (height) *height = 0;
//        return;
//    }
//    RECT rect;
//    if (GetClientRect(hwnd_, &rect)) {
//        if (width) *width = rect.right - rect.left;
//        if (height) *height = rect.bottom - rect.top;
//    }
//    else {
//        if (width) *width = 0;
//        if (height) *height = 0;
//    }
//}
//void GLwinCreateWindow::GLwinMakeContextCurrent()
//{
//	if (hglrc_ && hdc_) {
//		wglMakeCurrent(hdc_, hglrc_);
//	}
//}
//void GLwinCreateWindow::GLwinTerminate()
//{
//    // This is basically your destructor logic
//    if (hglrc_) {
//        wglMakeCurrent(nullptr, nullptr);
//        wglDeleteContext(hglrc_);
//        hglrc_ = nullptr;
//    }
//    if (hdc_ && hwnd_) {
//        ReleaseDC(hwnd_, hdc_);
//        hdc_ = nullptr;
//    }
//    if (hwnd_) {
//        DestroyWindow(hwnd_);
//        hwnd_ = nullptr;
//    }
//}
//// Swap OpenGL buffers
//void GLwinCreateWindow::swapBuffers()
//{
//    if (hdc_) SwapBuffers(hdc_);
//}
//
//void GLwinCreateWindow::updatePrevKeyStates()
//{
//    prevKeyState_ = keyState_;
//}
//
//bool GLwinCreateWindow::isKeyPressed(int keycode) const
//{
//    // Just pressed this frame
//    bool prev = prevKeyState_.count(keycode) ? prevKeyState_.at(keycode) : false;
//    bool curr = isKeyDown(keycode);
//    return !prev && curr;
//}
//
//bool GLwinCreateWindow::isKeyReleased(int keycode) const
//{
//    bool prev = prevKeyState_.count(keycode) ? prevKeyState_.at(keycode) : false;
//    bool curr = isKeyDown(keycode);
//    return prev && !curr;
//}
//
//bool GLwinCreateWindow::isKeyDown(int keycode) const
//{
//    auto it = keyState_.find(keycode);
//    return it != keyState_.end() && it->second;
//}
//
//bool GLwinCreateWindow::isKeyUp(int keycode) const
//{
//    auto it = keyState_.find(keycode);
//    // If key is not in the map, treat it as "up" (not pressed)
//    return it == keyState_.end() || !it->second;
//}
//
//
////LRESULT GLwinCreateWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
//LRESULT GLwinCreateWindow::GLwinGetProcAddress(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//    GLwinCreateWindow* self = nullptr;
//    
//
//    if (uMsg == WM_NCCREATE) {
//        // Store pointer to class in window user data
//        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
//        self = static_cast<GLwinCreateWindow*>(cs->lpCreateParams);
//        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
//        self->hwnd_ = hwnd;
//    }
//    else {
//        self = reinterpret_cast<GLwinCreateWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
//    }
//
//    if (self) {
//        switch (uMsg) {
//        case WM_CLOSE:
//            self->closed_ = true;
//            PostQuitMessage(0);
//            return 0;
//            //case WM_KEYDOWN: // close on Escape key
//        case GLWIN_KEYDOWN: // close on Escape key
//            self->keyState_[(int)wParam] = true;
//            break;
//        case GLWIN_KEYUP:
//
//            self->keyState_[(int)wParam] = false;
//            break;
//        /*case GLWIN_ESCAPE:
//			if (wParam == GLWIN_ESCAPE) {
//				 self->closed_ = true;
//				 PostQuitMessage(0);
//				 return 0;
//               break;
//        }*/
//                
//            case WM_SIZE: {
//                int w = LOWORD(lParam);
//                int h = HIWORD(lParam);
//                self->width_ = w;
//                self->height_ = h;
//                if (self->onResize) self->onResize(w, h);
//                return 0;
//            
//            }
//        
//        }
//    }
//
//    return DefWindowProc(hwnd, uMsg, wParam, lParam);
//}
//// Setup OpenGL context
//bool GLwinCreateWindow::setupOpenGL()
//{
//    hdc_ = GetDC(hwnd_);
//    if (!hdc_) return false;
//    if (!SetPixelFormatForGL(hdc_)) return false;
//
//    hglrc_ = wglCreateContext(hdc_);
//    if (!hglrc_) return false;
//    if (!wglMakeCurrent(hdc_, hglrc_)) return false;
//    return true;
//}
