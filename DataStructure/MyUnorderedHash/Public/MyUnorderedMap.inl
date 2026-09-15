#pragma once

template<typename K, typename V>
inline MyUnorderedMap<K, V>::MyUnorderedMap() : _HashTable() {}

template<typename K, typename V>
inline MyUnorderedMap<K, V>::MyUnorderedMap(unsigned int max_size) : _HashTable(max_size) {}

template<typename K, typename V>
inline K MyUnorderedMap<K, V>::_KeyExtractor(const _Key_Value_Type& _keyvalue) const {
    return _keyvalue.first;
}

template<typename K, typename V>
inline V MyUnorderedMap<K, V>::_ValueExtractor(const _Key_Value_Type& _keyvalue) const {
    return _keyvalue.second;
}

template<typename K, typename V>
inline V& MyUnorderedMap<K, V>::at(const K key) {
    auto hash_node = _HashTable::Find(key);
    if (hash_node == nullptr) throw std::runtime_error("invalid key");
    return (*(hash_node->it)).second;
}

template<typename K, typename V>
inline V& MyUnorderedMap<K, V>::operator[](const K key) {
    auto hash_node = _HashTable::Find(key);
    if (hash_node == nullptr) _HashTable::Insert({key, V()});
    hash_node = _HashTable::Find(key);
    return (*(hash_node->it)).second;
}
