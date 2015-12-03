//
//  AppDelegate.m
//  Facebook Notifications
//
//  Created by Jack Farrelly on 09/10/2015.
//  Copyright (c) 2015 Jack Farrelly. All rights reserved.
//

#import "AppDelegate.h"

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    [self performSelectorInBackground:@selector(run) withObject:nil];
}

- (void)run {
    AppDelegateBridgeNative *nativeBridge = [[AppDelegateBridgeNative alloc] initWithWindow :self.window :self.webView];
    AppDelegateBridge *bridge = new AppDelegateBridge;
    
    [nativeBridge setBridge:bridge];
    bridge->setBridge(nativeBridge);
    
    Main::main(bridge);
    
    //delete bridge;
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

@end
