#pragma once

template<typename K>
inline MyUnorderedSet<K>::MyUnorderedSet() : _HashTable() {}

template<typename K>
inline MyUnorderedSet<K>::MyUnorderedSet(unsigned int max_size) : _HashTable(max_size) {}

template<typename K>
inline K MyUnorderedSet<K>::_key_extractor(const K& _keyvalue) const {
    return _keyvalue;
}

template<typename K>
inline K MyUnorderedSet<K>::_value_extractor(const K& _keyvalue) const {
    return _keyvalue;
}
