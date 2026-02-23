//
// Created by ethan on 10/29/2025.
//

#include "logger.h"

#include "drivers/S25FL512.h"

Logger::Logger(FlashMemory* flash) {
    m_flash = flash;
}

void Logger::setup() {
    for (; m_logIndex * LOG_FLAG < m_flash->getMemorySizeBytes() - LOG_SIZE; m_logIndex++) {
        logDataStruct_s logData = readLogEntry(m_logIndex);
        if (logData.startFlag != LOG_FLAG) {
            break;
        }
    }
}

logDataStruct_s Logger::readLogEntry(uint32_t index) {
    logDataStruct_s logData{};
    m_flash->read(index* LOG_SIZE, (uint8_t*) &logData, LOG_SIZE);
    return logData;
}

void Logger::writeLogEntry(logDataStruct_s& data) {
    data.startFlag = LOG_FLAG;
    m_flash->write(m_logIndex * LOG_SIZE, (uint8_t*) &data, LOG_SIZE);
    m_logIndex++;
}

void Logger::offload() {
    Serial.println("offloading");
    for (uint32_t i = 0; i < m_logIndex; i++) {
        logDataStruct_s logData = readLogEntry(i);
        if (logData.startFlag != LOG_FLAG) {
            break;
        }
    }
    Serial.println("Done");
}

void Logger::erase() {
    Serial.println("Erasing log");
    m_flash->eraseAll();
    m_logIndex = 0;
    Serial.println("Done");
}

void Logger::printLogData(const logDataStruct_s& data) {
    Serial.print(data.timestampMs);
    Serial.print("\t");
    Serial.print(data.pressurePa);
    Serial.print("\t");
    Serial.print(data.temperatureK);
    Serial.print("\t");
    Serial.print(data.altitudeM);
    Serial.print("\t");
    Serial.print(data.velocityMs);
    Serial.print("\t");
    Serial.print(data.netAccelerationMss);
    Serial.print("\t");
    Serial.print(data.state);
    Serial.print("\t");
    Serial.print(data.maximumVelocityMs);
    Serial.print("\t");
    Serial.print(data.minimumAccelerationMss);
    Serial.print("\t");
}