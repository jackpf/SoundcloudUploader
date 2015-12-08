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
#include "Exception.h"
#include <unordered_map>
#include "Upload.h"

class Parser
{
private:
    void parseJson(std::stringstream *, boost::property_tree::ptree *) throw(SoundcloudDefaultException, boost::property_tree::json_parser::json_parser_error);
    
public:
    void parseAccessToken(std::stringstream *, std::string *) throw(SoundcloudDefaultException, boost::property_tree::json_parser::json_parser_error);
    void parseResponse(std::stringstream *, Upload *) throw(SoundcloudDefaultException, boost::property_tree::json_parser::json_parser_error);
};

#endif /* defined(__Facebook_Notifications__Parser__) */
