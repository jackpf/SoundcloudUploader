//
//  Request.cpp
//  Facebook Notifications
//
//  Created by Jack Farrelly on 10/10/2015.
//  Copyright (c) 2015 Jack Farrelly. All rights reserved.
//

#include "Request.h"

Request *Request::self;

static size_t write_callback(char *ptr, size_t size, size_t nmemb, std::ostream *userdata)
{
    size_t len = size * nmemb;
    
    if (userdata != nullptr) {
        userdata->write(ptr, len);
        userdata->flush();
    }
    
    return len;
}

Request::Request()
{
    
}

Request *Request::getInstance()
{
    if (self == nullptr) {
        self = new Request;
    }
    
    return self;
}

Request::~Request()
{
    if (self != nullptr) {
        delete self;
    }
}

void Request::request(const std::string path, Params params, bool post, std::ostream *response) throw(std::runtime_error)
{
    CURL *ch;
    CURLcode res;
    
    if (!(ch = curl_easy_init())) {
        throw std::runtime_error("Request: Unable to initialise curl");
    }
    
    std::string url = FB_API_URL + path + "?";
    
    for(Params::iterator it = params.begin(); it != params.end(); ++it) {
        Param pv = static_cast<Param>(*it);
        url += "&" + static_cast<std::string>(pv.first.data()) + "=" + static_cast<std::string>(pv.second.data());
    }
    
    curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(ch, CURLOPT_WRITEDATA, response);
    curl_easy_setopt(ch, CURLOPT_URL, url.c_str());
    curl_easy_setopt(ch, CURLOPT_FOLLOWLOCATION, 1);
    
    if (post) {
        curl_easy_setopt(ch, CURLOPT_POST, 1);
        curl_easy_setopt(ch, CURLOPT_POSTFIELDS, "");
        curl_easy_setopt(ch, CURLOPT_POSTFIELDSIZE, 0);
    }
    
    if ((res = curl_easy_perform(ch)) != CURLE_OK) {
        throw std::runtime_error("Request: " + std::string(strerror(res)));
    }
    
    curl_easy_cleanup(ch);
}

void Request::request(const std::string path, std::ostream *response) throw(std::runtime_error)
{
    request(path, Params(), false, response);
}

void Request::request(const std::string path, Params params, std::ostream *response) throw(std::runtime_error)
{
    request(path, params, false, response);
}