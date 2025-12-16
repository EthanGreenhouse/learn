#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <iostream>

using namespace std;

// Physical base address of FPGA Devices
const unsigned int LW_BRIDGE_BASE = 0xFF200000;

// Length of the memory mapped I/O window
const unsigned int LW_BRIDGE_SPAN = 0x00005000;

// PIO address offset
const unsigned int OUT_BASE = 0x00000020;

/*
 * Initialize general-purpose I/O 
 *  - Opens access to physical memory /dev/mem 
 *  - Maps memory into virtual address space 
 * 
 * @param fd	File descriptor passed by reference
 * @return	Address to virtual memory which is mapped to physical
 */ 
char *Initialize(int *fd) 
{
	// Open /dev/mem to give access to physical addresses
	*fd = open("/dev/mem", (O_RDWR | O_SYNC));
	if (*fd == -1)
	{
		cout << "ERROR: could not open /dev/mem..." << endl;
		exit(1);
	}
	
	// Get a mapping from physical addresses to virtual addresses
	char *virtual_base = (char *)mmap(NULL, LW_BRIDGE_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, *fd, LW_BRIDGE_BASE);
	if (virtual_base == MAP_FAILED)
	{
		cout << "ERROR: mmap() failed..." << endl;
		close(*fd);
		exit(1);
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
	if (munmap(pBase, LW_BRIDGE_SPAN) != 0)
	{
		cout << "ERROR: munmap() failed..." << endl;
		exit(1);
	}
	close(fd);
}

/*
 * Write a 4-byte value at the specified general-purpose I/O location. 
 * 
 * @param pBase		Base address returned by 'mmap'. 
 * @param reg_offset	Offset where device is mapped. 
 * @param value		Value to be written. 
 */ 
void RegisterWrite(char *pBase, unsigned int reg_offset, int value) 
{ 
	*(volatile unsigned int *)(pBase + reg_offset) = value; 
}

/*
 * Main Function - Control RC Servo
 */
int main() 
{
	// Initialize
	int fd;
	char *pBase = Initialize(&fd);
	
	// Variables
	int angle;
	
	// Main loop
	while (true)
	{
		// Ask user to enter an angle from 0 to 180
		cout << "Enter an angle from 0 to 180 (or -1 to exit): ";
		cin >> angle;
		
		// The program should exit if the user enters -1
		if (angle == -1)
		{
			break;
		}
		
		// Check valid range
		if (angle < 0 || angle > 180)
		{
			cout << "Invalid angle. Please enter 0-180." << endl;
			continue;
		}
		
		// Write the commanded angle to the FPGA RC Servo circuit
		RegisterWrite(pBase, OUT_BASE, angle);
		cout << "Servo moved to " << angle << " degrees" << endl;
	}
	
	// Close the memory mapped interface
	Finalize(pBase, fd);
	cout << "Program exited" << endl;
	
	return 0;
}