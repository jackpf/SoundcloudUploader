//
//  AppDelegateBridge.h
//  Facebook Notifications
//
//  Created by Jack Farrelly on 09/10/2015.
//  Copyright (c) 2015 Jack Farrelly. All rights reserved.
//

#ifndef Facebook_Notifications_AppDelegateBridge_h
#define Facebook_Notifications_AppDelegateBridge_h

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#include <iostream>
#include <unordered_map>
#include <thread>
#import <WebKit/WebKit.h>
#include "Data.h"
#include "Notifications.h"

class AppDelegateBridge;

@interface AppDelegateBridgeNative : NSObject <NSApplicationDelegate, NSUserNotificationCenterDelegate, WebFrameLoadDelegate>

@property (nonatomic) AppDelegateBridge *bridge;
@property (strong, nonatomic) NSStatusItem *statusBar;
@property (nonatomic) NSZone *menuZone;
@property (strong, nonatomic) NSMenu *menu;
@property (strong, nonatomic) NSMenuItem *markNotificationsReadMenuItem, *reauthenticateMenuItem, *exitMenuItem;
@property (strong, nonatomic) NSMutableDictionary *notificationPaths;
@property unsigned long notificationCount;
@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet WebView *webView;
@property (strong, nonatomic) NSString *authenticationCodeBuffer;

- (AppDelegateBridgeNative *) init;
- (AppDelegateBridgeNative *) initWithWindow :(NSWindow *)window :(WebView *)webView;
- (void) promptAuthenticationCode;
- (NSString *) consumeAuthenticationCode;
- (void) updateNotificationCount :(unsigned long)count;
- (void) notify :(NSString *)identifier :(NSString *)title :(NSString *)body :(NSString *) path :(NSString *)image;
- (void) getInput :(NSString *)prompt :(NSString **) r;
- (void) alert :(NSString *)prompt;
- (void) markNotificationsRead;
- (void) clearNotifications :(NSArray *)notificationIds;
- (void) exit;

@end

class AppDelegateBridge
{
private:
    AppDelegateBridgeNative *bridge;
    
public:
    typedef void (EventCallback)(void *);
    std::unordered_map<std::string, EventCallback *> callbacks;
    
    void setBridge(AppDelegateBridgeNative *);
    void updateNotificationCount(size_t);
    void notify(std::string, std::string, std::string, std::string, std::string = "");
    void clearNotifications(Notifications);
    std::string getInput(std::string);
    void alert(std::string);
    std::string retrieveAuthenticationCode();
    
    void addEvent(std::string, EventCallback);
    void removeEvent(std::string);
    void event(std::string, void * = NULL);
};

#endif
