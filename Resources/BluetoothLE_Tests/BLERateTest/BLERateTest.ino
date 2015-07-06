/***************************************************************************************************************
* Test BLE transmission rate with similar data to VMotion sensor
* Test BLE configuration as well
***************************************************************************************************************/

#include <SPI.h>
#include <BLEPeripheral.h>

#define BLE_REQ   10
#define BLE_RDY   2
#define BLE_RST   9

#define SERV_UUID "4217f675257a451da611ea4544e0d953"
#define YPR_UUID "d9fb13a41739402f8abff9cb9505408d"
#define BTN_UUID "de9d16d3d86f4a57a6c25238bc571213"
#define YPR_SIZE 12
//#define BTN_SIZE 1

unsigned long long lastSent = 0;

// create peripheral
BLEPeripheral vmotionP = BLEPeripheral(BLE_REQ, BLE_RDY, BLE_RST);

// create service
//
//42479282-4288-4970-827a-1967aefd7b25
//8da9cbb5-3798-4688-8c32-6787d6565d0d
//81bc0c24-3a5d-48b3-9f79-9c7a8b983253
BLEService vmotionS = BLEService(SERV_UUID);

// create charactersitics
// TODO: test write characteristics: motor, sensor modes, calibration...
// MAX 20 characters on nRF8001
BLEFixedLengthCharacteristic yprC = BLEFixedLengthCharacteristic(YPR_UUID, BLERead | BLENotify, YPR_SIZE);
BLECharCharacteristic btnC = BLECharCharacteristic(BTN_UUID, BLERead | BLENotify);

BLEDescriptor yprD = BLEDescriptor(YPR_UUID, "ypr");
BLEDescriptor btnD = BLEDescriptor(BTN_UUID, "btn");

// fake data
float ypr[3];
boolean btn;

void setup()
{
  Serial.begin(9600);
#if defined (__AVR_ATmega32U4__)
  delay(5000);  //5 seconds delay for enabling to see the start up comments on the serial board
#endif

  vmotionP.setLocalName("vmotion");
  vmotionP.setAdvertisedServiceUuid(vmotionS.uuid());

  // do we need to set manufacturer data?

  vmotionP.setDeviceName("vmotion");
  // what is appearance?

  vmotionP.addAttribute(vmotionS);
  vmotionP.addAttribute(yprC);
  vmotionP.addAttribute(yprD);
  vmotionP.addAttribute(btnC);
  vmotionP.addAttribute(btnD);

  // assign event handlers for connected, disconnected to peripheral
  vmotionP.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  vmotionP.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  // assign event handlers for characteristic
  yprC.setEventHandler(BLESubscribed, characteristicSubscribed);
  yprC.setEventHandler(BLEUnsubscribed, characteristicUnsubscribed);

  btnC.setEventHandler(BLESubscribed, characteristicSubscribed);
  btnC.setEventHandler(BLEUnsubscribed, characteristicUnsubscribed);

  // init fake data
  ypr[0] = 0.1;
  ypr[1] = 0.2;
  ypr[2] = 0.3;
  btn = LOW;

  yprC.setValue((byte*)ypr, YPR_SIZE);
  btnC.setValue((char)btn);

  vmotionP.begin();

  Serial.println(F("VMotion Test Peripheral"));
}

void loop()
{
  BLECentral central = vmotionP.central();

  if (central)
  {
    // central connected to peripheral
    Serial.print(F("Connected to central: "));
    Serial.println(central.address());

    while (central.connected())
    {
      if (millis() > 20 && (millis() - 20) > lastSent) {
        // atleast 20 ms have passed since last increment
        lastSent = millis();

        ypr[0] += 0.1;
        ypr[1] -= 0.1;
        btn = !btn;
        
        yprC.setValue((byte*)ypr, YPR_SIZE);
        btnC.setValue((char)btn);

      }
    }

    // central disconnected
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}

//------- CALLBACKS -------//
void blePeripheralConnectHandler(BLECentral& central) {
  // central connected event handler
  Serial.print(F("Connected event, central: "));
  Serial.println(central.address());
}

void blePeripheralDisconnectHandler(BLECentral& central) {
  // central disconnected event handler
  Serial.print(F("Disconnected event, central: "));
  Serial.println(central.address());
}

void characteristicSubscribed(BLECentral& central, BLECharacteristic& characteristic) {
  // characteristic subscribed event handler
  Serial.println(F("Characteristic event, subscribed"));
}

void characteristicUnsubscribed(BLECentral& central, BLECharacteristic& characteristic) {
  // characteristic unsubscribed event handler
  Serial.println(F("Characteristic event, unsubscribed"));
}

//END CALLBACKS



