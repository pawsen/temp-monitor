#ifndef HEATERCONTROL_H
#define HEATERCONTROL_H

#include <Arduino.h>

class HeaterControl {
public:
    HeaterControl(uint8_t heaterPin); // Constructor accepting the heater pin
    void init();
    void update(float currentTemperature);
    void enable();
    void disable();
    void toggleHeater();
    bool getHeaterStatus();
    uint32_t getTimeUntilDisable();
    void setTimeUntilDisable(uint32_t time);
    void setTargetTemperature(float targetTemp);
    float getTargetTemperature();
    float getCurrentTemperature();

private:
    uint8_t heaterPin;
    bool heaterStatus;
    uint32_t autoDisableTime;
    uint32_t lastEnabledTime;
    uint32_t lastToggleTime; // Track the last time the heater was toggled
    float targetTemperature;
    float currentTemperature;
    // Hysteresis band to prevent rapid toggling
    // The heater turns on when the temperature is below (targetTemperature - hysteresis).
    // The heater turns off when the temperature is above (targetTemperature + hysteresis).
    float hysteresis;
    uint32_t toggleDelay; // Minimum delay between toggles (in milliseconds)
};

#endif
