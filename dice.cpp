#include <random>
#include <string>

using namespace std;

#ifdef _WIN32
#include <windows.h>

namespace {
constexpr int ID_ROLL_BUTTON = 101;
constexpr int ID_QUIT_BUTTON = 102;

HWND g_die1Text = nullptr;
HWND g_die2Text = nullptr;
HWND g_totalText = nullptr;
HWND g_hintText = nullptr;

mt19937 g_rng{random_device{}()};
uniform_int_distribution<int> g_dist(1, 6);

void RollDice() {
    const int die1 = g_dist(g_rng);
    const int die2 = g_dist(g_rng);
    const int total = die1 + die2;

    SetWindowTextA(g_die1Text, ("Die 1: " + to_string(die1)).c_str());
    SetWindowTextA(g_die2Text, ("Die 2: " + to_string(die2)).c_str());
    SetWindowTextA(g_totalText, ("Total: " + to_string(total)).c_str());
    SetWindowTextA(g_hintText, "Roll again?");
}
}  // namespace

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            CreateWindowExA(0, "STATIC", "Dice Roller", WS_CHILD | WS_VISIBLE,
                            20, 20, 320, 30, hwnd, nullptr, nullptr, nullptr);

            g_die1Text = CreateWindowExA(0, "STATIC", "Die 1: -", WS_CHILD | WS_VISIBLE,
                                         20, 60, 200, 25, hwnd, nullptr, nullptr, nullptr);

            g_die2Text = CreateWindowExA(0, "STATIC", "Die 2: -", WS_CHILD | WS_VISIBLE,
                                         20, 90, 200, 25, hwnd, nullptr, nullptr, nullptr);

            g_totalText = CreateWindowExA(0, "STATIC", "Total: -", WS_CHILD | WS_VISIBLE,
                                          20, 120, 200, 25, hwnd, nullptr, nullptr, nullptr);

            g_hintText = CreateWindowExA(0, "STATIC", "Press Roll Again to start.",
                                         WS_CHILD | WS_VISIBLE, 20, 150, 220, 25, hwnd,
                                         nullptr, nullptr, nullptr);

            CreateWindowExA(0, "BUTTON", "Roll Again", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                            20, 190, 120, 35, hwnd, reinterpret_cast<HMENU>(ID_ROLL_BUTTON), nullptr, nullptr);

            CreateWindowExA(0, "BUTTON", "Quit", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                            160, 190, 120, 35, hwnd, reinterpret_cast<HMENU>(ID_QUIT_BUTTON), nullptr, nullptr);
            return 0;
        }

        case WM_COMMAND: {
            const int controlId = LOWORD(wParam);
            if (controlId == ID_ROLL_BUTTON) {
                RollDice();
            } else if (controlId == ID_QUIT_BUTTON) {
                PostMessage(hwnd, WM_CLOSE, 0, 0);
            }
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int RunApp(HINSTANCE hInstance, int nCmdShow) {
    const char* className = "DiceRollerWindowClass";

    WNDCLASSA wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0,
        className,
        "Dice Roller",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 320, 290,
        nullptr, nullptr, hInstance, nullptr
    );

    if (hwnd == nullptr) {
        MessageBoxA(nullptr, "Failed to create window.", "Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

int main() {
    return RunApp(GetModuleHandleA(nullptr), SW_SHOWDEFAULT);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    return RunApp(hInstance, nCmdShow);
}

#else

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <cstdlib>

namespace {
struct Rect {
    int x;
    int y;
    int w;
    int h;
};

mt19937 g_rng{random_device{}()};
uniform_int_distribution<int> g_dist(1, 6);

string g_die1 = "Die 1: -";
string g_die2 = "Die 2: -";
string g_total = "Total: -";
string g_hint = "Click Roll to start.";

const Rect kRollButton{20, 170, 120, 36};
const Rect kQuitButton{160, 170, 120, 36};

bool PointInRect(int x, int y, const Rect& r) {
    return x >= r.x && x <= (r.x + r.w) && y >= r.y && y <= (r.y + r.h);
}

void RollDice() {
    const int die1 = g_dist(g_rng);
    const int die2 = g_dist(g_rng);
    const int total = die1 + die2;

    g_die1 = "Die 1: " + to_string(die1);
    g_die2 = "Die 2: " + to_string(die2);
    g_total = "Total: " + to_string(total);
    g_hint = "Roll again?";
}

void DrawButton(Display* display, Window window, GC gc, const Rect& button, const string& label) {
    XDrawRectangle(display, window, gc, button.x, button.y, button.w, button.h);
    XDrawString(display, window, gc, button.x + 12, button.y + 23, label.c_str(), static_cast<int>(label.size()));
}

void Redraw(Display* display, Window window, GC gc) {
    XClearWindow(display, window);

    const string title = "Dice Roller";
    XDrawString(display, window, gc, 20, 30, title.c_str(), static_cast<int>(title.size()));
    XDrawString(display, window, gc, 20, 65, g_die1.c_str(), static_cast<int>(g_die1.size()));
    XDrawString(display, window, gc, 20, 95, g_die2.c_str(), static_cast<int>(g_die2.size()));
    XDrawString(display, window, gc, 20, 125, g_total.c_str(), static_cast<int>(g_total.size()));
    XDrawString(display, window, gc, 20, 150, g_hint.c_str(), static_cast<int>(g_hint.size()));

    DrawButton(display, window, gc, kRollButton, "Roll");
    DrawButton(display, window, gc, kQuitButton, "Quit");
}
}  // namespace

int main() {
    Display* display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        return 1;
    }

    const int screen = DefaultScreen(display);
    const unsigned long white = WhitePixel(display, screen);
    const unsigned long black = BlackPixel(display, screen);

    Window window = XCreateSimpleWindow(display, RootWindow(display, screen),
                                        100, 100, 320, 240, 1, black, white);

    XStoreName(display, window, "Dice Roller");
    XSelectInput(display, window, ExposureMask | ButtonPressMask | KeyPressMask | StructureNotifyMask);
    XMapWindow(display, window);

    GC gc = XCreateGC(display, window, 0, nullptr);
    XSetForeground(display, gc, black);

    bool running = true;
    while (running) {
        XEvent event;
        XNextEvent(display, &event);

        if (event.type == Expose) {
            Redraw(display, window, gc);
        } else if (event.type == ButtonPress) {
            const int x = event.xbutton.x;
            const int y = event.xbutton.y;

            if (PointInRect(x, y, kRollButton)) {
                RollDice();
                Redraw(display, window, gc);
            } else if (PointInRect(x, y, kQuitButton)) {
                running = false;
            }
        } else if (event.type == KeyPress) {
            KeySym key = XLookupKeysym(&event.xkey, 0);
            if (key == XK_Escape || key == XK_q || key == XK_Q) {
                running = false;
            }
        } else if (event.type == DestroyNotify) {
            running = false;
        }
    }

    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}

#endif
