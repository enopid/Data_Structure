#pragma once

template<typename K, typename Compare>
inline MyOrderedMultiSet<K, Compare>::MyOrderedMultiSet() : _Tree() {}

template<typename K, typename Compare>
inline K MyOrderedMultiSet<K, Compare>::_key_extractor(const K& _keyvalue) const {
    return _keyvalue;
}

template<typename K, typename Compare>
inline K MyOrderedMultiSet<K, Compare>::_value_extractor(const K& _keyvalue) const {
    return _keyvalue;
}
