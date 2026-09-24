#pragma once

template<typename K, typename Compare>
inline MyOrderedSet<K, Compare>::MyOrderedSet() : _Tree() {}

template<typename K, typename Compare>
inline K MyOrderedSet<K, Compare>::_KeyExtractor(const K& _keyvalue) const {
    return _keyvalue;
}

template<typename K, typename Compare>
inline K MyOrderedSet<K, Compare>::_ValueExtractor(const K& _keyvalue) const {
    return _keyvalue;
}
