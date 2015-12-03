//
//  ImageCache.h
//  Facebook Notifications
//
//  Created by Jack Farrelly on 12/10/2015.
//  Copyright (c) 2015 Jack Farrelly. All rights reserved.
//

#ifndef __Facebook_Notifications__ImageCache__
#define __Facebook_Notifications__ImageCache__

#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <vector>
#include <ctime>
#include "Request.h"

#define CACHE_TIME 60 * 60 * 24;

class ImageCache
{
private:
    std::string accessToken;
    std::string cacheDir;
    
    std::string getFilename(const std::string);
    bool isCached(const std::string);
    void cache(const std::string);
    
public:
    ImageCache();
    int prune();
    std::string fetch(const std::string);
};

#endif /* defined(__Facebook_Notifications__ImageCache__) */
