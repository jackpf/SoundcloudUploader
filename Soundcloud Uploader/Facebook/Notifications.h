//
//  Notifications.h
//  Facebook Notifications
//
//  Created by Jack Farrelly on 12/10/2015.
//  Copyright (c) 2015 Jack Farrelly. All rights reserved.
//

#ifndef __Facebook_Notifications__Notifications__
#define __Facebook_Notifications__Notifications__

#include <iostream>
#include <vector>

enum NotificationType
{
    NOTIFICATION,
    MESSAGE
};

struct Notification
{
    std::string id;
    std::string title;
    std::string body;
    std::string from;
    std::string link;
    std::time_t updatedAt;
    NotificationType type;
};

class Notifications : public std::vector<Notification>
{
private:
    Notifications *oldNotifications;
    
public:
    void reset();
    bool containsNotification(Notification);
    Notifications getNew();
    Notifications getCleared();
};

#endif /* defined(__Facebook_Notifications__Notifications__) */
