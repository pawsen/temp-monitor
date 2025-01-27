#include "log.h"
// #include <TimeLib.h> // To get timestamp (optional)

// Constructor
Log::Log(uint8_t numSensors, size_t bufferSize)
    : numSensors(numSensors), bufferSize(bufferSize), bufferIndex(0),
      writeBuffer(nullptr), loggingEnabled(false) {
  // Allocate memory for temperatures in the LogDdata struct
  data.temperatures = new double[numSensors];
  // Initialize to 0
  memset(data.temperatures, 0, numSensors * sizeof(double));
  memset(errorMessage, 0, sizeof(errorMessage));

  // Allocate memory for the write buffer
  writeBuffer = new uint8_t[bufferSize];
}

// Destructor for cleanup
Log::~Log() {
  delete[] data.temperatures;
  delete[] writeBuffer;
}

// Hardware initialization
int Log::init(uint8_t SD_CS_PIN) {
  // Initialize the SD card
  if (!sd.begin(SD_CS_PIN, SD_SCK_MHZ(25))) {
    loggingEnabled = false;
    // Set the internal error message
    strcpy(errorMessage, "SD card init failed");
    return -1;
  }

  loggingEnabled = true;
  Serial.println(F("SD card initialized."));
  return 0;
}

// Open a new log file
int Log::openNewLogFile() {
  if (!loggingEnabled)
    return 0; // Skip if logging is disabled

  logFile.close();

  int fileIndex = 0;
  bool fileCreated = false;
  int maxAttempts = 100; // Maximum attempts to create a log file

  char logFileName[] = "ExFatLogger10.bin";
  // Generate file name with index
  // while (!fileCreated && fileIndex < maxAttempts) {

  while (sd.exists(logFileName)) {
    // sprintf(logFileName, "LOG_%03d.bin", fileIndex++);
    Serial.println(logFileName);

    char *p = strchr(logFileName, '.');
    if (!p) {
      Serial.println("no dot in filename");
      return -1;
    }
    while (true) {
      p--;
      if (p < logFileName || *p < '0' || *p > '9') {
        Serial.println(F("Can't create file name"));
        return -1;
      }
      if (p[0] != '9') {
        p[0]++;
        break;
      }
      p[0] = '0';
    }
  }

  // if (sd.exists(logFileName))
  //   Serial.println("cont.");
  //   fileIndex++;
  //   continue;
  // }

  // if (fileIndex == maxAttempts) {
  //   Serial.println(F("Too many logfiles"));
  //   strcpy(errorMessage, "Too many logfiles");
  //   loggingEnabled = false;
  //   return -1;
  // }

  // create and allocate
  if (!logFile.open(logFileName, O_RDWR | O_CREAT)) {
    Serial.println(F("open Logfile failed"));
    strcpy(errorMessage, "open Logfile failed");
    loggingEnabled = false;
    return -1;
  }
  if (!logFile.preAllocate(PREALLOCATE_SIZE_MiB)) {
    Serial.println(F("preAllocate failed"));
    strcpy(errorMessage, "preAllocate failed");
    loggingEnabled = false;
    return -1;
  }

  writeHeader();
  Serial.print(F("Logging to: "));
  Serial.println(logFileName);
  return 0;
}

void Log::writeHeader() {
  if (!loggingEnabled)
    return; // Skip if logging is disabled
  // uint32_t timestamp = now();   // Current time as the logging start
  // timestamp
  uint32_t timestamp = millis(); // Current time as the logging start timestamp
  logFile.write("HEADER\n", 7);  // Identifier for the header section

  // Write metadata to the header
  logFile.write(&numSensors, sizeof(numSensors));
  logFile.write(&timestamp, sizeof(timestamp));

  logFile.sync(); // Ensure header is written to the card
}
void Log::logData(double *temperatures, bool heaterStatus) {
  if (!loggingEnabled)
    return; // Skip if logging is disabled

  if (isFileSizeExceeded()) {
    logFile.close();
    openNewLogFile(); // Create a new file if size exceeds limit
  }

  // Fill the logData struct
  memcpy(data.temperatures, temperatures, numSensors * sizeof(double));
  data.heaterStatus = heaterStatus;
  // data.timestamp = now();
  data.timestamp = millis();

  // Write the struct to the buffer
  size_t dataSize = sizeof(double) * numSensors + sizeof(bool);
  if (bufferIndex + dataSize > bufferSize) {
    logFile.write(writeBuffer, bufferIndex); // Flush buffer to the SD card
    bufferIndex = 0;                         // Reset buffer index
  }

  memcpy(writeBuffer + bufferIndex, data.temperatures,
         sizeof(double) * numSensors);
  bufferIndex += sizeof(double) * numSensors;

  memcpy(writeBuffer + bufferIndex, &data.heaterStatus, sizeof(bool));
  bufferIndex += sizeof(bool);
  memcpy(writeBuffer + bufferIndex, &data.timestamp, sizeof(uint32_t));
  bufferIndex += sizeof(uint32_t);

  // printLogData(data, numSensors);
}

bool Log::isFileSizeExceeded() {
  return loggingEnabled && logFile.fileSize() > PREALLOCATE_SIZE_MiB;
}

void Log::flushLogFile() {
  if (!loggingEnabled)
    return; // Skip if logging is disabled
  if (bufferIndex > 0) {
    logFile.write(writeBuffer, bufferIndex); // Write remaining buffer data
    bufferIndex = 0;
  }
  logFile.sync(); // Ensure all data is written to the SD card
}

// void printLogData(const LogData &data, uint8_t numSensors) {
//   Serial.print("Timestamp: ");
//   Serial.println(data.timestamp);

//   Serial.print(F("Temperatures: "));
//   for (uint8_t i = 0; i < numSensors; i++) {
//     Serial.print(data.temperatures[i]);
//     if (i < numSensors - 1)
//       Serial.print(", ");
//   }
//   Serial.println();

//   Serial.print("Heater Status: ");
//   Serial.println(data.heaterStatus ? "ON" : "OFF");
// }
