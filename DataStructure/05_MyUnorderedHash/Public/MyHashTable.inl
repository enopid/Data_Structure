#include "MyHashTable.h"
#pragma once

template<typename K, typename V, typename KV, bool M>
inline MyHashTable<K, V, KV, M>::MyHashTable() {
    arrBuckets = new std::pair<_lstNode*, _lstNode*>[iBucketSize];
    for (size_t i = 0; i < iBucketSize; i++) arrBuckets[i] = { nullptr , nullptr };
    hasher = MyHash<K>();
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, bool IsMulti>
inline MyHashTable<_Key_Type, _Value_Type, _Key_Value_Type, IsMulti>::MyHashTable(const MyHashTable& other)
{
    iBucketSize     = other.iBucketSize;
    hasher         = other.hasher;
    bUsingAutoRehash = other.bUsingAutoRehash;
    fMaxLoadFactor  = other.fMaxLoadFactor;
    lstGlobalNodes    = other.lstGlobalNodes;

    arrBuckets        = new  std::pair<_lstNode*, _lstNode*> [iBucketSize];
    for (size_t i = 0; i < iBucketSize; i++) arrBuckets[i] = { nullptr , nullptr };
    for (auto it = lstGlobalNodes.begin(); it != lstGlobalNodes.end(); it++) {
        int hash = hasher(other._key_extractor(*it)) % iBucketSize;

        if (!arrBuckets[hash].first)
            arrBuckets[hash] = { it.pNode, it.pNode };
        else
            arrBuckets[hash].second = it.pNode;
    }
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, bool IsMulti>
inline void MyHashTable<_Key_Type, _Value_Type, _Key_Value_Type, IsMulti>::DestroyBucket()
{
    delete[] arrBuckets;
    arrBuckets = nullptr;
    iBucketSize = 0;
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, bool IsMulti>
inline MyHashTable<_Key_Type, _Value_Type, _Key_Value_Type, IsMulti>& MyHashTable<_Key_Type, _Value_Type, _Key_Value_Type, IsMulti>::operator=(const MyHashTable& other)
{
    if (this == &other) return *this;

    DestroyBucket();

    iBucketSize     = other.iBucketSize;
    hasher         = other.hasher;
    bUsingAutoRehash = other.bUsingAutoRehash;
    fMaxLoadFactor  = other.fMaxLoadFactor;
    lstGlobalNodes    = other.lstGlobalNodes;

    arrBuckets = new  std::pair<_lstNode*, _lstNode*> [iBucketSize];
    for (size_t i = 0; i < iBucketSize; i++) arrBuckets[i] = { nullptr , nullptr };
    for (auto it = lstGlobalNodes.begin(); it != lstGlobalNodes.end(); it++) {
        int hash = hasher(other._key_extractor(*it)) % iBucketSize;

        if (!arrBuckets[hash].first)
            arrBuckets[hash] = { it.pNode, it.pNode };
        else
            arrBuckets[hash].second = it.pNode;
    }

    return *this;
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, bool IsMulti>
inline MyHashTable<_Key_Type, _Value_Type, _Key_Value_Type, IsMulti>::MyHashTable(MyHashTable&& other)
{
    iBucketSize     = other.iBucketSize;
    hasher         = other.hasher;
    bUsingAutoRehash = other.bUsingAutoRehash;
    fMaxLoadFactor  = other.fMaxLoadFactor;
    lstGlobalNodes    = std::move(other.lstGlobalNodes);
    arrBuckets        = other.arrBuckets;

    other.arrBuckets = new  std::pair<_lstNode*, _lstNode*> [iBucketSize];
    for (size_t i = 0; i < iBucketSize; i++) other.arrBuckets[i] = { nullptr , nullptr };
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, bool IsMulti>
inline MyHashTable<_Key_Type, _Value_Type, _Key_Value_Type, IsMulti>& MyHashTable<_Key_Type, _Value_Type, _Key_Value_Type, IsMulti>::operator=(MyHashTable&& other)
{
    if (this == &other) return *this;

    DestroyBucket();

    iBucketSize     = other.iBucketSize;
    hasher          = other.hasher;
    bUsingAutoRehash= other.bUsingAutoRehash;
    fMaxLoadFactor  = other.fMaxLoadFactor;
    lstGlobalNodes  = std::move(other.lstGlobalNodes);
    arrBuckets      = other.arrBuckets;

    other.arrBuckets = new  std::pair<_lstNode*, _lstNode*> [iBucketSize];
    for (size_t i = 0; i < iBucketSize; i++) other.arrBuckets[i] = { nullptr , nullptr };

    return *this;
}

template<typename K, typename V, typename KV, bool M>
inline MyHashTable<K, V, KV, M>::MyHashTable(unsigned int max_size) {
    iBucketSize = _get_next_bucketSize(max_size);
    arrBuckets = new  std::pair<_lstNode*, _lstNode*> [iBucketSize];
    for (size_t i = 0; i < iBucketSize; i++) arrBuckets[i] = { nullptr , nullptr };
    hasher = MyHash<K>();
}

template<typename K, typename V, typename KV, bool M>
inline MyHashTable<K, V, KV, M>::~MyHashTable() {
    DestroyBucket();
}

template<typename K, typename V, typename KV, bool M>
typename MyHashTable<K, V, KV, M>::_lstNode*
MyHashTable<K, V, KV, M>::find(K key) {
    int hash = hasher(key) % iBucketSize;
    _lstNode* _pCurNode = arrBuckets[hash].first;
    while (_pCurNode) {
        if (_key_extractor(_pCurNode->data) == key) return _pCurNode;
        _pCurNode = _pCurNode->next;
        if (arrBuckets[hash].second == _pCurNode->prev) break;
    }
    return nullptr;
}

template<typename K, typename V, typename KV, bool M>
inline void MyHashTable<K, V, KV, M>::insert(KV value) {
    if constexpr (!M)
        if (find(_key_extractor(value)) != nullptr) return;

    if (bUsingAutoRehash) rehash(0);

    int hash = hasher(_key_extractor(value)) % iBucketSize;

    _lstNode* _pCurNode = nullptr;
    if (arrBuckets[hash].first) {
        _pCurNode = new _lstNode{ value, arrBuckets[hash].first->prev, arrBuckets[hash].first };
    }
    else {
        _pCurNode = new _lstNode{ value, lstGlobalNodes.pSentinelNode->prev, lstGlobalNodes.pSentinelNode };
        arrBuckets[hash].second = _pCurNode;
    }

    arrBuckets[hash].first = _pCurNode;
    _pCurNode->prev->next = arrBuckets[hash].first;
    _pCurNode->next->prev = arrBuckets[hash].first;
    lstGlobalNodes.iSize++;
}

template<typename K, typename V, typename KV, bool M>
inline void MyHashTable<K, V, KV, M>::remove(K key) {
    int hash = hasher(key) % iBucketSize;
    _lstNode* _pCurNode = arrBuckets[hash].first;
    _lstNode* _pEndNode = arrBuckets[hash].second
        ? arrBuckets[hash].second->next : nullptr;
    while (_pCurNode != _pEndNode) {
        if (_key_extractor(_pCurNode->data) == key) {
            if (arrBuckets[hash].first == _pCurNode && arrBuckets[hash].second == _pCurNode) {
                arrBuckets[hash].first = arrBuckets[hash].second = nullptr;
            }
            else if (arrBuckets[hash].first  == _pCurNode) {
                arrBuckets[hash].first = _pCurNode->next;
            }
            else if (arrBuckets[hash].second == _pCurNode) {
                arrBuckets[hash].second = _pCurNode->prev;
            }

            _lstNode* deletedNode = _pCurNode;
            _pCurNode = deletedNode->next;
            delete deletedNode;
            lstGlobalNodes.iSize--;

            if constexpr (!M) return;
        } 
        else {
            _pCurNode  = _pCurNode->next;
        }

    }
}

template<typename K, typename V, typename KV, bool M>
inline unsigned MyHashTable<K, V, KV, M>::_get_next_bucketSize(unsigned _minBucketSize) {
    _minBucketSize = std::max(_minBucketSize, unsigned(lstGlobalNodes.size() / get_max_load_factor()));
    if (_minBucketSize < iBucketSize) return iBucketSize;
    unsigned _newBucketSize = iBucketSize;
    while (_minBucketSize > _newBucketSize) _newBucketSize <<= 1;
    return _newBucketSize;
}

template<typename K, typename V, typename KV, bool M>
inline void MyHashTable<K, V, KV, M>::reserve(unsigned _minElementSize) {
    rehash(static_cast<unsigned>(_minElementSize / get_max_load_factor()));
}

template<typename K, typename V, typename KV, bool M>
inline void MyHashTable<K, V, KV, M>::rehash(unsigned _minBucketSize) {
    unsigned _newBucketSize = _get_next_bucketSize(_minBucketSize);
    if (_newBucketSize <= iBucketSize) return;


    DestroyBucket();
    iBucketSize = _newBucketSize;
    arrBuckets = new  std::pair<_lstNode*, _lstNode*>[iBucketSize];
    for (size_t i = 0; i < iBucketSize; i++) arrBuckets[i] = { nullptr , nullptr };

    auto _tmpAry = new _lstNode*[lstGlobalNodes.size()];
    int iCnt = 0;
    for (auto it = lstGlobalNodes.begin(); it != lstGlobalNodes.end(); it++) {
        _tmpAry[iCnt++] = it.pNode;
    }

    lstGlobalNodes.pSentinelNode->prev = lstGlobalNodes.pSentinelNode;
    lstGlobalNodes.pSentinelNode->next = lstGlobalNodes.pSentinelNode;
    for (int i = 0; i < lstGlobalNodes.size(); i++) {
        int hash = hasher(_key_extractor(_tmpAry[i]->data)) % iBucketSize;

        if (!arrBuckets[hash].first) {
            _tmpAry[i]->next = lstGlobalNodes.pSentinelNode;
            _tmpAry[i]->prev = lstGlobalNodes.pSentinelNode->prev;

            lstGlobalNodes.pSentinelNode->prev->next  = _tmpAry[i];
            lstGlobalNodes.pSentinelNode->prev        = _tmpAry[i];

            arrBuckets[hash] = { _tmpAry[i], _tmpAry[i] };
        }
        else {
            _tmpAry[i]->next = arrBuckets[hash].first;
            _tmpAry[i]->prev = arrBuckets[hash].first->prev;

            arrBuckets[hash].first->prev->next = _tmpAry[i];
            arrBuckets[hash].first->prev = _tmpAry[i];

            arrBuckets[hash].first = _tmpAry[i];
        }
    }

    delete[] _tmpAry;
}

template<typename K, typename V, typename KV, bool M>
inline void MyHashTable<K, V, KV, M>::disable_auto_rehash() {
    bUsingAutoRehash = false;
}

template<typename K, typename V, typename KV, bool M>
inline int MyHashTable<K, V, KV, M>::size() {
    return lstGlobalNodes.size();
}

template<typename K, typename V, typename KV, bool M>
inline float MyHashTable<K, V, KV, M>::get_max_load_factor() {
    return fMaxLoadFactor;
}

template<typename K, typename V, typename KV, bool M>
inline typename MyLinkedList<KV>::template iterator<false>
MyHashTable<K, V, KV, M>::begin() {
    return lstGlobalNodes.begin();
}

template<typename K, typename V, typename KV, bool M>
inline typename MyLinkedList<KV>::template iterator<false>
MyHashTable<K, V, KV, M>::end() {
    return lstGlobalNodes.end();
}
