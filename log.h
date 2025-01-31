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

struct LogData {
  uint32_t timestamp;
  double
      *temperatures; // Pointer to dynamically allocated array for temperatures
  bool heaterStatus; // Heater status
};

class Log {
public:
  Log(uint8_t numSensors, size_t bufferSize = 50);
  ~Log();

  // Hardware initialization
  int init(uint8_t SD_CS_PIN);
  int openNewLogFile();
  void writeHeader();
  int logData(double *temperatures, bool heaterStatus);
  void flushLogFile();
  void flushBuffer();
  bool isFileSizeExceeded();
  const char *getErrorMessage() const { return errorMessage; }
  const char *getLogFileName() const {return logFileName; }

private:
  uint8_t numSensors;   // Number of thermocouples
  size_t bufferSize;    // Buffer size matching SD sector size
  size_t bufferIndex;   // Current index in the write buffer
  uint8_t *writeBuffer; // Write buffer to reduce SD card writes
  bool loggingEnabled;  // Flag to indicate whether logging is enabled

  sd_t sd;        // SdFat object
  file_t logFile; // SdFile object for the log file
  LogData data;   // Struct to hold the data to be logged

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

  // Preallocate 1GiB file.
  const uint32_t PREALLOCATE_SIZE_MiB = 1024UL;
  char errorMessage[50];
  char logFileName[30]; // Buffer for the log file name
};

extern Log logger;
// void printLogData(const LogData &data, uint8_t numSensors);

#endif
