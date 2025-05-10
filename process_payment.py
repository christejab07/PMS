import serial
import time
import sys
import csv
from datetime import datetime

SERIAL_PORT = '/dev/ttyUSB0'  # (COM3 on Windows)
BAUD_RATE = 9600
CSV_FILENAME = 'transactions.csv'

def wait_for_card(ser):
    print("Please place your RFID card on the reader...")

    while True:
        if ser.in_waiting:
            line = ser.readline().decode('utf-8').strip()
            if line.startswith("LOG:"):
                return parse_log_line(line)

def parse_log_line(line):
    try:
        # Strip prefix and split key-value pairs
        data = line[4:]  # Remove 'LOG:'
        parts = data.split(',')

        plate = parts[0].split('=')[1]
        old_bal = float(parts[1].split('=')[1])
        deducted = float(parts[2].split('=')[1])
        new_bal = float(parts[3].split('=')[1])

        return {
            'plate': plate,
            'old_balance': old_bal,
            'deducted': deducted,
            'new_balance': new_bal,
            'timestamp': datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        }
    except Exception as e:
        print(f"Error parsing log line: {line}")
        print(e)
        return None

def save_to_csv(data):
    file_exists = False
    try:
        file_exists = open(CSV_FILENAME)
    except FileNotFoundError:
        pass

    with open(CSV_FILENAME, mode='a', newline='') as csvfile:
        fieldnames = ['timestamp', 'plate', 'old_balance', 'deducted', 'new_balance']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)

        # Write header if file is new
        if not file_exists:
            writer.writeheader()

        writer.writerow({
            'timestamp': data['timestamp'],
            'plate': data['plate'],
            'old_balance': data['old_balance'],
            'deducted': data['deducted'],
            'new_balance': data['new_balance']
        })

def main():
    print("Initializing RFID Payment System...")

    try:
        with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=5) as ser:
            time.sleep(2)  # Wait for Arduino to be ready

            transaction = wait_for_card(ser)
            if transaction:
                print("\nPayment Processed:")
                print(f"  Plate Number : {transaction['plate']}")
                print(f"  Old Balance  : {transaction['old_balance']:.2f}")
                print(f"  Deducted     : {transaction['deducted']:.2f}")
                print(f"  New Balance  : {transaction['new_balance']:.2f}")

                save_to_csv(transaction)
                print("Transaction saved to 'transactions.csv'")
            else:
                print("Failed to read card or parse transaction.")

    except serial.SerialException as e:
        print(f"Serial connection error: {e}")
        sys.exit(1)

if __name__ == '__main__':
    main()
