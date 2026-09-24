#pragma once

#include "MyRedBlackTree.h"

template<typename _Key_Type, typename Compare = std::less<_Key_Type>>
class MyOrderedMultiSet : public MyRedBlackTree<_Key_Type, _Key_Type, _Key_Type, Compare, true> {
    using _Key_Value_Type = _Key_Type;
    using _Tree = MyRedBlackTree<_Key_Type, _Key_Type, _Key_Value_Type, Compare, true>;
public:
    MyOrderedMultiSet();
    _Key_Type _KeyExtractor(const _Key_Value_Type& _keyvalue) const;
    _Key_Type _ValueExtractor(const _Key_Value_Type& _keyvalue) const;
};

#include "MyOrderedMultiSet.inl"
