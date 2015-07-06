//
//  BLEManager.m
//  BLESample
//
//  Created by Andrea Melle on 05/07/2015.
//  Copyright (c) 2015 Andrea Melle. All rights reserved.
//

#import "BLEManager.h"

@implementation BLEManager
{
    @private
    dispatch_queue_t mBleDispatchQueue;
}

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        mBleDispatchQueue = dispatch_queue_create("com.blesample.blecentralmanager", DISPATCH_QUEUE_SERIAL);
        _centralManager = [[CBCentralManager alloc] initWithDelegate:self queue:mBleDispatchQueue];
    }
    
    return self;
}

- (void)scan
{
    [_centralManager scanForPeripheralsWithServices:nil options:nil];
}

#pragma mark - CBCentralManagerDelegate

- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral
{
    [peripheral setDelegate:self];
    [peripheral discoverServices:nil];
    self.connected = [NSString stringWithFormat:@"Connected: %@", peripheral.state == CBPeripheralStateConnected ? @"YES" : @"NO"];
    NSLog(@"%@", self.connected);
}

- (void)centralManager:(CBCentralManager *)central didDisconnectPeripheral:(CBPeripheral *)peripheral error:(NSError *)error
{
    
}

- (void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)peripheral advertisementData:(NSDictionary *)advertisementData RSSI:(NSNumber *)RSSI
{
    
    
    NSString* localName = [advertisementData objectForKey:CBAdvertisementDataLocalNameKey];
    NSString* manufacturer = [advertisementData objectForKey:CBAdvertisementDataManufacturerDataKey];
    NSString* dataServiceData = [advertisementData objectForKey:CBAdvertisementDataServiceDataKey];
    NSString* dataServiceUUID = [advertisementData objectForKey:CBAdvertisementDataServiceUUIDsKey];
    
    NSString* bleReport = [NSString stringWithFormat:@"  Local Name: %@\n  Manufacturer: %@\n  Data Service Data: %@\n  Data Service UUID: %@\n", localName, manufacturer, dataServiceData, dataServiceUUID];
    
    //dispatch_async(dispatch_get_main_queue(), ^() {
        
        
        NSLog(@"Peripheral %@ Report:\n%@", peripheral.name, bleReport);
    //});
    
//    NSString *const CBAdvertisementDataOverflowServiceUUIDsKey;
//    NSString *const CBAdvertisementDataTxPowerLevelKey;
//    NSString *const CBAdvertisementDataIsConnectable;
//    NSString *const CBAdvertisementDataSolicitedServiceUUIDsKey;
    
    [self.centralManager stopScan];
    self.pheriperal = peripheral;
    peripheral.delegate = self;
    [self.centralManager connectPeripheral:peripheral options:nil];
    
}

- (void)centralManagerDidUpdateState:(CBCentralManager *)central
{
    if ([central state] == CBCentralManagerStatePoweredOff)
    {
        dispatch_async(dispatch_get_main_queue(), ^(){
            NSLog(@"CoreBluetooth BLE hardware is powered off");
        });
    }
    else if ([central state] == CBCentralManagerStatePoweredOn) {
        dispatch_async(dispatch_get_main_queue(), ^(){
            NSLog(@"CoreBluetooth BLE hardware is powered on and ready");
        });
    }
    else if ([central state] == CBCentralManagerStateUnauthorized) {
        dispatch_async(dispatch_get_main_queue(), ^(){
            NSLog(@"CoreBluetooth BLE state is unauthorized");
        });
    }
    else if ([central state] == CBCentralManagerStateUnknown) {
        dispatch_async(dispatch_get_main_queue(), ^(){
            NSLog(@"CoreBluetooth BLE state is unknown");
        });
    }
    else if ([central state] == CBCentralManagerStateUnsupported) {
        dispatch_async(dispatch_get_main_queue(), ^(){
            NSLog(@"CoreBluetooth BLE hardware is unsupported on this platform");
        });
    }
}

//TODO: others...

#pragma mark - CBPeripheralDelegate

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverServices:(NSError *)error
{
    for (CBService *service in peripheral.services)
    {
        NSLog(@"Discovered service: %@", service.UUID);
        [peripheral discoverCharacteristics:nil forService:service];
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService:(CBService *)service error:(NSError *)error
{

    for (CBCharacteristic *aChar in service.characteristics)
    {
        NSLog(@"Discovered characteristic: %@ with descriptors: ", aChar.UUID);
        
        for (CBDescriptor *desc in aChar.descriptors)
        {
            NSLog(@"descriptor: %@", desc.UUID);
        }
        
        [self.pheriperal setNotifyValue:YES forCharacteristic:aChar];
    }
    
//    // Retrieve Device Information Services for the Manufacturer Name
//    if ([service.UUID isEqual:[CBUUID UUIDWithString:POLARH7_HRM_DEVICE_INFO_SERVICE_UUID]])  { // 4
//        for (CBCharacteristic *aChar in service.characteristics)
//        {
//            if ([aChar.UUID isEqual:[CBUUID UUIDWithString:POLARH7_HRM_MANUFACTURER_NAME_CHARACTERISTIC_UUID]]) {
//                [self.polarH7HRMPeripheral readValueForCharacteristic:aChar];
//                NSLog(@"Found a device manufacturer name characteristic");
//            }
//        }
//    }
}

- (void)peripheral:(CBPeripheral *)peripheral didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error
{
    NSData *data = characteristic.value;
    
    NSLog(@"Data: %@", data);
    
}

@end
