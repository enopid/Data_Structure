#pragma once

#include "MyRedBlackTree.h"

template<typename _Key_Type, typename Compare = std::less<_Key_Type>>
class MyOrderedSet : public MyRedBlackTree<_Key_Type, _Key_Type, _Key_Type, Compare, false> {
    using _Key_Value_Type = _Key_Type;
    using _Tree = MyRedBlackTree<_Key_Type, _Key_Type, _Key_Value_Type, Compare, false>;
public:
    MyOrderedSet();
    _Key_Type _key_extractor(const _Key_Value_Type& _keyvalue) const;
    _Key_Type _value_extractor(const _Key_Value_Type& _keyvalue) const;
};

#include "MyOrderedSet.inl"
