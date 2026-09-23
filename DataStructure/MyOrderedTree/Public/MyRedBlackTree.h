#pragma once

#include <functional>
#include <stdexcept>

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, typename Compare, bool IsMulti>
class MyRedBlackTree {
private:
    struct FTreeNode {
        FTreeNode*      pParentNode  = nullptr;
        FTreeNode*      pLeftNode    = nullptr;
        FTreeNode*      pRightNode   = nullptr;
        bool            bIsRed       = false;
        _Key_Value_Type value;

        bool IsRed()        const { return bIsRed; }
        bool IsBlack()      const { return !bIsRed; }
        bool IsNIL()        const { return !pLeftNode && !pRightNode; }
        void SetRed()   { bIsRed = true;  }
        void SetBlack() { bIsRed = false; }
    };

public:
    template<bool IsConst>
    class iterator {
    public:
		using reference = typename std::conditional<IsConst, const _Key_Value_Type&, _Key_Value_Type&>::type;
		using pointer	= typename std::conditional<IsConst, const _Key_Value_Type*, _Key_Value_Type*>::type;
        friend class MyRedBlackTree<
            _Key_Type,
            _Value_Type,
            _Key_Value_Type,
            Compare,
            IsMulti
        >;

        iterator(FTreeNode* _pNode) : pNode(_pNode){}
        iterator& operator =(const iterator& other)       { 
            pNode = other.pNode; 
            return *this;
        };
        bool	  operator!=(const iterator& other) const { return other.pNode != pNode; };
        bool	  operator==(const iterator& other) const { return other.pNode == pNode; };
        iterator&   operator++() { 
            if (!pNode) 
                throw std::out_of_range("invalid iterator!");

            if (!pNode->pRightNode->IsNIL()) {
                pNode = pNode->pRightNode;
                while (!pNode->pLeftNode->IsNIL()) pNode = pNode->pLeftNode;
            }
            else {
                while (pNode->pParentNode && pNode == pNode->pParentNode->pRightNode) {
                    pNode   = pNode->pParentNode;
                }

                pNode = pNode->pParentNode;
            }

            return *this;
        }
        iterator	operator++(int) {
            iterator old = *this;
            ++(*this);
            return old;
        }
        iterator&   operator--() {
            if (!pNode) {
                pNode = pRootNode;
                while (!pNode->pRightNode->IsNIL())  pNode = pNode->pRightNode;
                pNode = pNode->pRightNode;
                return *this;
            }
            
            if (!pNode->pLeftNode->IsNIL()) {
                pNode = pNode->pLeftNode;
                while (!pNode->pRightNode->IsNIL()) pNode = pNode->pRightNode;
            }
            else {
                while (pNode->pParentNode && pNode == pNode->pParentNode->pLeftNode) {
                    pNode = pNode->pParentNode;
                }

                pNode = pNode->pParentNode;
            }

            return *this;
        }
        iterator	operator--(int) {
            iterator old = *this;
            --(*this);
            return old;
        }
        reference	operator* () const {
            return pNode->value;
        }
        pointer		operator->() const {
            return static_cast<pointer>(&(pNode->value));
        }
    private:
        FTreeNode* pNode = nullptr;
    };
public:
    MyRedBlackTree();
    MyRedBlackTree(const MyRedBlackTree& other);
    MyRedBlackTree& operator=(const MyRedBlackTree& other);
    MyRedBlackTree(MyRedBlackTree&& other);
    MyRedBlackTree& operator=(MyRedBlackTree&& other);
    ~MyRedBlackTree();

    void            Insert(_Key_Value_Type value);
    void            Remove(_Key_Type key);
    int             Size() const { return iSize; };
    void            Clear();
    iterator<false> Find(_Key_Type key);


    iterator<false>			begin() {
        if (!pRootNode) return iterator<false>(nullptr);
        auto _pNode = pRootNode;
        while (!_pNode->pLeftNode->IsNIL())  _pNode = _pNode->pLeftNode;
        return iterator<false>(_pNode);
    }
    iterator<false>			end() {
        return iterator<false>(nullptr);
    }
    iterator<true>			cbegin()	const {
        if (!pRootNode) return iterator<true>(nullptr);
        auto _pNode = pRootNode;
        while (!_pNode->pLeftNode->IsNIL())  _pNode = _pNode->pLeftNode;
        return iterator<true>(_pNode);
    }
    iterator<true>			cend()		const {
        return iterator<true>(nullptr);
    }

private:
    FTreeNode* CreateNewNode(bool IsRed, _Key_Value_Type value);
    void CopyNode(FTreeNode* pSrc, FTreeNode* pSrcNIL, FTreeNode* pDst);

    void RotateRight(FTreeNode* pNode);
    void RotateLeft (FTreeNode* pNode);
    FTreeNode* FindNode(_Key_Type key);
    bool InsertNode (FTreeNode* pCurNode, FTreeNode* pNode);
    bool RemoveNode (FTreeNode** pSuccessorNode, FTreeNode* pNode);
    void RebuildTree_Insert(FTreeNode* pNode);
    void RebuildTree_Remove(FTreeNode* pNode);

    FTreeNode* pRootNode = nullptr;
    FTreeNode* pNILNode  = nullptr;
    Compare comp;
    int         iSize = 0;
protected:
    virtual _Key_Type _KeyExtractor(const _Key_Value_Type& _keyvalue) const = 0;
    virtual _Value_Type _ValueExtractor(const _Key_Value_Type& _keyvalue) const = 0;
};

#include "MyRedBlackTree.inl"

