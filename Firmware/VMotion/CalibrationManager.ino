void load_calibration_data()
{
  //TODO: now from the defines, later from EEPROM

  accCalib[0] = DEFAULT_ACCEL_MIN;
  accCalib[1] = DEFAULT_ACCEL_MIN;
  accCalib[2] = DEFAULT_ACCEL_MIN;

  accCalib[3] = DEFAULT_ACCEL_MAX;
  accCalib[4] = DEFAULT_ACCEL_MAX;
  accCalib[5] = DEFAULT_ACCEL_MAX;

  magCalib[0] = DEFAULT_MAGN_MIN;
  magCalib[1] = DEFAULT_MAGN_MIN;
  magCalib[2] = DEFAULT_MAGN_MIN;

  magCalib[3] = DEFAULT_MAGN_MAX;
  magCalib[4] = DEFAULT_MAGN_MAX;
  magCalib[5] = DEFAULT_MAGN_MAX;

  gyroCalib[0] = DEFAULT_GYRO_AVERAGE_OFFSET;
  gyroCalib[1] = DEFAULT_GYRO_AVERAGE_OFFSET;
  gyroCalib[2] = DEFAULT_GYRO_AVERAGE_OFFSET;
}

void save_calibration_data()
{
  // TODO: save to EEPROM
}
