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

static vector<Upload *> uploads;

static size_t progress_callback(void *data, double dltotal, double dlnow, double ultotal, double ulnow)
{
    Upload *upload = static_cast<Upload *>(data);
    upload->progress = ceil(ulnow / ultotal * 100.0);
    
    return 0;
}

void Main::progressMonitor()
{
    size_t totalProgress = 0;
    
    while (totalProgress <= 100) {
        totalProgress = 0;
        
        for (auto it = uploads.begin(); it != uploads.end(); it++) {
            Upload *upload = static_cast<Upload *>(*it);
            totalProgress += upload->progress;
        }
    
        bridge->updateUploadProgress(ceil((double) totalProgress / uploads.size()), uploads.size());
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
    Upload upload;
    upload.path = *static_cast<std::string *>(data);
    upload.name = boost::filesystem::basename(upload.path);
    uploads.push_back(&upload);
    
    try {
        std::string accessToken = tokenStorage->readAccessToken();
        std::stringstream buf;
        
        std::thread progressThread(progressMonitor);
        
        request->request(
            "/tracks",
            Request::Params(),
            Request::Params{
                Request::Param("oauth_token", accessToken),
                Request::Param("track[title]", upload.name)
            },
            Request::Params{
                Request::Param("track[asset_data]", upload.path),
            },
            &buf,
            progress_callback,
            &upload
        );
        
        // Just make sure we've definitely finished
        upload.progress = 100;
        progressThread.join();
        upload.progress = 0;
        
        std::cout <<"ret:"<<buf.str()<<std::endl;
    } catch (std::runtime_error e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    for (auto it = uploads.begin(); it != uploads.end(); it++) {
        Upload *currentUpload = static_cast<Upload *>(*it);
        if (currentUpload == &upload) {
            uploads.erase(it);
        }
    }
}