# ARCH Internship - Month 1

This repository contains two C++ Win32 desktop practice applications:

- **`dice.cpp`**: A simple dice roller GUI that rolls two dice and shows their total.
- **`todo.cpp`**: A basic to-do list GUI with add, remove selected, and clear-all actions.

## Requirements

- Windows OS
- A C++ compiler with Win32 support
  - Visual C++ (`cl`) from Visual Studio Developer Command Prompt, or
  - MinGW g++

## Build

### Using MSVC (`cl`)

```powershell
cl /EHsc dice.cpp user32.lib gdi32.lib
cl /EHsc todo.cpp user32.lib gdi32.lib
```

### Using MinGW g++

```powershell
g++ -std=c++17 dice.cpp -o dice.exe -municode -lgdi32 -luser32
g++ -std=c++17 todo.cpp -o todo.exe -municode -lgdi32 -luser32
```

## Run

```powershell
.\dice.exe
.\todo.exe
```
