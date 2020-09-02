#include <cstdio>
#include <cmath>
#include "raspberry_bmp280.h"

using namespace std;

static double p0 = -1;

static double getAlt(double p, double t)
{
    double R = pow(p0 / p, 1. / 5.257);
    return (R - 1.) * (t + 273.15) * 2000. / 13.;
}

int main()
{
    printf("Basic example\n\r");

    BMP280 bmp;

    if (bmp.init(BMP280::CSBState::High, BMP280_FILTER_COEFF_16, BMP280_OS_16X, BMP280_OS_2X, BMP280_ODR_62_5_MS) != 0)
        return -1;

    int i = 0;
    while (bmp.read() == 0)
    {
        double pressure = bmp.getPressure();
        double temp = bmp.getTemperature();

        if (i < 100)
        {
            printf("\rCalibrating... %d%%", i + 1);
            fflush(stdout);
        }

        if (i > 50 && i < 100)
        {
            if (p0 < 0)
                p0 = pressure;
            else
                p0 += 0.2 * (pressure - p0);
        }
        else if (i == 100)
        {
            printf("\n\rCalibration done. p0 = %.1f\n\r", p0);
        }
        else if (i > 100)
        {
            printf("%d: P = %.1f Pa\tT = %.1f °C\tAlt = %.2f m\n\r", i, pressure, temp, getAlt(pressure, temp));
        }

        i++;
        BMP280::delay_ms(63);
    }

    return 0;
}