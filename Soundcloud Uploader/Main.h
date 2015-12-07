//
//  Main.h
//  Facebook Notifications
//
//  Created by Jack Farrelly on 09/10/2015.
//  Copyright (c) 2015 Jack Farrelly. All rights reserved.
//

#ifndef Facebook_Notifications_Main_h
#define Facebook_Notifications_Main_h

#include <thread>
#include <chrono>
#include <mutex>
#include "AppDelegateBridge.h"
#include "Parser.h"
#include "AccessTokenStorage.h"
#include "Exception.h"
#include <unordered_map>
#include "Upload.h"

#define INITIAL_UPDATE_TIME 1
#define MAX_UPDATE_TIME     5

class Main
{
private:
    static AppDelegateBridge *bridge;
    static Request *request;
    static AccessTokenStorage *tokenStorage;
    static Parser *parser;
    
    Main();
    static void progressMonitor();
    
public:
    static int main(AppDelegateBridge *);
    
    static void uploadEvent(void *data);
    static void reauthenticateEvent(void *data);
    static void cancelUploadsEvent(void *data);
};

#endif
