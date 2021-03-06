//
//  Main.cpp
//  Facebook Notifications
//
//  Created by Jack Farrelly on 09/10/2015.
//  Copyright (c) 2015 Jack Farrelly. All rights reserved.
//

#include "Main.h"

AppDelegateBridge                               *Main::bridge;

static vector<Upload *> uploads;
bool isMonitoringProgress = false;
static std::thread *progressThread;

static size_t progress_callback(void *data, double dltotal, double dlnow, double ultotal, double ulnow)
{
    Upload *upload = static_cast<Upload *>(data);
    
    if (ultotal > 0) {
        upload->progress = ceil(ulnow / ultotal * 100.0);
    }
    
    std::cout << "Raw progress: " << upload->progress << "(" << ulnow << " / " << ultotal << ")" << std::endl;
    
    return !upload->cancelRequest ? 0 : 1;
}

void Main::progressMonitor()
{
    isMonitoringProgress = true;
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
    } while (progress < 100 && uploads.size() > 0);
    
    isMonitoringProgress = false;
    
    bridge->updateUploadProgress(0, 0);
}

int Main::main(AppDelegateBridge *bridge)
{
    Main::bridge = bridge;
    bridge->addEvent("upload", uploadEvent);
    bridge->addEvent("reauthenticate", reauthenticateEvent);
    bridge->addEvent("cancel_uploads", cancelUploadsEvent);
    
    try {
        if (AccessTokenStorage::getInstance()->readAccessToken().length() == 0) {
            std::string code = AccessTokenStorage::getInstance()->getCodeFromUrl(bridge->retrieveAuthenticationCode());
            AccessTokenStorage::getInstance()->storeAccessTokenFromCode(code);
        }
    } catch (std::runtime_error e) {
        std::cout << "Runtime error: " << e.what() << std::endl;
        bridge->alert(std::string("Runtime error: ") + e.what());
    }
    
    return 0;
}

void Main::uploadEvent(void *data)
{
    Upload upload;
    upload.path = *static_cast<std::string *>(data);
    upload.name = boost::filesystem::basename(upload.path);
    uploads.push_back(&upload);
    
    if (!isMonitoringProgress) {
        progressThread = new std::thread(progressMonitor);
    }
    
    try {
        std::string accessToken = AccessTokenStorage::getInstance()->readAccessToken();
        std::stringstream buf;
        
        Request::getInstance()->request(
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
        
        if (!upload.cancelRequest) {
            try {
                Parser parser(&buf);
                parser.parseResponse(&upload);
                
                bridge->notify(upload.data["id"], "Upload complete", upload.name + " has been successfully uploaded.",  upload.data["permalink_url"]);
            } catch (SoundcloudDefaultException e) {
                std::cout << "Soundcloud error: " << e.what() << std::endl;
                bridge->alert(std::string("Soundcloud error: ") + e.what());
            }
        }
    } catch (SoundcloudAuthenticationException e) {
        std::cout << "Authentication error: " << e.what() << std::endl;
        bridge->alert(std::string("Authentication error: ") + e.what());
        reauthenticateEvent(nullptr);
    } catch (std::runtime_error e) {
        std::cout << "Runtime error: " << e.what() << std::endl;
        bridge->alert(std::string("Runtime error: ") + e.what());
    }
    
    uploads.erase(std::remove(uploads.begin(), uploads.end(), &upload), uploads.end());
    
    if (uploads.size() == 0 && progressThread != nullptr) {
        progressThread->join();
        delete progressThread;
    }
}

void Main::reauthenticateEvent(void *data)
{
    try {
        std::string code = AccessTokenStorage::getInstance()->getCodeFromUrl(bridge->retrieveAuthenticationCode());
        AccessTokenStorage::getInstance()->storeAccessTokenFromCode(code);
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
}