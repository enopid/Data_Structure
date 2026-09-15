#pragma once

#include "MyHashTable.h"
#include <stdexcept>
#include <utility>

template<typename _Key_Type, typename _Value_Type>
class MyUnorderedMap : public MyHashTable<_Key_Type, _Value_Type,
                                          std::pair<_Key_Type, _Value_Type>, false> {
    using _Key_Value_Type = std::pair<_Key_Type, _Value_Type>;
    using _HashTable = MyHashTable<_Key_Type, _Value_Type, _Key_Value_Type, false>;
public:
    MyUnorderedMap();
    MyUnorderedMap(unsigned int max_size);
    _Key_Type _KeyExtractor(const _Key_Value_Type& _keyvalue) const;
    _Value_Type _ValueExtractor(const _Key_Value_Type& _keyvalue) const;
    _Value_Type& at(const _Key_Type key);
    _Value_Type& operator[](const _Key_Type key);
};

#include "MyUnorderedMap.inl"
