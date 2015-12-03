//
//  ImageCache.cpp
//  Facebook Notifications
//
//  Created by Jack Farrelly on 12/10/2015.
//  Copyright (c) 2015 Jack Farrelly. All rights reserved.
//

#include "ImageCache.h"

ImageCache::ImageCache()
{
    cacheDir = std::string(getenv("HOME")) + "/.FacebookNotifications/cache";
    
    if (!boost::filesystem::exists(cacheDir)) {
        boost::filesystem::create_directories(cacheDir);
    }
}

int ImageCache::prune()
{
    return 0;
}

std::string ImageCache::getFilename(const std::string userId)
{
    return cacheDir + "/" + userId + ".jpg";
}

bool ImageCache::isCached(const std::string userId)
{
    if (!boost::filesystem::exists(getFilename(userId))) {
        return false;
    }
    
    long t = std::time(nullptr) - boost::filesystem::last_write_time(getFilename(userId));
    return t < CACHE_TIME;
}

void ImageCache::cache(const std::string userId)
{
    std::string filename = getFilename(userId);
    std::fstream picture(filename, std::ios::out | std::ios::binary);
    
    Request::getInstance()->request("/" + userId + "/picture", Request::Params{Request::Param("width", "128"), Request::Param("height", "128")}, false, &picture);
    
    picture.close();
}

std::string ImageCache::fetch(const std::string userId)
{
    if (!isCached(userId)) {
        std::cout << "Caching " + getFilename(userId) << std::endl;
        cache(userId);
    } else {
        std::cout << getFilename(userId) << " loaded from cache" << std::endl;
    }
    
    return getFilename(userId);
}