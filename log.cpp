#include "log.h"
#include <TimeLib.h> // To get timestamp (optional)

// Constructor
Log::Log(uint8_t numSensors, size_t bufferSize)
    : numSensors(numSensors), bufferSize(bufferSize), bufferIndex(0),
      writeBuffer(nullptr) {
  // Allocate memory for temperatures in the LogDdata struct
  data.temperatures = new double[numSensors];
  // Initialize to 0
  memset(data.temperatures, 0, numSensors * sizeof(double));

  // Allocate memory for the write buffer
  writeBuffer = new uint8_t[bufferSize];
}

// Destructor for cleanup
Log::~Log() {
  delete[] data.temperatures;
  delete[] writeBuffer;
}

// Hardware initialization
void Log::init(uint8_t SD_CS_PIN) {
  // Initialize the SD card
  if (!sd.begin(SD_CS_PIN, SD_SCK_MHZ(25))) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");
}

// Open a new log file
void Log::openNewLogFile() {
  int fileIndex = 0;
  bool fileCreated = false;

  // Generate file name with index
  while (!fileCreated) {
    sprintf(logFileName, "LOG_%03d.bin", fileIndex++);
    // snprintf(logFileName, sizeof(logFileName), "log%03d.bin", fileIndex++);
    if (!sd.exists(logFileName)) {
      fileCreated = logFile.open(logFileName, O_CREAT | O_WRITE);
    }
  }

  if (fileCreated) {
    writeHeader();
    Serial.print("Logging to: ");
    Serial.println(logFileName);
  } else {
    Serial.println("Failed to create log file!");
  }
}

void Log::writeHeader() {
  uint32_t timestamp = now();   // Current time as the logging start timestamp
  logFile.write("HEADER\n", 7); // Identifier for the header section

  // Write metadata to the header
  logFile.write(&numSensors, sizeof(numSensors));
  logFile.write(&timestamp, sizeof(timestamp));

  logFile.sync(); // Ensure header is written to the card
}
void Log::logData(double *temperatures, bool heaterStatus) {
  if (isFileSizeExceeded()) {
    logFile.close();
    openNewLogFile(); // Create a new file if size exceeds limit
  }

  // Fill the logData struct
  memcpy(data.temperatures, temperatures, numSensors * sizeof(double));
  data.heaterStatus = heaterStatus;

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
}

bool Log::isFileSizeExceeded() { return logFile.fileSize() > maxFileSize; }

void Log::flushLogFile() {
  if (bufferIndex > 0) {
    logFile.write(writeBuffer, bufferIndex); // Write remaining buffer data
    bufferIndex = 0;
  }
  logFile.sync(); // Ensure all data is written to the SD card
}
