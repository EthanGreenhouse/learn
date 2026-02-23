#ifndef PLATFORMIO_NULI_AVIONICS_FLIGHT_SOFTWARE_MS5607Sensor_H
#define PLATFORMIO_NULI_AVIONICS_FLIGHT_SOFTWARE_MS5607Sensor_H

#include <Arduino.h>
#include <Wire.h>

/**
 * @class MS5607
 * @brief An implementation of Barometer for the MS8607 barometer.
 */
class MS5607 {
public:
    /**
        * @brief Injects sensor data directly
        * @details If a sensor can't be directly read from, you can inject data directly to the class
        * @param temperatureK Temperature in k
        * @param pressurePa Pressure in pascals
        */
    void inject(const float temperatureK, const float pressurePa) {
        m_temperatureK = temperatureK;
        m_pressurePa = pressurePa;
    }

    /**
     * @brief Gets current temperature of the barometer
     * @return Temperature in k
     */
    float getTemperatureK() const {
        return m_temperatureK;
    }

    /**
     * @brief Gets the current pressure
     * @return Pressure in atmospheres
     */
    float getPressurePa() const {
        return m_pressurePa;
    }

    /**
     * @brief Calculates the altitude from pressure and temperature
     * @details Specific algorithm used??????
     */
    static float calculateAltitudeM(const float pressurePa) {
        //        m_altitudeM = (m_temperatureK / Constants::LAPSE_RATE_K_M) *
        //                      (pow(m_pressurePa / Constants::ATMOSPHERIC_PRESSURE_PA, -Constants::GAS_CONSTANT_J_KG_K * -Constants::LAPSE_RATE_K_M / Constants::G_EARTH_MSS) - 1);
        return (286.0f / MS5607::LAPSE_RATE_K_M) *
                      (pow(pressurePa / MS5607::ATMOSPHERIC_PRESSURE_PA, -MS5607::GAS_CONSTANT_J_KG_K * MS5607::LAPSE_RATE_K_M / MS5607::G_EARTH_MSS) - 1);

    }

    /**
     * @brief Initialize the sensor
     * @details Enabling any peripherals, confirm sensor is talking, set configuration registers on the sensor
     */
    void setup();

    /**
     * @brief Read data from the sensor
     * @details Read in one reading from the sensor, and convert the data to usefully units/numbers.
     * Currently is allowed to block the loop to wait for data from the sensor for a few ms.
     */
    void read();

    /**
     * @brief Sets the over sampling rate (currently hardcoded)
     * @param OSR_U (2048, 4096, etc)
     */
    void setOSR(uint16_t OSR_U);

private:
    static constexpr double G_EARTH_MSS = 9.80665;
    static constexpr double ATMOSPHERIC_PRESSURE_PA = 101325;
    static constexpr double LAPSE_RATE_K_M = -0.0065;
    static constexpr double GAS_CONSTANT_J_KG_K = 287.0474909;
    static constexpr double C_TO_K = 273.15;
    static constexpr double MBAR_TO_PA = 100;

    float m_temperatureK = 0; ///<The measured temperature
    float m_pressurePa = 0; ///< The measured pressure

    uint16_t MS5607_ADDR = 0X77;      // default device address of MS5607 (CBS == HIGH)
    uint16_t OSR = 4096;              // default over sampling ratio
    uint16_t CONV_D1 = 0x48;          // corresponding temp conv. command for OSR
    uint16_t CONV_D2 = 0x58;          // corresponding pressure conv. command for OSR
    uint16_t Conv_Delay = 9040;          // corresponding conv. delay for OSR

    unsigned int C1, C2, C3, C4, C5, C6;
    unsigned long DP, DT;
    float dT, TEMP, P;
    int64_t OFF, SENS;

    bool readDigitalValue();
    float getTemperature();
    float getPressure();
    bool resetDevice() const;
    bool readCalibration();
    bool readUInt_16(char address, unsigned int &value);
    bool readBytes(unsigned char* values, uint8_t length) const;
    bool startConversion(char CMD) const;
    bool startMeasurement() const;
    bool getDigitalValue(unsigned long &value) const;
    void waitForI2C() const;
};

#endif //PLATFORMIO_NULI_AVIONICS_FLIGHT_SOFTWARE_MS5607Sensor_H
