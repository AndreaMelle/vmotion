//
//  VMotionController.h
//  VMotion
//
//  Created by Andrea Melle on 06/07/2015.
//  Copyright (c) 2015 Andrea Melle. All rights reserved.
//

#import <Foundation/Foundation.h>
@import CoreBluetooth;
@import QuartzCore;

FOUNDATION_EXPORT NSString *const VMOTION_SERVICE_UUID;
FOUNDATION_EXPORT NSString *const VMOTION_DATA_CH_UUID; // read-only
FOUNDATION_EXPORT NSString *const VMOTION_MODE_CH_UUID; //read/write

FOUNDATION_EXPORT NSString *const VMOTION_CALIB_ACC_CH_UUID; //read/write
FOUNDATION_EXPORT NSString *const VMOTION_CALIB_MAG_CH_UUID; //read/write
FOUNDATION_EXPORT NSString *const VMOTION_CALIB_GYRO_CH_UUID; //read/write

typedef enum {
    VMOTION_YPRB_MODE, // reports float[3] yaw, pitch, roll + bool button state
    VMOTION_SENSOR_RAW_MODE, // 9DOF data, 2byte / axis, not compensated for calibration
    VMOTION_SENSOR_CALIBRATED_MODE, // 9DOF data compensated for calibration. do we compute this client side?
} VMotionControllerMode;

FOUNDATION_EXPORT VMotionControllerMode const VMOTION_MODE_DEFAULT;

@interface VMotionController : NSObject <CBPeripheralDelegate>

@property (nonatomic, readonly) CBPeripheral* peripheral;
@property (nonatomic) VMotionControllerMode mode;

- (instancetype)initWithPeripheral:(CBPeripheral*)aPeripheral;


@end
