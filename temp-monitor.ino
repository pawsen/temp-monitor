
// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
// #define SD_FAT_TYPE 1

#include "heaterControl.h"
#include "I2C_LCD.h"
#include "log.h"
#include "menu.h"
// #include "tempReader.h"
#include <MAX6675.h>


// Change these two numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
// On Arduino Uno you can only use pin 2 and 3 for interrupts.
const uint8_t ENCODER_PIN_A = 2;
const uint8_t ENCODER_PIN_B = 3;
const uint8_t ENCODER_BUTTON_PIN = 5;

/// XXX SD_CS_PIN is set in log.h
// const uint8_t SD_CS_PIN = 4;
const uint8_t HEATER_PIN = 6;
// List of CS pins to disable
// example, with the Ethernet shield, set DISABLE_CS_PIN to 10 to disable the
// Ethernet controller.
// const uint8_t DISABLE_CS_PINS[] = {10};
const uint8_t DISABLE_CS_PIN= 10;

// HW SPI:
// | Name                       | pin |
// |----------------------------|-----|
// | CLOCK                      |  13 |
// | MISO (Master in/Slave out) |  12 |
// | MOSI (Master out/Slave in) |  11 |

// Number of thermocouples
const int ThermoCouplesNum = 1;
// SPI Pins (shared by SD card and thermocouples)
const uint8_t MAX6675_CS_PINS[ThermoCouplesNum] = {7}; // Chip Select pins
const uint8_t SPI_MISO_PIN = 8;
const uint8_t SPI_SCK_PIN = 9;

const uint8_t NUM_THERMOCOUPLES = 1;

unsigned long previousMillis = 0;
// XXX if the interval is less than 1000, the readings from the thermistor are incorrect.
// XXX I don't understand why that's the case
const long interval = 1000; /// logging interval, 1s

I2C_LCD lcd(0x27);
HeaterControl heaterControl(HEATER_PIN);
Log logger(ThermoCouplesNum );
Menu menu(ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_BUTTON_PIN);
// MAX6675 thermoCouple(MAX6675_CS_PINS[0], SPI_MISO_PIN , SPI_SCK_PIN);
MAX6675 thermoCouple(MAX6675_CS_PINS[0], &SPI);


float getTemperature() {
  int status = thermoCouple.read();
  if (status != STATUS_OK) {Serial.println(F("ThermoCouple ERROR!"));}
  return thermoCouple.getTemperature();
}

void setup() {
  Serial.begin(115200);

  // Initialize the LCD
  Wire.begin();
  lcd.begin(16, 2);
  lcd.print("Hello");

  delay(250);
  // Initialize thermocouples
  SPI.begin();
  thermoCouple.begin();
  // XXX I do not understand what setSPIspeed does or when it is needed.
  thermoCouple.setSPIspeed(4000000);
  Serial.print("Thermocouple ");
  Serial.print(": ");
  float currentTemp = getTemperature();
  Serial.println(currentTemp);

  // Initialize the heater control
  heaterControl.init();
  // Optionally set the initial target temperature
  heaterControl.setTargetTemperature(40.0);
  // make sure the heater is turned off
  heaterControl.disable();

  // Initialize the SD card for logging
  pinMode(DISABLE_CS_PIN, OUTPUT);
  digitalWrite(DISABLE_CS_PIN, HIGH);

  if (logger.init(SD_CS_PIN) != 0)
    displayError(logger);
  if (logger.startLogging() != 0)
    displayError(logger);
  else{
    lcd.setCursor(0, 0);
    lcd.print(F("logging to"));
    lcd.setCursor(0, 1);
    lcd.print(logger.getLogFileName());
  }

  // Initialize the rotary encoder menu
  menu.init();

  delay(2000);
}

void loop() {
  // Update the menu, handle button presses and rotary encoder inputs
  menu.update();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {

    float currentTemp = getTemperature();
    // Update the heater control logic using the first thermocouple as input
    heaterControl.update(currentTemp);

    float targetTemp = heaterControl.getTargetTemperature();

    // Display default screen when menu is not active
    menu.displayDefaultScreen(currentTemp, targetTemp);

    bool heaterStatus = heaterControl.getHeaterStatus();
    uint32_t autoDisableTime = heaterControl.getTimeUntilDisable();
    uint8_t hours, minutes;
    getHoursAndMinutes(autoDisableTime, hours, minutes);

    // Optional: Log the status or display it on an LCD
    Serial.print(F("Target °C:, "));
    Serial.print(targetTemp);
    Serial.print(F(" Current: "));
    Serial.print(currentTemp);
    Serial.print(F(" Heater: "));
    Serial.print(heaterStatus ? "ON" : "OFF");
    Serial.print(F(" Auto-disable: "));
    Serial.print(hours);
    Serial.print(F("h "));
    Serial.print(minutes);
    Serial.print(F("m"));
    Serial.print(F(" Log "));
    Serial.print(logger.isLoggingEnabled() ? "ON " : "OFF ");
    Serial.print(logger.getLogFileName());
    Serial.println();

    previousMillis = currentMillis;

    float temperatures[NUM_THERMOCOUPLES] = { currentTemp};
    // Log the current temperatures and heater status to the SD card
    if (logger.logData(temperatures, heaterStatus) != 0)
      displayError(logger);
  }

}
