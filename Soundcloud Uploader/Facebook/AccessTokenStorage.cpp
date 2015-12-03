//
//  AccessTokenStorage.cpp
//  Soundcloud Uploader
//
//  Created by Jack Farrelly on 14/10/2015.
//  Copyright (c) 2015 Jack Farrelly. All rights reserved.
//

#include "AccessTokenStorage.h"

std::mutex AccessTokenStorage::mutex;
AccessTokenStorage *AccessTokenStorage::self;

AccessTokenStorage::AccessTokenStorage()
{
    std::string storageFileDir = std::string(getenv("HOME")) + "/.FacebookNotifications";
    
    if (!boost::filesystem::exists(storageFileDir)) {
        boost::filesystem::create_directories(storageFileDir);
    }
    
    storageFile = storageFileDir + "/access_token";
}

AccessTokenStorage *AccessTokenStorage::getInstance()
{
    if (self == nullptr) {
        self = new AccessTokenStorage;
    }
    
    return self;
}

AccessTokenStorage::~AccessTokenStorage()
{
    if (self != nullptr) {
        delete self;
    }
}

std::string AccessTokenStorage::getCodeFromUrl(std::string url) throw(FacebookLoginException *)
{
    std::regex errorRgx("[?|&]error_description=([\\w\\+]+)");
    std::smatch errorMatch;
    
    if (std::regex_search(url, errorMatch, errorRgx)) {
        throw new FacebookLoginException(FacebookException::urlDecode(errorMatch[1]));
    }
    
    std::regex codeRgx("[?|&]code=([\\w|\\-|_]+)");
    std::smatch codeMatch;
    
    if (!std::regex_search(url, codeMatch, codeRgx) || codeMatch.length() < 1) {
        throw new FacebookLoginException("Unable to extract code");
    }
    
    std::cout << "Code: " << codeMatch[1] << std::endl;
    
    return codeMatch[1];
}

std::string AccessTokenStorage::getAccessTokenFromCode(std::string code) throw(FacebookLoginException *)
{
    std::stringstream buffer;
    
    Request::getInstance()->request("/oauth/access_token", Request::Params{Request::Param("client_id", CLIENT_ID), Request::Param("client_secret", CLIENT_SECRET), Request::Param("redirect_uri", REDIRECT_URI), Request::Param("code", code)}, &buffer);
    
    std::regex rgx("access_token=(\\w+)");
    std::smatch match;
    
    std::string str = buffer.str();
    if (!std::regex_search(str, match, rgx) || match.length() < 1) {
        throw new FacebookLoginException("Unable to extract access token");
    }
    
    std::cout << "Access token: " << match[1] << std::endl;
    
    return match[1];
}

void AccessTokenStorage::store(std::string accessToken)
{
    std::lock_guard<std::mutex> lock(mutex);
    
    std::ofstream out(storageFile);
    out << accessToken;
    out.close();
}

std::string AccessTokenStorage::read()
{
    std::lock_guard<std::mutex> lock(mutex);
    
    std::string accessToken;
    
    std::ifstream in(storageFile, std::ios::in);
    in >> accessToken;
    in.close();
    
    return accessToken;
}