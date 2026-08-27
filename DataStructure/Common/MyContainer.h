#pragma once
 
#include<string> 
#include<iostream> 

class MyContainer abstract {
public:
    virtual void print_info()       = 0;
    virtual void print_elements()   = 0;
};