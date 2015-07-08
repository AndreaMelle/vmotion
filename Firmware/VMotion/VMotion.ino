#include <Wire.h>
#include <SPI.h>
#include <BLEPeripheral.h>
#include "vmotion_defines.h"
#include "vmotion_dcm.h"
#include "vmotion_ble.h"

int output_mode = VMOTION_YPRB_MODE; // Startup output mode

// Sensor variables
float accel[3];  // Actually stores the NEGATED acceleration (equals gravity, if board not moving).
float magnetom[3];
float gyro[3];

// Euler angles
float yaw, pitch, roll;

// Sensor calibration scale and offset values
float acc_offset[3];
float acc_scale[3];
float mag_offset[3];
float mag_scale[3];
float gyro_offset[3];

// Button variables
boolean lastButtonState = LOW;
boolean currentButtonState = LOW;
long lastDebounceTime = 0;

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
  // write mode characteristics - will listen to client's writes
  // write calibration characteristics - will listen to client's write
  // init data characteristics - will wait for notify subscription to set new values each frame

  // Read sensors, init DCM algorithm
  delay(20);  // Give sensors enough time to collect data
  reset_sensor_fusion();

  // bluetooth begin
  
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
    }
  }

  lastButtonState = buttonReading;

  // Time to read the sensors again?
  if ((millis() - timestamp) >= OUTPUT__DATA_INTERVAL)
  {
    timestamp_old = timestamp;
    timestamp = millis();
    if (timestamp > timestamp_old)
    {
      G_Dt = (float) (timestamp - timestamp_old) / 1000.0f; // Real time of loop run. We use this on the DCM algorithm (gyro integration time)
    }
    else
    {
      G_Dt = 0;
    }

    // Update sensor readings
    read_sensors();

    if (output_mode == VMOTION_YPRB_MODE)  // We're in calibration mode
    {
      // output YPRB
      // Apply sensor calibration
      compensate_sensor_errors();

      // Run DCM algorithm
      Compass_Heading(); // Calculate magnetic heading
      Matrix_update();
      Normalize();
      Drift_correction();
      Euler_angles();

      //output_anglesbutton();

    }
    else if (output_mode == VMOTION_SENSOR_RAW_MODE)
    {
      //output_sensors();
    }
  }
}
