//
//  VMotion.m
//  VMotion
//
//  Created by Andrea Melle on 06/07/2015.
//  Copyright (c) 2015 Andrea Melle. All rights reserved.
//

#import "VMotion.h"


@implementation VMotion
{
    @private
    CBCentralManager *manager;
    dispatch_queue_t bleQueue;
}

- (instancetype)init
{
    self = [super init];
    if(self)
    {
        bleQueue = dispatch_queue_create("com.vmotion.centralmanager.workqueue", DISPATCH_QUEUE_SERIAL);
        NSDictionary *managerOptions = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:YES], CBCentralManagerOptionShowPowerAlertKey, nil];
        
        
        manager = [[CBCentralManager alloc] initWithDelegate:self queue:bleQueue options:managerOptions]
    }
    
    return self;
}



@end
