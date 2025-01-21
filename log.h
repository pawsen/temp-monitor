#ifndef LOG_H
#define LOG_H

#include <Arduino.h>
#include <SdFat.h>

struct LogData {
  uint32_t timestamp;
  double
      *temperatures; // Pointer to dynamically allocated array for temperatures
  bool heaterStatus; // Heater status
};

class Log {
public:
  Log(uint8_t numSensors, size_t bufferSize = 512);
  ~Log();

  // Hardware initialization
  void init(uint8_t SD_CS_PIN);
  void openNewLogFile();
  void writeHeader();
  void logData(double *temperatures, bool heaterStatus);
  void flushLogFile();
  bool isFileSizeExceeded();

private:
  uint8_t numSensors;   // Number of thermocouples
  size_t bufferSize;    // Buffer size matching SD sector size
  size_t bufferIndex;   // Current index in the write buffer
  uint8_t *writeBuffer; // Write buffer to reduce SD card writes

  SdFat sd;       // SdFat object
  SdFile logFile; // SdFile object for the log file
  LogData data;   // Struct to hold the data to be logged

  const uint32_t maxFileSize = 4UL * 1024UL * 1024UL; // 4MB max file size
  char logFileName[20]; // Buffer for the log file name
};

extern Log logger;

#endif
