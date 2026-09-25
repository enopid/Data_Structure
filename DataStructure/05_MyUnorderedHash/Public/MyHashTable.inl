#include "MyHashTable.h"
#pragma once

template<typename K, typename V, typename KV, bool M>
inline MyHashTable<K, V, KV, M>::MyHashTable() {
    _buckets = new std::pair<_lstNode*, _lstNode*>[_bucketSize];
    for (size_t i = 0; i < _bucketSize; i++) _buckets[i] = { nullptr , nullptr };
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

    _buckets        = new  std::pair<_lstNode*, _lstNode*> [_bucketSize];
    for (size_t i = 0; i < _bucketSize; i++) _buckets[i] = { nullptr , nullptr };
    for (auto it = _global_list.begin(); it != _global_list.end(); it++) {
        int hash = _hasher(other._KeyExtractor(*it)) % _bucketSize;

        if (!_buckets[hash].first) 
            _buckets[hash] = { it._ptr, it._ptr };
        else
            _buckets[hash].second = it._ptr;
    }
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, bool IsMulti>
inline void MyHashTable<_Key_Type, _Value_Type, _Key_Value_Type, IsMulti>::DestroyBucket()
{
    delete[] _buckets;
    _buckets = nullptr;
    _bucketSize = 0;
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

    _buckets = new  std::pair<_lstNode*, _lstNode*> [_bucketSize];
    for (size_t i = 0; i < _bucketSize; i++) _buckets[i] = { nullptr , nullptr };
    for (auto it = _global_list.begin(); it != _global_list.end(); it++) {
        int hash = _hasher(other._KeyExtractor(*it)) % _bucketSize;

        if (!_buckets[hash].first)
            _buckets[hash] = { it._ptr, it._ptr };
        else
            _buckets[hash].second = it._ptr;
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

    other._buckets = new  std::pair<_lstNode*, _lstNode*> [_bucketSize];
    for (size_t i = 0; i < _bucketSize; i++) other._buckets[i] = { nullptr , nullptr };
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

    other._buckets = new  std::pair<_lstNode*, _lstNode*> [_bucketSize];
    for (size_t i = 0; i < _bucketSize; i++) other._buckets[i] = { nullptr , nullptr };

    return *this;
}

template<typename K, typename V, typename KV, bool M>
inline MyHashTable<K, V, KV, M>::MyHashTable(unsigned int max_size) {
    _bucketSize = _getNextBucketSize(max_size);
    _buckets = new  std::pair<_lstNode*, _lstNode*> [_bucketSize];
    for (size_t i = 0; i < _bucketSize; i++) _buckets[i] = { nullptr , nullptr };
    _hasher = MyHash<K>();
}

template<typename K, typename V, typename KV, bool M>
inline MyHashTable<K, V, KV, M>::~MyHashTable() {
    DestroyBucket();
}

template<typename K, typename V, typename KV, bool M>
typename MyHashTable<K, V, KV, M>::_lstNode*
MyHashTable<K, V, KV, M>::Find(K key) {
    int hash = _hasher(key) % _bucketSize;
    _lstNode* curNode = _buckets[hash].first;
    while (curNode) {
        if (_KeyExtractor(curNode->data) == key) return curNode;
        curNode = curNode->next;
        if (_buckets[hash].second == curNode->prev) break;
    }
    return nullptr;
}

template<typename K, typename V, typename KV, bool M>
inline void MyHashTable<K, V, KV, M>::Insert(KV value) {
    if constexpr (!M)
        if (Find(_KeyExtractor(value)) != nullptr) return;

    if (usingAutoRehash) Rehash(0);

    int hash = _hasher(_KeyExtractor(value)) % _bucketSize;
    
    _lstNode* _curNode = nullptr;
    if (_buckets[hash].first) {
        _curNode = new _lstNode{ value, _buckets[hash].first->prev, _buckets[hash].first };
    }
    else {
        _curNode = new _lstNode{ value, _global_list._SentinelNode->prev, _global_list._SentinelNode };
        _buckets[hash].second = _curNode;
    }

    _buckets[hash].first = _curNode;
    _curNode->prev->next = _buckets[hash].first;
    _curNode->next->prev = _buckets[hash].first;
    _global_list._size++;
}

template<typename K, typename V, typename KV, bool M>
inline void MyHashTable<K, V, KV, M>::Remove(K key) {
    int hash = _hasher(key) % _bucketSize;
    _lstNode* curNode = _buckets[hash].first;
    while (curNode) {
        if (_KeyExtractor(curNode->data) == key) {
            if (_buckets[hash].first == curNode && _buckets[hash].second == curNode) {
                _buckets[hash].first = _buckets[hash].second = nullptr;
            }
            else if (_buckets[hash].first  == curNode) {
                _buckets[hash].first = curNode->next;
            }
            else if (_buckets[hash].second == curNode) {
                _buckets[hash].second = curNode->prev;
            }


            _lstNode* deletedNode = curNode;
            curNode = deletedNode->next;
            delete deletedNode;
            _global_list._size--;

            if constexpr (!M) return;
        } 
        else {
            curNode  = curNode->next;
        }

        if (curNode->prev == _buckets[hash].second) break;
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


    DestroyBucket();
    _bucketSize = _newBucketSize;
    _buckets = new  std::pair<_lstNode*, _lstNode*>[_bucketSize];
    for (size_t i = 0; i < _bucketSize; i++) _buckets[i] = { nullptr , nullptr };

    auto _tmpAry = new _lstNode*[_global_list.size()];
    int iCnt = 0;
    for (auto it = _global_list.begin(); it != _global_list.end(); it++) {
        _tmpAry[iCnt++] = it._ptr;
    }

    _global_list._SentinelNode->prev = _global_list._SentinelNode;
    _global_list._SentinelNode->next = _global_list._SentinelNode;
    for (int i = 0; i < _global_list.size(); i++) {
        int hash = _hasher(_KeyExtractor(_tmpAry[i]->data)) % _bucketSize;

        if (!_buckets[hash].first) {
            _tmpAry[i]->next = _global_list._SentinelNode;
            _tmpAry[i]->prev = _global_list._SentinelNode->prev;

            _global_list._SentinelNode->prev->next  = _tmpAry[i];
            _global_list._SentinelNode->prev        = _tmpAry[i];

            _buckets[hash] = { _tmpAry[i], _tmpAry[i] };
        }
        else {
            _tmpAry[i]->next = _buckets[hash].first;
            _tmpAry[i]->prev = _buckets[hash].first->prev;

            _buckets[hash].first->prev->next = _tmpAry[i];
            _buckets[hash].first->prev = _tmpAry[i];

            _buckets[hash].first = _tmpAry[i];
        }
    }

    delete[] _tmpAry;
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
