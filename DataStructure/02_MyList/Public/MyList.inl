#include "MyList.h"
template<typename T>
MyLinkedList<T>::Node::~Node() {
	prev->next = next;
	next->prev = prev;
}

template<typename T>
template<bool IsConst>
MyLinkedList<T>::iterator<IsConst>::iterator() : pNode(nullptr) {}

template<typename T>
template<bool IsConst>
MyLinkedList<T>::iterator<IsConst>::iterator(Node* ptr) : pNode(ptr) {}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template iterator<IsConst>& MyLinkedList<T>::iterator<IsConst>::operator=(const iterator& other) {
	if (this != &other) {
		this->pNode = other.pNode;
	}
	return *this;
}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template iterator<IsConst>& MyLinkedList<T>::iterator<IsConst>::operator++() {
	pNode = pNode->next;
	return *this;
}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template iterator<IsConst> MyLinkedList<T>::iterator<IsConst>::operator++(int) {
	iterator tmp(pNode);
	pNode = pNode->next;
	return tmp;
}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template iterator<IsConst>& MyLinkedList<T>::iterator<IsConst>::operator--() {
	pNode = pNode->prev;
	return *this;
}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template iterator<IsConst> MyLinkedList<T>::iterator<IsConst>::operator--(int) {
	iterator tmp(pNode);
	pNode = pNode->prev;
	return tmp;
}

template<typename T>
template<bool IsConst>
bool MyLinkedList<T>::iterator<IsConst>::operator!=(const iterator& other) const {
	return other.pNode != pNode;
}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template iterator<IsConst>::reference MyLinkedList<T>::iterator<IsConst>::operator*() const {
	return pNode->data;
}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template iterator<IsConst>::pointer MyLinkedList<T>::iterator<IsConst>::operator->() const {
	return static_cast<pointer>(&(pNode->data));
}

template<typename T>
template<bool IsConst>
MyLinkedList<T>::reverse_iterator<IsConst>::reverse_iterator(Node* ptr) : iter(iterator<IsConst>(ptr)) {}

template<typename T>
template<bool IsConst>
MyLinkedList<T>::reverse_iterator<IsConst>::reverse_iterator(iterator<IsConst> _iter) : iter(_iter) {}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template reverse_iterator<IsConst>& MyLinkedList<T>::reverse_iterator<IsConst>::operator++() {
	--iter;
	return *this;
}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template reverse_iterator<IsConst> MyLinkedList<T>::reverse_iterator<IsConst>::operator++(int) {
	reverse_iterator tmp(iter);
	--iter;
	return tmp;
}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template reverse_iterator<IsConst>& MyLinkedList<T>::reverse_iterator<IsConst>::operator--() {
	++iter;
	return *this;
}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template reverse_iterator<IsConst> MyLinkedList<T>::reverse_iterator<IsConst>::operator--(int) {
	reverse_iterator tmp(iter);
	++iter;
	return tmp;
}

template<typename T>
template<bool IsConst>
bool MyLinkedList<T>::reverse_iterator<IsConst>::operator!=(const reverse_iterator& other) const {
	return other.iter != iter;
}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template reverse_iterator<IsConst>::reference MyLinkedList<T>::reverse_iterator<IsConst>::operator*() {
	return *iter;
}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template reverse_iterator<IsConst>::pointer MyLinkedList<T>::reverse_iterator<IsConst>::operator->() {
	return &(*iter);
}

template<typename T>
MyLinkedList<T>::MyLinkedList() {
	pSentinelNode		= new Node;
	pSentinelNode->prev = pSentinelNode;
	pSentinelNode->next = pSentinelNode;
}

template<typename T>
MyLinkedList<T>::MyLinkedList(int n) : MyLinkedList()
{
	for (int i = 0; i < n; ++i) {
		push_back(T());
	}
}

template<typename T>
MyLinkedList<T>::MyLinkedList(int n, const T& val) : MyLinkedList()
{
	for (int i = 0; i < n; ++i) {
		push_back(val);
	}

}

template<typename T>
inline MyLinkedList<T>::MyLinkedList(const MyLinkedList& other) : MyLinkedList()
{
	for(auto it = other.cbegin(); it != other.cend(); ++it) {
		push_back(*it);
	}
}

template<typename T>
inline MyLinkedList<T>& MyLinkedList<T>::operator=(const MyLinkedList<T>& other)
{
	if (this == &other) return *this;

	auto curNode = pSentinelNode->next;
	while (curNode != pSentinelNode) {
		auto tmpNode = curNode;
		curNode = curNode->next;
		delete tmpNode;
	}
	pSentinelNode->prev = pSentinelNode;
	pSentinelNode->next = pSentinelNode;
	iSize = 0;

	for (auto it = other.cbegin(); it != other.cend(); ++it) {
		push_back(*it);
	}

	return *this;
}

template<typename T>
inline MyLinkedList<T>::MyLinkedList(MyLinkedList<T>&& other) : MyLinkedList()
{
	if (other.iSize == 0) return;

	iSize							= other.iSize;
	pSentinelNode->next				= other.pSentinelNode->next;
	pSentinelNode->prev				= other.pSentinelNode->prev;
	other.pSentinelNode->next->prev = pSentinelNode;
	other.pSentinelNode->prev->next = pSentinelNode;


	other.iSize						= 0;
	other.pSentinelNode->next		= other.pSentinelNode;
	other.pSentinelNode->prev		= other.pSentinelNode;
}

template<typename T>
inline MyLinkedList<T>& MyLinkedList<T>::operator=(MyLinkedList<T>&& other) noexcept
{
	if (this == &other) return *this;

	auto curNode = pSentinelNode->next;
	while (curNode != pSentinelNode) {
		auto tmpNode = curNode;
		curNode = curNode->next;
		delete tmpNode;
	}
	pSentinelNode->next = pSentinelNode;
	pSentinelNode->prev = pSentinelNode;
	iSize = 0;
	if (other.iSize == 0) return *this;

	iSize							= other.iSize;
	pSentinelNode->next				= other.pSentinelNode->next;
	pSentinelNode->prev				= other.pSentinelNode->prev;
	other.pSentinelNode->next->prev = pSentinelNode;
	other.pSentinelNode->prev->next = pSentinelNode;


	other.iSize						= 0;
	other.pSentinelNode->next		= other.pSentinelNode;
	other.pSentinelNode->prev		= other.pSentinelNode;

	return *this;
}

template<typename T>
inline MyLinkedList<T>::~MyLinkedList()
{
	iSize = 0;
	auto _pCurNode= pSentinelNode->next;
	while (_pCurNode != pSentinelNode) {
		auto _pTmpNode = _pCurNode;
		_pCurNode = _pCurNode->next;
		delete _pTmpNode;
	}
	delete pSentinelNode;
}

template<typename T>
void MyLinkedList<T>::push_front(const T& data) {
	Node* node = new Node{data, pSentinelNode, pSentinelNode->next};
	(pSentinelNode->next)->prev = node;
	pSentinelNode->next = node;
	iSize++;
}

template<typename T>
void MyLinkedList<T>::push_back(const T& data) {
	Node* node = new Node{data, pSentinelNode->prev, pSentinelNode};
	(pSentinelNode->prev)->next = node;
	pSentinelNode->prev = node;
	iSize++;
}

template<typename T>
void MyLinkedList<T>::pop_front() {
	if (iSize <= 0) return;
	Node* _pDeletedNode = (pSentinelNode->next);
	delete _pDeletedNode;
	iSize--;
}

template<typename T>
void MyLinkedList<T>::pop_back() {
	if (iSize <= 0) return;
	Node* _pDeletedNode = (pSentinelNode->prev);
	delete _pDeletedNode;
	iSize--;
}

template<typename T>
void MyLinkedList<T>::clear() {
	iSize = 0;
	Node* curNode = (pSentinelNode->next);
	while (curNode != pSentinelNode) {
		curNode = (curNode->next);
		delete (curNode->prev);
	}
}

template<typename T>
template<bool isconst>
void MyLinkedList<T>::insert(const iterator<isconst>& it, T data) {
	Node* _pNode = new Node{data, (it.pNode)->prev, (it.pNode)};
	((it.pNode)->prev)->next = _pNode;
	(it.pNode)->prev = _pNode;
	iSize++;
}

template<typename T>
template<bool isconst>
void MyLinkedList<T>::erase(const iterator<isconst>& it) {
	if (it.pNode == pSentinelNode) return;
	delete (it.pNode);
	iSize--;
}

template<typename T>
T& MyLinkedList<T>::front() {
	return (pSentinelNode->next)->data;
}

template<typename T>
T& MyLinkedList<T>::back() {
	return (pSentinelNode->prev)->data;
}

template<typename T>
const T& MyLinkedList<T>::front() const {
	return (pSentinelNode->next)->data;
}

template<typename T>
const T& MyLinkedList<T>::back() const {
	return (pSentinelNode->prev)->data;
}

template<typename T>
bool MyLinkedList<T>::empty() const {
	return (iSize == 0) ? true : false;
}

template<typename T>
int MyLinkedList<T>::size() const {
	return iSize;
}

template<typename T>
typename MyLinkedList<T>::template iterator<false> MyLinkedList<T>::begin() {
	return iterator<false>(pSentinelNode->next);
}

template<typename T>
typename MyLinkedList<T>::template iterator<false> MyLinkedList<T>::end() {
	return iterator<false>(pSentinelNode);
}

template<typename T>
typename MyLinkedList<T>::template iterator<true> MyLinkedList<T>::cbegin() const {
	return iterator<true>(pSentinelNode->next);
}

template<typename T>
typename MyLinkedList<T>::template iterator<true> MyLinkedList<T>::cend() const {
	return iterator<true>(pSentinelNode);
}

template<typename T>
typename MyLinkedList<T>::template reverse_iterator<false> MyLinkedList<T>::rbegin() {
	return reverse_iterator<false>(pSentinelNode->prev);
}

template<typename T>
typename MyLinkedList<T>::template reverse_iterator<false> MyLinkedList<T>::rend() {
	return reverse_iterator<false>(pSentinelNode);
}

template<typename T>
typename MyLinkedList<T>::template reverse_iterator<true> MyLinkedList<T>::crbegin() const {
	return reverse_iterator<true>(pSentinelNode->prev);
}

template<typename T>
typename MyLinkedList<T>::template reverse_iterator<true> MyLinkedList<T>::crend() const {
	return reverse_iterator<true>(pSentinelNode);
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const MyLinkedList<T>& mylinkedlist) {
	typename MyLinkedList<T>::Node* _pNode = mylinkedlist.pSentinelNode->next;
	while (_pNode != mylinkedlist.pSentinelNode) {
		os << _pNode->data << " ";
		_pNode = _pNode->next;
	}
	return os;
}
