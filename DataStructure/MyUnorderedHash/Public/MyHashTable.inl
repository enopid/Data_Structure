#include "MyHashTable.h"
#pragma once

template<typename K, typename V, typename KV, bool M>
inline MyHashTable<K, V, KV, M>::HashNode::HashNode(int _hash, HashNode* next)
    : hash(_hash), _next(next) {}

template<typename K, typename V, typename KV, bool M>
inline MyHashTable<K, V, KV, M>::MyHashTable() {
    _buckets = new HashNode*[_bucketSize];
    for (size_t i = 0; i < _bucketSize; i++) _buckets[i] = nullptr;
    _hasher = MyHash<K>();
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, bool IsMulti>
inline MyHashTable<_Key_Type, _Value_Type, _Key_Value_Type, IsMulti>::MyHashTable(const MyHashTable& other)
{
    _bucketSize     = other._bucketSize;
    _hasher         = other._hasher;
    usingAutoRehash = other.usingAutoRehash;
    _maxLoadFactor  = other._maxLoadFactor;
    _global_list    = other._global_list;

    _buckets        = new HashNode * [_bucketSize];
    for (size_t i = 0; i < _bucketSize; i++) _buckets[i] = nullptr;
    for (auto it = _global_list.begin(); it != _global_list.end(); it++) {
        int hash = _hasher(other._KeyExtractor(*it)) % _bucketSize;
        HashNode* curHashNode = _buckets[hash];
        _buckets[hash] = new HashNode(hash, curHashNode);
        _buckets[hash]->it = it;
    }
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, bool IsMulti>
inline void MyHashTable<_Key_Type, _Value_Type, _Key_Value_Type, IsMulti>::DestroyBucket()
{
    for (size_t i = 0; i < _bucketSize; i++) {
        HashNode* curHashNode = _buckets[i];
        while (curHashNode != nullptr) {
            auto tmpHashNode = curHashNode;
            curHashNode = curHashNode->_next;
            delete tmpHashNode;
        }
    }
    delete[] _buckets;
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, bool IsMulti>
inline MyHashTable<_Key_Type, _Value_Type, _Key_Value_Type, IsMulti>& MyHashTable<_Key_Type, _Value_Type, _Key_Value_Type, IsMulti>::operator=(const MyHashTable& other)
{
    if (this == &other) return *this;

    DestroyBucket();

    _bucketSize     = other._bucketSize;
    _hasher         = other._hasher;
    usingAutoRehash = other.usingAutoRehash;
    _maxLoadFactor  = other._maxLoadFactor;
    _global_list    = other._global_list;

    _buckets = new HashNode * [_bucketSize];
    for (size_t i = 0; i < _bucketSize; i++) _buckets[i] = nullptr;
    for (auto it = _global_list.begin(); it != _global_list.end(); it++) {
        int hash = _hasher(other._KeyExtractor(*it)) % _bucketSize;
        HashNode* curHashNode = _buckets[hash];
        _buckets[hash] = new HashNode(hash, curHashNode);
        _buckets[hash]->it = it;
    }

    return *this;
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, bool IsMulti>
inline MyHashTable<_Key_Type, _Value_Type, _Key_Value_Type, IsMulti>::MyHashTable(MyHashTable&& other)
{
    _bucketSize     = other._bucketSize;
    _hasher         = other._hasher;
    usingAutoRehash = other.usingAutoRehash;
    _maxLoadFactor  = other._maxLoadFactor;
    _global_list    = std::move(other._global_list);
    _buckets        = other._buckets;

    other._buckets  = new HashNode * [_bucketSize];
    for (size_t i = 0; i < _bucketSize; i++) other._buckets[i] = nullptr;
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, bool IsMulti>
inline MyHashTable<_Key_Type, _Value_Type, _Key_Value_Type, IsMulti>& MyHashTable<_Key_Type, _Value_Type, _Key_Value_Type, IsMulti>::operator=(MyHashTable&& other)
{
    if (this == &other) return *this;

    DestroyBucket();

    _bucketSize     = other._bucketSize;
    _hasher         = other._hasher;
    usingAutoRehash = other.usingAutoRehash;
    _maxLoadFactor  = other._maxLoadFactor;
    _global_list    = std::move(other._global_list);
    _buckets        = other._buckets;

    other._buckets = new HashNode * [_bucketSize];
    for (size_t i = 0; i < _bucketSize; i++) other._buckets[i] = nullptr;

    return *this;
}

template<typename K, typename V, typename KV, bool M>
inline MyHashTable<K, V, KV, M>::MyHashTable(unsigned int max_size) {
    _bucketSize = _getNextBucketSize(max_size);
    _buckets = new HashNode*[_bucketSize];
    for (size_t i = 0; i < _bucketSize; i++) _buckets[i] = nullptr;
    _hasher = MyHash<K>();
}

template<typename K, typename V, typename KV, bool M>
inline MyHashTable<K, V, KV, M>::~MyHashTable() {
    DestroyBucket();
}

template<typename K, typename V, typename KV, bool M>
inline typename MyHashTable<K, V, KV, M>::HashNode*
MyHashTable<K, V, KV, M>::Find(K key) {
    int hash = _hasher(key) % _bucketSize;
    HashNode* curHashNode = _buckets[hash];
    while (curHashNode != nullptr) {
        if (_KeyExtractor(*(curHashNode->it)) == key) return curHashNode;
        curHashNode = curHashNode->_next;
    }
    return nullptr;
}

template<typename K, typename V, typename KV, bool M>
inline void MyHashTable<K, V, KV, M>::Insert(KV value) {
    if constexpr (!M)
        if (Find(_KeyExtractor(value)) != nullptr) return;

    if (usingAutoRehash) Rehash(0);

    int hash = _hasher(_KeyExtractor(value)) % _bucketSize;
    HashNode* curHashNode = _buckets[hash];
    _buckets[hash] = new HashNode(hash, curHashNode);
    _global_list.push_front(value);
    _buckets[hash]->it = _global_list.begin();
}

template<typename K, typename V, typename KV, bool M>
inline void MyHashTable<K, V, KV, M>::Remove(K key) {
    int hash = _hasher(key) % _bucketSize;
    HashNode* curHashNode = _buckets[hash];
    HashNode* prevHashNode = nullptr;
    while (curHashNode != nullptr) {
        if (_KeyExtractor(*(curHashNode->it)) == key) {
            if (prevHashNode == nullptr) _buckets[hash] = curHashNode->_next;
            else prevHashNode->_next = curHashNode->_next;

            _global_list.erase(curHashNode->it);
            HashNode* tmp = curHashNode->_next;
            delete curHashNode;
            curHashNode = tmp;
            if constexpr (!M) return;
        } else {
            prevHashNode = curHashNode;
            curHashNode = curHashNode->_next;
        }
    }
}

template<typename K, typename V, typename KV, bool M>
inline unsigned MyHashTable<K, V, KV, M>::_getNextBucketSize(unsigned _minBucketSize) {
    _minBucketSize = std::max(_minBucketSize, unsigned(_global_list.size() / MaxLoadFactor()));
    if (_minBucketSize < _bucketSize) return _bucketSize;
    unsigned _newBucketSize = _bucketSize;
    while (_minBucketSize > _newBucketSize) _newBucketSize <<= 1;
    return _newBucketSize;
}

template<typename K, typename V, typename KV, bool M>
inline void MyHashTable<K, V, KV, M>::Reserve(unsigned _minElementSize) {
    Rehash(_minElementSize / MaxLoadFactor());
}

template<typename K, typename V, typename KV, bool M>
inline void MyHashTable<K, V, KV, M>::Rehash(unsigned _minBucketSize) {
    unsigned _newBucketSize = _getNextBucketSize(_minBucketSize);
    if (_newBucketSize <= _bucketSize) return;


    auto _tmpBucketSize = _newBucketSize;
    auto _tmpBuckets    = new HashNode*[_newBucketSize];
    for (size_t i = 0; i < _tmpBucketSize; i++) _tmpBuckets[i] = nullptr;

    for (auto it = _global_list.begin(); it != _global_list.end(); it++) {
        int hash = _hasher(_KeyExtractor(*it)) % _tmpBucketSize;
        HashNode* curHashNode = _tmpBuckets[hash];
        _tmpBuckets[hash] = new HashNode(hash, curHashNode);
        _tmpBuckets[hash]->it = it;
    }

    DestroyBucket();
    _buckets    = _tmpBuckets;
    _bucketSize = _tmpBucketSize;
}

template<typename K, typename V, typename KV, bool M>
inline void MyHashTable<K, V, KV, M>::DisableAutoReHash() {
    usingAutoRehash = false;
}

template<typename K, typename V, typename KV, bool M>
inline int MyHashTable<K, V, KV, M>::Size() {
    return _global_list.size();
}

template<typename K, typename V, typename KV, bool M>
inline float MyHashTable<K, V, KV, M>::MaxLoadFactor() {
    return _maxLoadFactor;
}

template<typename K, typename V, typename KV, bool M>
inline typename MyLinkedList<KV>::template iterator<false>
MyHashTable<K, V, KV, M>::begin() {
    return _global_list.begin();
}

template<typename K, typename V, typename KV, bool M>
inline typename MyLinkedList<KV>::template iterator<false>
MyHashTable<K, V, KV, M>::end() {
    return _global_list.end();
}
