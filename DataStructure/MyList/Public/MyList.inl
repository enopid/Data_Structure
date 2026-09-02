#include "MyList.h"
template<typename T>
MyLinkedList<T>::Node::~Node() {
	prev->next = next;
	next->prev = prev;
}

template<typename T>
template<bool IsConst>
MyLinkedList<T>::iterator<IsConst>::iterator() : _ptr(nullptr) {}

template<typename T>
template<bool IsConst>
MyLinkedList<T>::iterator<IsConst>::iterator(Node* ptr) : _ptr(ptr) {}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template iterator<IsConst>& MyLinkedList<T>::iterator<IsConst>::operator=(const iterator& other) {
	if (this != &other) {
		this->_ptr = other._ptr;
	}
	return *this;
}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template iterator<IsConst>& MyLinkedList<T>::iterator<IsConst>::operator++() {
	_ptr = _ptr->next;
	return *this;
}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template iterator<IsConst> MyLinkedList<T>::iterator<IsConst>::operator++(int) {
	iterator tmp(_ptr);
	_ptr = _ptr->next;
	return tmp;
}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template iterator<IsConst>& MyLinkedList<T>::iterator<IsConst>::operator--() {
	_ptr = _ptr->prev;
	return *this;
}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template iterator<IsConst> MyLinkedList<T>::iterator<IsConst>::operator--(int) {
	iterator tmp(_ptr);
	_ptr = _ptr->prev;
	return tmp;
}

template<typename T>
template<bool IsConst>
bool MyLinkedList<T>::iterator<IsConst>::operator!=(const iterator& other) const {
	return other._ptr != _ptr;
}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template iterator<IsConst>::reference MyLinkedList<T>::iterator<IsConst>::operator*() const {
	return _ptr->data;
}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template iterator<IsConst>::pointer MyLinkedList<T>::iterator<IsConst>::operator->() const {
	return static_cast<pointer>(&(_ptr->data));
}

template<typename T>
template<bool IsConst>
MyLinkedList<T>::reverse_iterator<IsConst>::reverse_iterator(Node* ptr) : _iter(iterator<IsConst>(ptr)) {}

template<typename T>
template<bool IsConst>
MyLinkedList<T>::reverse_iterator<IsConst>::reverse_iterator(iterator<IsConst> _iter) : _iter(_iter) {}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template reverse_iterator<IsConst>& MyLinkedList<T>::reverse_iterator<IsConst>::operator++() {
	--_iter;
	return *this;
}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template reverse_iterator<IsConst> MyLinkedList<T>::reverse_iterator<IsConst>::operator++(int) {
	reverse_iterator tmp(_iter);
	--_iter;
	return tmp;
}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template reverse_iterator<IsConst>& MyLinkedList<T>::reverse_iterator<IsConst>::operator--() {
	++_iter;
	return *this;
}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template reverse_iterator<IsConst> MyLinkedList<T>::reverse_iterator<IsConst>::operator--(int) {
	reverse_iterator tmp(_iter);
	++_iter;
	return tmp;
}

template<typename T>
template<bool IsConst>
bool MyLinkedList<T>::reverse_iterator<IsConst>::operator!=(const reverse_iterator& other) const {
	return other._iter != _iter;
}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template reverse_iterator<IsConst>::reference MyLinkedList<T>::reverse_iterator<IsConst>::operator*() {
	return *_iter;
}

template<typename T>
template<bool IsConst>
typename MyLinkedList<T>::template reverse_iterator<IsConst>::pointer MyLinkedList<T>::reverse_iterator<IsConst>::operator->() {
	return &(*_iter);
}

template<typename T>
MyLinkedList<T>::MyLinkedList() {
	_SentinelNode		= new Node;
	_SentinelNode->prev = _SentinelNode;
	_SentinelNode->next = _SentinelNode;
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

	auto curNode = _SentinelNode->next;
	while (curNode != _SentinelNode) {
		auto tmpNode = curNode;
		curNode = curNode->next;
		delete tmpNode;
	}
	_SentinelNode->prev = _SentinelNode;
	_SentinelNode->next = _SentinelNode;
	_size = 0;

	for (auto it = other.cbegin(); it != other.cend(); ++it) {
		push_back(*it);
	}

	return *this;
}

template<typename T>
inline MyLinkedList<T>::MyLinkedList(MyLinkedList<T>&& other) : MyLinkedList()
{
	_size							= other._size;
	_SentinelNode->next				= other._SentinelNode->next;
	_SentinelNode->prev				= other._SentinelNode->prev;
	other._SentinelNode->next->prev = _SentinelNode;
	other._SentinelNode->prev->next = _SentinelNode;


	other._size						= 0;
	other._SentinelNode->next		= other._SentinelNode;
	other._SentinelNode->prev		= other._SentinelNode;
}

template<typename T>
inline MyLinkedList<T>& MyLinkedList<T>::operator=(MyLinkedList<T>&& other) noexcept
{
	if (this == &other) return *this;

	auto curNode = _SentinelNode->next;
	while (curNode != _SentinelNode) {
		auto tmpNode = curNode;
		curNode = curNode->next;
		delete tmpNode;
	}

	_size							= other._size;
	_SentinelNode->next				= other._SentinelNode->next;
	_SentinelNode->prev				= other._SentinelNode->prev;
	other._SentinelNode->next->prev = _SentinelNode;
	other._SentinelNode->prev->next = _SentinelNode;


	other._size						= 0;
	other._SentinelNode->next		= other._SentinelNode;
	other._SentinelNode->prev		= other._SentinelNode;

	return *this;
}

template<typename T>
inline MyLinkedList<T>::~MyLinkedList()
{
	_size = 0;
	auto curNode = _SentinelNode->next;
	while (curNode != _SentinelNode) {
		auto tmpNode = curNode;
		curNode = curNode->next;
		delete tmpNode;
	}
	delete _SentinelNode;
}

template<typename T>
void MyLinkedList<T>::push_front(const T& data) {
	Node* node = new Node{data, _SentinelNode, _SentinelNode->next};
	(_SentinelNode->next)->prev = node;
	_SentinelNode->next = node;
	_size++;
}

template<typename T>
void MyLinkedList<T>::push_back(const T& data) {
	Node* node = new Node{data, _SentinelNode->prev, _SentinelNode};
	(_SentinelNode->prev)->next = node;
	_SentinelNode->prev = node;
	_size++;
}

template<typename T>
void MyLinkedList<T>::pop_front() {
	if (_size <= 0) return;
	Node* deletedNode = (_SentinelNode->next);
	delete deletedNode;
	_size--;
}

template<typename T>
void MyLinkedList<T>::pop_back() {
	if (_size <= 0) return;
	Node* deletedNode = (_SentinelNode->prev);
	delete deletedNode;
	_size--;
}

template<typename T>
void MyLinkedList<T>::clear() {
	_size = 0;
	Node* curNode = (_SentinelNode->next);
	while (curNode != _SentinelNode) {
		curNode = (curNode->next);
		delete (curNode->prev);
	}
}

template<typename T>
template<bool isconst>
void MyLinkedList<T>::insert(const iterator<isconst>& it, T data) {
	Node* node = new Node{data, (it._ptr)->prev, (it._ptr)};
	((it._ptr)->prev)->next = node;
	(it._ptr)->prev = node;
	_size++;
}

template<typename T>
template<bool isconst>
void MyLinkedList<T>::erase(const iterator<isconst>& it) {
	if (it._ptr == _SentinelNode) return;
	delete (it._ptr);
	_size--;
}

template<typename T>
T& MyLinkedList<T>::front() {
	return (_SentinelNode->next)->data;
}

template<typename T>
T& MyLinkedList<T>::back() {
	return (_SentinelNode->prev)->data;
}

template<typename T>
const T& MyLinkedList<T>::front() const {
	return (_SentinelNode->next)->data;
}

template<typename T>
const T& MyLinkedList<T>::back() const {
	return (_SentinelNode->prev)->data;
}

template<typename T>
bool MyLinkedList<T>::empty() const {
	return (_size == 0) ? true : false;
}

template<typename T>
int MyLinkedList<T>::size() const {
	return _size;
}

template<typename T>
typename MyLinkedList<T>::template iterator<false> MyLinkedList<T>::begin() {
	return iterator<false>(_SentinelNode->next);
}

template<typename T>
typename MyLinkedList<T>::template iterator<false> MyLinkedList<T>::end() {
	return iterator<false>(_SentinelNode);
}

template<typename T>
typename MyLinkedList<T>::template iterator<true> MyLinkedList<T>::cbegin() const {
	return iterator<true>(_SentinelNode->next);
}

template<typename T>
typename MyLinkedList<T>::template iterator<true> MyLinkedList<T>::cend() const {
	return iterator<true>(_SentinelNode);
}

template<typename T>
typename MyLinkedList<T>::template reverse_iterator<false> MyLinkedList<T>::rbegin() {
	return reverse_iterator<false>(_SentinelNode->prev);
}

template<typename T>
typename MyLinkedList<T>::template reverse_iterator<false> MyLinkedList<T>::rend() {
	return reverse_iterator<false>(_SentinelNode);
}

template<typename T>
typename MyLinkedList<T>::template reverse_iterator<true> MyLinkedList<T>::crbegin() const {
	return reverse_iterator<true>(_SentinelNode->prev);
}

template<typename T>
typename MyLinkedList<T>::template reverse_iterator<true> MyLinkedList<T>::crend() const {
	return reverse_iterator<true>(_SentinelNode);
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const MyLinkedList<T>& mylinkedlist) {
	typename MyLinkedList<T>::Node* ptr = mylinkedlist._SentinelNode->next;
	while (ptr != mylinkedlist._SentinelNode) {
		os << ptr->data << " ";
		ptr = ptr->next;
	}
	return os;
}
