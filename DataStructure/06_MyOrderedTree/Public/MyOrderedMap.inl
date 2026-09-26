#pragma once

template<typename K, typename V, typename Compare>
inline MyOrderedMap<K, V, Compare>::MyOrderedMap() : _Tree() {}

template<typename K, typename V, typename Compare>
inline K MyOrderedMap<K, V, Compare>::_key_extractor(const _Key_Value_Type& _keyvalue) const {
    return _keyvalue.first;
}

template<typename K, typename V, typename Compare>
inline V MyOrderedMap<K, V, Compare>::_value_extractor(const _Key_Value_Type& _keyvalue) const {
    return _keyvalue.second;
}

// at and operator[] require the tree's search/insert interface.
