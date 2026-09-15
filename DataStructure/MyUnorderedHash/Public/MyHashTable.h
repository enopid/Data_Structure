#pragma once

#include "MyHash.h"
#include "MyList.h"

#include <algorithm>
#include <utility>

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, bool IsMulti>
class MyHashTable {
private:
    struct HashNode {
        HashNode(int _hash, HashNode* next);
        int hash;
        HashNode* _next;
        typename MyLinkedList<_Key_Value_Type>::template iterator<false> it;
    };

    virtual _Key_Type   _KeyExtractor   (const _Key_Value_Type& _keyvalue) const = 0;
    virtual _Value_Type _ValueExtractor (const _Key_Value_Type& _keyvalue) const = 0;

public:
    MyHashTable();
    MyHashTable(const MyHashTable& other);
    void DestroyBucket();
    MyHashTable& operator=(const MyHashTable& other);
    MyHashTable(MyHashTable&& other);
    MyHashTable& operator=(MyHashTable&& other);
    MyHashTable(unsigned int max_size);
    ~MyHashTable();

    HashNode* Find(_Key_Type key);
    void    Insert(_Key_Value_Type value);
    void    Remove(_Key_Type key);
    unsigned _getNextBucketSize(unsigned _minBucketSize);
    void Reserve(unsigned _minElementSize);
    void Rehash(unsigned _minBucketSize);
    void DisableAutoReHash();
    int Size();
    float MaxLoadFactor();
    typename MyLinkedList<_Key_Value_Type>::template iterator<false> begin();
    typename MyLinkedList<_Key_Value_Type>::template iterator<false> end();

private:
    unsigned int _bucketSize = 8;
    MyHash<_Key_Type> _hasher;
    HashNode** _buckets;
    bool usingAutoRehash = true;
    float _maxLoadFactor = 1.0f;
    MyLinkedList<_Key_Value_Type> _global_list;
};

#include "MyHashTable.inl"
