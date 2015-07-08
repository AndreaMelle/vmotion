//
//  VMotionController.m
//  VMotion
//
//  Created by Andrea Melle on 06/07/2015.
//  Copyright (c) 2015 Andrea Melle. All rights reserved.
//

#import "VMotionController.h"

NSString *const VMOTION_SERVICE_UUID = @"";
NSString *const VMOTION_DATA_CH_UUID = @"";
NSString *const VMOTION_MODE_CH_UUID = @"";
NSString *const VMOTION_CALIB_ACC_CH_UUID = @"";
NSString *const VMOTION_CALIB_MAG_CH_UUID = @"";
NSString *const VMOTION_CALIB_GYRO_CH_UUID = @"";

VMotionControllerMode const VMOTION_MODE_DEFAULT = VMOTION_YPRB_MODE;

@implementation VMotionController
{
    @private
    CBPeripheral *peripheral;
    
    //characteristics
    CBCharacteristic *modeCh;
    CBCharacteristic *dataCh;
    CBCharacteristic *accCalibCh;
    CBCharacteristic *magCalibCh;
    CBCharacteristic *gyroCalibCh;
    
    // data reports
    float ypr[3]; //makes sense only in VMOTION_YPRB_MODE data is 3 x 4 bytes (float)
    BOOL button; //makes sense only in VMOTION_YPRB_MODE data is 1 bytes (char)
    
    float acc_raw[3]; //makes sense only in VMOTION_SENSOR_RAW_MODE data is 3 x 2 bytes (int16_t)
    float mag_raw[3]; //makes sense only in VMOTION_SENSOR_RAW_MODE data is 3 x 2 bytes (int16_t)
    float gyro_raw[3]; //makes sense only in VMOTION_SENSOR_RAW_MODE data is 3 x 2 bytes (int16_t)
    
    float acc_calibrated[3]; //computed on client side (float)
    float mag_calibrated[3]; //computed on client side (float)
    float gyro_calibrated[3]; //computed on client side (float)
    
    float acc_calibration_report[6]; //max min on 3 axis -> 3 x 2 x 2 = 12 bytes (int16_t)
    float mag_calibration_report[6]; //max min on 3 axis -> 3 x 2 x 2 = 12 bytes (int16_t)
    float gyro_calibration_report[3]; //offset on 3 axis -> 3 x 4 = 12 bytes (float)
    
}

@synthesize peripheral = _peripheral;
@synthesize mode = _mode;

- (instancetype)init
{
    @throw [NSException exceptionWithName:NSInternalInconsistencyException reason:@"init not a valid initializer for class" userInfo:nil];
    return nil;
}

- (instancetype)initWithPeripheral:(CBPeripheral*)aPeripheral
{
    self = [super init];
    
    if(self)
    {
        _peripheral = aPeripheral;
        [_peripheral setDelegate:self];
        [peripheral discoverServices:nil];
    }
    
    return self;
}

- (void)dealloc
{
    [_peripheral setDelegate:nil];
    _peripheral = nil;
}

- (void)setMode:(VMotionControllerMode)mode
{
    if(_mode != mode && modeCh != NULL)
    {
        uint8_t val = 0;
        
        if(mode == VMOTION_YPRB_MODE)
        {
            val = VMOTION_YPRB_MODE;
        }
        else if(mode == VMOTION_SENSOR_RAW_MODE || mode == VMOTION_SENSOR_CALIBRATED_MODE)
        {
            val = VMOTION_SENSOR_RAW_MODE;
        }
        else
        {
            return;
        }
        
        _mode = mode;
        NSData* valData = [NSData dataWithBytes:(void*)&val length:sizeof(val)];
        [peripheral writeValue:valData forCharacteristic:modeCh type:CBCharacteristicWriteWithResponse];
        
    }
}

#pragma mark - CBPeripheral delegate methods
/*
 Invoked upon completion of a -[discoverServices:] request.
 Discover available characteristics on interested services
 */
- (void) peripheral:(CBPeripheral *)aPeripheral didDiscoverServices:(NSError *)error
{
    for (CBService *aService in aPeripheral.services)
    {
        NSLog(@"Service found with UUID: %@", aService.UUID);
        
        /* VMotion Service */
        if ([aService.UUID isEqual:[CBUUID UUIDWithString:VMOTION_SERVICE_UUID]])
        {
            [aPeripheral discoverCharacteristics:nil forService:aService];
        }

//        /* Device Information Service */
//        if ([aService.UUID isEqual:[CBUUID UUIDWithString:@"180A"]])
//        {
//            [aPeripheral discoverCharacteristics:nil forService:aService];
//        }
//        
//        /* GAP (Generic Access Profile) for Device Name */
//        if ( [aService.UUID isEqual:[CBUUID UUIDWithString:CBUUIDGenericAccessProfileString]] )
//        {
//            [aPeripheral discoverCharacteristics:nil forService:aService];
//        }
    }
}

/*
 Invoked upon completion of a -[discoverCharacteristics:forService:] request.
 Perform appropriate operations on interested characteristics
 */
- (void) peripheral:(CBPeripheral *)aPeripheral didDiscoverCharacteristicsForService:(CBService *)service error:(NSError *)error
{
    if ([service.UUID isEqual:[CBUUID UUIDWithString:VMOTION_SERVICE_UUID]])
    {
        for (CBCharacteristic *aChar in service.characteristics)
        {
            /* Set notification on data */
            if ([aChar.UUID isEqual:[CBUUID UUIDWithString:VMOTION_DATA_CH_UUID]])
            {
                dataCh = aChar;
                [peripheral setNotifyValue:YES forCharacteristic:aChar];
                NSLog(@"Found Data Characteristic");
            }
            
            /* Read initial calibration data */
            if ([aChar.UUID isEqual:[CBUUID UUIDWithString:VMOTION_CALIB_ACC_CH_UUID]])
            {
                accCalibCh = aChar;
                [aPeripheral readValueForCharacteristic:aChar];
                NSLog(@"Found Accelerometer Calibration Characteristic");
            }
            
            if ([aChar.UUID isEqual:[CBUUID UUIDWithString:VMOTION_CALIB_MAG_CH_UUID]])
            {
                magCalibCh = aChar;
                [aPeripheral readValueForCharacteristic:aChar];
                NSLog(@"Found Magnetometer Calibration Characteristic");
            }
            
            if ([aChar.UUID isEqual:[CBUUID UUIDWithString:VMOTION_CALIB_GYRO_CH_UUID]])
            {
                gyroCalibCh = aChar;
                [aPeripheral readValueForCharacteristic:aChar];
                NSLog(@"Found Gyroscope Calibration Characteristic");
            }
            
            /* Write default data mode */
            if ([aChar.UUID isEqual:[CBUUID UUIDWithString:VMOTION_MODE_CH_UUID]])
            {
                modeCh = aChar;
                [self setMode:VMOTION_MODE_DEFAULT];
                NSLog(@"Found Mode Characteristic. Setting to defualt.");
            }
        }
    }
//
//    if ( [service.UUID isEqual:[CBUUID UUIDWithString:CBUUIDGenericAccessProfileString]] )
//    {
//        for (CBCharacteristic *aChar in service.characteristics)
//        {
//            /* Read device name */
//            if ([aChar.UUID isEqual:[CBUUID UUIDWithString:CBUUIDDeviceNameString]])
//            {
//                [aPeripheral readValueForCharacteristic:aChar];
//                NSLog(@"Found a Device Name Characteristic");
//            }
//        }
//    }
//    
//    if ([service.UUID isEqual:[CBUUID UUIDWithString:@"180A"]])
//    {
//        for (CBCharacteristic *aChar in service.characteristics)
//        {
//            /* Read manufacturer name */
//            if ([aChar.UUID isEqual:[CBUUID UUIDWithString:@"2A29"]])
//            {
//                [aPeripheral readValueForCharacteristic:aChar];
//                NSLog(@"Found a Device Manufacturer Name Characteristic");
//            }
//        }
//    }
}

/*
 Invoked upon completion of a -[readValueForCharacteristic:] request or on the reception of a notification/indication.
 */
- (void) peripheral:(CBPeripheral *)aPeripheral didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error
{
//    /* Updated value for heart rate measurement received */
//    if ([characteristic.UUID isEqual:[CBUUID UUIDWithString:@"2A37"]])
//    {
//        if( (characteristic.value)  || !error )
//        {
//            /* Update UI with heart rate data */
//            [self updateWithHRMData:characteristic.value];
//        }
//    }
//    /* Value for body sensor location received */
//    else  if ([characteristic.UUID isEqual:[CBUUID UUIDWithString:@"2A38"]])
//    {
//        NSData * updatedValue = characteristic.value;
//        uint8_t* dataPointer = (uint8_t*)[updatedValue bytes];
//        if(dataPointer)
//        {
//            uint8_t location = dataPointer[0];
//            NSString*  locationString;
//            switch (location)
//            {
//                case 0:
//                    locationString = @"Other";
//                    break;
//                case 1:
//                    locationString = @"Chest";
//                    break;
//                case 2:
//                    locationString = @"Wrist";
//                    break;
//                case 3:
//                    locationString = @"Finger";
//                    break;
//                case 4:
//                    locationString = @"Hand";
//                    break;
//                case 5:
//                    locationString = @"Ear Lobe";
//                    break;
//                case 6:
//                    locationString = @"Foot";
//                    break;
//                default:
//                    locationString = @"Reserved";
//                    break;
//            }
//            NSLog(@"Body Sensor Location = %@ (%d)", locationString, location);
//        }
//    }
//    /* Value for device Name received */
//    else if ([characteristic.UUID isEqual:[CBUUID UUIDWithString:CBUUIDDeviceNameString]])
//    {
//        NSString * deviceName = [[[NSString alloc] initWithData:characteristic.value encoding:NSUTF8StringEncoding] autorelease];
//        NSLog(@"Device Name = %@", deviceName);
//    }
//    /* Value for manufacturer name received */
//    else if ([characteristic.UUID isEqual:[CBUUID UUIDWithString:@"2A29"]])
//    {
//        self.manufacturer = [[[NSString alloc] initWithData:characteristic.value encoding:NSUTF8StringEncoding] autorelease];
//        NSLog(@"Manufacturer Name = %@", self.manufacturer);
//    }
}

- (void)peripheral:(CBPeripheral *)peripheral didWriteValueForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error
{
    
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverDescriptorsForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error
{
    
}

@end
