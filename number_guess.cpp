#include <iostream>
#include <limits>
#include <random>
#include <string>

using namespace std;

namespace {
int ReadGuess() {
    while (true) {
        cout << "Enter your guess (1-100): ";

        int guess = 0;
        cin >> guess;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Please enter a valid whole number.\n";
            continue;
        }

        if (guess < 1 || guess > 100) {
            cout << "Guess must be between 1 and 100.\n";
            continue;
        }

        return guess;
    }
}
}  // namespace

int main() {
    mt19937 rng{random_device{}()};
    uniform_int_distribution<int> dist(1, 100);
    const int secret = dist(rng);

    cout << "=== Number Guessing Game ===\n";
    cout << "I have selected a number between 1 and 100.\n";

    int attempts = 0;
    while (true) {
        const int guess = ReadGuess();
        ++attempts;

        if (guess < secret) {
            cout << "Too low. Try a higher number.\n";
        } else if (guess > secret) {
            cout << "Too high. Try a lower number.\n";
        } else {
            cout << "Correct! You guessed the number in " << attempts << " attempt(s).\n";
            break;
        }
    }

    return 0;
}
