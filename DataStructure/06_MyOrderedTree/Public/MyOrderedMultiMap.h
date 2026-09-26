#pragma once

#include "MyRedBlackTree.h"

#include <utility>

template<typename _Key_Type, typename _Value_Type, typename Compare = std::less<_Key_Type>>
class MyOrderedMultiMap : public MyRedBlackTree<_Key_Type, _Value_Type,
                                               std::pair<_Key_Type, _Value_Type>, Compare, true> {
    using _Key_Value_Type = std::pair<_Key_Type, _Value_Type>;
    using _Tree = MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, true>;
public:
    MyOrderedMultiMap();
    _Key_Type _key_extractor(const _Key_Value_Type& _keyvalue) const;
    _Value_Type _value_extractor(const _Key_Value_Type& _keyvalue) const;
};

#include "MyOrderedMultiMap.inl"
