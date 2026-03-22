#include <iomanip>
#include <iostream>
#include <limits>

using namespace std;

class ATMAccount {
public:
    explicit ATMAccount(double startingBalance) : balance(startingBalance) {}

    double GetBalance() const {
        return balance;
    }

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

class ATMController {
public:
    explicit ATMController(double openingBalance) : account(openingBalance) {}

    void Run() {
        cout << fixed << setprecision(2);
        cout << "=== ATM Simulation ===\n";

        while (true) {
            PrintMenu();
            const int choice = ReadMenuChoice();

            if (choice == 1) {
                ShowBalance();
            } else if (choice == 2) {
                HandleDeposit();
            } else if (choice == 3) {
                HandleWithdrawal();
            } else if (choice == 4) {
                cout << "Thank you for using the ATM. Goodbye!\n";
                break;
            }
        }
    }

private:
    ATMAccount account;

    void PrintMenu() const {
        cout << "\nChoose an option:\n";
        cout << "1. Check Balance\n";
        cout << "2. Deposit\n";
        cout << "3. Withdraw\n";
        cout << "4. Exit\n";
    }

    int ReadMenuChoice() const {
        while (true) {
            cout << "Enter choice (1-4): ";
            int choice = 0;
            cin >> choice;

            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Please enter a valid menu option.\n";
                continue;
            }

            if (choice < 1 || choice > 4) {
                cout << "Choice must be between 1 and 4.\n";
                continue;
            }

            return choice;
        }
    }

    double ReadAmount(const char* prompt) const {
        while (true) {
            cout << prompt;
            double amount = 0.0;
            cin >> amount;

            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Please enter a valid amount.\n";
                continue;
            }

            return amount;
        }
    }

    void ShowBalance() const {
        cout << "Current balance: $" << account.GetBalance() << "\n";
    }

    void HandleDeposit() {
        const double amount = ReadAmount("Enter amount to deposit: $");
        if (account.Deposit(amount)) {
            cout << "Deposit successful.\n";
            ShowBalance();
        } else {
            cout << "Deposit failed. Amount must be greater than $0.00.\n";
        }
    }

    void HandleWithdrawal() {
        const double amount = ReadAmount("Enter amount to withdraw: $");
        if (account.Withdraw(amount)) {
            cout << "Withdrawal successful.\n";
            ShowBalance();
        } else {
            cout << "Withdrawal failed. Check amount and available balance.\n";
        }
    }
};

int main() {
    ATMController controller(1000.00);
    controller.Run();
    return 0;
}
