#ifndef LCD_H
#define LCD_H

#include <Arduino.h>
#include <I2C_LCD.h> // Include the I2C_LCD library

class LCD {
public:
    LCD(uint8_t i2cAddress); // Constructor with I2C address
    void init();
    void displayTemperature(double currentTemperature, double targetTemperature);
    void displayHeaterStatus(bool heaterOn);
    void displayAutoDisableTime(uint32_t timeRemaining);
    void displayMenu(const String &menuItem);
    void clear();

private:
    I2C_LCD lcd; // LCD object
};

#endif
