//
//  Notifications.cpp
//  Facebook Notifications
//
//  Created by Jack Farrelly on 12/10/2015.
//  Copyright (c) 2015 Jack Farrelly. All rights reserved.
//

#include "Notifications.h"

void Notifications::reset()
{
    delete oldNotifications;
    oldNotifications = new Notifications(*this);
    clear();
}

bool Notifications::containsNotification(Notification notificationToFind)
{
    for(iterator it = begin(); it != end(); ++it) {
        auto notification = static_cast<Notification>(*it);
        
        if (notification.id.compare(notificationToFind.id) == 0 && notification.updatedAt == notificationToFind.updatedAt) {
            return true;
        }
    }
    
    return false;
}

Notifications Notifications::getNew()
{
    Notifications newNotifications;
    
    for(iterator it = begin(); it != end(); ++it) {
        auto notification = static_cast<Notification>(*it);
        
        if (!oldNotifications->containsNotification(notification)) {
            newNotifications.push_back(notification);
        }
    }
    
    return newNotifications;
}

Notifications Notifications::getCleared()
{
    Notifications readNotifications;
    
    for(iterator it = oldNotifications->begin(); it != oldNotifications->end(); ++it) {
        auto notification = static_cast<Notification>(*it);
        
        if (!containsNotification(notification)) {
            readNotifications.push_back(notification);
        }
    }
    
    return readNotifications;
}