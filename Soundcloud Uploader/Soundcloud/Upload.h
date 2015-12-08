//
//  Upload.h
//  Soundcloud Uploader
//
//  Created by Jack Farrelly on 19/10/2015.
//  Copyright (c) 2015 Jack Farrelly. All rights reserved.
//

#include <unordered_map>

#ifndef __UPLOAD__
#define __UPLOAD__

typedef struct {
    std::string name;
    std::string path;
    size_t progress = 0;
    std::string url;
    bool cancelRequest = false;
    std::unordered_map<std::string, std::string> data;
} Upload;

#endif
