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


// SENSOR CALIBRATION
/*****************************************************************/
// Accelerometer
// "accel x,y,z (min/max) = X_MIN/X_MAX  Y_MIN/Y_MAX  Z_MIN/Z_MAX"
#define DEFAULT_ACCEL_MIN ((word) -250)
#define DEFAULT_ACCEL_MAX ((word) 250)

// Magnetometer (standard calibration mode)
// "magn x,y,z (min/max) = X_MIN/X_MAX  Y_MIN/Y_MAX  Z_MIN/Z_MAX"
#define DEFAULT_MAGN_MIN ((word) -600)
#define DEFAULT_MAGN_MAX ((word) 600)

// Magnetometer (extended calibration mode)
// Uncommend to use extended magnetometer calibration (compensates hard & soft iron errors)
//#define CALIBRATION__MAGN_USE_EXTENDED true
//const float magn_ellipsoid_center[3] = {0, 0, 0};
//const float magn_ellipsoid_transform[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

// Gyroscope
// "gyro x,y,z (current/average) = .../OFFSET_X  .../OFFSET_Y  .../OFFSET_Z
#define DEFAULT_GYRO_AVERAGE_OFFSET ((word) 0.0)

// Stuff
#define STATUS_LED_PIN 13  // Pin number of status LED
#define GRAVITY 256.0f // "1G reference" used for DCM filter and accelerometer calibration

#define BUTTON_PIN 2
#define DEBOUNCE_DELAY 20 //ms

#endif //__VMOTION_DEFINES_H__
