#include "heaterControl.h"

// PID Tuning Parameters
#define KP 2.0
#define KI 5.0
#define KD 1.0

/* The init() method is used to handle initialization that cannot/should not be
** done in the constructor. This allows to separate object creation and hardware
** initialization.
*/

// Constructor
HeaterControl::HeaterControl(uint8_t heaterPin)
    : heaterPin(heaterPin),
      heaterStatus(false),
      autoDisableTime(12UL * 60UL * 60UL * 1000UL), // Default to 12 hours
      lastEnabledTime(0),
      targetTemperature(0.0),
      pidInput(0.0),
      pidOutput(0.0),
      pidSetpoint(0.0),
      pid(&pidInput, &pidOutput, &pidSetpoint, KP, KI, KD, DIRECT) {}

// Initialize the heater control
void HeaterControl::init() {
    pinMode(heaterPin, OUTPUT);
    digitalWrite(heaterPin, LOW); // Ensure the heater is off initially

    // Initialize the PID controller
    pid.SetMode(AUTOMATIC);
    pid.SetOutputLimits(0, 255); // Control output range (e.g., PWM 0-255)
}

// Update heater state based on current temperature and PID logic
void HeaterControl::update(double currentTemperature) {
    pidInput = currentTemperature;
    pidSetpoint = targetTemperature;

    // Compute the PID output
    pid.Compute();

    // Heater control logic (simple on/off using PID output)
    if (pidOutput > 0) {
        digitalWrite(heaterPin, HIGH);
        heaterStatus = true;
    } else {
        digitalWrite(heaterPin, LOW);
        heaterStatus = false;
    }

    // Auto-disable the heater after the timeout period
    if (heaterStatus && millis() - lastEnabledTime >= autoDisableTime) {
        disable();
    }
}

// Enable the heater and reset the auto-disable timer
void HeaterControl::enable() {
    heaterStatus = true;
    lastEnabledTime = millis();
    digitalWrite(heaterPin, HIGH);
}

// Disable the heater
void HeaterControl::disable() {
    heaterStatus = false;
    digitalWrite(heaterPin, LOW);
}

void HeaterControl::toggleHeater(){
  if (heaterStatus)
    disable();
  else
    enable();
  Serial.println("Toggling heaterStatus");
}

// Return heater status
bool HeaterControl::getHeaterStatus() {
    return heaterStatus;
}

// Calculate the time left until auto-disable
uint32_t HeaterControl::getTimeUntilDisable() {
    if (!heaterStatus) return 0;
    uint32_t elapsed = millis() - lastEnabledTime;
    return (elapsed < autoDisableTime) ? (autoDisableTime - elapsed) : 0;
}

// Set a new target temperature
void HeaterControl::setTargetTemperature(double targetTemp) {
    targetTemperature = targetTemp;
    pidSetpoint = targetTemp; // Update the PID setpoint
}

// Get the current target temperature
double HeaterControl::getTargetTemperature() {
    return targetTemperature;
}

double HeaterControl::getCurrentTemperature() {
    return currentTemperature;
}
