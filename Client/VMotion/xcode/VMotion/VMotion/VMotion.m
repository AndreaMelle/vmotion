//
//  VMotion.m
//  VMotion
//
//  Created by Andrea Melle on 06/07/2015.
//  Copyright (c) 2015 Andrea Melle. All rights reserved.
//

#import "VMotion.h"
#import "VMotionController.h"

@implementation VMotion
{
    @private
    CBCentralManager* manager;
    NSArray* peripheralScanList;
    
    dispatch_queue_t bleQueue;
    
    NSMutableArray* discoveredPeripherals; //list of all discovered peripherals ever, connected or not
    NSMutableArray* connectedPeripherals; //list of all currently connected peripherals
    
    NSMutableArray* motionControllers;
    
    NSString *manufacturer;
    BOOL autoConnect;
}

- (instancetype)init
{
    self = [super init];
    if(self)
    {
        autoConnect = YES;
        
        discoveredPeripherals = [[NSMutableArray alloc] init];
        connectedPeripherals = [[NSMutableArray alloc] init];
        motionControllers = [[NSMutableArray alloc] init];
        
        peripheralScanList = nil; //[NSArray arrayWithObject:[CBUUID UUIDWithString:@"180D"]];
        bleQueue = dispatch_queue_create("com.vmotion.centralmanager.workqueue", DISPATCH_QUEUE_SERIAL);
        NSDictionary *managerOptions = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:YES], CBCentralManagerOptionShowPowerAlertKey, nil];
        manager = [[CBCentralManager alloc] initWithDelegate:self queue:bleQueue options:managerOptions];
        
        if( autoConnect )
        {
            [self startScan];
        }
        
    }
    
    return self;
}

- (void) dealloc
{
    [self stopScan];
    
    for(CBPeripheral* peripheral in connectedPeripherals)
    {
        [manager cancelPeripheralConnection:peripheral];
    }
}

- (void) startScan
{
    [manager scanForPeripheralsWithServices:peripheralScanList options:nil];
}

- (void) stopScan
{
    [manager stopScan];
}

- (BOOL) isLECapableHardware
{
    NSString * state = nil;
    
    switch ([manager state])
    {
        case CBCentralManagerStateUnsupported:
            state = @"The platform/hardware doesn't support Bluetooth Low Energy.";
            break;
        case CBCentralManagerStateUnauthorized:
            state = @"The app is not authorized to use Bluetooth Low Energy.";
            break;
        case CBCentralManagerStatePoweredOff:
            state = @"Bluetooth is currently powered off.";
            break;
        case CBCentralManagerStatePoweredOn:
            return TRUE;
        case CBCentralManagerStateUnknown:
        default:
            return FALSE;
            
    }
    
    NSLog(@"Central manager state: %@", state);

    return FALSE;
}

#pragma mark - CBCentralManagerDelegate delegate methods

- (void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)aPeripheral advertisementData:(NSDictionary *)advertisementData RSSI:(NSNumber *)RSSI
{
    if( ![discoveredPeripherals containsObject:aPeripheral] )
    {
        [discoveredPeripherals addObject:aPeripheral];
    }
    
    /* Retreive already known devices */
    if(autoConnect)
    {
        [manager retrievePeripheralsWithIdentifiers:[NSArray arrayWithObject:(id)aPeripheral.identifier]];
    }
}

- (void)centralManager:(CBCentralManager *)central didRetrievePeripherals:(NSArray *)peripherals
{
    NSLog(@"Retrieved peripheral: %lu - %@", [peripherals count], peripherals);
    
    [self stopScan];
    
    /* If there are any known devices, automatically connect to it.*/
    //TODO: which options?
    for(CBPeripheral *peripheral in peripherals)
    {
        [manager connectPeripheral:peripheral options:[NSDictionary dictionaryWithObject:[NSNumber numberWithBool:YES] forKey:CBConnectPeripheralOptionNotifyOnDisconnectionKey]];
    }
}

- (void)centralManager:(CBCentralManager *)central didRetrieveConnectedPeripherals:(NSArray *)peripherals
{
    
}

- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)aPeripheral
{
    //TODO: which options?
    if(![connectedPeripherals containsObject:aPeripheral])
    {
        [connectedPeripherals addObject:aPeripheral];
        [motionControllers addObject:[[VMotionController alloc] initWithPeripheral:aPeripheral]];
    }
    else
    {
        NSLog(@"Invalid state. Connected to existing peripheral.");
    }
}

- (void)centralManager:(CBCentralManager *)central didDisconnectPeripheral:(CBPeripheral *)aPeripheral error:(NSError *)error
{
    
    if([connectedPeripherals containsObject:aPeripheral])
    {
        [connectedPeripherals removeObject:aPeripheral];
        for(VMotionController* ctrl in motionControllers)
        {
            if (ctrl.peripheral == aPeripheral)
            {
                [motionControllers removeObject:ctrl];
                break;
            }
        }
    }
    else
    {
        NSLog(@"Invalid state. Disconnected from non-existing peripheral.");
    }
    
    if(autoConnect)
    {
        [self startScan];
    }
    
}

- (void)centralManager:(CBCentralManager *)central didFailToConnectPeripheral:(CBPeripheral *)aPeripheral error:(NSError *)error
{
    NSLog(@"Fail to connect to peripheral: %@ with error = %@", aPeripheral, [error localizedDescription]);

    if([connectedPeripherals containsObject:aPeripheral])
    {
        [connectedPeripherals removeObject:aPeripheral];
        for(VMotionController* ctrl in motionControllers)
        {
            if (ctrl.peripheral == aPeripheral)
            {
                [motionControllers removeObject:ctrl];
                break;
            }
        }
    }
}

- (void)centralManager:(CBCentralManager *)central willRestoreState:(NSDictionary *)dict
{
    //TODO: what exactly is this...
}

- (void)centralManagerDidUpdateState:(CBCentralManager *)central
{
    [self isLECapableHardware];
}


@end
