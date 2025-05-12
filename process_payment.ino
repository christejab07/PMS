#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN 10

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println("Ready to deduct from card.");
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

  // Authenticate and read plate number
  MFRC522::StatusCode status = rfid.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A, plateBlock, &key, &(rfid.uid)
  );
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Auth failed (plate): ");
    Serial.println(rfid.GetStatusCodeName(status));
    rfid.PICC_HaltA();
    return;
  }

  byte buffer[18];
  byte size = sizeof(buffer);
  status = rfid.MIFARE_Read(plateBlock, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Failed to read plate: ");
    Serial.println(rfid.GetStatusCodeName(status));
    rfid.PICC_HaltA();
    return;
  }

  String plate = "";
  for (int i = 0; i < 16; i++) {
    if (buffer[i] != 0) plate += (char)buffer[i];
  }

  // Authenticate and read balance
  status = rfid.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A, balanceBlock, &key, &(rfid.uid)
  );
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Auth failed (balance): ");
    Serial.println(rfid.GetStatusCodeName(status));
    rfid.PICC_HaltA();
    return;
  }

  size = sizeof(buffer);
  status = rfid.MIFARE_Read(balanceBlock, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Failed to read balance: ");
    Serial.println(rfid.GetStatusCodeName(status));
    rfid.PICC_HaltA();
    return;
  }

  String balanceStr = "";
  for (int i = 0; i < 16; i++) {
    if (buffer[i] != 0) balanceStr += (char)buffer[i];
  }

  float balance = balanceStr.toFloat();
  float deduction = 200.00;

  float oldBalance = balance;
  balance -= deduction;
  if (balance < 0) balance = 0;

  // Write new balance back to card
  byte newBalanceData[16] = {0};
  String newBalanceStr = String(balance, 2);
  newBalanceStr.getBytes(newBalanceData, sizeof(newBalanceData));

  status = rfid.MIFARE_Write(balanceBlock, newBalanceData, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Failed to write new balance: ");
    Serial.println(rfid.GetStatusCodeName(status));
    rfid.PICC_HaltA();
    return;
  }

  // Log transaction
  Serial.println("LOG: Plate=" + plate + ", Old=" + String(oldBalance, 2) +
                 ", Deducted=" + String(deduction, 2) +
                 ", New=" + String(balance, 2));

  rfid.PICC_HaltA();
  delay(5000); // Wait before next scan
}