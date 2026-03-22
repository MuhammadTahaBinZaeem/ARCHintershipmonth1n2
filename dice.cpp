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

#include <iostream>
#include <limits>

namespace {
mt19937 g_rng{random_device{}()};
uniform_int_distribution<int> g_dist(1, 6);

void RollDice() {
    const int die1 = g_dist(g_rng);
    const int die2 = g_dist(g_rng);
    const int total = die1 + die2;

    cout << "Die 1: " << die1 << "\n";
    cout << "Die 2: " << die2 << "\n";
    cout << "Total: " << total << "\n";
}
}  // namespace

int main() {
    cout << "=== Dice Roller (Console mode for non-Windows systems) ===\n";

    while (true) {
        cout << "\n1. Roll Dice\n";
        cout << "2. Exit\n";
        cout << "Choice: ";

        int choice = 0;
        if (!(cin >> choice)) {
            cout << "Invalid choice.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (choice == 1) {
            RollDice();
        } else if (choice == 2) {
            cout << "Goodbye!\n";
            break;
        } else {
            cout << "Please choose 1 or 2.\n";
        }
    }

    return 0;
}

#endif
