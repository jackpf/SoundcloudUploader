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
Parser                                          *Main::parser           = new Parser();

static vector<Upload *> uploads;

static size_t progress_callback(void *data, double dltotal, double dlnow, double ultotal, double ulnow)
{
    Upload *upload = static_cast<Upload *>(data);
    upload->progress = ceil(ulnow / ultotal * 100.0);
    
    std::cout << "Raw progress: " << upload->progress << std::endl;
    
    return 0;
}

void Main::progressMonitor()
{
    size_t progress = 0;
    
    do {
        size_t totalProgress = 0;
        
        for (auto it = uploads.begin(); it != uploads.end(); it++) {
            Upload *upload = static_cast<Upload *>(*it);
            totalProgress += upload->progress;
        }
        
        progress = ceil((double) totalProgress / uploads.size());
        
        std::cout << "Progress: " << progress << std::endl;
    
        bridge->updateUploadProgress(progress, uploads.size());
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (progress < 100);
}

int Main::main(AppDelegateBridge *bridge)
{
    Main::bridge = bridge;
    bridge->addEvent("upload", uploadEvent);
    bridge->addEvent("reauthenticate", reauthenticateEvent);
    bridge->addEvent("cancel_uploads", cancelUploadsEvent);
    
    if (tokenStorage->readAccessToken().length() == 0) {
        std::string code = tokenStorage->getCodeFromUrl(bridge->retrieveAuthenticationCode());
        tokenStorage->storeAccessTokenFromCode(code);
    }
    
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
        
        upload.thread = std::thread(progressMonitor);
        
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
        
        std::cout << "Response: " << buf.str() << std::endl;
        
        try {
            parser->parseResponse(&buf, nullptr);
        } catch (SoundcloudDefaultException e) {
            std::cout << "Soundcloud error: " << e.what() << std::endl;
            bridge->alert(std::string("Soundcloud error: ") + e.what());
        }
    } catch (std::runtime_error e) {
        std::cout << "Runtime error: " << e.what() << std::endl;
        bridge->alert(std::string("Runtime error: ") + e.what());
    }
    
    // Just make sure we've definitely finished
    upload.progress = 100;
    upload.thread.join();
    
    uploads.erase(std::remove(uploads.begin(), uploads.end(), &upload), uploads.end());
}

void Main::reauthenticateEvent(void *data)
{
    try {
        std::string code = tokenStorage->getCodeFromUrl(bridge->retrieveAuthenticationCode());
        tokenStorage->storeAccessTokenFromCode(code);
    } catch (SoundcloudDefaultException e) {
        std::string errMsg = std::string("Login error: ") + e.what();
        std::cout << errMsg << std::endl;
        bridge->alert(errMsg);
    }
}

void Main::cancelUploadsEvent(void *data)
{
    for (int i = 0; i < uploads.size(); i++) {
        uploads[i]->cancelRequest = true;
    }
    for (int i = 0; i < uploads.size(); i++) {
        uploads[i]->thread.join();
    }
}