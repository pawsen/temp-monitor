#ifndef TEMP_READER_H
#define TEMP_READER_H

#include <Arduino.h>
#include <MAX6675.h>

class ThermocoupleReader {
private:
// // MAX6675 Thermocouple Pins
// // Chip Select for thermocouples (array)
// const uint8_t MAX6675_CS_PINS[] = {7};
// // const uint8_t NUM_THERMOCOUPLES = 2;  // Number of thermocouples
// const uint8_t NUM_THERMOCOUPLES =
//     sizeof(MAX6675_CS_PINS) / sizeof(MAX6675_CS_PINS[0]);
// const uint8_t SPI_MISO_PIN = 8;
// const uint8_t SPI_SCK_PIN = 9;
    MAX6675** thermocouples;   // Array of pointers to MAX6675 objects
    double* temperatures;      // Array to store temperature readings
    uint8_t nT;                // Number of thermocouples

public:
    void init( uint8_t numThermocouples, const uint8_t* csPins,uint8_t miso, uint8_t clock);
    double* getTemperatures();
    ~ThermocoupleReader();  // Destructor to clean up allocated memory
};

extern ThermocoupleReader thermocoupleReader;

#endif

  // // Initialize the thermocouple reader
  // thermocoupleReader.init(NUM_THERMOCOUPLES, MAX6675_CS_PINS, SPI_MISO_PIN,
  //                         SPI_SCK_PIN);

  // // Test readings
  // double *temperatures = thermocoupleReader.getTemperatures();
  // for (uint8_t i = 0; i < NUM_THERMOCOUPLES; i++) {
  //   Serial.print("Thermocouple ");
  //   Serial.print(i);
  //   Serial.print(" Temperature: ");
  //   Serial.println(temperatures[i]);
  // }

  // Get the current temperatures from thermocouples
  // double *temperatures = thermocoupleReader.getTemperatures();
  // double currentTemp = temperatures[0];
