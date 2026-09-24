#pragma once

#include "MyRedBlackTree.h"

#include <utility>

template<typename _Key_Type, typename _Value_Type, typename Compare = std::less<_Key_Type>>
class MyOrderedMap : public MyRedBlackTree<_Key_Type, _Value_Type,
                                          std::pair<_Key_Type, _Value_Type>, Compare, false> {
    using _Key_Value_Type = std::pair<_Key_Type, _Value_Type>;
    using _Tree = MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, false>;
public:
    MyOrderedMap();
    _Key_Type _KeyExtractor(const _Key_Value_Type& _keyvalue) const;
    _Value_Type _ValueExtractor(const _Key_Value_Type& _keyvalue) const;
    _Value_Type& at(const _Key_Type key);
    _Value_Type& operator[](const _Key_Type key);
};

#include "MyOrderedMap.inl"
