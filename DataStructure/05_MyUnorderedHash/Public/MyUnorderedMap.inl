#pragma once

template<typename K, typename V>
inline MyUnorderedMap<K, V>::MyUnorderedMap() : _HashTable() {}

template<typename K, typename V>
inline MyUnorderedMap<K, V>::MyUnorderedMap(unsigned int max_size) : _HashTable(max_size) {}

template<typename K, typename V>
inline K MyUnorderedMap<K, V>::_key_extractor(const _Key_Value_Type& _keyvalue) const {
    return _keyvalue.first;
}

template<typename K, typename V>
inline V MyUnorderedMap<K, V>::_value_extractor(const _Key_Value_Type& _keyvalue) const {
    return _keyvalue.second;
}

template<typename K, typename V>
inline V& MyUnorderedMap<K, V>::at(const K key) {
    auto _node = _HashTable::find(key);
    if (_node == nullptr) throw std::runtime_error("invalid key");
    return _node->data.second;
}

template<typename K, typename V>
inline V& MyUnorderedMap<K, V>::operator[](const K key) {
    auto _node = _HashTable::find(key);
    if (_node == nullptr) _HashTable::insert({key, V()});
    _node = _HashTable::find(key);
    return _node->data.second;
}
