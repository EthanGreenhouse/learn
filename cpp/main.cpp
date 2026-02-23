/*
 * EECE2160 Homework #7 - Bit Manipulation Program
 * Author: Ethan Greenhouse
 * Date: November 5, 2025
 */

#include <iostream>
#include <bitset>
using namespace std;

/*
 * Purpose: Prints integer value in binary format (8 bits)
 * Input: value - integer to be printed in binary (0-255)
 */
void printBinary (int value) {
    bitset<8> binary(value); // Convert to binary
    cout << binary << endl;
}

/*
 * Purpose: Get state (0 or 1) of specific bit at given index
 * Input: dataValue - integer containing bits to check
 *       index - position of bit to check
 * Output: 0 or 1 representing bitstate
 */
int getBitState(int dataValue, int index) {
    int mask = 1 << index; // Create mask with 1 at index
    int results = (dataValue & mask) >> index; // AND dataValue with mask and shift right to get bit value
    return results;
}

/*
 * Purpose: Sets specific bit at given index
 * Input: dataValue - integer containing the bits
 *        index - position of bit to set (0-31)
 *        state - value (0 or 1) to get set bit to
 * Output: Value
 */
int setBitValue(int dataValue, int index, int state) {
    int mask;
    if (state == 1) {
        mask = 1 << index; // Create mask with 1 at index
        dataValue |= mask; // OR to set bit to 1
    } else {
        mask = 1 << index; // Create mask with 1 at index
        mask = ~mask; // Invert mask
        dataValue &= mask; // AND to clear bit to 0
    }
    return dataValue;
}

/*
 * Purpose: Test bit manipulation functions
 * Output: Print "Done" on success, return 0
 */
int main() {
    int number;
    char continueChoice;

    while (true) {  // Main loop to repeat program
        bool validInput = false;

        while (validInput == false) { // Loop until input is valid
            cout << "Enter integer from 0 to 255: ";
            if (cin >> number) { // Check if number is integer
                if (number >= 0 && number <= 255) { // Check if number in range
                    validInput = true;
                } else {
                    cout << "Value is out of range! Try again: " << endl;;
                }
            } else {
                cout << "Incorrect data type! Please try again: " << endl;
                cin.clear();
                cin.ignore(10000, '\n'); // Clear everything
            }
        }

        // Print given integer in binary
        cout << "Binary value: ";
        printBinary(number);
        cout << endl;

        // Print bit state of the first 8 bits
        cout << "Bit states: " << endl;
        for (int i = 0; i < 8; i++) { // Loop through each bit
            int bitValue = getBitState(number, i); // Get bit at position i
            cout << "Bit " << i << " = " << bitValue << endl;
        }
        cout << endl;

        // Complement each bit
        int complement = number; // Start with original number
        for (int i = 0; i < 8; i++) { // Loop through each bit
            int currentBit = getBitState(complement, i); // Get current bit
            int newBit = !currentBit;
            complement = setBitValue(complement, i, newBit); // Set new bit
        }

        // Print complemented integer in binary
        cout << "Complemented integer in binary: ";
        printBinary(complement);
        cout << endl;

        // Ask if user wants to continue
        cout << "Do you want to input another integer? (Y/N): ";
        cin >> continueChoice;
        while (continueChoice != 'Y' && continueChoice != 'N') { // Check if input is valid
            cout << "Invalid input! Please try again: ";
            cin >> continueChoice;
        }

        if (continueChoice == 'N') {
            break;  // Exit loop
        }
    }

    cout << "Done" << endl;
    return 0;
}