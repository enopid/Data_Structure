#pragma once

#include "MyHashTable.h"

template<typename _Key_Type>
class MyUnorderedSet : public MyHashTable<_Key_Type, _Key_Type, _Key_Type, false> {
    using _Key_Value_Type = _Key_Type;
    using _HashTable = MyHashTable<_Key_Type, _Key_Type, _Key_Value_Type, false>;
public:
    MyUnorderedSet();
    MyUnorderedSet(unsigned int max_size);
    _Key_Type _KeyExtractor(const _Key_Value_Type& _keyvalue) const;
    _Key_Type _ValueExtractor(const _Key_Value_Type& _keyvalue) const;
};

#include "MyUnorderedSet.inl"
