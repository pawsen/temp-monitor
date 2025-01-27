#include "tempReader.h"

/* This line creates a single, global instance of the ThermocoupleReader class
 * that can be used throughout the program. By defining the global instance in
 * the .cpp file and declaring it as extern in the header file, the global
 * instance is accessible from other parts of the program while maintaining
 * proper encapsulation and avoiding multiple definitions.
 */
ThermocoupleReader thermocoupleReader;

void ThermocoupleReader::init(uint8_t numThermocouples, const uint8_t *csPins,
                              uint8_t miso, uint8_t clock) {

  // uint8_t NUM_THERMOCOUPLES = sizeof(csPins) / sizeof(csPins[0]);
  nT = numThermocouples;
  // Allocate memory for thermocouples and temperatures
  thermocouples = new MAX6675 *[numThermocouples];
  temperatures = new double[numThermocouples];

  // Initialize each thermocouple with the appropriate CS pin and shared
  // MISO/CLOCK pins
  SPI.begin();
  for (uint8_t i = 0; i < numThermocouples; i++) {
    // Note: When using multiple SPI devices, you should disable all by
    // setting CS high for all, before initializing any device. Libraries
    // for SPI devices can not detect other devices that may interfere.
    pinMode(csPins[i], OUTPUT);   // Set CS pin as OUTPUT
    digitalWrite(csPins[i], HIGH); // Set CS pin HIGH (inactive state)

    // selectPin, dataPin, clockPin
    thermocouples[i] = new MAX6675(csPins[i], miso, clock);
    thermocouples[i]->begin();
    // ThermoCouples[i]->setSPIspeed(4000000);
  }
}

double *ThermocoupleReader::getTemperatures() {

  for (uint8_t i = 0; i < nT; i++) {
    // Debug output for each thermocouple
    Serial.print("Thermocouple ");
    Serial.print(i);
    Serial.print(": ");

    int status = thermocouples[i]->read();
    if (status != STATUS_OK) {Serial.println("ERROR!");}
    temperatures[i] = thermocouples[i]->getTemperature();

    Serial.println(temperatures[i]);
  }
  return temperatures;
}

ThermocoupleReader::~ThermocoupleReader() {
  // Clean up dynamically allocated memory
  for (uint8_t i = 0; i < nT; i++) {
    delete thermocouples[i];
  }
  delete[] thermocouples;
  delete[] temperatures;
}
