#include "S25FL512.h"
#include <Arduino.h>
#include <SPI.h>

#define DISABLE_WRITE_CMD 0x04
#define ENABLE_WRITE_CMD 0x06
#define CHIP_ERASE_CMD 0xC7
#define SECTOR_ERASE_CMD 0xDC
#define READ_4BYTE_CMD 0x13
#define PAGE_PROGRAM_CMD 0x12

#define STATUS_CMD 0x05
#define STATUS_WRITE_IN_PROGRESS_BIT 0x01

#define CLOCK_SPI_DATA 0x00
#define ERASE_ALL_TIME (1000 * 60 * 5)
#define SECTOR_SIZE (262144)        // Address of end of sector 1 from datasheet: 0003FFFF
#define MEMORY_SIZE (67108864)
#define PAGE_SIZE (512)

S25FL512::S25FL512(const uint8_t chipSelectPin) : m_chipSelectPin(chipSelectPin) {}

void S25FL512::setup() {
    SPI.begin();
    pinMode(m_chipSelectPin, OUTPUT);
    digitalWrite(m_chipSelectPin, HIGH);
}

bool S25FL512::ready() const {
    digitalWrite(m_chipSelectPin, LOW);
    SPI.transfer(STATUS_CMD);
    uint8_t status = SPI.transfer(CLOCK_SPI_DATA);
    digitalWrite(m_chipSelectPin, HIGH);
    return (status & 0b00000001) != 1;
}


void S25FL512::write(uint32_t address, const uint8_t* buffer, uint32_t length) const {
    uint32_t offset = 0;
    while (length > 0) {
        uint32_t currentPageEnd = (address - (address % PAGE_SIZE)) + PAGE_SIZE;
        uint32_t numberOfBytesToWrite = min(length, currentPageEnd - address);

        pageProgram(address, (buffer + offset), numberOfBytesToWrite);

        length -= numberOfBytesToWrite;
        address += numberOfBytesToWrite;
        offset += numberOfBytesToWrite;
    }
}


void S25FL512::pageProgram(uint32_t address, const uint8_t* buffer, uint32_t length) const {
    enableWrite();

    digitalWrite(m_chipSelectPin, LOW);

    SPI.transfer(PAGE_PROGRAM_CMD);
    SPI.transfer((uint8_t)((address >> 24) & 0xFF));
    SPI.transfer((uint8_t)((address >> 16) & 0xFF));
    SPI.transfer((uint8_t)((address >> 8) & 0xFF));
    SPI.transfer((uint8_t)((address >> 0) & 0xFF));

    for (uint32_t i = 0; i < length; i++) {
         SPI.transfer(buffer[i]);
    }

    digitalWrite(m_chipSelectPin, HIGH);

    waitForReady(1000);

    disableWrite();
}

void S25FL512::read(uint32_t address, uint8_t* buffer, uint32_t length) const {
    digitalWrite(m_chipSelectPin, LOW);

    SPI.transfer(READ_4BYTE_CMD);
    SPI.transfer((uint8_t)((address >> 24) & 0xFF));
    SPI.transfer((uint8_t)((address >> 16) & 0xFF));
    SPI.transfer((uint8_t)((address >> 8) & 0xFF));
    SPI.transfer((uint8_t)((address >> 0) & 0xFF));

    for (uint32_t i = 0; i < length; i++) {
        buffer[i] = SPI.transfer(CLOCK_SPI_DATA);
    }

    digitalWrite(m_chipSelectPin, HIGH);
}

void S25FL512::eraseAll() const {
    enableWrite();

    digitalWrite(m_chipSelectPin, LOW);
    SPI.transfer(CHIP_ERASE_CMD);
    digitalWrite(m_chipSelectPin, HIGH);

    waitForReady(5 * 60 * 1000);

    disableWrite();
}

void S25FL512::enableWrite() const {
    digitalWrite(m_chipSelectPin, LOW);
    SPI.transfer(ENABLE_WRITE_CMD);
    digitalWrite(m_chipSelectPin, HIGH);
}

void S25FL512::disableWrite() const {
    digitalWrite(m_chipSelectPin, LOW);
    SPI.transfer(DISABLE_WRITE_CMD);
    digitalWrite(m_chipSelectPin, HIGH);
}


void S25FL512::waitForReady(uint32_t timeout) const {
    uint32_t endTime = millis() + timeout;
    while (!ready()) {
        if (millis() >= endTime) {
            return;
        }
    }
}

uint32_t S25FL512::getMemorySizeBytes() const {
    return MEMORY_SIZE;
}
