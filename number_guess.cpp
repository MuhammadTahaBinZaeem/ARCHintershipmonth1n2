#include <limits>
#include <random>
#include <string>

using namespace std;

namespace {
mt19937 g_rng{random_device{}()};
uniform_int_distribution<int> g_dist(1, 100);

int NewSecret() {
    return g_dist(g_rng);
}
}  // namespace

#ifdef _WIN32
#include <windows.h>

namespace {
constexpr int ID_INPUT = 301;
constexpr int ID_GUESS = 302;
constexpr int ID_RESET = 303;

HWND g_input = nullptr;
HWND g_status = nullptr;
HWND g_attemptsLabel = nullptr;
int g_secret = NewSecret();
int g_attempts = 0;

void UpdateAttempts() {
    SetWindowTextA(g_attemptsLabel, ("Attempts: " + to_string(g_attempts)).c_str());
}

void ResetGame() {
    g_secret = NewSecret();
    g_attempts = 0;
    SetWindowTextA(g_input, "");
    SetWindowTextA(g_status, "I picked a number from 1 to 100.");
    UpdateAttempts();
}

void HandleGuess(HWND hwnd) {
    int len = GetWindowTextLengthA(g_input);
    if (len <= 0) {
        MessageBoxA(hwnd, "Enter a number between 1 and 100.", "Number Guessing", MB_OK | MB_ICONINFORMATION);
        return;
    }

    string value(len + 1, '\0');
    GetWindowTextA(g_input, &value[0], len + 1);
    value.resize(len);

    char* end = nullptr;
    const long guess = strtol(value.c_str(), &end, 10);
    if (*end != '\0' || guess < 1 || guess > 100) {
        MessageBoxA(hwnd, "Please enter a valid whole number (1-100).", "Number Guessing", MB_OK | MB_ICONWARNING);
        return;
    }

    ++g_attempts;
    UpdateAttempts();

    if (guess < g_secret) {
        SetWindowTextA(g_status, "Too low. Try a higher number.");
    } else if (guess > g_secret) {
        SetWindowTextA(g_status, "Too high. Try a lower number.");
    } else {
        SetWindowTextA(g_status, ("Correct! Solved in " + to_string(g_attempts) + " attempt(s). Press New Game.").c_str());
    }
}
}  // namespace

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            CreateWindowExA(0, "STATIC", "Number Guessing Game", WS_CHILD | WS_VISIBLE,
                            20, 20, 300, 24, hwnd, nullptr, nullptr, nullptr);

            CreateWindowExA(0, "STATIC", "Guess (1-100):", WS_CHILD | WS_VISIBLE,
                            20, 60, 120, 24, hwnd, nullptr, nullptr, nullptr);

            g_input = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                                      WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                      140, 58, 120, 26, hwnd,
                                      reinterpret_cast<HMENU>(ID_INPUT), nullptr, nullptr);

            CreateWindowExA(0, "BUTTON", "Guess", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                            270, 58, 80, 26, hwnd, reinterpret_cast<HMENU>(ID_GUESS), nullptr, nullptr);

            CreateWindowExA(0, "BUTTON", "New Game", WS_CHILD | WS_VISIBLE,
                            360, 58, 100, 26, hwnd, reinterpret_cast<HMENU>(ID_RESET), nullptr, nullptr);

            g_status = CreateWindowExA(0, "STATIC", "I picked a number from 1 to 100.", WS_CHILD | WS_VISIBLE,
                                       20, 105, 440, 24, hwnd, nullptr, nullptr, nullptr);
            g_attemptsLabel = CreateWindowExA(0, "STATIC", "Attempts: 0", WS_CHILD | WS_VISIBLE,
                                              20, 135, 200, 24, hwnd, nullptr, nullptr, nullptr);

            SetFocus(g_input);
            return 0;
        }

        case WM_COMMAND: {
            const int id = LOWORD(wParam);
            if (id == ID_GUESS) {
                HandleGuess(hwnd);
            } else if (id == ID_RESET) {
                ResetGame();
            }
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

int RunApp(HINSTANCE hInstance, int nCmdShow) {
    const char* className = "NumberGuessWindowClass";

    WNDCLASSA wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(0, className, "Number Guessing Game",
                                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                                CW_USEDEFAULT, CW_USEDEFAULT, 500, 230,
                                nullptr, nullptr, hInstance, nullptr);

    if (!hwnd) {
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessageA(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
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
#include <cctype>

namespace {
struct Rect {
    int x;
    int y;
    int w;
    int h;
};

int g_secret = NewSecret();
int g_attempts = 0;
string g_input;
string g_status = "I picked a number from 1 to 100.";

const Rect kInputBox{20, 55, 140, 28};
const Rect kGuessButton{170, 55, 90, 28};
const Rect kResetButton{270, 55, 110, 28};
const Rect kQuitButton{390, 55, 70, 28};

bool PointInRect(int x, int y, const Rect& r) {
    return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h;
}

void DrawButton(Display* d, Window w, GC gc, const Rect& r, const string& label) {
    XDrawRectangle(d, w, gc, r.x, r.y, r.w, r.h);
    XDrawString(d, w, gc, r.x + 10, r.y + 19, label.c_str(), static_cast<int>(label.size()));
}

void ResetGame() {
    g_secret = NewSecret();
    g_attempts = 0;
    g_input.clear();
    g_status = "I picked a number from 1 to 100.";
}

void EvaluateGuess() {
    if (g_input.empty()) {
        g_status = "Enter a number first.";
        return;
    }

    for (char ch : g_input) {
        if (!isdigit(static_cast<unsigned char>(ch))) {
            g_status = "Use digits only (1-100).";
            return;
        }
    }

    const int guess = stoi(g_input);
    if (guess < 1 || guess > 100) {
        g_status = "Guess must be between 1 and 100.";
        return;
    }

    ++g_attempts;
    if (guess < g_secret) {
        g_status = "Too low. Try higher.";
    } else if (guess > g_secret) {
        g_status = "Too high. Try lower.";
    } else {
        g_status = "Correct in " + to_string(g_attempts) + " attempt(s)! Press New.";
    }
}

void Draw(Display* d, Window w, GC gc) {
    XClearWindow(d, w);

    const string title = "Number Guessing Game";
    const string hint = "Guess a number from 1 to 100:";
    const string attempts = "Attempts: " + to_string(g_attempts);

    XDrawString(d, w, gc, 20, 25, title.c_str(), static_cast<int>(title.size()));
    XDrawString(d, w, gc, 20, 45, hint.c_str(), static_cast<int>(hint.size()));

    XDrawRectangle(d, w, gc, kInputBox.x, kInputBox.y, kInputBox.w, kInputBox.h);
    XDrawString(d, w, gc, kInputBox.x + 8, kInputBox.y + 19, g_input.c_str(), static_cast<int>(g_input.size()));

    DrawButton(d, w, gc, kGuessButton, "Guess");
    DrawButton(d, w, gc, kResetButton, "New");
    DrawButton(d, w, gc, kQuitButton, "Quit");

    XDrawString(d, w, gc, 20, 110, g_status.c_str(), static_cast<int>(g_status.size()));
    XDrawString(d, w, gc, 20, 135, attempts.c_str(), static_cast<int>(attempts.size()));
}
}  // namespace

int main() {
    Display* d = XOpenDisplay(nullptr);
    if (!d) {
        return 1;
    }

    int screen = DefaultScreen(d);
    unsigned long white = WhitePixel(d, screen);
    unsigned long black = BlackPixel(d, screen);

    Window w = XCreateSimpleWindow(d, RootWindow(d, screen), 100, 100, 480, 180, 1, black, white);
    XStoreName(d, w, "Number Guessing Game");
    XSelectInput(d, w, ExposureMask | ButtonPressMask | KeyPressMask);
    XMapWindow(d, w);

    GC gc = XCreateGC(d, w, 0, nullptr);
    XSetForeground(d, gc, black);

    bool running = true;
    while (running) {
        XEvent e;
        XNextEvent(d, &e);

        if (e.type == Expose) {
            Draw(d, w, gc);
        } else if (e.type == KeyPress) {
            char buff[16] = {};
            KeySym key = 0;
            int len = XLookupString(&e.xkey, buff, sizeof(buff), &key, nullptr);

            if (key == XK_Return) {
                EvaluateGuess();
            } else if (key == XK_BackSpace) {
                if (!g_input.empty()) {
                    g_input.pop_back();
                }
            } else if (len > 0 && isprint(static_cast<unsigned char>(buff[0])) != 0) {
                g_input.push_back(buff[0]);
            }
            Draw(d, w, gc);
        } else if (e.type == ButtonPress) {
            int x = e.xbutton.x;
            int y = e.xbutton.y;

            if (PointInRect(x, y, kGuessButton)) {
                EvaluateGuess();
            } else if (PointInRect(x, y, kResetButton)) {
                ResetGame();
            } else if (PointInRect(x, y, kQuitButton)) {
                running = false;
            }
            Draw(d, w, gc);
        }
    }

    XFreeGC(d, gc);
    XDestroyWindow(d, w);
    XCloseDisplay(d);
    return 0;
}

#endif
