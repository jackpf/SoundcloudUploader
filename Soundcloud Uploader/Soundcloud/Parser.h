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
#include <ctime>

class Parser
{
private:
    boost::property_tree::ptree pt;
    
public:
    Parser(std::stringstream *);
    void parseAccessToken(std::string *, const std::string = "access_token") throw(SoundcloudDefaultException, boost::property_tree::json_parser::json_parser_error);
    void parseResponse(Upload *) throw(SoundcloudDefaultException, boost::property_tree::json_parser::json_parser_error);
};

#endif /* defined(__Facebook_Notifications__Parser__) */
