#ifndef PLATFORMIO_NULI_AVIONICS_FLIGHT_SOFTWARE_ICM20602_H
#define PLATFORMIO_NULI_AVIONICS_FLIGHT_SOFTWARE_ICM20602_H

#include <Arduino.h>

struct Vector3D_s {
    float x; ///< X axis
    float y; ///< Y axis
    float z; ///< Z axis
};

/**
 * @class ICM20602
 * @brief Driver for ICM20602
 * @details I2C only
 */
class ICM20602 {
public:
    /**
     * @brief Initializes the sensor
     * @details Starts communication, and configures parameters
     */
    void setup();

    /**
     * @brief Reads data from the sensor
     * @details This uses injector classes, meaning that the data is read in, but then passed into "dummy" classes.
     * These "dummy" classes are what are actually used by the rest of the code to access data.
     */
    void read();


    /**
     * @brief Gets the angular velocities
     * @return Angular velocities in Rad/s
     */
    Vector3D_s getVelocityRadS() const {
        return m_velocitiesRadS;
    }

    /**
     * @brief Gets the temperature
     * @return Temperature in K
     */
    float getTemperatureK() const {
        return m_temperatureK;
    }

    /**
     * @brief Injects sensor data directly
     * @details If a sensor can't be directly read from, you can inject data directly to the class
     * @param accelerationsMSS Accelerations in m/s^2
     * @param velocitiesRadS Angular velocity's in rad/s
     * @param temperatureK Temperature in kelvin
     */
    void inject(const Vector3D_s& accelerationsMSS, const Vector3D_s& velocitiesRadS, const float temperatureK) {
        m_accelerationsMSS = accelerationsMSS;
        m_temperatureK = temperatureK;
        m_velocitiesRadS = velocitiesRadS;
    }

    /**
     * @brief Gets the Accelerations
     * @return Accelerations in m/s^2
     */
    Vector3D_s getAccelerationsMSS() const {
        return m_accelerationsMSS;
    }

private:
    void readAccelAndGyroBatch();

    Vector3D_s m_velocitiesRadS = {}; ///< Sensor data vector
    Vector3D_s m_accelerationsMSS = {}; ///< Sensor data vector
    float m_temperatureK = 0; ///< Sensor temperature

    // For accelerometer
    int16_t accelData[3] = {};
    int16_t gyroData[3] = {};
    int16_t tempRaw = {};
};


#endif //PLATFORMIO_NULI_AVIONICS_FLIGHT_SOFTWARE_ICM20602_H
