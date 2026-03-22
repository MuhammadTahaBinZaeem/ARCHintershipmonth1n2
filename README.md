# ARCH Internship - Month 1

This repository now contains four C++ practice projects:

1. **`dice.cpp`** (Project 1): A Win32 dice roller GUI that rolls two dice and shows their total.
2. **`todo.cpp`** (Project 2): A Win32 to-do list GUI with add, remove selected, and clear-all actions.
3. **`number_guess.cpp`** (Project 3): A console number guessing game between 1 and 100 with high/low hints.
4. **`atm_simulation.cpp`** (Project 4): A console ATM simulation using OOP for balance, deposit, and withdrawal.

---

## Requirements

- Windows OS for the GUI projects (`dice.cpp`, `todo.cpp`)
- A C++ compiler with Win32 support for GUI apps
  - Visual C++ (`cl`) from Visual Studio Developer Command Prompt, or
  - MinGW g++
- Any C++17-compatible compiler for console apps (`number_guess.cpp`, `atm_simulation.cpp`)

---

## Build

### Using MSVC (`cl`)

```powershell
cl /EHsc dice.cpp user32.lib gdi32.lib
cl /EHsc todo.cpp user32.lib gdi32.lib
cl /EHsc number_guess.cpp
cl /EHsc atm_simulation.cpp
```

### Using MinGW g++

```powershell
g++ -std=c++17 dice.cpp -o dice.exe -municode -lgdi32 -luser32
g++ -std=c++17 todo.cpp -o todo.exe -municode -lgdi32 -luser32
g++ -std=c++17 number_guess.cpp -o number_guess.exe
g++ -std=c++17 atm_simulation.cpp -o atm_simulation.exe
```

---

## Run

```powershell
.\dice.exe
.\todo.exe
.\number_guess.exe
.\atm_simulation.exe
```

---

## Screenshots

A `screenshots/` folder is included with stage images for all four projects.

### Project 1 - Dice Roller

![Dice Roller - Stage 1](screenshots/dice_stage1.svg)
![Dice Roller - Stage 2](screenshots/dice_stage2.svg)

### Project 2 - To-Do List

![To-Do List - Stage 1](screenshots/todo_stage1.svg)
![To-Do List - Stage 2](screenshots/todo_stage2.svg)

### Project 3 - Number Guessing Game

![Number Guessing - Stage 1](screenshots/number_guess_stage1.svg)
![Number Guessing - Stage 2](screenshots/number_guess_stage2.svg)

### Project 4 - ATM Simulation

![ATM Simulation - Stage 1](screenshots/atm_stage1.svg)
![ATM Simulation - Stage 2](screenshots/atm_stage2.svg)
