#include "MyRedBlackTree.h"
#pragma once

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, typename Compare, bool IsMulti>
MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, IsMulti>::MyRedBlackTree()
{
	pNILNode = new FTreeNode;
	pNILNode->SetBlack();
	iSize = 0;
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, typename Compare, bool IsMulti>
MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, IsMulti>::MyRedBlackTree(const MyRedBlackTree& other) : MyRedBlackTree()
{
	comp	= other.comp;
	iSize	= other.iSize;
	if (other.pRootNode) pRootNode = new FTreeNode;
	copy_node(other.pRootNode, other.pNILNode, pRootNode);
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, typename Compare, bool IsMulti>
MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, IsMulti>& MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, IsMulti>::operator=(const MyRedBlackTree& other)
{
	if (&other == this) return *this;
	clear();
	comp	= other.comp;
	iSize	= other.iSize;

	if(other.pRootNode) pRootNode = new FTreeNode;
	copy_node(other.pRootNode, other.pNILNode, pRootNode);
	return *this;
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, typename Compare, bool IsMulti>
MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, IsMulti>::MyRedBlackTree(MyRedBlackTree&& other)
{
	pRootNode	= other.pRootNode;
	pNILNode	= other.pNILNode;
	comp		= other.comp;
	iSize		= other.iSize;

	other.pRootNode = nullptr;
	other.pNILNode	= new FTreeNode;
	other.pNILNode->SetBlack();
	other.iSize = 0;
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, typename Compare, bool IsMulti>
MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, IsMulti>& MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, IsMulti>::operator=(MyRedBlackTree&& other)
{
	if (&other == this) return *this;
	clear();

	delete pNILNode;
	pRootNode	= other.pRootNode;
	pNILNode	= other.pNILNode;
	comp		= other.comp;
	iSize		= other.iSize;

	other.pRootNode = nullptr;
	other.pNILNode = new FTreeNode;
	other.pNILNode->SetBlack();
	other.iSize=0;

	return *this;
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, typename Compare, bool IsMulti>
MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, IsMulti>::~MyRedBlackTree()
{
	clear();
	delete pNILNode;
	pNILNode = nullptr;
}


template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, typename Compare, bool IsMulti>
void MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, IsMulti>::clear()
{
	auto DeleteNode = [&](auto&& self, FTreeNode* pCurNode)->void {
		if (!pCurNode || pCurNode == pNILNode) return;
		self(self, pCurNode->pLeftNode);
		pCurNode->pLeftNode = nullptr;
		self(self, pCurNode->pRightNode);
		pCurNode->pRightNode = nullptr;
		delete pCurNode;
	};

	DeleteNode(DeleteNode, pRootNode);
	pRootNode = nullptr;
	iSize = 0;
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, typename Compare, bool IsMulti>
typename MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, IsMulti>::iterator<false> MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, IsMulti>::find(_Key_Type key)
{
	return iterator<false>(find_node(key), this);
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, typename Compare, bool IsMulti>
typename MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, IsMulti>::FTreeNode* MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, IsMulti>::find_node(_Key_Type key)
{
	auto pNode = pRootNode;
	if (!pNode) return nullptr;
	while (true) {
		if (pNode == pNILNode) return nullptr;

		if		(comp(_key_extractor(pNode->value), key)) {
			pNode = pNode->pRightNode;
		}
		else if (comp(key, _key_extractor(pNode->value))) {
			pNode = pNode->pLeftNode;
		}
		else {
			return pNode;
		}
	}

	return nullptr;
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, typename Compare, bool IsMulti>
typename MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, IsMulti>::FTreeNode* MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, IsMulti>::create_new_node(bool IsRed, _Key_Value_Type value)
{
	FTreeNode* pNode = new FTreeNode;
	pNode->value = value;
	pNode->bIsRed = IsRed;
	pNode->pLeftNode = pNode->pRightNode = pNILNode;

	return pNode;
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, typename Compare, bool IsMulti>
void MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, IsMulti>::copy_node(FTreeNode* pSrc, FTreeNode* pSrcNIL, FTreeNode* pDst)
{
	if (!pSrc) return;
	pDst->value		= pSrc->value;
	pDst->bIsRed	= pSrc->bIsRed;

	if (pSrc->pLeftNode != pSrcNIL) {
		pDst->pLeftNode = new FTreeNode;
		pDst->pLeftNode->pParentNode = pDst;
		copy_node(pSrc->pLeftNode, pSrcNIL, pDst->pLeftNode);
	}
	else {
		pDst->pLeftNode = pNILNode;
	}

	if (pSrc->pRightNode != pSrcNIL) {
		pDst->pRightNode = new FTreeNode;
		pDst->pRightNode->pParentNode = pDst;
		copy_node(pSrc->pRightNode, pSrcNIL, pDst->pRightNode);
	}
	else {
		pDst->pRightNode = pNILNode;
	}

}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, typename Compare, bool IsMulti>
void MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, IsMulti>::rotate_right(FTreeNode* pNode)
{
	if (pNode->pLeftNode == pNILNode) return;
	auto _pLRNode = pNode->pLeftNode->pRightNode;
	auto _pLNode  = pNode->pLeftNode;
	auto _pPNode  = pNode->pParentNode;

	_pLNode->pParentNode	= pNode->pParentNode;
	_pLNode->pRightNode		= pNode;
	
	pNode->pParentNode		= _pLNode;
	pNode->pLeftNode		= pNILNode;
	if (!_pPNode) {
		pRootNode = _pLNode;
	}
	else {
		if (_pPNode->pLeftNode == pNode) {
			_pPNode->pLeftNode = _pLNode;
		}
		else {
			_pPNode->pRightNode = _pLNode;
		}
	}

	pNode->pLeftNode = _pLRNode;
	if (_pLRNode!= pNILNode)
		_pLRNode->pParentNode	= pNode;
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, typename Compare, bool IsMulti>
inline void MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, IsMulti>::rotate_left(FTreeNode* pNode)
{
	if (pNode->pRightNode == pNILNode) return;
	auto _pRLNode = pNode->pRightNode->pLeftNode;
	auto _pRNode = pNode->pRightNode;
	auto _pPNode = pNode->pParentNode;

	_pRNode->pParentNode = pNode->pParentNode;
	_pRNode->pLeftNode = pNode;

	pNode->pParentNode = _pRNode;
	pNode->pRightNode = pNILNode;
	if (!_pPNode) {
		pRootNode = _pRNode;
	}
	else {
		if (_pPNode->pLeftNode == pNode) {
			_pPNode->pLeftNode = _pRNode;
		}
		else {
			_pPNode->pRightNode = _pRNode;
		}
	}

	pNode->pRightNode = _pRLNode;
	if (_pRLNode != pNILNode)
		_pRLNode->pParentNode = pNode;
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, typename Compare, bool IsMulti>
inline void MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, IsMulti>::insert(_Key_Value_Type value)
{
	FTreeNode* pNode = create_new_node(true, value);
	if (insert_node(pRootNode, pNode)) {
		iSize++;
		rebuild_tree_insert(pNode);
	}
	else {
		delete pNode;
	}
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, typename Compare, bool IsMulti>
inline void MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, IsMulti>::remove(_Key_Type key)
{
	FTreeNode* pNode = find_node(key);

	if (!pNode) return;
	FTreeNode* pSuccessorNode;
	if (remove_node(&pSuccessorNode, pNode)) {
		rebuild_tree_remove(pSuccessorNode);
	}
	iSize--;
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, typename Compare, bool IsMulti>
inline bool MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, IsMulti>::insert_node(FTreeNode* pCurNode, FTreeNode* pNewNode)
{
	if (!pCurNode) {
		pRootNode = pNewNode;
		return true;
	}
	
	if (comp(_key_extractor(pCurNode->value), _key_extractor(pNewNode->value)))
	{
		if (pCurNode->pRightNode == pNILNode) {
			pCurNode->pRightNode	= pNewNode;
			pNewNode->pParentNode	= pCurNode;
			return true;
		}
		else {
			return insert_node(pCurNode->pRightNode, pNewNode);
		}
	}
	else
	{
		if constexpr (!IsMulti) {
			if (!comp(_key_extractor(pNewNode->value), _key_extractor(pCurNode->value)))
				return false;
		}

		if (pCurNode->pLeftNode == pNILNode) {
			pCurNode->pLeftNode		= pNewNode;
			pNewNode->pParentNode	= pCurNode;
			return true;
		}
		else {
			return insert_node(pCurNode->pLeftNode, pNewNode);
		}
	}
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, typename Compare, bool IsMulti>
bool MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, IsMulti>::remove_node(FTreeNode** ppSuccessorNode, FTreeNode* pNode)
{
	FTreeNode* pRemoveNode = nullptr;
	if (pNode->pLeftNode == pNILNode || pNode->pRightNode == pNILNode) {
		pRemoveNode = pNode;
	}
	else {
		pRemoveNode = pNode->pRightNode;
		while (pRemoveNode->pLeftNode != pNILNode) {
			pRemoveNode = pRemoveNode->pLeftNode;
		}
		pNode->value = pRemoveNode->value;
	}

	FTreeNode* pSuccessorNode = nullptr;
	if (pRemoveNode->pLeftNode == pNILNode) {
		pSuccessorNode = pRemoveNode->pRightNode;
	}
	else {
		pSuccessorNode = pRemoveNode->pLeftNode;
	}

	if (auto pRemovePNode = pRemoveNode->pParentNode) {
		if (pRemovePNode->pRightNode == pRemoveNode) {
			pRemovePNode->pRightNode = pSuccessorNode;
		}
		else {
			pRemovePNode->pLeftNode = pSuccessorNode;
		}
		pSuccessorNode->pParentNode = pRemovePNode;
	}
	else {
		if (pSuccessorNode == pNILNode) {
			pRootNode = nullptr;
		}
		else {
			pRootNode = pSuccessorNode;
		}
		pSuccessorNode->pParentNode = nullptr;
	}
	*ppSuccessorNode = pSuccessorNode;
	bool bDoRebuild = !pRemoveNode->IsRed();
	delete pRemoveNode;
	return bDoRebuild;
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, typename Compare, bool IsMulti>
inline void MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, IsMulti>::rebuild_tree_insert(FTreeNode* pNode)
{
	if (!pNode->pParentNode) {
		pRootNode->SetBlack();
		return;
	}
	while (pNode != pRootNode && pNode->pParentNode->IsRed()) {
		auto pPNode			= pNode->pParentNode;
		auto pGPNode		= pPNode->pParentNode;
		bool bIsRight		= pPNode->pRightNode  == pNode;
		bool bIsParentRight = pGPNode->pRightNode == pPNode;
		auto pUCNode = (bIsParentRight) ? pGPNode->pLeftNode : pGPNode->pRightNode;
		if (pUCNode->IsRed()) {
			pUCNode	->SetBlack();
			pPNode	->SetBlack();
			pGPNode	->SetRed();
			pNode = pGPNode;
		}
		else if (bIsRight != bIsParentRight) { // Convert a zig-zag case into a straight-line case.
			(bIsRight) ? rotate_left(pPNode) : rotate_right(pPNode);
			pNode = pPNode;
		}
		else {
			pPNode ->SetBlack();
			pGPNode->SetRed();

			(bIsParentRight) ? rotate_left(pGPNode) : rotate_right(pGPNode);
		}
	}
	pRootNode->SetBlack();
}

template<typename _Key_Type, typename _Value_Type, typename _Key_Value_Type, typename Compare, bool IsMulti>
inline void MyRedBlackTree<_Key_Type, _Value_Type, _Key_Value_Type, Compare, IsMulti>::rebuild_tree_remove(FTreeNode* pNode)
{
	FTreeNode* pSNode = nullptr;
	FTreeNode* pPNode = nullptr;

	while (pNode->pParentNode && pNode->IsBlack()) {
		pPNode = pNode->pParentNode;
		bool bIsRight = (pNode == pPNode->pRightNode);
		pSNode = (bIsRight ? pPNode->pLeftNode : pPNode->pRightNode);
		
		if (pSNode->IsRed()) {//1. SNode is red
			pPNode->SetRed();
			pSNode->SetBlack();

			if (bIsRight)	rotate_right	(pPNode);
			else			rotate_left	(pPNode);
			
			//now snode is black
		}
		else {//2. SNode is black
			//2-1. both SNode's child black
			if (pSNode->pLeftNode->IsBlack() && pSNode->pRightNode->IsBlack()) {
				pSNode->SetRed();
				pNode = pPNode;
				// toss double black to parent
			}
			//2-2. straight red
			else if (
				(!bIsRight && pSNode->pRightNode->IsRed()) ||
				( bIsRight && pSNode->pLeftNode ->IsRed())
				) {
				pPNode->IsRed() ? pSNode->SetRed() : pSNode->SetBlack();
				pPNode->SetBlack();
				if (bIsRight)	pSNode->pLeftNode ->SetBlack();
				else			pSNode->pRightNode->SetBlack();

				if (bIsRight)	rotate_right(pPNode);
				else			rotate_left(pPNode);
				pNode = pRootNode;
			}
			//2-3. curve red
			else {
				bool bIsSRRed = pSNode->pRightNode->IsRed();

				if (bIsSRRed)	pSNode->pRightNode->SetBlack();
				else			pSNode->pLeftNode->SetBlack();
				pSNode->SetRed();

				(bIsSRRed ? rotate_left(pSNode) : rotate_right(pSNode));
			}
			//bb
			//rb
			//rr or br
		}
	}
	pNode->SetBlack();
}
