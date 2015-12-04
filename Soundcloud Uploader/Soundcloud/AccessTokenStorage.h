//
//  AccessTokenStorage.h
//  Facebook Notifications
//
//  Created by Jack Farrelly on 14/10/2015.
//  Copyright (c) 2015 Jack Farrelly. All rights reserved.
//

#ifndef __Facebook_Notifications__AccessTokenStorage__
#define __Facebook_Notifications__AccessTokenStorage__

#include <iostream>
#include <fstream>
#include <vector>
#include <boost/filesystem.hpp>
#include <map>
#include <regex>
#include <mutex>
#include "Request.h"
#include "Parser.h"
#include "Data.h"
#include "Exception.h"

class AccessTokenStorage
{
private:
    static AccessTokenStorage *self;
    std::string storageFile;
    static std::mutex mutex;
    Parser parser;
    
    AccessTokenStorage();
    void store(std::string);
    std::string read();
    
public:
    ~AccessTokenStorage();
    static AccessTokenStorage *getInstance();
    std::string storeAccessTokenFromCode(std::string) throw(SoundcloudDefaultException);
    std::string getCodeFromUrl(std::string) throw(SoundcloudDefaultException);
    std::string readAccessToken() throw(SoundcloudDefaultException);
};

#endif /* defined(__Facebook_Notifications__AccessTokenStorage__) */
