#include "heaterControl.h"
#include "I2C_LCD.h"
#include "log.h"
#include "menu.h"
#include "tempReader.h"

// Change these two numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
// On Arduino Uno you can only use pin 2 and 3 for interrupts.
const uint8_t ENCODER_PIN_A = 2;
const uint8_t ENCODER_PIN_B = 3;
const uint8_t ENCODER_BUTTON_PIN = 4;

const uint8_t HEATER_PIN = 5;
const uint8_t SD_CS_PIN = 10; // Chip Select for the SD card

// SPI Pins (shared by SD card and thermocouples)
const uint8_t SPI_SCK_PIN = 13; // SPI Clock pin
const uint8_t SPI_MISO_PIN =
    12; // SPI MISO pin (used for reading data from thermocouples)
const uint8_t SPI_MOSI_PIN = 11; // SPI MOSI pin (not used by the thermocouples)

// MAX6675 Thermocouple Pins
const uint8_t MAX6675_CS_PINS[] = {6,
                                   7}; // Chip Select for thermocouples (array)
// const uint8_t NUM_THERMOCOUPLES = 2;  // Number of thermocouples
const uint8_t NUM_THERMOCOUPLES =
    sizeof(MAX6675_CS_PINS) / sizeof(MAX6675_CS_PINS[0]);

HeaterControl heaterControl(HEATER_PIN);
Log logger(NUM_THERMOCOUPLES);
I2C_LCD lcd(0x27);

void setup() {
  Serial.begin(9600);

  // Initialize the heater control
  heaterControl.init();
  // Optionally set the initial target temperature
  heaterControl.setTargetTemperature(40.0);

  // Initialize the SD card for logging
  logger.init(SD_CS_PIN);
  logger.openNewLogFile();

  // Initialize the rotary encoder menu
  menu.init(ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_BUTTON_PIN);

  // Initialize the LCD
  lcd.begin(16, 2);

  // Initialize the thermocouple reader
  thermocoupleReader.init(NUM_THERMOCOUPLES, MAX6675_CS_PINS, SPI_MISO_PIN,
                          SPI_SCK_PIN);
}

void loop() {
  // Update the menu, handle button presses and rotary encoder inputs
  menu.update();

  // Get the current temperatures from thermocouples
  double *temperatures = thermocoupleReader.getTemperatures();

  // Update the heater control logic using the first thermocouple as input
  heaterControl.update(temperatures[0]);

  double targetTemp = heaterControl.getTargetTemperature();
  bool heaterStatus = heaterControl.getHeaterStatus();

  // Optional: Log the status or display it on an LCD
  Serial.print("Target °C:, ");
  Serial.print(targetTemp);
  Serial.print(" Current: ");
  Serial.print(temperatures[0]);
  Serial.print(" Heater: ");
  Serial.println(heaterStatus ? "ON" : "OFF");

  lcd.setCursor(0, 0);
  lcd.print("Target: ");
  lcd.print(targetTemp);
  lcd.setCursor(0, 1);
  lcd.print("Current: ");
  lcd.print(temperatures[0]);
  lcd.setCursor(0, 2);
  lcd.print("Heater: ");
  lcd.println(heaterStatus ? "ON" : "OFF");

  // Log the current temperatures and heater status to the SD card
  logger.logData(temperatures, heaterStatus);
  // Periodically flush the log file to ensure data is saved
  logger.flushLogFile();
  delay(1000);
}
