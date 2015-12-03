//
//  Parser.h
//  Facebook Notifications
//
//  Created by Jack Farrelly on 09/10/2015.
//  Copyright (c) 2015 Jack Farrelly. All rights reserved.
//

#ifndef __Facebook_Notifications__Parser__
#define __Facebook_Notifications__Parser__

#include <iostream>
#include <exception>
#include <sstream>
#include <vector>
#include <algorithm>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include "Request.h"
#include "Notifications.h"
#include "FacebookException.h"
#include "User.h"
#include <unordered_map>

class Parser
{
private:
    void parseJson(std::stringstream *, boost::property_tree::ptree *) throw(FacebookDefaultException *, std::runtime_error);
    std::time_t convertDate(std::string);
    
public:
    void parseNotifications(std::stringstream *, Notifications *) throw(FacebookDefaultException *, std::runtime_error);
    void parseUnreadMessages(std::stringstream *, Notifications *, std::unordered_map<std::string, std::time_t>, User) throw(FacebookDefaultException *, std::runtime_error);
    void parseUser(std::stringstream *, User *) throw(FacebookDefaultException *, std::runtime_error);
};

#endif /* defined(__Facebook_Notifications__Parser__) */
