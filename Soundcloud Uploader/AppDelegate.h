//
//  AppDelegate.h
//  Facebook Notifications
//
//  Created by Jack Farrelly on 09/10/2015.
//  Copyright (c) 2015 Jack Farrelly. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "AppDelegateBridge.h"
#include "Main.h"
#import <WebKit/WebKit.h>

@interface AppDelegate : NSObject

@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet WebView *webView;

@end

