//
//  AppDelegateBridge.m
//  Facebook Notifications
//
//  Created by Jack Farrelly on 09/10/2015.
//  Copyright (c) 2015 Jack Farrelly. All rights reserved.
//

#include "AppDelegateBridge.h"

@implementation AppDelegateBridgeNative

- (void) markNotificationReadThread :(NSString *) identifier
{
    std::string str = [identifier UTF8String];
    self.bridge->event("markNotificationRead", &str);
}

- (void) userNotificationCenter:(NSUserNotificationCenter *)center didActivateNotification:(NSUserNotification *)notification
{
    [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString:[self.notificationPaths objectForKey:notification.identifier]]];
    [self performSelectorInBackground:@selector(markNotificationReadThread:) withObject:notification.identifier];
    [NSUserNotificationCenter.defaultUserNotificationCenter removeDeliveredNotification:notification];
    [self updateNotificationCount:self.notificationCount - 1];
}

- (void)webView:(WebView *)sender didFinishLoadForFrame:(WebFrame *)frame
{
    NSString *url = [[[[frame dataSource] request] URL] absoluteString];
    
    printf("Request URL: %s\n", [url UTF8String]);
    
    if ([url hasPrefix:@REDIRECT_URI]) {
        self.authenticationCodeBuffer = url;
        [self.window setIsVisible:FALSE];
    }
}

- (NSString *) consumeAuthenticationCode
{
    if (self.authenticationCodeBuffer.length == 0) {
        return @"";
    } else {
        NSString *r = self.authenticationCodeBuffer;
        self.authenticationCodeBuffer = @"";
        return r;
    }
}

- (void) promptAuthenticationCode
{
    [self.window setIsVisible:TRUE];
    
    NSURL *url = [NSURL URLWithString:[NSString stringWithFormat:@"%s?client_id=%s&redirect_uri=%s&scope=%s&display=popup", FB_OAUTH_URL, CLIENT_ID, REDIRECT_URI, FB_PERMISSIONS]];
    
    NSHTTPCookieStorage *storage = [NSHTTPCookieStorage sharedHTTPCookieStorage];
    for (NSHTTPCookie *cookie in [storage cookies]) {
        [storage deleteCookie:cookie];
    }
    [[NSUserDefaults standardUserDefaults] synchronize];
    
    [[[self webView] mainFrame] loadRequest:[NSURLRequest requestWithURL:url]];
    [self.window setContentView:self.webView];
}

- (AppDelegateBridgeNative *) initWithWindow:(NSWindow *)window :(WebView *)webView
{
    self.window = window;
    self.webView = webView;
    
    [self.window setLevel:NSFloatingWindowLevel];
    [NSApp activateIgnoringOtherApps:YES];
    [self.webView setFrameLoadDelegate:self];
    
    return [self init];
}

- (AppDelegateBridgeNative *) init
{
    if (self = [super init]) {
        self.notificationPaths = [[NSMutableDictionary alloc] init];
        
        [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:self];
        
        self.statusBar = [[NSStatusBar systemStatusBar] statusItemWithLength:NSSquareStatusItemLength];
        self.statusBar.image = [NSImage imageNamed:@"notification_light"];
        self.statusBar.alternateImage =[NSImage imageNamed:@"notification_alt"];
        self.statusBar.highlightMode = YES;
        
        self.menuZone = [NSMenu menuZone];
        self.menu = [[NSMenu allocWithZone:self.menuZone] init];
        [self.menu setAutoenablesItems:NO];
        
        self.markNotificationsReadMenuItem = [self.menu addItemWithTitle:@"Mark all as read"
                                                                  action:@selector(markNotificationsRead)
                                                           keyEquivalent:@""];
        self.markNotificationsReadMenuItem.enabled = false;
        [self.markNotificationsReadMenuItem setTarget:self];
        
        [self.menu addItem:[NSMenuItem separatorItem]];
        
        self.reauthenticateMenuItem = [self.menu addItemWithTitle:@"Change user"
                                                           action:@selector(reauthenticate)
                                                    keyEquivalent:@""];
        self.reauthenticateMenuItem.enabled = true;
        [self.reauthenticateMenuItem setTarget:self];
        
        self.exitMenuItem = [self.menu addItemWithTitle:@"Exit"
                                                 action:@selector(exit)
                                          keyEquivalent:@""];
        [self.exitMenuItem setTarget:self];
        
        self.statusBar.menu = self.menu;
    }
    
    return self;
}

- (void) reauthenticateThread
{
    self.bridge->event("reauthenticate");
}

- (void) reauthenticate
{
    [self performSelectorInBackground:@selector(reauthenticateThread) withObject:nil];
}

- (void) updateNotificationCount :(unsigned long) count
{
    self.notificationCount = count;
    
    if (count > 0) {
        self.statusBar.image = [NSImage imageNamed:@"notification_dark"];
        self.markNotificationsReadMenuItem.enabled = YES;
    } else {
        self.statusBar.image = [NSImage imageNamed:@"notification_light"];
        self.markNotificationsReadMenuItem.enabled = NO;
    }
    
    self.statusBar.toolTip = [NSString stringWithFormat:@"%lu notifications", count];
}

- (void) notify :(NSString *) identifier :(NSString *) title :(NSString *) body :(NSString *) path :(NSString *) image;
{
    [self.notificationPaths setObject:path forKey:identifier];
    
    NSUserNotification *notification = [[NSUserNotification alloc] init];
    notification.identifier = identifier;
    notification.title = title;
    notification.informativeText = body;
    notification.contentImage = [[NSImage alloc] initWithContentsOfFile:image];
    [NSUserNotificationCenter.defaultUserNotificationCenter deliverNotification:notification];
}

- (void) clearNotifications :(NSArray *)notificationIds
{
    for(NSUserNotification *notification in NSUserNotificationCenter.defaultUserNotificationCenter.deliveredNotifications) {
        if ([notificationIds containsObject:notification.identifier]) {
            [NSUserNotificationCenter.defaultUserNotificationCenter removeDeliveredNotification:notification];
        }
    }
}

- (void) getInput :(NSString *) prompt :(NSString **) r
{
    NSAlert *alert = [NSAlert alertWithMessageText: prompt
                                     defaultButton:@"OK"
                                   alternateButton:@"Cancel"
                                       otherButton:nil
                         informativeTextWithFormat:@""];
    
    NSTextField *input = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 200, 24)];
    //[input setStringValue:defaultValue];
    //[input autorelease];
    [alert setAccessoryView:input];
    NSInteger button = [alert runModal];
    if (button == NSAlertDefaultReturn) {
        [input validateEditing];
        *r = [input stringValue];
    } else {
        *r = @"";
    }
}

- (void) alert :(NSString *) prompt
{
    NSAlert *alert = [NSAlert alertWithMessageText: prompt
                                     defaultButton:@"OK"
                                   alternateButton:nil
                                       otherButton:nil
                         informativeTextWithFormat:@""];
    
    [alert runModal];
}

- (void) markNotificationsReadThread
{
    [NSUserNotificationCenter.defaultUserNotificationCenter removeAllDeliveredNotifications];
    self.bridge->event("markNotificationsRead");
}

- (void) markNotificationsRead
{
    [self performSelectorInBackground:@selector(markNotificationsReadThread) withObject:nil];
}

- (void) exit
{
    [[NSApplication sharedApplication] terminate:nil];
}

@end

void AppDelegateBridge::setBridge(AppDelegateBridgeNative *bridge)
{
    this->bridge = bridge;
}

void AppDelegateBridge::notify(std::string identifier, std::string title, std::string body, std::string path, std::string image)
{
    [bridge notify:[NSString stringWithUTF8String:identifier.c_str()] :[NSString stringWithUTF8String:title.c_str()] :[NSString stringWithUTF8String:body.c_str()] :[NSString stringWithUTF8String:path.c_str()] :[NSString stringWithUTF8String:image.c_str()]];
}

void AppDelegateBridge::updateNotificationCount(size_t count)
{
    [bridge updateNotificationCount:count];
}

void AppDelegateBridge::clearNotifications(Notifications notifications)
{
    
    NSMutableArray *notificationIds = [[NSMutableArray alloc] init];
    
    int i = 0;
    for(Notifications::iterator it = notifications.begin(); it != notifications.end(); ++it) {
        notificationIds[i] = [NSString stringWithUTF8String:static_cast<Notification>(*it).id.c_str()];
        i++;
    }
    
    [bridge clearNotifications:notificationIds];
}

std::string AppDelegateBridge::getInput(std::string prompt)
{
    __block NSString *r;
    
    dispatch_sync(dispatch_get_main_queue(), ^{
        [bridge getInput :[NSString stringWithUTF8String:prompt.c_str()] :&r];
    });
    
    return [r UTF8String];
}

std::string AppDelegateBridge::retrieveAuthenticationCode()
{
    dispatch_sync(dispatch_get_main_queue(), ^{
        [bridge promptAuthenticationCode];
    });
    
    std::string code;
    
    while (code.length() == 0) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        code = [[bridge consumeAuthenticationCode] UTF8String];
    }
    
    return code;
}

void AppDelegateBridge::alert(std::string prompt)
{
    [bridge performSelectorOnMainThread:@selector(alert:) withObject:[NSString stringWithUTF8String:prompt.c_str()] waitUntilDone:NO];
}

void AppDelegateBridge::addEvent(std::string eventName, EventCallback *callback)
{
    callbacks[eventName] = callback;
}

void AppDelegateBridge::removeEvent(std::string eventName)
{
    callbacks[eventName] = NULL;
}

void AppDelegateBridge::event(std::string eventName, void *data)
{
    callbacks[eventName](data);
}