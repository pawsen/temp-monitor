#ifndef TEMP_READER_H
#define TEMP_READER_H

#include <Arduino.h>
#include <MAX6675.h>

class ThermocoupleReader {
private:
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
