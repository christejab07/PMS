# import time
# import serial
# import os
# import csv
# from datetime import datetime

# SERIAL_PORT = '/dev/ttyACM0'  # Change if needed
# BAUD_RATE = 9600
# CSV_FILE = 'transactions.csv'

# def parse_log(line):
#     # Format: LOG: Plate=RAH972U, Old=600.00, Deducted=200.00, New=400.00
#     try:
#         parts = line.replace("LOG:", "").strip().split(", ")
#         return {
#             'timestamp': datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
#             'plate': parts[0].split('=')[1],
#             'old_balance': float(parts[1].split('=')[1]),
#             'deducted': float(parts[2].split('=')[1]),
#             'new_balance': float(parts[3].split('=')[1])
#         }
#     except Exception as e:
#         print(f"Error parsing: {line}\n{e}")
#         return None

# def save_to_csv(data):
#     new_file = not os.path.exists(CSV_FILE)
#     with open(CSV_FILE, 'a', newline='') as file:
#         writer = csv.DictWriter(file, fieldnames=data.keys())
#         if new_file:
#             writer.writeheader()
#         writer.writerow(data)

# def main():
#     print("Initializing RFID Payment System...")
#     try:
#         with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=5) as ser:
#             time.sleep(2)  # <--- Give Arduino time to reset
#             print("Waiting for card log from Arduino...")
#             while True:
#                 line = ser.readline().decode('utf-8').strip()
#                 if line.startswith("LOG:"):
#                     data = parse_log(line)
#                     if data:
#                         print("\n--- Transaction ---")
#                         for k, v in data.items():
#                             print(f"{k.capitalize():>12}: {v}")
#                         save_to_csv(data)
#                         print("Saved to transactions.csv\n")
#                         break  # Exit after one read
#     except Exception as e:
#         print(f"Serial error: {e}")

# if __name__ == '__main__':
#     main()

import time
import serial
import os
import csv
from datetime import datetime

SERIAL_PORT = '/dev/ttyACM0'  # Adjust if needed
BAUD_RATE = 9600
CSV_FILE = 'transactions.csv'
DEDUCTION_AMOUNT = 200.00  # Can be changed

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
            time.sleep(2)  # Allow Arduino reset
            print("Waiting for data from Arduino...")

            plate = None
            amount = None

            while True:
                line = ser.readline().decode('utf-8').strip()
                if line.startswith("Plate:"):
                    plate = line.split(":")[1].strip()
                elif line.startswith("Amount:"):
                    try:
                        amount = float(line.split(":")[1].strip())
                    except ValueError:
                        print("Invalid amount format.")
                        continue

                # Once both plate and amount are read
                if plate and amount is not None:
                    deducted = DEDUCTION_AMOUNT
                    new_balance = amount - deducted

                    log_line = f"LOG: Plate={plate}, Old={amount:.2f}, Deducted={deducted:.2f}, New={new_balance:.2f}"
                    print("\n" + log_line)

                    # Build the data dictionary
                    data = {
                        'timestamp': datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
                        'plate': plate,
                        'old_balance': amount,
                        'deducted': deducted,
                        'new_balance': new_balance
                    }

                    save_to_csv(data)
                    print("Saved to transactions.csv\n")
                    break  # Process only one transaction
    except Exception as e:
        print(f"Serial error: {e}")

if __name__ == '__main__':
    main()
