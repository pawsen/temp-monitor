#include "heaterControl.h"

// Constructor
HeaterControl::HeaterControl(uint8_t heaterPin)
    : heaterPin(heaterPin), heaterStatus(false),
      autoDisableTime(12UL * 60UL * 60UL * 1000UL), // Default to 12 hours
      lastEnabledTime(0), lastToggleTime(0), targetTemperature(0.0),
      currentTemperature(0.0), hysteresis(2.0), // Hysteresis band of ±2°C
      toggleDelay(5000) // Minimum delay of 5 seconds between toggles
{}

// Initialize the heater control
void HeaterControl::init() {
  // Ensure the heater is off initially
  heaterStatus = false;
  pinMode(heaterPin, OUTPUT);
  digitalWrite(heaterPin, LOW);
}

// Update heater state based on current temperature
void HeaterControl::update(double _currentTemperature) {
  currentTemperature = _currentTemperature;

  // Check if the heater is enabled
  if (!heaterStatus) {
    return;
  }

  // Check if the minimum toggle delay has passed
  if (millis() - lastToggleTime < toggleDelay) {
    return; // Skip updating if the delay hasn't passed
  }

  // Auto-disable the heater after the timeout period
  if (heaterStatus && millis() - lastEnabledTime >= autoDisableTime) {
    Serial.println(F("auto disable"));
    disable();
  }

  // Simple on/off control with hysteresis
  if (currentTemperature < (targetTemperature - hysteresis)) {
    // Turn the heater on if the temperature is below the lower threshold
    digitalWrite(heaterPin, HIGH);
    lastToggleTime = millis(); // Update the last toggle time
  } else if (currentTemperature > (targetTemperature + hysteresis)) {
    // Turn the heater off if the temperature is above the upper threshold
    digitalWrite(heaterPin, LOW);
    lastToggleTime = millis(); // Update the last toggle time
  }
}

// Enable the heater and reset the auto-disable timer
void HeaterControl::enable() {
  Serial.println(F("heater is on"));
  heaterStatus = true;
  lastEnabledTime = millis();
  digitalWrite(heaterPin, HIGH);
}

// Disable the heater
void HeaterControl::disable() {
  Serial.println(F("heater is off"));
  // save the elapsed time so we can contiue from this time if heating is enabled again.
  autoDisableTime -= millis() - lastEnabledTime;
  lastEnabledTime = millis();
  heaterStatus = false;
  digitalWrite(heaterPin, LOW);
}

// Toggle the heater state
void HeaterControl::toggleHeater() {
  Serial.println(F("Toggling heaterStatus"));
  if (heaterStatus) {
    disable();
  } else {
    enable();
  }
}

// Return heater status
bool HeaterControl::getHeaterStatus() { return heaterStatus; }

// Calculate the time left until auto-disable
uint32_t HeaterControl::getTimeUntilDisable() {
  if (!heaterStatus)
    return 0;
  uint32_t elapsed = millis() - lastEnabledTime;
  return (elapsed < autoDisableTime) ? (autoDisableTime - elapsed) : 0;
}

// Set the time until auto-disable
void HeaterControl::setTimeUntilDisable(uint32_t time) {
  autoDisableTime = millis() + time;
}

// Set a new target temperature
void HeaterControl::setTargetTemperature(double targetTemp) {
  targetTemperature = targetTemp;
}

// Get the current target temperature
double HeaterControl::getTargetTemperature() { return targetTemperature; }

// Get the current temperature
double HeaterControl::getCurrentTemperature() { return currentTemperature; }
