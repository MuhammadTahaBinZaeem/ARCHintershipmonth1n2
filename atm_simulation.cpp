#include <iomanip>
#include <string>

using namespace std;

class ATMAccount {
public:
    explicit ATMAccount(double startingBalance) : balance(startingBalance) {}

    double GetBalance() const { return balance; }

    bool Deposit(double amount) {
        if (amount <= 0.0) {
            return false;
        }
        balance += amount;
        return true;
    }

    bool Withdraw(double amount) {
        if (amount <= 0.0 || amount > balance) {
            return false;
        }
        balance -= amount;
        return true;
    }

private:
    double balance;
};

#ifdef _WIN32
#include <windows.h>

namespace {
constexpr int ID_AMOUNT_INPUT = 401;
constexpr int ID_BALANCE_BTN = 402;
constexpr int ID_DEPOSIT_BTN = 403;
constexpr int ID_WITHDRAW_BTN = 404;
constexpr int ID_EXIT_BTN = 405;

ATMAccount g_account(1000.00);
HWND g_amountInput = nullptr;
HWND g_status = nullptr;
HWND g_balance = nullptr;

void UpdateBalanceLabel() {
    ostringstream out;
    out << fixed << setprecision(2) << "Balance: $" << g_account.GetBalance();
    SetWindowTextA(g_balance, out.str().c_str());
}

bool ReadAmount(double& amount) {
    const int len = GetWindowTextLengthA(g_amountInput);
    if (len <= 0) {
        return false;
    }

    string text(len + 1, '\0');
    GetWindowTextA(g_amountInput, &text[0], len + 1);
    text.resize(len);

    char* end = nullptr;
    amount = strtod(text.c_str(), &end);
    return *end == '\0';
}

void SetStatus(const string& text) {
    SetWindowTextA(g_status, text.c_str());
}
}  // namespace

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            CreateWindowExA(0, "STATIC", "ATM Simulation", WS_CHILD | WS_VISIBLE,
                            20, 18, 180, 24, hwnd, nullptr, nullptr, nullptr);

            g_balance = CreateWindowExA(0, "STATIC", "Balance: $1000.00", WS_CHILD | WS_VISIBLE,
                                        20, 48, 220, 24, hwnd, nullptr, nullptr, nullptr);

            CreateWindowExA(0, "STATIC", "Amount:", WS_CHILD | WS_VISIBLE,
                            20, 82, 80, 24, hwnd, nullptr, nullptr, nullptr);

            g_amountInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                                            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                            90, 80, 160, 26, hwnd,
                                            reinterpret_cast<HMENU>(ID_AMOUNT_INPUT), nullptr, nullptr);

            CreateWindowExA(0, "BUTTON", "Check Balance", WS_CHILD | WS_VISIBLE,
                            20, 122, 120, 30, hwnd, reinterpret_cast<HMENU>(ID_BALANCE_BTN), nullptr, nullptr);
            CreateWindowExA(0, "BUTTON", "Deposit", WS_CHILD | WS_VISIBLE,
                            150, 122, 90, 30, hwnd, reinterpret_cast<HMENU>(ID_DEPOSIT_BTN), nullptr, nullptr);
            CreateWindowExA(0, "BUTTON", "Withdraw", WS_CHILD | WS_VISIBLE,
                            250, 122, 90, 30, hwnd, reinterpret_cast<HMENU>(ID_WITHDRAW_BTN), nullptr, nullptr);
            CreateWindowExA(0, "BUTTON", "Exit", WS_CHILD | WS_VISIBLE,
                            350, 122, 90, 30, hwnd, reinterpret_cast<HMENU>(ID_EXIT_BTN), nullptr, nullptr);

            g_status = CreateWindowExA(0, "STATIC", "Enter amount and choose an action.", WS_CHILD | WS_VISIBLE,
                                       20, 170, 430, 24, hwnd, nullptr, nullptr, nullptr);
            return 0;
        }

        case WM_COMMAND: {
            const int id = LOWORD(wParam);
            if (id == ID_BALANCE_BTN) {
                UpdateBalanceLabel();
                SetStatus("Balance refreshed.");
            } else if (id == ID_DEPOSIT_BTN) {
                double amount = 0.0;
                if (!ReadAmount(amount) || !g_account.Deposit(amount)) {
                    SetStatus("Deposit failed. Enter amount > 0.");
                } else {
                    UpdateBalanceLabel();
                    SetStatus("Deposit successful.");
                }
            } else if (id == ID_WITHDRAW_BTN) {
                double amount = 0.0;
                if (!ReadAmount(amount) || !g_account.Withdraw(amount)) {
                    SetStatus("Withdrawal failed. Check amount/balance.");
                } else {
                    UpdateBalanceLabel();
                    SetStatus("Withdrawal successful.");
                }
            } else if (id == ID_EXIT_BTN) {
                PostMessage(hwnd, WM_CLOSE, 0, 0);
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
    const char* className = "ATMWindowClass";

    WNDCLASSA wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(0, className, "ATM Simulation",
                                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                                CW_USEDEFAULT, CW_USEDEFAULT, 480, 260,
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

int main() { return RunApp(GetModuleHandleA(nullptr), SW_SHOWDEFAULT); }

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    return RunApp(hInstance, nCmdShow);
}

#else

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <cctype>
#include <sstream>

namespace {
struct Rect {
    int x;
    int y;
    int w;
    int h;
};

ATMAccount g_account(1000.00);
string g_input;
string g_status = "Enter amount and click Deposit/Withdraw.";

const Rect kInput{20, 70, 170, 28};
const Rect kBalanceBtn{210, 70, 120, 28};
const Rect kDepositBtn{340, 70, 100, 28};
const Rect kWithdrawBtn{210, 110, 120, 28};
const Rect kQuitBtn{340, 110, 100, 28};

bool PointInRect(int x, int y, const Rect& r) {
    return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h;
}

void DrawButton(Display* d, Window w, GC gc, const Rect& r, const string& label) {
    XDrawRectangle(d, w, gc, r.x, r.y, r.w, r.h);
    XDrawString(d, w, gc, r.x + 10, r.y + 19, label.c_str(), static_cast<int>(label.size()));
}

double ParseAmount(bool& ok) {
    ok = false;
    if (g_input.empty()) {
        return 0.0;
    }

    int dotCount = 0;
    for (char c : g_input) {
        if (c == '.') {
            ++dotCount;
            if (dotCount > 1) {
                return 0.0;
            }
        } else if (!isdigit(static_cast<unsigned char>(c))) {
            return 0.0;
        }
    }

    double value = 0.0;
    try {
        value = stod(g_input);
    } catch (...) {
        return 0.0;
    }

    ok = true;
    return value;
}

string BalanceText() {
    ostringstream out;
    out << fixed << setprecision(2) << "Balance: $" << g_account.GetBalance();
    return out.str();
}

void Draw(Display* d, Window w, GC gc) {
    XClearWindow(d, w);

    const string title = "ATM Simulation";
    const string inputTitle = "Amount:";
    const string balance = BalanceText();

    XDrawString(d, w, gc, 20, 24, title.c_str(), static_cast<int>(title.size()));
    XDrawString(d, w, gc, 20, 46, balance.c_str(), static_cast<int>(balance.size()));

    XDrawString(d, w, gc, 20, 64, inputTitle.c_str(), static_cast<int>(inputTitle.size()));
    XDrawRectangle(d, w, gc, kInput.x, kInput.y, kInput.w, kInput.h);
    XDrawString(d, w, gc, kInput.x + 8, kInput.y + 19, g_input.c_str(), static_cast<int>(g_input.size()));

    DrawButton(d, w, gc, kBalanceBtn, "Check Balance");
    DrawButton(d, w, gc, kDepositBtn, "Deposit");
    DrawButton(d, w, gc, kWithdrawBtn, "Withdraw");
    DrawButton(d, w, gc, kQuitBtn, "Quit");

    XDrawString(d, w, gc, 20, 170, g_status.c_str(), static_cast<int>(g_status.size()));
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

    Window w = XCreateSimpleWindow(d, RootWindow(d, screen), 100, 100, 470, 210, 1, black, white);
    XStoreName(d, w, "ATM Simulation");
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

            if (key == XK_BackSpace) {
                if (!g_input.empty()) {
                    g_input.pop_back();
                }
            } else if (len > 0 && (isdigit(static_cast<unsigned char>(buff[0])) || buff[0] == '.')) {
                g_input.push_back(buff[0]);
            }
            Draw(d, w, gc);
        } else if (e.type == ButtonPress) {
            int x = e.xbutton.x;
            int y = e.xbutton.y;

            if (PointInRect(x, y, kBalanceBtn)) {
                g_status = "Balance shown.";
            } else if (PointInRect(x, y, kDepositBtn)) {
                bool ok = false;
                const double amount = ParseAmount(ok);
                if (!ok || !g_account.Deposit(amount)) {
                    g_status = "Deposit failed. Enter amount > 0.";
                } else {
                    g_status = "Deposit successful.";
                }
            } else if (PointInRect(x, y, kWithdrawBtn)) {
                bool ok = false;
                const double amount = ParseAmount(ok);
                if (!ok || !g_account.Withdraw(amount)) {
                    g_status = "Withdrawal failed. Check amount/balance.";
                } else {
                    g_status = "Withdrawal successful.";
                }
            } else if (PointInRect(x, y, kQuitBtn)) {
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
