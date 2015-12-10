//
//  Parser.cpp
//  Facebook Notifications
//
//  Created by Jack Farrelly on 09/10/2015.
//  Copyright (c) 2015 Jack Farrelly. All rights reserved.
//

#include "Parser.h"

Parser::Parser(std::stringstream *json)
{
    boost::property_tree::read_json(*json, pt);
    
    // Sometimes errors is an array...
    if (pt.count("errors") > 0) {
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt.get_child("errors")) {
            // Just throw the first error
            throw SoundcloudDefaultException(v.second.get_child("error_message").data());
        }
    }
    // ...sometimes not!
    if (pt.count("error") > 0) {
        throw SoundcloudDefaultException(pt.get_child("error").data());
    }
}

void Parser::parseAccessToken(std::string *accessToken, const std::string key) throw (SoundcloudDefaultException, boost::property_tree::json_parser::json_parser_error)
{
    *accessToken = pt.get_child(key).data();
}

void Parser::parseResponse(Upload *upload) throw (SoundcloudDefaultException, boost::property_tree::json_parser::json_parser_error)
{
    BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt) {
        upload->data[v.first.data()] = v.second.data();
    }
}