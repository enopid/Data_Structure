#pragma once

#include "MyHashTable.h"

template<typename _Key_Type>
class MyUnorderedMultiSet : public MyHashTable<_Key_Type, _Key_Type, _Key_Type, true> {
    using _Key_Value_Type = _Key_Type;
    using _HashTable = MyHashTable<_Key_Type, _Key_Type, _Key_Value_Type, true>;
public:
    MyUnorderedMultiSet();
    MyUnorderedMultiSet(unsigned int max_size);
    _Key_Type _KeyExtractor(const _Key_Value_Type& _keyvalue) const;
    _Key_Type _ValueExtractor(const _Key_Value_Type& _keyvalue) const;
};

#include "MyUnorderedMultiSet.inl"
