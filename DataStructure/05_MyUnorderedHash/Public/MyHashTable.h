#pragma once

#include "MyHash.h"
#include "MyList.h"

#include <algorithm>
#include <utility>

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, bool IsMulti>
class MyHashTable {
    using _lstNode = typename MyLinkedList<_Key_Value_Type>::Node;
private:
    virtual _Key_Type   _key_extractor   (const _Key_Value_Type& _keyvalue) const = 0;
    virtual _Value_Type _value_extractor (const _Key_Value_Type& _keyvalue) const = 0;

public:
    MyHashTable();
    MyHashTable(const MyHashTable& other);
    void DestroyBucket();
    MyHashTable& operator=(const MyHashTable& other);
    MyHashTable(MyHashTable&& other);
    MyHashTable& operator=(MyHashTable&& other);
    MyHashTable(unsigned int max_size);
    ~MyHashTable();

    _lstNode* find(_Key_Type key);
    void    insert(_Key_Value_Type value);
    void    remove(_Key_Type key);
    unsigned _get_next_bucketSize(unsigned _minBucketSize);
    void reserve(unsigned _minElementSize);
    void rehash(unsigned _minBucketSize);
    void disable_auto_rehash();
    int size();
    float get_max_load_factor();
    typename MyLinkedList<_Key_Value_Type>::template iterator<false> begin();
    typename MyLinkedList<_Key_Value_Type>::template iterator<false> end();

private:
    unsigned int iBucketSize = 8;
    MyHash<_Key_Type> hasher;
    bool bUsingAutoRehash = true;
    float fMaxLoadFactor = 1.0f;

    std::pair<_lstNode*, _lstNode*>* arrBuckets;
    MyLinkedList<_Key_Value_Type> lstGlobalNodes;
};

#include "MyHashTable.inl"
