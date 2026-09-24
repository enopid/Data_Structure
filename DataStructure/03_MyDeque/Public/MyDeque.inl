#include "MyDeque.h"
#pragma once

// Deque
template<typename T, int CHUNKMAXSIZE>
inline MyDeque<T, CHUNKMAXSIZE>::MyDeque()
{
}
template<typename T, int CHUNKMAXSIZE>
inline MyDeque<T, CHUNKMAXSIZE>::MyDeque(const MyDeque& other)
{
	if (std::is_trivially_copyable_v<T>) {
		m_vecRingBuffer.reserve(other.m_vecRingBuffer.size());
		for (int i = 0; i < other.m_vecRingBuffer.size(); i++) {
			m_vecRingBuffer.push_back(new Chunk);
			memcpy(m_vecRingBuffer[i], other.m_vecRingBuffer[i], CHUNKSIZE);
		}
	}
	else {
		m_vecRingBuffer.reserve(other.m_vecRingBuffer.size());
		for (int i = 0; i < other.m_vecRingBuffer.size(); i++) {
			m_vecRingBuffer.push_back(new Chunk);
		}

		for (int i = 0; i < other.m_iSize; i++) {
			int iNextOffset = (other.m_iStartOffset + i) % (m_vecRingBuffer.size() * ELEMENTCOUNT);
			int iChunkOffset= iNextOffset / ELEMENTCOUNT;
			int iOffset		= iNextOffset % ELEMENTCOUNT;

			T* pSrc = reinterpret_cast<T*>(other.m_vecRingBuffer[iChunkOffset]);
			T* pDst = reinterpret_cast<T*>(m_vecRingBuffer[iChunkOffset]);

			new (pDst + iOffset) T(pSrc[iOffset]);
		}
	}

	m_iSize			= other.m_iSize;
	m_iStartOffset	= other.m_iStartOffset;
}
template<typename T, int CHUNKMAXSIZE>
inline MyDeque<T, CHUNKMAXSIZE>& MyDeque<T, CHUNKMAXSIZE>::operator=(const MyDeque& other)
{
	if (this == &other) return *this;
	
	clear();
	for (int i = 0; i < m_vecRingBuffer.size(); ++i) {
		delete m_vecRingBuffer[i];
	}
	m_vecRingBuffer.clear();

	if (std::is_trivially_copyable_v<T>) {
		m_vecRingBuffer.reserve(other.m_vecRingBuffer.size());
		for (int i = 0; i < other.m_vecRingBuffer.size(); i++) {
			m_vecRingBuffer.push_back(new Chunk);
			memcpy(m_vecRingBuffer[i], other.m_vecRingBuffer[i], CHUNKSIZE);
		}
	}
	else {
		m_vecRingBuffer.reserve(other.m_vecRingBuffer.size());
		for (int i = 0; i < other.m_vecRingBuffer.size(); i++) {
			m_vecRingBuffer.push_back(new Chunk);
		}

		for (int i = 0; i < other.m_iSize; i++) {
			int iNextOffset = (other.m_iStartOffset + i) % (m_vecRingBuffer.size() * ELEMENTCOUNT);
			int iChunkOffset = iNextOffset / ELEMENTCOUNT;
			int iOffset = iNextOffset % ELEMENTCOUNT;

			T* pSrc = reinterpret_cast<T*>(other.m_vecRingBuffer[iChunkOffset]);
			T* pDst = reinterpret_cast<T*>(m_vecRingBuffer[iChunkOffset]);

			new (pDst + iOffset) T(pSrc[iOffset]);
		}
	}
	m_iSize = other.m_iSize;
	m_iStartOffset = other.m_iStartOffset;

	return *this;
}
template<typename T, int CHUNKMAXSIZE>
inline MyDeque<T, CHUNKMAXSIZE>::MyDeque(MyDeque&& other) noexcept
{

	m_vecRingBuffer = std::move(other.m_vecRingBuffer);
	m_iSize			= other.m_iSize;
	m_iStartOffset	= other.m_iStartOffset;
	other.m_vecRingBuffer.clear();
	other.m_iSize		= 0;
	other.m_iStartOffset= 0;
}
template<typename T, int CHUNKMAXSIZE>
inline MyDeque<T, CHUNKMAXSIZE>& MyDeque<T, CHUNKMAXSIZE>::operator=(MyDeque&& other) noexcept
{
	if (this == &other) return *this;

	clear();
	for (int i = 0; i < m_vecRingBuffer.size(); ++i) {
		delete m_vecRingBuffer[i];
	}
	m_vecRingBuffer.clear();

	m_vecRingBuffer = std::move(other.m_vecRingBuffer);
	m_iSize			= other.m_iSize;
	m_iStartOffset	= other.m_iStartOffset;
	other.m_vecRingBuffer.clear();
	other.m_iSize		= 0;
	other.m_iStartOffset= 0;

	return *this;
}
template<typename T, int CHUNKMAXSIZE>
inline MyDeque<T, CHUNKMAXSIZE>::~MyDeque()
{
	clear();
	for (int i = 0; i < m_vecRingBuffer.size(); ++i) {
		delete m_vecRingBuffer[i];
	}
}

template<typename T, int CHUNKMAXSIZE>
inline void MyDeque<T, CHUNKMAXSIZE>::Grow()
{
	if (m_iSize < m_vecRingBuffer.size() * ELEMENTCOUNT) return;
	if (m_iSize == 0) {
		m_vecRingBuffer.push_back(new Chunk);
		return;
	}

	int iChunkOffset = (m_iStartOffset + m_iSize) % (m_vecRingBuffer.size() * ELEMENTCOUNT);
	iChunkOffset /= ELEMENTCOUNT;
	int iOffset = m_iStartOffset % ELEMENTCOUNT;
	int iOldChunkCount = m_vecRingBuffer.size();

	MyVector<Chunk*> m_vecTmp;
	m_vecTmp.reserve(iOldChunkCount * 2);
	for (int i = 0; i < iOldChunkCount; i++) {
		m_vecTmp.push_back(m_vecRingBuffer[(i + iChunkOffset) % iOldChunkCount]);
	}
	for (int i = 0; i < iOldChunkCount; i++) {
		m_vecTmp.push_back(new Chunk);
	}

	T* pDst = reinterpret_cast<T*>(m_vecTmp[iOldChunkCount]);
	T* pSrc = reinterpret_cast<T*>(m_vecRingBuffer[iChunkOffset]);
	for (int i = 0; i < iOffset; i++) {
		new (pDst + i) T(std::move(pSrc[i]));
		pSrc[i].~T();
	}

	m_vecRingBuffer = std::move(m_vecTmp);
	m_iStartOffset	= iOffset;
}

template<typename T, int CHUNKMAXSIZE>
inline void MyDeque<T, CHUNKMAXSIZE>::print_info()
{
	std::cout << "Size \t: "<< m_iSize 
	<< "CHUNKSIZE \t: "		<< CHUNKSIZE
	<< "ELEMENTCOUNT \t: "	<< ELEMENTCOUNT
	<< "\n";
}

template<typename T, int CHUNKMAXSIZE>
inline void MyDeque<T, CHUNKMAXSIZE>::print_elements()
{
}

template<typename T, int CHUNKMAXSIZE>
inline void MyDeque<T, CHUNKMAXSIZE>::push_front(const T& data)
{
	if (m_iSize >= m_vecRingBuffer.size() * ELEMENTCOUNT) {
		Grow();
	}
	m_iSize++;

	m_iStartOffset	+= m_vecRingBuffer.size() * ELEMENTCOUNT - 1;
	m_iStartOffset	%= (m_vecRingBuffer.size() * ELEMENTCOUNT);
	int iChunkOffset= m_iStartOffset / ELEMENTCOUNT;
	int iOffset		= m_iStartOffset % ELEMENTCOUNT;
	Chunk* pCurChunk= m_vecRingBuffer[iChunkOffset];

	new (reinterpret_cast<T*>(pCurChunk) + iOffset) T(data);
}

template<typename T, int CHUNKMAXSIZE>
inline void MyDeque<T, CHUNKMAXSIZE>::push_back(const T& data)
{
	if (m_iSize >= m_vecRingBuffer.size() * ELEMENTCOUNT) {
		Grow();
	}
	int iNextOffset = (m_iStartOffset + m_iSize) % (m_vecRingBuffer.size() * ELEMENTCOUNT);
	int iChunkOffset= iNextOffset / ELEMENTCOUNT;
	int iOffset		= iNextOffset % ELEMENTCOUNT;
	Chunk* pCurChunk= m_vecRingBuffer[iChunkOffset];
	m_iSize++;

	new (reinterpret_cast<T*>(pCurChunk) + iOffset) T(data);
}

template<typename T, int CHUNKMAXSIZE>
inline void MyDeque<T, CHUNKMAXSIZE>::pop_front()
{
	if (empty()) { return; }
	m_iSize--;

	int iChunkOffset = m_iStartOffset / ELEMENTCOUNT;
	int iOffset		 = m_iStartOffset % ELEMENTCOUNT;
	Chunk* pCurChunk = m_vecRingBuffer[iChunkOffset];

	reinterpret_cast<T*>(pCurChunk)[iOffset].~T();

	m_iStartOffset += m_vecRingBuffer.size() * ELEMENTCOUNT + 1;
	m_iStartOffset %= (m_vecRingBuffer.size() * ELEMENTCOUNT);
}

template<typename T, int CHUNKMAXSIZE>
inline void MyDeque<T, CHUNKMAXSIZE>::pop_back()
{
	if (empty()) { return; }
	m_iSize--;

	int iNextOffset = (m_iStartOffset + m_iSize) % (m_vecRingBuffer.size() * ELEMENTCOUNT);
	int iChunkOffset= iNextOffset / ELEMENTCOUNT;
	int iOffset		= iNextOffset % ELEMENTCOUNT;
	Chunk* pCurChunk= m_vecRingBuffer[iChunkOffset];

	reinterpret_cast<T*>(pCurChunk)[iOffset].~T();
}

template<typename T, int CHUNKMAXSIZE>
inline void MyDeque<T, CHUNKMAXSIZE>::clear()
{
	if (empty()) { return; }
	for (int i = 0; i < m_iSize; i++) {
		int iNextOffset = (m_iStartOffset + i) % (m_vecRingBuffer.size() * ELEMENTCOUNT);
		int iChunkOffset = iNextOffset / ELEMENTCOUNT;
		int iOffset = iNextOffset % ELEMENTCOUNT;
		Chunk* pCurChunk = m_vecRingBuffer[iChunkOffset];

		reinterpret_cast<T*>(pCurChunk)[iOffset].~T();
	}

	m_iSize = 0;
	m_iStartOffset = 0;
}

template<typename T, int CHUNKMAXSIZE>
T& MyDeque<T, CHUNKMAXSIZE>::front() {
	return const_cast<T&>(static_cast<const MyDeque&>(*this).front());
}

template<typename T, int CHUNKMAXSIZE>
T& MyDeque<T, CHUNKMAXSIZE>::back() {
	return const_cast<T&>(static_cast<const MyDeque&>(*this).back());
}

template<typename T, int CHUNKMAXSIZE>
const T& MyDeque<T, CHUNKMAXSIZE>::front() const {
	if (empty()) {
		throw std::out_of_range("deque is empty!");
	}

	int iChunkOffset= m_iStartOffset / ELEMENTCOUNT;
	int iOffset		= m_iStartOffset % ELEMENTCOUNT;

	return reinterpret_cast<T*>(m_vecRingBuffer[iChunkOffset])[iOffset];
}

template<typename T, int CHUNKMAXSIZE>
const T& MyDeque<T, CHUNKMAXSIZE>::back() const {
	if (empty()) {
		throw std::out_of_range("deque is empty!");
	}

	int iEndOffset	= (m_iStartOffset + m_iSize - 1) % (m_vecRingBuffer.size() * ELEMENTCOUNT);
	int iChunkOffset= iEndOffset / ELEMENTCOUNT;
	int iOffset		= iEndOffset % ELEMENTCOUNT;

	return reinterpret_cast<T*>(m_vecRingBuffer[iChunkOffset])[iOffset];
}

template<typename T, int CHUNKMAXSIZE>
inline T& MyDeque<T, CHUNKMAXSIZE>::operator[] (int ind) {
	return const_cast<T&>(static_cast<const MyDeque&>(*this)[ind]);
}

template<typename T, int CHUNKMAXSIZE>
inline const T& MyDeque<T, CHUNKMAXSIZE>::operator[](int ind) const
{
	if (ind <0 || ind  >= m_iSize) {
		throw std::out_of_range("Index out of range!");
	}

	int iEndOffset	= (m_iStartOffset + ind) % (m_vecRingBuffer.size() * ELEMENTCOUNT);
	int iChunkOffset= iEndOffset / ELEMENTCOUNT;
	int iOffset		= iEndOffset % ELEMENTCOUNT;

	return reinterpret_cast<T*>(m_vecRingBuffer[iChunkOffset])[iOffset];
}
