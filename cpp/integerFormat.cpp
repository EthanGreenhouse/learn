// Takes 2 integers and prints in different formats then calculates power and finds maximum value
// Ethan Greenhouse
// 10/3/2025

#include <iostream>
#include <cmath>
#include <algorithm>

using namespace std;

// Function to get a valid integer from the user within the range [0, 15]
int getValidInteger(const string& prompt) {
    int number;
    bool validInput = false;

    // Looping until input is valid
    while (validInput == false) {
        cout << prompt;
        // Check if number is integer
        if (cin >> number) {
            // Check if number in range
            if (number >= 0 && number <= 15) {
                validInput = true;
            } else {
                cout << "Value is out of range! Try again: ";
            }
        } else {
            cout << "Incorrect data type! Please try again: ";
            cin.clear();
            cin.ignore(10000, '\n');  // Clear everything
        }
    }
    return number;
}

int main() {
    // Prompt user and get numbers
    int firstNumber = getValidInteger("Enter the first positive integer (0-15): ");
    int secondNumber = getValidInteger("Enter the second positive integer (0-15): ");

    // Print results
    cout << "=======" << endl;
    cout << "Results" << endl;
    cout << "=======" << endl;
    cout << endl;

    // Print in decimal format
    cout << "Decimal:" << endl;
        cout << "First number: " << dec << firstNumber << endl;
        cout << "Second number: " << dec << secondNumber << endl;
        cout << endl;

    // Print in hexadecimal format
    cout << "Hexadecimal:" << endl;
        cout << "First number: 0x" << hex << firstNumber << endl;
        cout << "Second number: 0x" << hex << secondNumber << endl;
        cout << endl;


    // Print in octal format
    cout << "Octal:" << endl;
        cout << "First number: 0" << oct << firstNumber << endl;
        cout << "Second number: 0" << oct << secondNumber << endl;
        cout << endl;

    // Calculate and print first number raised to power of second
    double power = pow(firstNumber, secondNumber);
    cout << dec;  // Switch back to decimal
    cout << "First number raised to the power of second: " << firstNumber << "^" << secondNumber << " = " << power << endl;
    cout << endl;

    // Find and print maximum of the two numbers
    int maximum = max(firstNumber, secondNumber);
    cout << "Maximum of the two numbers: " << maximum << endl;

    return 0;
}
