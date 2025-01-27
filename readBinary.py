#!/usr/bin/env python3

import struct
import matplotlib.pyplot as plt
import numpy as np

# Define the structure of the LogHeader and LogData
# LogHeader contains the number of sensors (uint8_t)
# LogData contains a timestamp (unsigned long), temperatures (array of doubles), and relayState (bool)
LOG_HEADER_FORMAT = 'B'  # 'B' is for unsigned char (1 byte for the number of sensors)
LOG_DATA_FORMAT = 'L 2d ?'  # Timestamp (unsigned long), 2 doubles (temperatures), bool (relayState)

# Adjust these values according to your log file's path
log_file_path = 'temp_log_0.bin'

def read_log_file(file_path):
    timestamps = []
    temperatures = []
    relay_states = []

    with open('LOG_1234.bin', 'rb') as f:
        # Read the header
        header = f.readline().strip()  # HEADER identifier
        num_sensors = int.from_bytes(f.read(1), byteorder='little')  # Number of sensors
        timestamp = int.from_bytes(f.read(4), byteorder='little')  # Timestamp
        print(f"Log file started at: {timestamp}")
        print(f"Number of sensors: {num_sensors}")

    # Open the log file and read the number of sensors from the header
    with open(file_path, 'rb') as file:
        # Read the header to get the number of sensors
        header_data = file.read(struct.calcsize(LOG_HEADER_FORMAT))
        num_sensors = struct.unpack(LOG_HEADER_FORMAT, header_data)[0]

        print(f"Number of sensors: {num_sensors}")

        # Read the log data entries
        while True:
            # Read the data as per the structure of LogData
            data = file.read(struct.calcsize(LOG_DATA_FORMAT))
            if not data:
                break

            # Unpack the data from the binary file
            timestamp, *sensor_data, relay_state = struct.unpack(LOG_DATA_FORMAT, data)

            # Append to lists
            timestamps.append(timestamp)
            temperatures.append(sensor_data)
            relay_states.append(relay_state)

    return timestamps, temperatures, relay_states, num_sensors

def plot_data(timestamps, temperatures, relay_states, num_sensors):
    # Convert timestamps to a time difference (e.g., seconds from the first timestamp)
    time_diffs = np.array(timestamps) - timestamps[0]

    # Create a plot
    plt.figure(figsize=(10, 6))

    # Plot temperature data for each sensor
    for i in range(num_sensors):
        temp = [temps[i] for temps in temperatures]
        plt.subplot(2, 1, 1)  # First subplot
        plt.plot(time_diffs, temp, label=f'Temp{i+1} (°C)')

    plt.xlabel('Time (s)')
    plt.ylabel('Temperature (°C)')
    plt.title('Temperature over Time')
    plt.legend()

    # Plot heating status
    plt.subplot(2, 1, 2)  # Second subplot
    plt.plot(time_diffs, relay_states, label='Heating Status', color='g')
    plt.xlabel('Time (s)')
    plt.ylabel('Heating Status (0=Off, 1=On)')
    plt.title('Heating Status over Time')
    plt.ylim(-0.1, 1.1)  # Keep the y-axis between 0 and 1 (Off/On)
    plt.legend()

    # Show the plots
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    # Read the binary log file
    timestamps, temperatures, relay_states, num_sensors = read_log_file(log_file_path)

    # Plot the data
    plot_data(timestamps, temperatures, relay_states, num_sensors)
