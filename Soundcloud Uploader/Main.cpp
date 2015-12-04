//
//  Main.cpp
//  Facebook Notifications
//
//  Created by Jack Farrelly on 09/10/2015.
//  Copyright (c) 2015 Jack Farrelly. All rights reserved.
//

#include "Main.h"

AppDelegateBridge                               *Main::bridge;
Request                                         *Main::request          = Request::getInstance();
AccessTokenStorage                              *Main::tokenStorage     = AccessTokenStorage::getInstance();

static int progress = 0;
static bool inProgress = false;

void Main::progressMonitor()
{
    std::cout << "Printing progress" << std::endl;
    
    while (progress <= 100) {
        bridge->updateUploadProgress(progress);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int Main::main(AppDelegateBridge *bridge)
{
    Main::bridge = bridge;
    bridge->addEvent("upload", &uploadEvent);
    
    return 0;
}

void Main::uploadEvent(void *data)
{
    std::string *path = static_cast<std::string *>(data);
    
    try {
        std::string accessToken = tokenStorage->readAccessToken();
        std::stringstream buf;
        
        std::thread progressThread(progressMonitor);
        
        request->request(
            "/tracks",
            Request::Params(),
            Request::Params{
                Request::Param("oauth_token", accessToken),
                Request::Param("track[title]", boost::filesystem::basename(path->c_str()))
            },
            Request::Params{
                Request::Param("track[asset_data]", *path),
            },
            &buf,
            &progress
        );
        
        // Just make sure we've definitely finished
        progress = 100;
        progressThread.join();
        progress = 0;
        
        std::cout <<"ret:"<<buf.str()<<std::endl;
    } catch (std::runtime_error e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}