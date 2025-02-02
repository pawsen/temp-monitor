import struct
from datetime import datetime
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import glob
from pathlib import Path
import argparse

def read_log_file(file_path):
    timestamps = []
    temperatures = []
    heater_statuses = []
    num_sensors = None

    with open(file_path, 'rb') as log_file:
        # Read the header
        header = log_file.read(7)  # Read the first 7 bytes ("HEADER\n")
        if header != b'HEADER\n':
            print(f"Invalid log file format: Header not found in {file_path}.")
            return None, None, None

        # Read the number of sensors (1 byte)
        num_sensors = struct.unpack('<B', log_file.read(1))[0]
        print(f"Number of sensors in {file_path}: {num_sensors}")

        # Read the timestamp (4 bytes)
        timestamp = struct.unpack('<I', log_file.read(4))[0]
        year, month, day, hour, minute, second = decode_timestamp(timestamp)
        print(f"Log start time in {file_path}: {year}-{month:02d}-{day:02d} {hour:02d}:{minute:02d}:{second:02d}")

        # Read the size of each temperature value (4 bytes for float, 8 for double)
        # temp_size = struct.unpack('<B', log_file.read(1))[0]  # Read 1 byte to determine size (4 or 8)
        temp_size = 4;


        # Read the data entries
        while True:
            # Read temperatures (num_sensors * 8 bytes each)
            temp_data = log_file.read(num_sensors * temp_size)
            if not temp_data:
                break
            # Unpack the temperatures based on detected size
            if temp_size == 4:  # If temperature size is 4 bytes (float)
                temperatures.append(struct.unpack(f'<{num_sensors}f', temp_data))
            elif temp_size == 8:  # If temperature size is 8 bytes (double)
                temperatures.append(struct.unpack(f'<{num_sensors}d', temp_data))


            # Read heater status (1 byte)
            heater_status = log_file.read(1)
            if not heater_status:
                break
            heater_statuses.append(struct.unpack('<?', heater_status)[0])

            # Read timestamp (4 bytes)
            entry_timestamp = log_file.read(4)
            if not entry_timestamp:
                break
            entry_timestamp = struct.unpack('<I', entry_timestamp)[0]
            year, month, day, hour, minute, second = decode_timestamp(entry_timestamp)
            timestamps.append(datetime(year + 2000, month, day, hour, minute, second))

        print(f"Log end time in {file_path}: {year}-{month:02d}-{day:02d} {hour:02d}:{minute:02d}:{second:02d}")
        print(f"# mesurements in {file_path}: {len(timestamps)}")

    return timestamps, temperatures, heater_statuses

def decode_timestamp(timestamp):
    """Decode the timestamp using bitwise operations

    # Example usage
    timestamp = 0x9912312359  # Example encoded timestamp
    year, month, day, hour, minute, second = decode_timestamp(timestamp)
    print(f"Year: {year}, Month: {month}, Day: {day}, Hour: {hour}, Minute: {minute}, Second: {second}")
    """
    year = (timestamp >> 26) & 0x3F  # 6 bits for year
    month = (timestamp >> 22) & 0x0F  # 4 bits for month
    day = (timestamp >> 17) & 0x1F    # 5 bits for day
    hour = (timestamp >> 12) & 0x1F   # 5 bits for hour
    minute = (timestamp >> 6) & 0x3F  # 6 bits for minute
    second = timestamp & 0x3F         # 6 bits for second

    return year, month, day, hour, minute, second



def plot_data(timestamps, temperatures, heater_statuses):
    if not timestamps or not temperatures or not heater_statuses:
        print("No data to plot.")
        return

    # Flatten temperature data (since it's a list of lists)
    temperatures = [temp[0] for temp in temperatures]

    # Create the plot
    fig, ax1 = plt.subplots(figsize=(12, 6))

    # Plot temperature on the primary y-axis
    ax1.plot(timestamps, temperatures, 'b-', label='Temperature (°C)')
    ax1.set_xlabel('Time')
    ax1.set_ylabel('Temperature (°C)', color='b')
    ax1.tick_params(axis='y', labelcolor='b')

    # Plot heater status on the secondary y-axis
    ax2 = ax1.twinx()
    ax2.plot(timestamps, heater_statuses, 'r-', label='Heater Status (ON/OFF)')
    ax2.set_ylabel('Heater Status', color='r')
    ax2.tick_params(axis='y', labelcolor='r')
    ax2.set_ylim(-0.1, 1.1)  # Heater status is binary (0 or 1)

    # Format the x-axis to show time properly
    ax1.xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m-%d %H:%M:%S'))
    fig.autofmt_xdate()

    # Add a legend
    fig.legend(loc="upper left", bbox_to_anchor=(0.1, 0.9))

    # Add a title
    plt.title('Temperature and Heater Status Over Time')

    # Show the plot
    plt.show()

def main(pattern):
    # Path to the folder containing the files
    folder_path = Path('./logs')
    # Find all matching files
    files = sorted(glob.glob(str(folder_path / pattern)), key=lambda x: int(x[-6:-4]))  # Sort by XX part

    # Initialize combined data lists
    all_timestamps = []
    all_temperatures = []
    all_heater_statuses = []

    # Read data from all files
    for file_path in files:
        print(f"Reading file: {file_path}")
        timestamps, temperatures, heater_statuses = read_log_file(file_path)
        if timestamps and temperatures and heater_statuses:
            all_timestamps.extend(timestamps)
            all_temperatures.extend(temperatures)
            all_heater_statuses.extend(heater_statuses)

    # Plot the combined data
    if all_timestamps and all_temperatures and all_heater_statuses:
        plot_data(all_timestamps, all_temperatures, all_heater_statuses)
        return (all_timestamps, all_temperatures, all_heater_statuses)
    else:
        print("No valid data found in any log files.")

if __name__ == "__main__":
    # Set up argument parsing
    parser = argparse.ArgumentParser(description="Plot temperature and heater status data from log files.")
    parser.add_argument(
        "--pattern",
        type=str,
        default='TempLog_250131_230846_*.bin',
        help="File pattern to match log files (e.g., 'TempLog_250131_230846_*.bin')."
    )
    args = parser.parse_args()

    # Run the main function with the provided pattern
    main(args.pattern)
