// pinoeanut
#define BLE_REQ   10
#define BLE_RDY   2
#define BLE_RST   9

// define the same UUID in the client library
#define VMOTION_SERVICE_UUID "34a189eb3b604c6dbbbb5459a1da4ef5"
#define VMOTION_DATA_CH_UUID "93f8c6c16910496ea38f2e2f4fb2bf37"

#define VMOTION_CALIB_ACC_CH_UUID "8f9451b160e84ff4a2238bde735ef613"
#define VMOTION_CALIB_MAG_CH_UUID "c0cd1beedf214a59bf80cfd92e7a6c14"
#define VMOTION_CALIB_GYRO_CH_UUID "23344faec0f54befb36f641ab2633ac4"

#define MAX_CHARACTERISTICS_SIZE 20
#define MAXMIN_CALIB_REPORT_SIZE 12
#define OFFSET_CALIB_REPORT_SIZE 6

BLEPeripheral VMotionPeripheral = BLEPeripheral(BLE_REQ, BLE_RDY, BLE_RST);
BLEService VMotionService = BLEService(VMOTION_SERVICE_UUID);

BLEFixedLengthCharacteristic VMotionDataCh = BLEFixedLengthCharacteristic(VMOTION_DATA_CH_UUID, BLERead | BLENotify, MAX_CHARACTERISTICS_SIZE);

BLEFixedLengthCharacteristic VMotionCalibAccCh = BLEFixedLengthCharacteristic(VMOTION_CALIB_ACC_CH_UUID, BLERead | BLEWrite, MAXMIN_CALIB_REPORT_SIZE);
BLEFixedLengthCharacteristic VMotionCalibMagCh = BLEFixedLengthCharacteristic(VMOTION_CALIB_MAG_CH_UUID, BLERead | BLEWrite, MAXMIN_CALIB_REPORT_SIZE);
BLEFixedLengthCharacteristic VMotionCalibGyroCh = BLEFixedLengthCharacteristic(VMOTION_CALIB_GYRO_CH_UUID, BLERead | BLEWrite, OFFSET_CALIB_REPORT_SIZE);

boolean isPeripheralConnected = LOW;
boolean isDataChSubscribed = LOW;

//------- CALLBACKS -------//
void blePeripheralConnectHandler(BLECentral& central) {
  // central connected event handler
  //  Serial.print(F("Connected event, central: "));
  //  Serial.println(central.address());
  isPeripheralConnected = HIGH;
}

void blePeripheralDisconnectHandler(BLECentral& central) {
  // central disconnected event handler
  //  Serial.print(F("Disconnected event, central: "));
  //  Serial.println(central.address());
  isPeripheralConnected = LOW;
}

void characteristicSubscribed(BLECentral& central, BLECharacteristic& characteristic) {
  // characteristic subscribed event handler
  //  Serial.println(F("Characteristic event, subscribed"));
  if (characteristic.uuid() == VMOTION_DATA_CH_UUID)
  {
    isDataChSubscribed = HIGH;
  }
}

void characteristicUnsubscribed(BLECentral& central, BLECharacteristic& characteristic) {
  // characteristic unsubscribed event handler
  //  Serial.println(F("Characteristic event, unsubscribed"));
  if (characteristic.uuid() == VMOTION_DATA_CH_UUID)
  {
    isDataChSubscribed = LOW;
  }
}

void characteristicWritten(BLECentral& central, BLECharacteristic& characteristic) {
  // characteristic value written event handler
  //  Serial.print(F("Characteristic event, writen: "));
  //  Serial.println(testCharacteristic.value(), DEC);

  // we receive new calibration data -> store in EEPROM and local vars

}

//------- BLUETOOTH METHODS -------//
void bleInit()
{
  VMotionPeripheral.setLocalName("vmotion");
  VMotionPeripheral.setAdvertisedServiceUuid(VMotionService.uuid());

  //TODO: manufacturing data

  VMotionPeripheral.setDeviceName("vmotion");

  //TODO: appearance

  VMotionPeripheral.addAttribute(VMotionService);
  VMotionPeripheral.addAttribute(VMotionDataCh);
  VMotionPeripheral.addAttribute(VMotionCalibAccCh);
  VMotionPeripheral.addAttribute(VMotionCalibMagCh);
  VMotionPeripheral.addAttribute(VMotionCalibGyroCh);

  VMotionPeripheral.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  VMotionPeripheral.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  VMotionDataCh.setEventHandler(BLESubscribed, characteristicSubscribed);
  VMotionDataCh.setEventHandler(BLEUnsubscribed, characteristicUnsubscribed);

  VMotionCalibAccCh.setEventHandler(BLEWritten, characteristicWritten);
  VMotionCalibMagCh.setEventHandler(BLEWritten, characteristicWritten);
  VMotionCalibGyroCh.setEventHandler(BLEWritten, characteristicWritten);

  VMotionCalibAccCh.setValue((byte*)accCalib, MAXMIN_CALIB_REPORT_SIZE);
  VMotionCalibMagCh.setValue((byte*)magCalib, MAXMIN_CALIB_REPORT_SIZE);
  VMotionCalibGyroCh.setValue((byte*)gyroCalib, OFFSET_CALIB_REPORT_SIZE);

}

void bleUpdateState()
{
  VMotionDataCh.setValue((byte*)sensorData, MAX_CHARACTERISTICS_SIZE);
}


