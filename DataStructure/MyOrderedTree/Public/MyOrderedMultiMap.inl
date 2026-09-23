#pragma once

template<typename K, typename V, typename Compare>
inline MyOrderedMultiMap<K, V, Compare>::MyOrderedMultiMap() : _Tree() {}

template<typename K, typename V, typename Compare>
inline K MyOrderedMultiMap<K, V, Compare>::_KeyExtractor(const _Key_Value_Type& _keyvalue) const {
    return _keyvalue.first;
}

template<typename K, typename V, typename Compare>
inline V MyOrderedMultiMap<K, V, Compare>::_ValueExtractor(const _Key_Value_Type& _keyvalue) const {
    return _keyvalue.second;
}
