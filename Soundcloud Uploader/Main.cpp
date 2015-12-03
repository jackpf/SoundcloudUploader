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
Parser                                          *Main::parser           = new Parser;
ImageCache                                      *Main::cache            = new ImageCache;
AccessTokenStorage                              *Main::tokenStorage     = AccessTokenStorage::getInstance();
Notifications                                   Main::notifications;
std::string                                     Main::accessToken;
User                                            Main::user;
std::chrono::minutes                            Main::updateTime        = std::chrono::minutes(INITIAL_UPDATE_TIME);
std::mutex                                      Main::updateMutex;
std::unordered_map<std::string, std::time_t>    Main::readMessages; // Should be persisted to file?

int Main::main(AppDelegateBridge *bridge)
{
    Main::bridge = bridge;
    bridge->addEvent("markNotificationRead", &markNotificationRead);
    bridge->addEvent("markNotificationsRead", &markNotificationsRead);
    bridge->addEvent("reauthenticate", &reauthenticate);
    
    std::cout << "Pruned " << cache->prune() << " cached images" << std::endl;
    
    accessToken = tokenStorage->read();
    
    while (true) {
        try {
            std::lock_guard<std::mutex> lock(updateMutex);
            std::stringstream buffer;
            
            if (user.id.empty()) {
                std::cout << "Loading user data" << std::endl;
                request->request("/me", Request::Params{Request::Param("access_token", accessToken)}, &buffer);
                parser->parseUser(&buffer, &user);
            }
            
            notifications.reset();
            
            request->request("/me/notifications", Request::Params{Request::Param("access_token", accessToken)}, &buffer);
            parser->parseNotifications(&buffer, &notifications);
            
            request->request("/me/inbox", Request::Params{Request::Param("access_token", accessToken)}, &buffer);
            parser->parseUnreadMessages(&buffer, &notifications, readMessages, user);
            
            Notifications newNotifications = notifications.getNew(), clearedNotifications = notifications.getCleared();
            
            std::cout << "Found " << notifications.size() << " notifications, " << newNotifications.size() << " new, " << clearedNotifications.size() << " cleared" << std::endl;
            
            bridge->updateNotificationCount(notifications.size());
            
            for(Notifications::reverse_iterator it = newNotifications.rbegin(); it != newNotifications.rend(); ++it) {
                auto notification = static_cast<Notification>(*it);
                
                bridge->notify(notification.id, notification.title, notification.body, notification.link, cache->fetch(notification.from));
            }
            
            bridge->clearNotifications(clearedNotifications);
        } catch (const FacebookDefaultException *e) {
            if (auto real = dynamic_cast<const FacebookAuthenticationException *>(e)) {
                std::cout << "Login error: " << real->what() << std::endl;
                reauthenticate(nullptr);
            } else {
                std::string errMsg = std::string("Facebook error: ") + e->what();
                std::cout << errMsg << std::endl;
                bridge->alert(errMsg);
            }
            delete e;
        } catch (const std::runtime_error &e) {
            std::string errMsg = std::string("Runtime error: ") + e.what();
            std::cout << errMsg << std::endl;
            //bridge->alert(errMsg);
        }
            
        std::this_thread::sleep_for(nextUpdateTime(notifications.getNew()));
    }
    
    return 0;
}

std::chrono::minutes Main::nextUpdateTime(const Notifications newNotifications)
{
    std::chrono::minutes r = updateTime;
    
    if (updateTime.count() < MAX_UPDATE_TIME && newNotifications.size() == 0) {
        updateTime++;
    } else if (newNotifications.size() > 0) {
        updateTime = std::chrono::minutes(INITIAL_UPDATE_TIME);
    }
    
    std::cout << "Interval: " << r.count() << "m, next: " << updateTime.count() << "m" << std::endl;
    
    return r;
}

void Main::reauthenticate(void *data)
{
    try {
        std::string code = tokenStorage->getCodeFromUrl(bridge->retrieveAuthenticationCode());
        accessToken = tokenStorage->getAccessTokenFromCode(code);
        tokenStorage->store(accessToken);
    } catch (FacebookLoginException *e) {
        std::string errMsg = std::string("Login error: ") + e->what();
        std::cout << errMsg << std::endl;
        bridge->alert(errMsg);
        delete e;
    }
}

void Main::markNotificationRead(void *data)
{
    std::lock_guard<std::mutex> lock(updateMutex);
    
    try {
        request->request("/" + *static_cast<std::string *>(data), Request::Params{Request::Param("unread", "false"), Request::Param("access_token", accessToken)}, true, nullptr);
    } catch (const std::runtime_error e) {
        std::string errMsg = std::string("Runtime error: ") + e.what();
        std::cout << errMsg << std::endl;
        bridge->alert(errMsg);
    }
}

void Main::markNotificationsRead(void *data)
{
    std::lock_guard<std::mutex> lock(updateMutex);
    
    int i = 0, j = 0;
    
    for(Notifications::iterator it = notifications.begin(); it != notifications.end(); ++it) {
        auto notification = static_cast<Notification>(*it);
        
        if (notification.type == NotificationType::NOTIFICATION) {
            try {
                request->request("/" + notification.id, Request::Params{Request::Param("unread", "false"), Request::Param("access_token", accessToken)}, true, nullptr);
                i++;
            } catch (const std::runtime_error e) {
                std::string errMsg = std::string("Runtime error: ") + e.what();
                std::cout << errMsg << std::endl;
                bridge->alert(errMsg);
            }
        } else if (notification.type == NotificationType::MESSAGE) {
            readMessages[notification.id] = notification.updatedAt;
            j++;
        }
    }
    
    notifications.reset();
    bridge->updateNotificationCount(0);
    
    std::cout << "Marked " << i << " notifications as read, added " << j << " messages to ignore list" << std::endl;
}