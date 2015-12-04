//
//  FacebookException.h
//  
//
//  Created by Jack Farrelly on 21/10/2015.
//
//

#ifndef __Exception__
#define __Exception__

#include <iostream>
#include <exception>

class SoundcloudDefaultException : public std::runtime_error
{
public:
    SoundcloudDefaultException(std::string what) : std::runtime_error(what){}
};

#endif /* defined(____FacebookException__) */
