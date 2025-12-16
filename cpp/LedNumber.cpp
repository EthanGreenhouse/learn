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

void WriteAllLeds(char *pBase, int value)
{
    RegisterWrite(pBase, LEDR_BASE, value);
}

/*
 * Read the value of a single switch
 * - Uses base address of I/O
 * @param pBase Base address returned by 'mmap'
 * @param switchNum Switch number (0 to 9)
 * @return Switch value read (0 or 1)
 */
int Read1Switch(char *pBase, int switchNum) {
   // Read all switches from the switch register
   int allSwitches = RegisterRead(pBase, SW_BASE);
   // Create a mask to isolate the specific switch bit
   int mask = 1 << switchNum;
   // Apply mask and shift to get 0 or 1
   int switchValue = (allSwitches & mask) >> switchNum;
   return switchValue;
}
 
/*
 * Changes the state of an LED (ON or OFF)
 * @param pBase     Base address returned by 'mmap'
 * @param ledNum    LED number (0 to 9)
 * @param state     State to change to (0=OFF, 1=ON)
 */
void Write1Led(char *pBase, int ledNum, int state) {
   // Read current LED states to preserve other LEDs
   int currentLeds = RegisterRead(pBase, LEDR_BASE);
   // Create mask for the specific LED
   int mask = 1 << ledNum;
   if (state == 1) { 
     // Turn ON the LED using OR operation
     currentLeds = currentLeds | mask;
   } else { 
     // Turn OFF the LED using AND with inverted mask
     currentLeds = currentLeds & ~mask;
   }
   // Write the updated LED states back
   RegisterWrite(pBase, LEDR_BASE, currentLeds);
}

/*
 * Reads all the switches and returns their value in a single integer.
 * @param pBase Base address for general-purpose I/O
 * @return A value that represents the value of the switches (0-1023)
 */
int ReadAllSwitches(char *pBase) {
    // Read the entire switch register which contains all 10 switches
    // The lower 10 bits represent switches 0-9
    int allSwitches = RegisterRead(pBase, SW_BASE);
    // Mask to keep only the lower 10 bits (0x3FF = 1111111111 in binary)
    allSwitches = allSwitches & 0x3FF;
    return allSwitches;
}

/*
 * Main Function - Tests Lab 6.1 and 6.2 functions
 */
int main() 
{ 
	// Initialize 
	int fd; 
	char *pBase = Initialize(&fd); 

	cout << "=== LAB 6.1 TESTS ===" << endl;
	
	// Test WriteAllLeds 
	int value = 0; 
	cout << "Enter an int value between 0 to 1023: "; 
	cin >> value; 
	cout << "Value to be written to LEDs = " << value << endl; 
	WriteAllLeds(pBase, value);
	int readLEDs = RegisterRead(pBase, LEDR_BASE);
	cout << "Value of LEDS read = " << readLEDs << endl;
  
	// Test Write1Led
	cout << "\nTesting Write1Led" << endl;
	cout << "Enter LED number (0-9): ";
	int ledNum;
	cin >> ledNum;
  
	cout << "Enter state (0 = OFF, 1 = ON): ";
	int state;
	cin >> state; 
  
	Write1Led(pBase, ledNum, state);
	cout << "LED " << ledNum << " has been turned ";
	if (state == 1) {
		cout << "ON" << endl;
	} else {
		cout << "OFF" << endl;
	}
  
	// Test Read1Switch
	cout << "\nTesting Read1Switch" << endl;
	cout << "Enter switch number to read (0-9): ";
	int switchNum;
	cin >> switchNum;
   
	int switchValue = Read1Switch(pBase, switchNum);
	cout << "Switch " << switchNum << " is: " << switchValue << endl;
	if (switchValue == 1) {
		cout << "The switch is ON" << endl;
	} else {
		cout << "The switch is OFF" << endl;
	}
	
	// LAB 6.2 TEST - ReadAllSwitches
	cout << "\n=== LAB 6.2 TEST ===" << endl;
	cout << "Testing ReadAllSwitches" << endl;
	cout << "Set your switches to a pattern, then press Enter...";
	cin.ignore();
	cin.get();
	
	int switchesValue = ReadAllSwitches(pBase);
	cout << "All switches value (decimal): " << switchesValue << endl;
	cout << "Writing this value to LEDs..." << endl;
	WriteAllLeds(pBase, switchesValue);
	
	// Test continuous reading
	cout << "\nContinuous test: Switches will control LEDs for 10 seconds" << endl;
	cout << "Flip switches to see LEDs change!" << endl;
	for (int i = 0; i < 20; i++) {
		switchesValue = ReadAllSwitches(pBase);
		WriteAllLeds(pBase, switchesValue);
		usleep(500000);  // Wait 0.5 seconds
	}

	cout << "\nAll tests completed!" << endl;

	// Done 
	Finalize(pBase, fd);
	return 0; 
}