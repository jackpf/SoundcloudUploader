//
//  AppDelegateBridge.m
//  Facebook Notifications
//
//  Created by Jack Farrelly on 09/10/2015.
//  Copyright (c) 2015 Jack Farrelly. All rights reserved.
//

#include "AppDelegateBridge.h"

@implementation AppDelegateBridgeNative

- (void) userNotificationCenter:(NSUserNotificationCenter *)center didActivateNotification:(NSUserNotification *)notification
{
    
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
    
    NSURL *url = [NSURL URLWithString:[NSString stringWithFormat:@"%s?client_id=%s&redirect_uri=%s&scope=%s&display=popup&response_type=%s", OAUTH_URL, CLIENT_ID, REDIRECT_URI, SCOPE, RESPONSE_TYPE]];
    
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
        self.statusBar.image = [NSImage imageNamed:@"StatusBarIcon"];
        self.statusBar.alternateImage =[NSImage imageNamed:@"StatusBarIcon"];
        self.statusBar.toolTip = @"No uploads in progress";
        self.statusBar.highlightMode = YES;
        
        self.menuZone = [NSMenu menuZone];
        self.menu = [[NSMenu allocWithZone:self.menuZone] init];
        [self.menu setAutoenablesItems:NO];
        
        self.uploadMenuItem = [self.menu addItemWithTitle:@"Upload"
                                                           action:@selector(upload)
                                                    keyEquivalent:@""];
        self.uploadMenuItem.enabled = true;
        [self.uploadMenuItem setTarget:self];
        
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

- (void) upload
{
    NSOpenPanel *openDlg = [NSOpenPanel openPanel];
    [openDlg setCanChooseFiles:YES];
    [openDlg setCanChooseDirectories:NO];
    [openDlg setAllowsMultipleSelection:NO];
    
    if ([openDlg runModalForDirectory:nil file:nil] == NSOKButton) {
        NSArray* files = [openDlg filenames];
        
        for(int i = 0; i < [files count]; i++) {
            [self performSelectorInBackground:@selector(uploadThread:) withObject:[files objectAtIndex:i]];
        }
    }
}

- (void) uploadThread :(NSString *) filename
{
    std::string filenameStr = [filename UTF8String];
    self.bridge->event("upload", &filenameStr);
}

- (void) reauthenticateThread
{
    self.bridge->event("reauthenticate");
}

- (void) reauthenticate
{
    [self performSelectorInBackground:@selector(reauthenticateThread) withObject:nil];
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

- (void) updateUploadProgress :(NSInteger) status
{
    NSString *statusText = [NSString stringWithFormat: @"Upload progress: %d%%", status];
    [self.statusBar setToolTip:statusText];
    
    if (status == 100) {
        [self notify:@"UPLOAD_PROGRESS" :@"Upload Complete" :@"Upload complete" :@"" :@""];
        [self.statusBar setToolTip:@"No uploads in progress"];
    }
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

void AppDelegateBridge::updateUploadProgress(int status)
{
    [bridge updateUploadProgress :(NSInteger)status];
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