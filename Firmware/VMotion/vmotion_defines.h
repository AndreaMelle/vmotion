#ifndef __VMOTION_DEFINES_H__
#define __VMOTION_DEFINES_H__

// HARDWARE OPTIONS - Select your hardware here by uncommenting one line
/*****************************************************************/
//#define HW__VERSION_CODE 10125 // SparkFun "9DOF Razor IMU" version "SEN-10125" (HMC5843 magnetometer)
//#define HW__VERSION_CODE 10736 // SparkFun "9DOF Razor IMU" version "SEN-10736" (HMC5883L magnetometer)
//#define HW__VERSION_CODE 10183 // SparkFun "9DOF Sensor Stick" version "SEN-10183" (HMC5843 magnetometer)
//#define HW__VERSION_CODE 10321 // SparkFun "9DOF Sensor Stick" version "SEN-10321" (HMC5843 magnetometer)
#define HW__VERSION_CODE 10724 // SparkFun "9DOF Sensor Stick" version "SEN-10724" (HMC5883L magnetometer)

// OUTPUT OPTIONS - Set your serial port baud rate used to send out data here
/*****************************************************************/
#define OUTPUT__BAUD_RATE 57600
#define OUTPUT__DATA_INTERVAL 20  // in milliseconds. Do not change

// Output mode definitions - same number of client api
#define VMOTION_YPRB_MODE 0 // Outputs yaw/pitch/roll/button state. Angles in radians. (13  bytes)
#define VMOTION_SENSOR_RAW_MODE 1 //Outputs 9DOF 2 bytes per axis (18 bytes)

// SENSOR CALIBRATION
/*****************************************************************/
// Accelerometer
// "accel x,y,z (min/max) = X_MIN/X_MAX  Y_MIN/Y_MAX  Z_MIN/Z_MAX"
#define DEFAULT_ACCEL_MIN ((float) -250)
#define DEFAULT_ACCEL_MAX ((float) 250)

// Magnetometer (standard calibration mode)
// "magn x,y,z (min/max) = X_MIN/X_MAX  Y_MIN/Y_MAX  Z_MIN/Z_MAX"
#define DEFAULT_MAGN_MIN ((float) -600)
#define DEFAULT_MAGN_MAX ((float) 600)

// Magnetometer (extended calibration mode)
// Uncommend to use extended magnetometer calibration (compensates hard & soft iron errors)
//#define CALIBRATION__MAGN_USE_EXTENDED true
//const float magn_ellipsoid_center[3] = {0, 0, 0};
//const float magn_ellipsoid_transform[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

// Gyroscope
// "gyro x,y,z (current/average) = .../OFFSET_X  .../OFFSET_Y  .../OFFSET_Z
#define DEFAULT_GYRO_AVERAGE_OFFSET ((float) 0.0)

// Gain for gyroscope (ITG-3200)
#define GYRO_GAIN 0.06957 // Same gain on all axes
#define GYRO_SCALED_RAD(x) (x * TO_RAD(GYRO_GAIN)) // Calculate the scaled gyro readings in radians per second

// DCM parameters
#define Kp_ROLLPITCH 0.02f
#define Ki_ROLLPITCH 0.00002f
#define Kp_YAW 1.2f
#define Ki_YAW 0.00002f

// Stuff
#define STATUS_LED_PIN 13  // Pin number of status LED
#define GRAVITY 256.0f // "1G reference" used for DCM filter and accelerometer calibration
#define TO_RAD(x) (x * 0.01745329252)  // *pi/180
#define TO_DEG(x) (x * 57.2957795131)  // *180/pi

// More output-state variables
//int num_accel_errors = 0;
//int num_magn_errors = 0;
//int num_gyro_errors = 0;

#define BUTTON_PIN 2
#define DEBOUNCE_DELAY 20 //ms

#endif //__VMOTION_DEFINES_H__
