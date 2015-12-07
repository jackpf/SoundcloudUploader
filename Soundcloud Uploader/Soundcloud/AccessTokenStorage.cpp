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
    std::string storageFileDir = std::string(getenv("HOME")) + "/" + STORAGE_DIR;
    
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

std::string AccessTokenStorage::getCodeFromUrl(std::string url) throw(SoundcloudDefaultException)
{
    std::regex codeRgx("[?|&]code=([\\w|\\-|_]+)");
    std::smatch codeMatch;
    
    if (!std::regex_search(url, codeMatch, codeRgx) || codeMatch.length() < 1) {
        throw SoundcloudDefaultException("Unable to extract code");
    }
    
    std::cout << "Code: " << codeMatch[1] << std::endl;
    
    return codeMatch[1];
}

std::string AccessTokenStorage::storeAccessTokenFromCode(std::string code) throw(SoundcloudDefaultException)
{
    std::stringstream buffer;
    
    Request::getInstance()->request(TOKEN_PATH, Request::Params(), Request::Params{Request::Param("client_id", CLIENT_ID), Request::Param("client_secret", CLIENT_SECRET), Request::Param("redirect_uri", REDIRECT_URI), Request::Param("code", code), Request::Param("grant_type", "authorization_code")}, &buffer);
    
    std:: string accessToken;
    parser.parseAccessToken(&buffer, &accessToken);
    
    std::cout << "Access token: " << accessToken << std::endl;
    
    store(buffer.str());
    
    return accessToken;
}

std::string AccessTokenStorage::readAccessToken() throw(SoundcloudDefaultException)
{
    std::string tokenData = read(), accessToken;
    
    if (tokenData.length() > 0) {
        try {
            std::stringstream buf(tokenData);
            parser.parseAccessToken(&buf, &accessToken);
        } catch (boost::property_tree::json_parser::json_parser_error e) {
            // Corrupted data?
            store("");
            throw SoundcloudDefaultException("Corrupted access token data, deleting data");
        }
    }
    
    return accessToken;
}