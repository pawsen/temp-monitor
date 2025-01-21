#include "tempReader.h"


/* This line creates a single, global instance of the ThermocoupleReader class
 * that can be used throughout the program. By defining the global instance in
 * the .cpp file and declaring it as extern in the header file, the global
 * instance is accessible from other parts of the program while maintaining
 * proper encapsulation and avoiding multiple definitions.
 */
ThermocoupleReader thermocoupleReader;

void ThermocoupleReader::init(uint8_t numThermocouples, const uint8_t* csPins,uint8_t miso, uint8_t clock) {

    // uint8_t NUM_THERMOCOUPLES = sizeof(csPins) / sizeof(csPins[0]);
    nT = numThermocouples;

    // Allocate memory for thermocouples and temperatures
    thermocouples = new MAX6675*[numThermocouples];  // Array of pointers to MAX6675
    temperatures = new double[numThermocouples];

    // Initialize each thermocouple with the appropriate CS pin and shared MISO/CLOCK pins
    for (uint8_t i = 0; i < numThermocouples; i++) {
        thermocouples[i] = new MAX6675(clock, csPins[i], miso);  // Dynamically allocate
    }
}

double* ThermocoupleReader::getTemperatures() {
    for (uint8_t i = 0; i < nT; i++) {
        temperatures[i] = thermocouples[i]->getTemperature();  // Use pointer dereference to call
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
