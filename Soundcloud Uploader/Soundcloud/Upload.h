//
//  Upload.h
//  Soundcloud Uploader
//
//  Created by Jack Farrelly on 19/10/2015.
//  Copyright (c) 2015 Jack Farrelly. All rights reserved.
//

#ifndef __UPLOAD__
#define __UPLOAD__

typedef struct {
    std::string name;
    std::string path;
    size_t progress = 0;
    std::string url;
} Upload;

#endif
