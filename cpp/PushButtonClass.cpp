#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <iostream>
using namespace std;

// Physical base address of FPGA Devices 
const unsigned int LW_BRIDGE_BASE = 0xFF200000;  // Base offset 

// Length of memory-mapped IO window 
const unsigned int LW_BRIDGE_SPAN = 0x00005000;  // Address map size

// Cyclone V FPGA device addresses
const unsigned int LEDR_BASE = 0x00000000;  // Leds offset 
const unsigned int SW_BASE = 0x00000040;  // Switches offset
const unsigned int KEY_BASE = 0x00000050;  // Push buttons offset

class DE1SoCfpga {
private:
    char *pBase;  // Base pointer for virtual memory
    int fd;       // File descriptor

public:
    // Constructor
    DE1SoCfpga() {
        // Open /dev/mem
        fd = open("/dev/mem", (O_RDWR | O_SYNC));
        
        // Map physical addresses to virtual addresses
        pBase = (char *)mmap(NULL, LW_BRIDGE_SPAN, (PROT_READ | PROT_WRITE), 
                            MAP_SHARED, fd, LW_BRIDGE_BASE);
    }
    
    // Destructor 
    ~DE1SoCfpga() {
        close(fd);
    }
    
    // Write to a register
    void RegisterWrite(unsigned int offset, int value) {
        *(volatile unsigned int *)(pBase + offset) = value;
    }
    
    // Read from a register
    int RegisterRead(unsigned int offset) {
        return *(volatile unsigned int *)(pBase + offset);
    }
};

class LEDControl {
private:
    DE1SoCfpga *fpga;  // Pointer to FPGA object

public:
    // Constructor
    LEDControl(DE1SoCfpga *fpgaObj) {
        fpga = fpgaObj;
    }
    
    // Write value to all LEDs
    void WriteAllLeds(int value) {
        fpga->RegisterWrite(LEDR_BASE, value);
    }
    
    // Control single LED
    void Write1Led(int ledNum, int state) {
        int current = fpga->RegisterRead(LEDR_BASE);
        
        if (state == 1) {
            current = current | (1 << ledNum);  // Turn ON
        } else {
            current = current & ~(1 << ledNum);  // Turn OFF
        }
        
        fpga->RegisterWrite(LEDR_BASE, current);
    }
    
    // Read single switch
    int Read1Switch(int switchNum) {
        int switches = fpga->RegisterRead(SW_BASE);
        return (switches >> switchNum) & 1;
    }
    
    // Read all switches
    int ReadAllSwitches() {
        return fpga->RegisterRead(SW_BASE) & 0x3FF;  // 10 bits only
    }
    
    // Get push button state
    int PushButtonGet() {
        int buttons = fpga->RegisterRead(KEY_BASE) & 0xF;
        
        // No button pressed
        if (buttons == 0) {
            return -1;
        }
        
        // Check for multiple buttons
        int count = 0;
        for (int i = 0; i < 4; i++) {
            if (buttons & (1 << i)) {
                count++;
            }
        }
        
        // Multiple buttons pressed
        if (count > 1) {
            return 4;
        }
        
        // Single button - find which one
        for (int i = 0; i < 4; i++) {
            if (buttons & (1 << i)) {
                return i;
            }
        }
        
        return -1;
    }
};

int main() {
    // Create objects
    DE1SoCfpga fpga;
    LEDControl leds(&fpga);
    
    cout << "Push Button Counter Program" << endl;
    cout << "KEY0: Increment" << endl;
    cout << "KEY1: Decrement" << endl;
    cout << "KEY2: Shift right" << endl;
    cout << "KEY3: Shift left" << endl;
    cout << "Multiple buttons: Reset" << endl;
    
    // Initialize counter from switches
    int counter = leds.ReadAllSwitches();
    leds.WriteAllLeds(counter);
    
    int lastButton = -1;
    
    // Main loop
    while (true) {
        int button = leds.PushButtonGet();
        
        // Process button press (with debouncing)
        if (button != lastButton && button != -1) {
            
            if (button == 0) {  // KEY0 - Increment
                counter++;
                if (counter > 1023) counter = 0;
            }
            else if (button == 1) {  // KEY1 - Decrement
                counter--;
                if (counter < 0) counter = 1023;
            }
            else if (button == 2) {  // KEY2 - Shift right
                counter = counter >> 1;
            }
            else if (button == 3) {  // KEY3 - Shift left
                counter = counter << 1;
                if (counter > 1023) counter = counter & 0x3FF;
            }
            else if (button == 4) {  // Multiple buttons - Reset
                counter = leds.ReadAllSwitches();
            }
            
            // Update LEDs
            leds.WriteAllLeds(counter);
        }
        
        lastButton = button;
        usleep(50000);  // 50ms delay
    }
    
    return 0;
}