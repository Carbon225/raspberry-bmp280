#include "raspberry_bmp280.h"
#include <bmp280.h>
#include <ctime>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cstring>
#include <cmath>

#define MAX_BMP_NUM (256)

BMP280 *BMP280::_sensors[MAX_BMP_NUM];
int BMP280::_nSensors = 0;

BMP280::BMP280()
{
    _dev.dev_id = _nSensors++;
    _sensors[_dev.dev_id] = this;

    _dev.delay_ms = &BMP280::delay_ms;
    _dev.intf = BMP280_I2C_INTF;
    _dev.read = &BMP280::i2c_reg_read;
    _dev.write = &BMP280::i2c_reg_write;
}

BMP280::~BMP280()
{
    _sensors[_dev.dev_id] = _sensors[_nSensors - 1];
    _sensors[_dev.dev_id]->_dev.dev_id = _dev.dev_id;
    _nSensors--;

    if (_i2cfd >= 0)
        close(_i2cfd);
}

int8_t BMP280::init(CSBState csb, uint8_t filter, uint8_t osPres, uint8_t osTemp, uint8_t odr, const char *i2cPath)
{
    int addr = csb == CSBState::Low ? BMP280_I2C_ADDR_PRIM : BMP280_I2C_ADDR_SEC;

    _i2cfd = open(i2cPath, O_RDWR);
    if (_i2cfd < 0)
        return BMP280_E_I2C_OPEN_FAIL;

    if (ioctl(_i2cfd, I2C_SLAVE, addr) < 0)
        return BMP280_E_I2C_ADDR_SET_FAIL;

    int8_t ret = bmp280_init(&_dev);

    if (ret != 0)
        return ret;

    bmp280_config conf{};
    ret = bmp280_get_config(&conf, &_dev);

    if (ret != 0)
        return ret;

    conf.filter = filter;
    conf.os_pres = osPres;
    conf.os_temp = osTemp;
    conf.odr = odr;

    ret = bmp280_set_config(&conf, &_dev);

    if (ret != 0)
        return ret;

    ret = bmp280_set_power_mode(BMP280_NORMAL_MODE, &_dev);

    return ret;

}

int8_t BMP280::read()
{
    bmp280_uncomp_data ucomp_data{};

    int8_t ret = bmp280_get_uncomp_data(&ucomp_data, &_dev);
    if (ret != 0)
        return ret;

    ret = bmp280_get_comp_temp_double(&_temp, ucomp_data.uncomp_temp, &_dev);
    if (ret != 0)
        return ret;

    ret = bmp280_get_comp_pres_double(&_press, ucomp_data.uncomp_press, &_dev);
    if (ret != 0)
        return ret;

    return ret;
}

double BMP280::getPressure() const
{
    return _press;
}

double BMP280::getTemperature() const
{
    return _temp;
}

double BMP280::getAltitude(double p0) const
{
    double R = pow(p0 / _press, 1. / 5.257);
    return (R - 1.) * (_temp + 273.15) * 2000. / 13.;
}

void BMP280::delay_ms(uint32_t period_ms)
{
    long msec = period_ms;

    struct timespec ts{};
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    int res;
    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);
}

int8_t BMP280::i2c_reg_write(uint8_t sensorNum, uint8_t reg_addr, uint8_t *reg_data, uint16_t length)
{
    BMP280 *bmp = _sensors[sensorNum];
    if (!bmp)
        return 1;

    uint8_t buf[length + 1];
    buf[0] = reg_addr;
    memcpy(buf + 1, reg_data, length);

    int n = ::write(bmp->_i2cfd, buf, length + 1);

    if (n != length + 1)
        return 1;

    return 0;
}

int8_t BMP280::i2c_reg_read(uint8_t sensorNum, uint8_t reg_addr, uint8_t *reg_data, uint16_t length)
{
    BMP280 *bmp = _sensors[sensorNum];
    if (!bmp)
        return 1;

    if (::write(bmp->_i2cfd, &reg_addr, 1) != 1)
        return 1;

    if (::read(bmp->_i2cfd, reg_data, length) != length)
        return 1;

    return 0;
}
