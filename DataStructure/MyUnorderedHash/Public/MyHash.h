#pragma once

#include <string>

template<typename T>
class MyHash {
public:
    enum HashType { None, MidSquare, Double, Multiple };
    MyHash();
    unsigned int operator()(T value);

private:
    HashType _hashType = HashType::None;
    unsigned int ConvertToInt(T value);
};

template<>
unsigned int MyHash<std::string>::ConvertToInt(std::string value);

#include "MyHash.inl"
