#include <Arduino.h>
#include "drivers/MS5607.h"
#include "drivers/ICM20602.h"
#include "drivers/S25FL512.h"
#include "core/Logger.h"

MS5607 barometer;
ICM20602 imu;
S25FL512 flash(12);
Logger logger(&flash);
logDataStruct_s logData;
uint32_t tickEndTime = 0;

#define LIGHT_PIN (1)

void runCLI();

void setup() {
    Serial.begin(9600);
    while (!Serial) {}
    Serial.println("Starting setup");

    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);

    pinMode(LIGHT_PIN, OUTPUT);
    barometer.setup();
    imu.setup();
    flash.setup();
    flash.waitForReady(60 * 1000);

    logger.setup();

    Serial.println("Setup done");
}

void loop() {
    while (millis() < tickEndTime) {}
    tickEndTime = millis() + 20;

    barometer.read();
    imu.read();

    const float pressurePa = barometer.getPressurePa();
    const float temperatureK = barometer.getTemperatureK();
    const float altitudeM = MS5607::calculateAltitudeM(pressurePa);
    const Vector3D_s accelerationMSS = imu.getAccelerationsMSS();
    const Vector3D_s angularVelocityRadS =  imu.getVelocityRadS();
    const float netAccelerationMss = sqrt(accelerationMSS.x * accelerationMSS.x +
                                            accelerationMSS.y * accelerationMSS.y +
                                            accelerationMSS.z * accelerationMSS.z);

    logData.timestampMs = millis();
    logData.pressurePa = pressurePa;
    logData.temperatureK = temperatureK;
    logData.altitudeM = altitudeM;
    logData.velocityMs = 0;
    logData.netAccelerationMss = 0;
    logData.state = 0;
    logData.maximumVelocityMs = 0;
    logData.minimumAccelerationMss = 0;

    runCLI();

    if ((millis() / 1000) % 2 == 0) {
        digitalWrite(LIGHT_PIN, HIGH);
    } else {
        digitalWrite(LIGHT_PIN, LOW);
    }
}

void runCLI() {
    if (Serial.available() > 0) {
        char c = Serial.read();
        if (c == 'e') {
            logger.erase();
        } else if (c == 'o') {
            logger.offload();
        } else if (c == 'p') {
            Logger::printLogData(logData);
        } else {
            Serial.println("Unrecognized command");
        }
        while (Serial.available() > 0) {
            Serial.read();
        }
    }
}