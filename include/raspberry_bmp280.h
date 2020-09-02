#ifndef RASPBERRY_BMP280_RASPBERRY_BMP280_H
#define RASPBERRY_BMP280_RASPBERRY_BMP280_H

#include <bmp280_defs.h>
#include <cstdint>

#define BMP280_E_I2C_OPEN_FAIL (-20)
#define BMP280_E_I2C_ADDR_SET_FAIL (-21)

class BMP280
{
public:
    enum class CSBState {
        High,
        Low
    };

    BMP280();
    ~BMP280();

    int8_t init(CSBState csb,
             uint8_t filter = BMP280_FILTER_COEFF_2,
             uint8_t osPres = BMP280_OS_4X,
             uint8_t osTemp = BMP280_OS_4X,
             uint8_t odr = BMP280_ODR_250_MS,
             const char *i2cPath = "/dev/i2c-1");

    int8_t read();
    double getPressure() const;
    double getTemperature() const;
    double getAltitude(double p0) const;

private:
    static BMP280 *_sensors[];
    static int _nSensors;

    bmp280_dev _dev{};
    int _i2cfd = -1;

    double _temp = 0.;
    double _press = 0.;

    static int8_t i2c_reg_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);
    static int8_t i2c_reg_read(uint8_t sensorNum, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);

public:
    static void delay_ms(uint32_t period_ms);
};

#endif //RASPBERRY_BMP280_RASPBERRY_BMP280_H
