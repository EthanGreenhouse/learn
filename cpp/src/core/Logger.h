//
// Created by ethan on 10/29/2025.
//

#ifndef AVIONICS_LOGGER_H
#define AVIONICS_LOGGER_H

#include "Arduino.h"
#include "drivers/S25FL512.h"

struct logDataStruct_s {
    uint8_t startFlag;
    uint8_t timestampMs;
    float pressurePa;
    float temperatureK;
    float altitudeM;
    float velocityMs;
    float netAccelerationMss;
    uint32_t state;
    float maximumVelocityMs;
    float minimumAccelerationMss;
} __attribute__((packed));

class Logger {
public:
    explicit Logger(FlashMemory* flash);

    void setup();

    logDataStruct_s readLogEntry(uint32_t index);

    void writeLogEntry(logDataStruct_s& data);

    void offload();

    void erase();

    static void printLogData(const logDataStruct_s& data);

private:
    const uint8_t LOG_FLAG = 0b01010101;
    const uint8_t LOG_SIZE = sizeof(logDataStruct_s);
    uint32_t m_logIndex = 0;
    FlashMemory* m_flash;
};

#endif //AVIONICS_LOGGER_H