// pinout
#define BLE_REQ   10
#define BLE_RDY   2
#define BLE_RST   9

// define the same UUID in the client library
#define VMOTION_SERVICE_UUID "34a189eb3b604c6dbbbb5459a1da4ef5"
#define VMOTION_DATA_CH_UUID "93f8c6c16910496ea38f2e2f4fb2bf37"
#define VMOTION_MODE_CH_UUID "1e97874e068f4c9bb20dbfaa952d3e87"

#define VMOTION_CALIB_ACC_CH_UUID "8f9451b160e84ff4a2238bde735ef613"
#define VMOTION_CALIB_MAG_CH_UUID "c0cd1beedf214a59bf80cfd92e7a6c14"
#define VMOTION_CALIB_GYRO_CH_UUID "23344faec0f54befb36f641ab2633ac4"

#define MAX_CHARACTERISTICS_SIZE 20
#define CALIB_REPORT_CH_SIZE 12

BLEPeripheral VMotionPeripheral = BLEPeripheral(BLE_REQ, BLE_RDY, BLE_RST);
BLEService VMotionService = BLEService(VMOTION_SERVICE_UUID);

BLEFixedLengthCharacteristic VMotionDataCh = BLEFixedLengthCharacteristic(VMOTION_DATA_CH_UUID, BLERead | BLENotify, MAX_CHARACTERISTICS_SIZE);
BLECharCharacteristic VMotionModeCh = BLECharCharacteristic(VMOTION_MODE_CH_UUID, BLERead | BLEWrite);

BLEFixedLengthCharacteristic VMotionCalibAccCh = BLEFixedLengthCharacteristic(VMOTION_CALIB_ACC_CH_UUID, BLERead | BLEWrite, CALIB_REPORT_CH_SIZE);
BLEFixedLengthCharacteristic VMotionCalibMagCh = BLEFixedLengthCharacteristic(VMOTION_CALIB_MAG_CH_UUID, BLERead | BLEWrite, CALIB_REPORT_CH_SIZE);
BLEFixedLengthCharacteristic VMotionCalibGyroCh = BLEFixedLengthCharacteristic(VMOTION_CALIB_GYRO_CH_UUID, BLERead | BLEWrite, CALIB_REPORT_CH_SIZE);

//------- CALLBACKS -------//
void blePeripheralConnectHandler(BLECentral& central) {
  // central connected event handler
//  Serial.print(F("Connected event, central: "));
//  Serial.println(central.address());
}

void blePeripheralDisconnectHandler(BLECentral& central) {
  // central disconnected event handler
//  Serial.print(F("Disconnected event, central: "));
//  Serial.println(central.address());
}

void characteristicSubscribed(BLECentral& central, BLECharacteristic& characteristic) {
  // characteristic subscribed event handler
//  Serial.println(F("Characteristic event, subscribed"));
}

void characteristicUnsubscribed(BLECentral& central, BLECharacteristic& characteristic) {
  // characteristic unsubscribed event handler
//  Serial.println(F("Characteristic event, unsubscribed"));
}
