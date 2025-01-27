#ifndef HEATERCONTROL_H
#define HEATERCONTROL_H

#include <PID_v1.h>
// Include Arduino.h for uint8_t. Alternative is to do a typedef
// typedef unsigned char uint8_t;
#include <Arduino.h>


class HeaterControl {
public:
    HeaterControl(uint8_t heaterPin); // Constructor accepting the heater pin
    void init();
    void update(double currentTemperature);
    void enable();
    void disable();
    void toggleHeater();
    bool getHeaterStatus();
    uint32_t getTimeUntilDisable();
    void setTimeUntilDisable(uint32_t time);
    void setTargetTemperature(double targetTemp);
    double getTargetTemperature();
    double getCurrentTemperature();


private:
    uint8_t heaterPin;
    bool heaterStatus;
    uint32_t autoDisableTime;
    uint32_t lastEnabledTime;
    double targetTemperature;
    double currentTemperature;

    // PID variables
    double pidInput;     // Current temperature from thermocouple
    double pidOutput;    // PID output (used to control heater)
    double pidSetpoint;  // Target temperature
    PID pid;             // PID object
};

#endif
