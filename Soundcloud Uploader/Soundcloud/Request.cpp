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

static size_t progress_callback(int *ptr, double dltotal, double dlnow, double ultotal, double ulnow)
{
    *ptr = (int) (ulnow / ultotal * 100);
    
    return 0;
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

void Request::request(const std::string path, Params getParams, Params postParams, Params files, std::ostream *response, int *progress) throw(std::runtime_error)
{
    CURL *ch;
    CURLcode res;
    
    if (!(ch = curl_easy_init())) {
        throw std::runtime_error("Request: Unable to initialise curl");
    }
    
    std::string url = API_URL + path + "?";
    
    std::cout << "Request: " << url << ", post data:" << postParams.size() << ", files: " << files.size() << std::endl;
    
    struct curl_httppost *postData = nullptr, *lastPostDataPtr = nullptr;
    
    for(Params::iterator it = getParams.begin(); it != getParams.end(); ++it) {
        Param pv = static_cast<Param>(*it);
        std::string key = static_cast<std::string>(pv.first.data()),
        value = static_cast<std::string>(pv.second.data());
        
        url += "&" + std::string(curl_easy_escape(ch, key.c_str(), key.length())) + "=" + std::string(curl_easy_escape(ch, value.c_str(), value.length()));
    }
    
    for(Params::iterator it = postParams.begin(); it != postParams.end(); ++it) {
        Param pv = static_cast<Param>(*it);
        std::string key = static_cast<std::string>(pv.first.data()),
        value = static_cast<std::string>(pv.second.data());
        
        curl_formadd(
            &postData,
            &lastPostDataPtr,
            CURLFORM_COPYNAME, key.c_str(),
            CURLFORM_COPYCONTENTS, value.c_str(),
            CURLFORM_END
        );
    }
    
    for(Params::iterator it = files.begin(); it != files.end(); ++it) {
        Param pv = static_cast<Param>(*it);
        std::string key = static_cast<std::string>(pv.first.data()), path = static_cast<std::string>(pv.second.data());
        
        FILE *fd = std::fopen(path.c_str(), "rb");
        struct stat file_info;
        if (!fd) {
            throw std::runtime_error("File " + path + " not found");
        }
        if (fstat(fileno(fd), &file_info) != 0) {
            throw std::runtime_error("Unable to determine filesize of " + path);
        }
        
        curl_formadd(
            &postData,
            &lastPostDataPtr,
            CURLFORM_COPYNAME, key.c_str(),
            CURLFORM_FILE, path.c_str(),
            CURLFORM_END
        );
    }
    
    curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(ch, CURLOPT_WRITEDATA, response);
    curl_easy_setopt(ch, CURLOPT_URL, url.c_str());
    curl_easy_setopt(ch, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(ch, CURLOPT_HTTPPOST, postData);
    curl_easy_setopt(ch, CURLOPT_NOPROGRESS, false);
    curl_easy_setopt(ch, CURLOPT_PROGRESSDATA, progress);
    curl_easy_setopt(ch, CURLOPT_PROGRESSFUNCTION, progress_callback);
    
    if ((res = curl_easy_perform(ch)) != CURLE_OK) {
        throw std::runtime_error("Request: " + std::string(strerror(res)));
    }
    
    curl_formfree(postData);
    curl_easy_cleanup(ch);
}

void Request::request(const std::string path, Params getParams, Params postParams, Params files, std::ostream *response) throw(std::runtime_error)
{
    request(path, getParams, postParams, files, response, nullptr);
}

void Request::request(const std::string path, Params getParams, Params postParams, std::ostream *response) throw(std::runtime_error)
{
    request(path, getParams, postParams, Params(), response, nullptr);
}

void Request::request(const std::string path, std::ostream *response) throw(std::runtime_error)
{
    request(path, Params(), Params(), Params(), response, nullptr);
}

void Request::request(const std::string path, Params getParams, std::ostream *response) throw(std::runtime_error)
{
    request(path, getParams, Params(), Params(), response, nullptr);
}