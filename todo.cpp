#include <string>
#include <vector>

using namespace std;

#ifdef _WIN32
#include <windows.h>

namespace {
constexpr int ID_TASK_INPUT = 201;
constexpr int ID_TASK_LIST = 202;
constexpr int ID_ADD_BUTTON = 203;
constexpr int ID_REMOVE_BUTTON = 204;
constexpr int ID_CLEAR_BUTTON = 205;
constexpr int ID_STATUS_LABEL = 206;

HWND g_taskInput = nullptr;
HWND g_taskList = nullptr;
HWND g_statusLabel = nullptr;
vector<string> g_tasks;

void UpdateStatusText() {
    string status = "Tasks: " + to_string(g_tasks.size());
    SetWindowTextA(g_statusLabel, status.c_str());
}

void ShowInputError(HWND hwnd, const char* message) {
    MessageBoxA(hwnd, message, "To-Do List", MB_OK | MB_ICONINFORMATION);
}

void AddTask(HWND hwnd) {
    int textLength = GetWindowTextLengthA(g_taskInput);
    if (textLength <= 0) {
        ShowInputError(hwnd, "Please enter a task before adding.");
        return;
    }

    string task(textLength + 1, '\0');
    GetWindowTextA(g_taskInput, &task[0], textLength + 1);
    task.resize(textLength);

    if (task.find_first_not_of(" \t\r\n") == string::npos) {
        ShowInputError(hwnd, "Task cannot be only spaces.");
        return;
    }

    g_tasks.push_back(task);
    SendMessageA(g_taskList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(task.c_str()));
    SetWindowTextA(g_taskInput, "");
    SetFocus(g_taskInput);
    UpdateStatusText();
}

void RemoveSelectedTask(HWND hwnd) {
    LRESULT selectedIndex = SendMessageA(g_taskList, LB_GETCURSEL, 0, 0);
    if (selectedIndex == LB_ERR) {
        ShowInputError(hwnd, "Select a task to remove.");
        return;
    }

    int index = static_cast<int>(selectedIndex);
    g_tasks.erase(g_tasks.begin() + index);
    SendMessageA(g_taskList, LB_DELETESTRING, selectedIndex, 0);
    UpdateStatusText();
}

void ClearAllTasks(HWND hwnd) {
    if (g_tasks.empty()) {
        ShowInputError(hwnd, "There are no tasks to clear.");
        return;
    }

    int answer = MessageBoxA(hwnd, "Clear all tasks?", "Confirm", MB_YESNO | MB_ICONQUESTION);
    if (answer != IDYES) {
        return;
    }

    g_tasks.clear();
    SendMessageA(g_taskList, LB_RESETCONTENT, 0, 0);
    UpdateStatusText();
}
}  // namespace

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            CreateWindowExA(0, "STATIC", "Task:", WS_CHILD | WS_VISIBLE,
                            20, 20, 60, 24, hwnd, nullptr, nullptr, nullptr);

            g_taskInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                                          WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                          70, 18, 280, 28, hwnd,
                                          reinterpret_cast<HMENU>(ID_TASK_INPUT), nullptr, nullptr);

            CreateWindowExA(0, "BUTTON", "Add Task", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                            360, 18, 100, 28, hwnd, reinterpret_cast<HMENU>(ID_ADD_BUTTON), nullptr, nullptr);

            g_taskList = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", "",
                                         WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL,
                                         20, 60, 440, 220, hwnd,
                                         reinterpret_cast<HMENU>(ID_TASK_LIST), nullptr, nullptr);

            CreateWindowExA(0, "BUTTON", "Remove Selected", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                            20, 295, 140, 32, hwnd, reinterpret_cast<HMENU>(ID_REMOVE_BUTTON), nullptr, nullptr);

            CreateWindowExA(0, "BUTTON", "Clear All", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                            170, 295, 120, 32, hwnd, reinterpret_cast<HMENU>(ID_CLEAR_BUTTON), nullptr, nullptr);

            g_statusLabel = CreateWindowExA(0, "STATIC", "Tasks: 0", WS_CHILD | WS_VISIBLE,
                                            360, 302, 100, 24, hwnd,
                                            reinterpret_cast<HMENU>(ID_STATUS_LABEL), nullptr, nullptr);

            SetFocus(g_taskInput);
            return 0;
        }

        case WM_COMMAND: {
            int controlId = LOWORD(wParam);
            if (controlId == ID_ADD_BUTTON) {
                AddTask(hwnd);
            } else if (controlId == ID_REMOVE_BUTTON) {
                RemoveSelectedTask(hwnd);
            } else if (controlId == ID_CLEAR_BUTTON) {
                ClearAllTasks(hwnd);
            }
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

int RunApp(HINSTANCE hInstance, int nCmdShow) {
    const char* className = "TodoListWindowClass";

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
        "To-Do List",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 390,
        nullptr, nullptr, hInstance, nullptr
    );

    if (hwnd == nullptr) {
        MessageBoxA(nullptr, "Failed to create To-Do window.", "Error", MB_ICONERROR | MB_OK);
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
#include <algorithm>
#include <cctype>
#include <cstdlib>

namespace {
struct Rect {
    int x;
    int y;
    int w;
    int h;
};

vector<string> g_tasks;
string g_input;
string g_status = "Type task text, then click Add.";
int g_selected = -1;

const Rect kInputBox{20, 20, 320, 30};
const Rect kAddButton{360, 20, 100, 30};
const Rect kListBox{20, 70, 440, 220};
const Rect kRemoveButton{20, 305, 150, 32};
const Rect kClearButton{180, 305, 120, 32};
const Rect kQuitButton{340, 305, 120, 32};

bool PointInRect(int x, int y, const Rect& r) {
    return x >= r.x && x <= (r.x + r.w) && y >= r.y && y <= (r.y + r.h);
}

bool IsBlank(const string& text) {
    return all_of(text.begin(), text.end(), [](unsigned char ch) { return isspace(ch) != 0; });
}

void AddTask() {
    if (g_input.empty() || IsBlank(g_input)) {
        g_status = "Please enter a non-empty task.";
        return;
    }

    g_tasks.push_back(g_input);
    g_input.clear();
    g_selected = static_cast<int>(g_tasks.size()) - 1;
    g_status = "Task added.";
}

void RemoveSelected() {
    if (g_selected < 0 || g_selected >= static_cast<int>(g_tasks.size())) {
        g_status = "Select a task first.";
        return;
    }

    g_tasks.erase(g_tasks.begin() + g_selected);
    if (g_tasks.empty()) {
        g_selected = -1;
    } else if (g_selected >= static_cast<int>(g_tasks.size())) {
        g_selected = static_cast<int>(g_tasks.size()) - 1;
    }
    g_status = "Task removed.";
}

void ClearAll() {
    if (g_tasks.empty()) {
        g_status = "No tasks to clear.";
        return;
    }

    g_tasks.clear();
    g_selected = -1;
    g_status = "All tasks cleared.";
}

void DrawButton(Display* display, Window window, GC gc, const Rect& button, const string& label) {
    XDrawRectangle(display, window, gc, button.x, button.y, button.w, button.h);
    XDrawString(display, window, gc, button.x + 10, button.y + 21, label.c_str(), static_cast<int>(label.size()));
}

void Redraw(Display* display, Window window, GC gc) {
    XClearWindow(display, window);

    const string title = "To-Do List";
    XDrawString(display, window, gc, 20, 14, title.c_str(), static_cast<int>(title.size()));

    XDrawRectangle(display, window, gc, kInputBox.x, kInputBox.y, kInputBox.w, kInputBox.h);
    XDrawString(display, window, gc, kInputBox.x + 8, kInputBox.y + 20, g_input.c_str(), static_cast<int>(g_input.size()));
    DrawButton(display, window, gc, kAddButton, "Add");

    XDrawRectangle(display, window, gc, kListBox.x, kListBox.y, kListBox.w, kListBox.h);
    for (size_t i = 0; i < g_tasks.size(); ++i) {
        const int y = kListBox.y + 22 + static_cast<int>(i) * 18;
        if (y > kListBox.y + kListBox.h - 8) {
            break;
        }

        string line = to_string(i + 1) + ". " + g_tasks[i];
        if (static_cast<int>(i) == g_selected) {
            XFillRectangle(display, window, gc, kListBox.x + 3, y - 13, kListBox.w - 6, 16);
            XSetForeground(display, gc, WhitePixel(display, DefaultScreen(display)));
            XDrawString(display, window, gc, kListBox.x + 8, y, line.c_str(), static_cast<int>(line.size()));
            XSetForeground(display, gc, BlackPixel(display, DefaultScreen(display)));
        } else {
            XDrawString(display, window, gc, kListBox.x + 8, y, line.c_str(), static_cast<int>(line.size()));
        }
    }

    DrawButton(display, window, gc, kRemoveButton, "Remove Selected");
    DrawButton(display, window, gc, kClearButton, "Clear All");
    DrawButton(display, window, gc, kQuitButton, "Quit");

    const string count = "Tasks: " + to_string(g_tasks.size());
    XDrawString(display, window, gc, 360, 355, count.c_str(), static_cast<int>(count.size()));
    XDrawString(display, window, gc, 20, 355, g_status.c_str(), static_cast<int>(g_status.size()));
}

void HandleKeyPress(XKeyEvent* keyEvent) {
    char buffer[16] = {};
    KeySym key = 0;
    int len = XLookupString(keyEvent, buffer, sizeof(buffer), &key, nullptr);

    if (key == XK_Return) {
        AddTask();
    } else if (key == XK_BackSpace) {
        if (!g_input.empty()) {
            g_input.pop_back();
        }
    } else if (key == XK_Escape) {
        g_input.clear();
        g_status = "Input cleared.";
    } else if (len > 0 && isprint(static_cast<unsigned char>(buffer[0])) != 0) {
        g_input.push_back(buffer[0]);
    }
}

void HandleListSelection(int x, int y) {
    if (!PointInRect(x, y, kListBox)) {
        return;
    }

    const int row = (y - (kListBox.y + 8)) / 18;
    if (row >= 0 && row < static_cast<int>(g_tasks.size())) {
        g_selected = row;
        g_status = "Task selected.";
    }
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
                                        100, 100, 500, 380, 1, black, white);

    XStoreName(display, window, "To-Do List");
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
        } else if (event.type == KeyPress) {
            HandleKeyPress(&event.xkey);
            Redraw(display, window, gc);
        } else if (event.type == ButtonPress) {
            const int x = event.xbutton.x;
            const int y = event.xbutton.y;

            if (PointInRect(x, y, kAddButton)) {
                AddTask();
            } else if (PointInRect(x, y, kRemoveButton)) {
                RemoveSelected();
            } else if (PointInRect(x, y, kClearButton)) {
                ClearAll();
            } else if (PointInRect(x, y, kQuitButton)) {
                running = false;
            } else {
                HandleListSelection(x, y);
            }

            Redraw(display, window, gc);
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
