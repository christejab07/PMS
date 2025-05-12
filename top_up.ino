#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN 10

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

String plateNumber = "RAH972U";       // Remove spaces to fit block cleanly
String amount = "1000.00";            // Convert float to string for simplicity

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  // Set the default key (0xFF is factory default for all key bytes)
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println("Place the RFID card to write data...");
}

void loop() {
  // Wait for a new card
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println("Card detected!");

  // Authenticate and write plate number to block 4
  byte blockPlate = 4;
  MFRC522::StatusCode status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockPlate, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Auth failed for block 4: ");
    Serial.println(rfid.GetStatusCodeName(status));
    rfid.PICC_HaltA();
    return;
  }
  byte plateData[16] = {0};
  plateNumber.getBytes(plateData, sizeof(plateData));
  status = rfid.MIFARE_Write(blockPlate, plateData, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Write failed for plate: ");
    Serial.println(rfid.GetStatusCodeName(status));
    rfid.PICC_HaltA();
    return;
  }

  // Authenticate and write amount to block 5
  byte blockAmount = 5;
  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockAmount, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Auth failed for block 5: ");
    Serial.println(rfid.GetStatusCodeName(status));
    rfid.PICC_HaltA();
    return;
  }
  byte amountData[16] = {0};
  amount.getBytes(amountData, sizeof(amountData));
  status = rfid.MIFARE_Write(blockAmount, amountData, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Write failed for amount: ");
    Serial.println(rfid.GetStatusCodeName(status));
    rfid.PICC_HaltA();
    return;
  }

  Serial.println("Data written successfully!");
  Serial.print("Plate: ");
  Serial.println(plateNumber);
  Serial.print("Amount: ");
  Serial.println(amount);

  // Halt and stop further reading until the next card
  rfid.PICC_HaltA();
  delay(3000); // 3 second delay before reactivating
}
