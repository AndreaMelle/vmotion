//
//  main.m
//  BLESample
//
//  Created by Andrea Melle on 05/07/2015.
//  Copyright (c) 2015 Andrea Melle. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "BLEManager.h"

int main(int argc, const char * argv[]) {
    @autoreleasepool
    {
        BLEManager* ble = [[BLEManager alloc] init];
        
        [ble scan];
        
        [[NSRunLoop currentRunLoop] run];
        
    }
    return 0;
}
