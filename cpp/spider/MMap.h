#ifndef MMAP_H_
#define MMAP_H_
// This file contains the base offsets for each PWM_BASE circuit defined as macros
// E.g., PWM0_BASE is defined as 0x110, PWM17_BASE is defined as 0x0
#include "hps_0.h"
#include <sys/mman.h>
#include <stdint.h>
#include <iostream>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>


/**
 * This object represents a memory mapped IO interface for a single device. 
 * It manages the internal state of the memory mapping, and
 * exposes 32-bit register read and write methods to clients.
 */
class MMap {

private:
	//Stores the file descriptor returned by mmap
	int m_fd;
	//Stores the virtual base pointer returned by mmap (translation of physical base addr)
	void *m_virtual_base;
	//Stores the size of the mapped memory region
	uint32_t m_addr_span;

 // Use this array to translate MotorIDs to Byte Offsets into the MMIO region.
	const uint32_t motor_offsets[18] = {
		PWM0_BASE,
		PWM1_BASE,
		PWM2_BASE,
		PWM3_BASE,
		PWM4_BASE,
		PWM5_BASE,
		PWM6_BASE,
		PWM7_BASE,
		PWM8_BASE,
		PWM9_BASE,
		PWM10_BASE,
		PWM11_BASE,
		PWM12_BASE,
		PWM13_BASE,
		PWM14_BASE,
		PWM15_BASE,
		PWM16_BASE,
		PWM17_BASE,
	};
	//creates the memory mapping
	bool map(uint32_t addr_base, uint32_t addr_span);
	//releases the memory mapping
	void unmap();
	/**
	 * Computes the address of the beginning of the memory
	 * region which corresponds to the given motor as a 32-bit integer pointer.
	 * @param motorId - The motor id (between 0 and 17)
	 * @return a pointer to the memory corresponding to the first register of the given motor 
	 */
	uint32_t* getMotorStart(int motorId);

public:
	MMap();
	virtual ~MMap();
	bool isMapped();
	bool Motor_Reg32_Write(uint32_t motorId, uint32_t regOffset, uint32_t value);
	uint32_t Motor_Reg32_Read(uint32_t motorId, uint32_t regOffset);
};

#endif /* MMAP_H_ */
