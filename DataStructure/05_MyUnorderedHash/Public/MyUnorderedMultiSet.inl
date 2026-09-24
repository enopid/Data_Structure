#pragma once

template<typename K>
inline MyUnorderedMultiSet<K>::MyUnorderedMultiSet() : _HashTable() {}

template<typename K>
inline MyUnorderedMultiSet<K>::MyUnorderedMultiSet(unsigned int max_size) : _HashTable(max_size) {}

template<typename K>
inline K MyUnorderedMultiSet<K>::_KeyExtractor(const K& _keyvalue) const {
    return _keyvalue;
}

template<typename K>
inline K MyUnorderedMultiSet<K>::_ValueExtractor(const K& _keyvalue) const {
    return _keyvalue;
}
