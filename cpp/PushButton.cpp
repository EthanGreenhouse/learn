#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <iostream>
using namespace std;

// Physical base address of FPGA Devices 
const unsigned int LW_BRIDGE_BASE 	= 0xFF200000;  // Base offset 

// Length of memory-mapped IO window 
const unsigned int LW_BRIDGE_SPAN 	= 0x00005000;  // Address map size

// Cyclone V FPGA device addresses
const unsigned int LEDR_BASE 		= 0x00000000;  // Leds offset 
const unsigned int SW_BASE 			= 0x00000040;  // Switches offset
const unsigned int KEY_BASE 		= 0x00000050;  // Push buttons offset

/*
 * Initialize general-purpose I/O 
 *  - Opens access to physical memory /dev/mem 
 *  - Maps memory into virtual address space 
 * 
 * @param fd	File descriptor passed by reference, where the result 
 *			of function 'open' will be stored. 
 * @return	Address to virtual memory which is mapped to physical, or MAP_FAILED on error. 
 */ 
char *Initialize(int *fd) 
{
	// Open /dev/mem to give access to physical addresses
	*fd = open( "/dev/mem", (O_RDWR | O_SYNC));
	if (*fd == -1)			//  check for errors in openning /dev/mem
	{
        cout << "ERROR: could not open /dev/mem..." << endl;
        exit(1);
	}
	
   // Get a mapping from physical addresses to virtual addresses
   char *virtual_base = (char *)mmap (NULL, LW_BRIDGE_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, *fd, LW_BRIDGE_BASE);
   if (virtual_base == MAP_FAILED)		// check for errors
   {
      cout << "ERROR: mmap() failed..." << endl;
      close (*fd);		// close memory before exiting
      exit(1);        // Returns 1 to the operating system;
   }
   return virtual_base;
}

/*
 * Close general-purpose I/O. 
 * 
 * @param pBase	Virtual address where I/O was mapped. 
 * @param fd	File descriptor previously returned by 'open'. 
 */ 
void Finalize(char *pBase, int fd) 
{
	if (munmap (pBase, LW_BRIDGE_SPAN) != 0)
	{
      cout << "ERROR: munmap() failed..." << endl;
      exit(1);
	}
   close (fd); 	// close memory
}

/*
 * Write a 4-byte value at the specified general-purpose I/O location. 
 * 
 * @param pBase		Base address returned by 'mmap'. 
 * @parem offset	Offset where device is mapped. 
 * @param value	Value to be written. 
 */ 
void RegisterWrite(char *pBase, unsigned int reg_offset, int value) 
{ 
	* (volatile unsigned int *)(pBase + reg_offset) = value; 
} 

/*
 * Read a 4-byte value from the specified general-purpose I/O location. 
 * 
 * @param pBase		Base address returned by 'mmap'. 
 * @param offset	Offset where device is mapped. 
 * @return		Value read. 
 */ 
int RegisterRead(char *pBase, unsigned int reg_offset) 
{ 
	return * (volatile unsigned int *)(pBase + reg_offset); 
} 

/*
 * Write a value to all LEDs
 * @param pBase Base address for general-purpose I/O
 * @param value Value to write to LEDs (0-1023)
 */
void WriteAllLeds(char *pBase, int value)
{
    RegisterWrite(pBase, LEDR_BASE, value);
}

/*
 * Reads all the switches and returns their value in a single integer.
 * @param pBase Base address for general-purpose I/O
 * @return A value that represents the value of the switches (0-1023)
 */
int ReadAllSwitches(char *pBase) {
    // Read the entire switch register which contains all 10 switches
    int allSwitches = RegisterRead(pBase, SW_BASE);
    // Mask to keep only the lower 10 bits
    allSwitches = allSwitches & 0x3FF;
    return allSwitches;
}

/*
 * Reads the push buttons and returns which button is pressed
 * @param pBase Base address for general-purpose I/O
 * @return -1 if no button pressed, 0-3 for individual buttons, 4 for multiple buttons
 */
int PushButtonGet(char *pBase) {
    // Read the push button register
    int buttons = RegisterRead(pBase, KEY_BASE);
    // Only look at the lower 4 bits (KEY0-KEY3)
    buttons = buttons & 0xF;
    
    // Check if no buttons are pressed
    if (buttons == 0) {
        return -1;
    }
    
    // Check if multiple buttons are pressed (more than 1 bit set)
    int count = 0;
    for (int i = 0; i < 4; i++) {
        if (buttons & (1 << i)) {
            count++;
        }
    }
    
    if (count > 1) {
        return 4;  // Multiple buttons pressed
    }
    
    // Single button pressed - find which one
    for (int i = 0; i < 4; i++) {
        if (buttons & (1 << i)) {
            return i;
        }
    }
    
    return -1;
}

/*
 * Main Function - Implements counter with push button controls
 */
int main() 
{ 
	// Initialize 
	int fd; 
	char *pBase = Initialize(&fd); 
	
	cout << "=== Push Button Counter Program (Lab 6.3) ===" << endl;
	cout << "Controls:" << endl;
	cout << "  KEY0: Increment counter" << endl;
	cout << "  KEY1: Decrement counter" << endl;
	cout << "  KEY2: Shift right (divide by 2)" << endl;
	cout << "  KEY3: Shift left (multiply by 2)" << endl;
	cout << "  Multiple buttons: Reset to switch value" << endl;
	cout << "Press Ctrl+C to exit\n" << endl;
	
	// Read initial counter value from switches
	int counter = ReadAllSwitches(pBase);
	WriteAllLeds(pBase, counter);
	cout << "Initial counter value from switches: " << counter << endl;
	
	// Variable for button debouncing
	int lastButton = -1;
	
	// Main loop
	while (true) {
		int currentButton = PushButtonGet(pBase);
		
		// Only act when button state changes (for debouncing)
		if (currentButton != lastButton && currentButton != -1) {
			
			switch(currentButton) {
				case 0:  // KEY0 - Increment
					counter++;
					if (counter > 1023) counter = 0;  // Roll over
					cout << "KEY0 pressed - Increment: counter = " << counter << endl;
					break;
					
				case 1:  // KEY1 - Decrement  
					counter--;
					if (counter < 0) counter = 1023;  // Roll under
					cout << "KEY1 pressed - Decrement: counter = " << counter << endl;
					break;
					
				case 2:  // KEY2 - Shift right
					counter = counter >> 1;  // Shift right by 1
					cout << "KEY2 pressed - Shift right: counter = " << counter << endl;
					break;
					
				case 3:  // KEY3 - Shift left
					counter = counter << 1;
					if (counter > 1023) counter = counter & 0x3FF;  // Keep in 10-bit range
					cout << "KEY3 pressed - Shift left: counter = " << counter << endl;
					break;
					
				case 4:  // Multiple buttons - Reset to switches
					counter = ReadAllSwitches(pBase);
					cout << "Multiple buttons - Reset to switches: counter = " << counter << endl;
					break;
			}
			
			// Update LEDs with new counter value
			WriteAllLeds(pBase, counter);
		}
		
		lastButton = currentButton;
		usleep(50000);  // Small delay for debouncing (50ms)
	}

	// Done 
	Finalize(pBase, fd);
	return 0;
}