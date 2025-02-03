#ifndef LOG_H
#define LOG_H

// for PROGMEM stuff, strcpy_P, strcat_P, PSTR
#include <Arduino.h>
#include <SdFat.h>
#include <avr/pgmspace.h>

#define USE_RTC 1
#if USE_RTC
#include <uRTCLib.h>
// URTCLIB_MODEL_DS1307
// URTCLIB_MODEL_DS3231
// URTCLIB_MODEL_DS3232
const byte RTC_MODEL = URTCLIB_MODEL_DS1307;
const byte RTC_ADDRESS = 0x68;
#endif // USE_RTC

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
//
// XXX: 3 will probaly not work on Arduino Uno; too high progmem requirement.

// Default SD_FAT_TYPE set to 1
#ifndef SD_FAT_TYPE
#define SD_FAT_TYPE 1
#endif

#if SD_FAT_TYPE == 0
typedef SdFat sd_t;
typedef File file_t;
#elif SD_FAT_TYPE == 1
typedef SdFat32 sd_t;
typedef File32 file_t;
#elif SD_FAT_TYPE == 2
typedef SdExFat sd_t;
typedef ExFile file_t;
#elif SD_FAT_TYPE == 3
typedef SdFs sd_t;
typedef FsFile file_t;
#else
#error Invalid SD_FAT_TYPE
#endif // SD_FAT_TYPE

const uint8_t SD_CS_PIN = 4; // Chip Select for the SD card

// Max SPI rate for AVR is 10 MHz for F_CPU 20 MHz, 8 MHz for F_CPU 16 MHz.
#define SPI_CLOCK SD_SCK_MHZ(10)
#ifdef ENABLE_DEDICATED_SPI
#undef ENABLE_DEDICATED_SPI
#endif
#define ENABLE_DEDICATED_SPI 0

// Select fastest interface.
#if ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
#else  // ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
#endif  // ENABLE_DEDICATED_SPI

struct LogData {
  uint32_t timestamp;
  float
      *temperatures; // Pointer to dynamically allocated array for temperatures
  bool heaterStatus; // Is the heater heating
};

class Log {
public:
  Log(uint8_t numSensors);
  ~Log();

  // Hardware initialization
  int init(uint8_t SD_CS_PIN);
  int logData(float *temperatures, bool heaterEnabled);
  const char *getErrorMessage() const { return errorMessage; }
  const char *getLogFileName() const {return logFileName; }
  bool isLoggingEnabled();
  int toggleLogging();
  int startLogging();
  int stopLogging();

private:
  uint8_t numSensors;   // Number of thermocouples
  bool loggingEnabled;  // Flag to indicate whether logging is enabled
  bool loggingStartet; // Flag to indicate if logging is startet
  uint8_t _SD_CS_PIN;   // CS pin for SD reader
  // Preallocate 100MiB file, 1024UL * 1024UL * 100UL .
  const uint32_t PREALLOCATE_SIZE_MiB = 1024UL;
  char errorMessage[50];
  char logFileName[30]; // Buffer for the log file name

  sd_t sd;        // SdFat object
  file_t logFile; // SdFile object for the log file
  LogData data;   // Struct to hold the data to be logged

  int enableLogging();
  bool isFileSizeExceeded();
  int openNewLogFile();
  void writeHeader();
  void printSDInfo();
  void errorPrint(const __FlashStringHelper* msg);
  uint32_t getCurrentTimestamp();
#if USE_RTC

  uRTCLib rtc;
  // Call back for file timestamps.  Only called for file create and sync().
  static void SdFat_dateTime(uint16_t *date, uint16_t *time, uint8_t *ms10);
  uint32_t encodeTimestamp(uint8_t year, uint8_t month, uint8_t day,
                           uint8_t hour, uint8_t minute, uint8_t second);
  void decodeTimestamp(uint32_t timestamp, uint8_t &year, uint8_t &month,
                       uint8_t &day, uint8_t &hour, uint8_t &minute,
                       uint8_t &second);
  void printRTCTime();
#endif // USE_RTC
};

extern Log logger;
// void printLogData(const LogData &data, uint8_t numSensors);

#endif
