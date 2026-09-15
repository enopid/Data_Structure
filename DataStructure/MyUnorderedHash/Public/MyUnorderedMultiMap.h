#pragma once

#include "MyHashTable.h"
#include <stdexcept>
#include <utility>

template<typename _Key_Type, typename _Value_Type>
class MyUnorderedMultiMap : public MyHashTable<_Key_Type, _Value_Type,
                                               std::pair<_Key_Type, _Value_Type>, true> {
    using _Key_Value_Type = std::pair<_Key_Type, _Value_Type>;
    using _HashTable = MyHashTable<_Key_Type, _Value_Type, _Key_Value_Type, true>;
public:
    MyUnorderedMultiMap();
    MyUnorderedMultiMap(unsigned int max_size);
    _Key_Type _KeyExtractor(const _Key_Value_Type& _keyvalue) const;
    _Value_Type _ValueExtractor(const _Key_Value_Type& _keyvalue) const;
};

#include "MyUnorderedMultiMap.inl"
