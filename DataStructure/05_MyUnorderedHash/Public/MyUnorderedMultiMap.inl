#pragma once

template<typename K, typename V>
inline MyUnorderedMultiMap<K, V>::MyUnorderedMultiMap() : _HashTable() {}

template<typename K, typename V>
inline MyUnorderedMultiMap<K, V>::MyUnorderedMultiMap(unsigned int max_size) : _HashTable(max_size) {}

template<typename K, typename V>
inline K MyUnorderedMultiMap<K, V>::_key_extractor(const _Key_Value_Type& _keyvalue) const {
    return _keyvalue.first;
}

template<typename K, typename V>
inline V MyUnorderedMultiMap<K, V>::_value_extractor(const _Key_Value_Type& _keyvalue) const {
    return _keyvalue.second;
}
