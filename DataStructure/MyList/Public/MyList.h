#pragma once
#include<iostream>
#include<list>

template<typename T>
class MyLinkedList;

template<typename T>
std::ostream& operator<<(std::ostream& os, const MyLinkedList<T>& mylinkedlist);

template<typename T>
class MyLinkedList {
private:
	struct Node {
		~Node();
		T data;
		Node* prev;
		Node* next;
	};
public:
	template<bool IsConst>
	class iterator {
	public:
		using reference = typename std::conditional<IsConst, const T&, T&>::type;
		using pointer	= typename std::conditional<IsConst, const T*, T*>::type;
		iterator();
		iterator(Node* ptr);
		iterator&	operator=(const iterator& other);
		iterator&	operator++();
		iterator	operator++(int);
		iterator&	operator--();
		iterator	operator--(int);
		bool		operator!=(const iterator& other) const;
		reference	operator*() const;
		pointer		operator->() const;
		template<typename> friend class MyLinkedList;

	private:
		Node* _ptr = nullptr;
	};

	template<bool IsConst>
	class reverse_iterator {
	public:
		using reference = typename std::conditional<IsConst, const T&, T&>::type;
		using pointer	= typename std::conditional<IsConst, const T*, T*>::type;

		reverse_iterator(Node* ptr);
		reverse_iterator(iterator<IsConst> _iter);
		reverse_iterator&	operator++();
		reverse_iterator	operator++(int);
		reverse_iterator&	operator--();
		reverse_iterator	operator--(int);
		bool				operator!=(const reverse_iterator& other) const;
		reference			operator*();
		pointer				operator->();
	private:
		iterator<IsConst> _iter;
	};

	int _size = 0;
	Node* _SentinelNode = nullptr;
public:
	MyLinkedList();
	MyLinkedList(int n);
	MyLinkedList(int n, const T& val);
	MyLinkedList(const MyLinkedList& other);
	MyLinkedList& operator=(const MyLinkedList& other);
	MyLinkedList(MyLinkedList&& other);
	MyLinkedList& operator=(MyLinkedList&&) noexcept;
	~MyLinkedList();

	void push_front(const T& data);
	void push_back(const T& data);
	void pop_front();
	void pop_back();
	void clear();

	template<bool isconst>
	void insert(const iterator<isconst>& it, T data);
	template<bool isconst>
	void erase(const iterator<isconst>& it);

	T&						front();
	T&						back();
	const T&				front()		const;
	const T&				back()		const;

	bool					empty()		const;
	int						size()		const;

	iterator<false>			begin();
	iterator<false>			end();
	iterator<true>			cbegin()	const;
	iterator<true>			cend()		const;
	reverse_iterator<false> rbegin();
	reverse_iterator<false> rend();
	reverse_iterator<true>  crbegin()	const;
	reverse_iterator<true>  crend()		const;

	friend std::ostream& operator<< <T>(std::ostream& os, const MyLinkedList<T>& mylinkedlist);
};

#include "MyList.inl"
