//
//  VMotion.h
//  VMotion
//
//  Created by Andrea Melle on 06/07/2015.
//  Copyright (c) 2015 Andrea Melle. All rights reserved.
//

#import <Foundation/Foundation.h>
@import CoreBluetooth;
@import QuartzCore;

@interface VMotion : NSObject <CBCentralManagerDelegate>

- (void) startScan;
- (void) stopScan;
- (BOOL) isLECapableHardware;

@end
