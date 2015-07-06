//
//  BLEManager.h
//  BLESample
//
//  Created by Andrea Melle on 05/07/2015.
//  Copyright (c) 2015 Andrea Melle. All rights reserved.
//

#import <Foundation/Foundation.h>

#import <IOBluetooth/IOBluetooth.h>
#import <Cocoa/Cocoa.h>

//TODO: add defines for UUID

@interface BLEManager : NSObject <CBCentralManagerDelegate, CBPeripheralDelegate>

@property (nonatomic, strong) CBCentralManager *centralManager;
@property (nonatomic, strong) CBPeripheral *pheriperal;
@property (nonatomic, strong) NSString* connected;
//TODO: add a peripheral reference

- (void) scan;

@end
