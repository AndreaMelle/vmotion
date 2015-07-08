void load_calibration_data()
{ 
  //TODO: now from the defines, later from EEPROM
  acc_offset[0] = (DEFAULT_ACCEL_MIN + DEFAULT_ACCEL_MAX) / 2.0f;
  acc_offset[1] = (DEFAULT_ACCEL_MIN + DEFAULT_ACCEL_MAX) / 2.0f;
  acc_offset[2] = (DEFAULT_ACCEL_MIN + DEFAULT_ACCEL_MAX) / 2.0f;

  acc_scale[0] = GRAVITY / (DEFAULT_ACCEL_MAX - acc_offset[0]);
  acc_scale[1] = GRAVITY / (DEFAULT_ACCEL_MAX - acc_offset[1]);
  acc_scale[2] = GRAVITY / (DEFAULT_ACCEL_MAX - acc_offset[2]);

  mag_offset[0] = (DEFAULT_MAGN_MIN + DEFAULT_MAGN_MAX) / 2.0f;
  mag_offset[1] = (DEFAULT_MAGN_MIN + DEFAULT_MAGN_MAX) / 2.0f;
  mag_offset[2] = (DEFAULT_MAGN_MIN + DEFAULT_MAGN_MAX) / 2.0f;

  mag_scale[0] = 100.0f / (DEFAULT_MAGN_MAX - mag_offset[0]);
  mag_scale[1] = 100.0f / (DEFAULT_MAGN_MAX - mag_offset[1]);
  mag_scale[2] = 100.0f / (DEFAULT_MAGN_MAX - mag_offset[2]);

  gyro_offset[0] = DEFAULT_GYRO_AVERAGE_OFFSET;
  gyro_offset[1] = DEFAULT_GYRO_AVERAGE_OFFSET;
  gyro_offset[2] = DEFAULT_GYRO_AVERAGE_OFFSET;
}

void save_calibration_data()
{
  // TODO: save to EEPROM
}

// Apply calibration to raw sensor readings
void compensate_sensor_errors() {
  // Compensate accelerometer error
  accel[0] = (accel[0] - acc_offset[0]) * acc_scale[0];
  accel[1] = (accel[1] - acc_offset[1]) * acc_scale[1];
  accel[2] = (accel[2] - acc_offset[2]) * acc_scale[2];

  // Compensate magnetometer error
#if CALIBRATION__MAGN_USE_EXTENDED == true
  for (int i = 0; i < 3; i++)
    magnetom_tmp[i] = magnetom[i] - magn_ellipsoid_center[i];
  Matrix_Vector_Multiply(magn_ellipsoid_transform, magnetom_tmp, magnetom);
#else
  magnetom[0] = (magnetom[0] - mag_offset[0]) * mag_scale[0];
  magnetom[1] = (magnetom[1] - mag_offset[1]) * mag_scale[1];
  magnetom[2] = (magnetom[2] - mag_offset[2]) * mag_scale[2];
#endif

  // Compensate gyroscope error
  gyro[0] -= gyro_offset[0];
  gyro[1] -= gyro_offset[1];
  gyro[2] -= gyro_offset[2];
}
