#include <Wire.h>
#include <SPI.h>
#include <BLEPeripheral.h>
#include "vmotion_defines.h"

// Raw sensor data - this is all we send!
word sensorData[10];  // acc mag gyro. Acc actually stores the NEGATED acceleration (equals gravity, if board not moving).

// Sensor calibration scale and offset values
word accCalib[6]; //accMin accMax
word magCalib[6]; //magMin magMax
word gyroCalib[3]; //offset

#include "vmotion_ble.h"

// Button variables
boolean lastButtonState = LOW;
boolean currentButtonState = LOW;
unsigned long lastDebounceTime = 0;

unsigned long timestamp;
unsigned long timestamp_old;

void reset_sensors() {
  read_sensors();
  timestamp = millis();
}

void setup()
{
  Serial.begin(OUTPUT__BAUD_RATE);

  // Init status LED
  pinMode (STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  pinMode(BUTTON_PIN, INPUT); // init button pin

  load_calibration_data(); //load calibration data

  // Init sensors
  delay(50);  // Give sensors enough time to start
  I2C_Init();
  Accel_Init();
  Magn_Init();
  Gyro_Init();

  //Init bluetooth stack
  bleInit();

  // bluetooth begin
  VMotionPeripheral.begin();

  // Read sensors, init DCM algorithm
  delay(20);  // Give sensors enough time to collect data
  sensorData[9] = (word)currentButtonState;
  reset_sensors();
}

void loop()
{
  //Update button readings
  int buttonReading = digitalRead(BUTTON_PIN);

  if (buttonReading != lastButtonState)
  {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY)
  {
    if (buttonReading != currentButtonState)
    {
      currentButtonState = buttonReading;
      sensorData[9] = (word)currentButtonState;
    }
  }

  lastButtonState = buttonReading;

  // Time to read the sensors again?
  if ((millis() - timestamp) >= OUTPUT__DATA_INTERVAL)
  {
    timestamp_old = timestamp;
    timestamp = millis();

    // Update sensor readings
    read_sensors();

    // update characteristics
    if (isPeripheralConnected)
    {
      bleUpdateState();
    }
  }
}
