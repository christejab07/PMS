import serial
import os
import csv
from datetime import datetime

SERIAL_PORT = '/dev/ttyACM0'  # Change if needed
BAUD_RATE = 9600
CSV_FILE = 'transactions.csv'

def parse_log(line):
    # Format: LOG: Plate=RAH972U, Old=600.00, Deducted=200.00, New=400.00
    try:
        parts = line.replace("LOG:", "").strip().split(", ")
        return {
            'timestamp': datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
            'plate': parts[0].split('=')[1],
            'old_balance': float(parts[1].split('=')[1]),
            'deducted': float(parts[2].split('=')[1]),
            'new_balance': float(parts[3].split('=')[1])
        }
    except Exception as e:
        print(f"Error parsing: {line}\n{e}")
        return None

def save_to_csv(data):
    new_file = not os.path.exists(CSV_FILE)
    with open(CSV_FILE, 'a', newline='') as file:
        writer = csv.DictWriter(file, fieldnames=data.keys())
        if new_file:
            writer.writeheader()
        writer.writerow(data)

def main():
    print("Initializing RFID Payment System...")
    try:
        with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=5) as ser:
            print("Waiting for card log from Arduino...")
            while True:
                line = ser.readline().decode('utf-8').strip()
                if line.startswith("LOG:"):
                    data = parse_log(line)
                    if data:
                        print("\n--- Transaction ---")
                        for k, v in data.items():
                            print(f"{k.capitalize():>12}: {v}")
                        save_to_csv(data)
                        print("Saved to transactions.csv\n")
                        break  # Exit after one read
    except Exception as e:
        print(f"Serial error: {e}")

if __name__ == '__main__':
    main()
