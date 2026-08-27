#pragma once


#include <algorithm>
#include <utility>
#include <string> 
#include <iostream> 

class MyContainer abstract {
public:
    virtual void print_info()       = 0;
    virtual void print_elements()   = 0;
};