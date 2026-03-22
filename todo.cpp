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

#include <iostream>
#include <limits>

namespace {
vector<string> g_tasks;

bool IsBlankTask(const string& task) {
    return task.find_first_not_of(" \t\r\n") == string::npos;
}

void PrintTasks() {
    cout << "\n==== To-Do List (" << g_tasks.size() << " task" << (g_tasks.size() == 1 ? "" : "s") << ") ====" << '\n';
    if (g_tasks.empty()) {
        cout << "(no tasks)\n";
        return;
    }

    for (size_t i = 0; i < g_tasks.size(); ++i) {
        cout << i + 1 << ". " << g_tasks[i] << '\n';
    }
}

void AddTask() {
    cout << "Enter task: ";
    string task;
    getline(cin, task);

    if (task.empty() || IsBlankTask(task)) {
        cout << "Please enter a non-empty task.\n";
        return;
    }

    g_tasks.push_back(task);
    cout << "Task added.\n";
}

void RemoveTask() {
    if (g_tasks.empty()) {
        cout << "No tasks to remove.\n";
        return;
    }

    PrintTasks();
    cout << "Enter task number to remove: ";
    size_t index = 0;
    if (!(cin >> index)) {
        cout << "Invalid input.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (index == 0 || index > g_tasks.size()) {
        cout << "Task number out of range.\n";
        return;
    }

    g_tasks.erase(g_tasks.begin() + static_cast<long>(index - 1));
    cout << "Task removed.\n";
}

void ClearAllTasks() {
    if (g_tasks.empty()) {
        cout << "No tasks to clear.\n";
        return;
    }

    cout << "Clear all tasks? (y/n): ";
    string response;
    getline(cin, response);

    if (!response.empty() && (response[0] == 'y' || response[0] == 'Y')) {
        g_tasks.clear();
        cout << "All tasks cleared.\n";
    } else {
        cout << "Canceled.\n";
    }
}
}  // namespace

int main() {
    cout << "To-Do List (Console mode for non-Windows systems)\n";

    while (true) {
        PrintTasks();
        cout << "\nChoose an action:\n"
             << "1. Add task\n"
             << "2. Remove task\n"
             << "3. Clear all tasks\n"
             << "4. Exit\n"
             << "Choice: ";

        int choice = 0;
        if (!(cin >> choice)) {
            cout << "Invalid choice.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 1) {
            AddTask();
        } else if (choice == 2) {
            RemoveTask();
        } else if (choice == 3) {
            ClearAllTasks();
        } else if (choice == 4) {
            cout << "Goodbye!\n";
            break;
        } else {
            cout << "Please choose 1-4.\n";
        }
    }

    return 0;
}

#endif
