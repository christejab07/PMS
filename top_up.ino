#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN 10

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

String plateNumber = "RAH 972 U";
String balance = "10000.00"; // Initial balance to write

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  // Set default key (factory default for MIFARE)
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println("Place the RFID card to write data...");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print("Card UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();

  byte plateBlock = 4;
  byte balanceBlock = 5;

  // Authenticate for plate number block
  MFRC522::StatusCode status = rfid.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A, plateBlock, &key, &(rfid.uid)
  );
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Auth failed for block 4: ");
    Serial.println(rfid.GetStatusCodeName(status));
    rfid.PICC_HaltA();
    return;
  }

  // Write plate number
  byte plateData[16] = {0}; // RFID block size is 16 bytes
  plateNumber.getBytes(plateData, sizeof(plateData));
  status = rfid.MIFARE_Write(plateBlock, plateData, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Failed to write plate: ");
    Serial.println(rfid.GetStatusCodeName(status));
    rfid.PICC_HaltA();
    return;
  }

  // Authenticate and write balance
  status = rfid.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A, balanceBlock, &key, &(rfid.uid)
  );
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Auth failed for block 5: ");
    Serial.println(rfid.GetStatusCodeName(status));
    rfid.PICC_HaltA();
    return;
  }

  byte balanceData[16] = {0};
  balance.getBytes(balanceData, sizeof(balanceData));
  status = rfid.MIFARE_Write(balanceBlock, balanceData, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Failed to write balance: ");
    Serial.println(rfid.GetStatusCodeName(status));
    rfid.PICC_HaltA();
    return;
  }

  Serial.println("Successfully wrote plate number and initial balance to card.");

  rfid.PICC_HaltA();
  delay(5000); // Prevent repeated writing until next scan
}
