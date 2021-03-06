//
//  Request.h
//  Facebook Notifications
//
//  Created by Jack Farrelly on 10/10/2015.
//  Copyright (c) 2015 Jack Farrelly. All rights reserved.
//

#ifndef __Facebook_Notifications__Request__
#define __Facebook_Notifications__Request__

#include <stdio.h>
#include <iostream>
#include <exception>
#include <sstream>
#include <vector>
#include "curlcpp/curl_easy.h"
#include "Data.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

class Request
{
private:
    static Request *self;
    
    Request();
    
public:
    typedef std::pair<std::string, std::string> Param;
    typedef std::vector<Param> Params;
    
    ~Request();
    static Request *getInstance();
    void request(const std::string, Params, Params, Params, std::ostream *, size_t (*progressFuncPtr)(void *, double, double, double, double), void *) throw(std::runtime_error);
    void request(const std::string, Params, Params, Params, std::ostream *) throw(std::runtime_error);
    void request(const std::string, Params, Params, std::ostream *) throw(std::runtime_error);
    void request(const std::string, std::ostream *) throw(std::runtime_error);
    void request(const std::string, Params, std::ostream *) throw(std::runtime_error);
};

#endif /* defined(__Facebook_Notifications__Request__) */
