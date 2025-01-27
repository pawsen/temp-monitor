#ifndef LOG_H
#define LOG_H

#include <Arduino.h>
#include <SdFat.h>

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.

// Default SD_FAT_TYPE set to 3
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
#endif  // SD_FAT_TYPE

struct LogData {
  uint32_t timestamp;
  double
      *temperatures; // Pointer to dynamically allocated array for temperatures
  bool heaterStatus; // Heater status
};

class Log {
public:
  Log(uint8_t numSensors, size_t bufferSize = 20);
  ~Log();

  // Hardware initialization
  int init(uint8_t SD_CS_PIN);
  int openNewLogFile();
  void writeHeader();
  void logData(double *temperatures, bool heaterStatus);
  void flushLogFile();
  bool isFileSizeExceeded();
  const char *getErrorMessage() const { return errorMessage; }

private:
  uint8_t numSensors;   // Number of thermocouples
  size_t bufferSize;    // Buffer size matching SD sector size
  size_t bufferIndex;   // Current index in the write buffer
  uint8_t *writeBuffer; // Write buffer to reduce SD card writes
  bool loggingEnabled;  // Flag to indicate whether logging is enabled

  sd_t sd;       // SdFat object
  file_t logFile; // SdFile object for the log file
  LogData data;   // Struct to hold the data to be logged

  // Preallocate 1GiB file.
  const uint32_t PREALLOCATE_SIZE_MiB = 1024UL;
  char logFileName[20]; // Buffer for the log file name
  char errorMessage[50];
};

extern Log logger;
// void printLogData(const LogData &data, uint8_t numSensors);

#endif
