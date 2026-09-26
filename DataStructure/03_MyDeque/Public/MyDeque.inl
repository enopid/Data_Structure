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
		vecRingBuffer.reserve(other.vecRingBuffer.size());
		for (int i = 0; i < other.vecRingBuffer.size(); i++) {
			vecRingBuffer.push_back(new Chunk);
			memcpy(vecRingBuffer[i], other.vecRingBuffer[i], CHUNKSIZE);
		}
	}
	else {
		vecRingBuffer.reserve(other.vecRingBuffer.size());
		for (int i = 0; i < other.vecRingBuffer.size(); i++) {
			vecRingBuffer.push_back(new Chunk);
		}

		for (int i = 0; i < other.iSize; i++) {
			int iNextOffset = (other.iStartOffset + i) % (vecRingBuffer.size() * ELEMENTCOUNT);
			int iChunkOffset= iNextOffset / ELEMENTCOUNT;
			int iOffset		= iNextOffset % ELEMENTCOUNT;

			T* pSrc = reinterpret_cast<T*>(other.vecRingBuffer[iChunkOffset]);
			T* pDst = reinterpret_cast<T*>(vecRingBuffer[iChunkOffset]);

			new (pDst + iOffset) T(pSrc[iOffset]);
		}
	}

	iSize			= other.iSize;
	iStartOffset	= other.iStartOffset;
}
template<typename T, int CHUNKMAXSIZE>
inline MyDeque<T, CHUNKMAXSIZE>& MyDeque<T, CHUNKMAXSIZE>::operator=(const MyDeque& other)
{
	if (this == &other) return *this;
	
	clear();
	for (int i = 0; i < vecRingBuffer.size(); ++i) {
		delete vecRingBuffer[i];
	}
	vecRingBuffer.clear();

	if (std::is_trivially_copyable_v<T>) {
		vecRingBuffer.reserve(other.vecRingBuffer.size());
		for (int i = 0; i < other.vecRingBuffer.size(); i++) {
			vecRingBuffer.push_back(new Chunk);
			memcpy(vecRingBuffer[i], other.vecRingBuffer[i], CHUNKSIZE);
		}
	}
	else {
		vecRingBuffer.reserve(other.vecRingBuffer.size());
		for (int i = 0; i < other.vecRingBuffer.size(); i++) {
			vecRingBuffer.push_back(new Chunk);
		}

		for (int i = 0; i < other.iSize; i++) {
			int iNextOffset = (other.iStartOffset + i) % (vecRingBuffer.size() * ELEMENTCOUNT);
			int iChunkOffset = iNextOffset / ELEMENTCOUNT;
			int iOffset = iNextOffset % ELEMENTCOUNT;

			T* pSrc = reinterpret_cast<T*>(other.vecRingBuffer[iChunkOffset]);
			T* pDst = reinterpret_cast<T*>(vecRingBuffer[iChunkOffset]);

			new (pDst + iOffset) T(pSrc[iOffset]);
		}
	}
	iSize = other.iSize;
	iStartOffset = other.iStartOffset;

	return *this;
}
template<typename T, int CHUNKMAXSIZE>
inline MyDeque<T, CHUNKMAXSIZE>::MyDeque(MyDeque&& other) noexcept
{

	vecRingBuffer = std::move(other.vecRingBuffer);
	iSize			= other.iSize;
	iStartOffset	= other.iStartOffset;
	other.vecRingBuffer.clear();
	other.iSize		= 0;
	other.iStartOffset= 0;
}
template<typename T, int CHUNKMAXSIZE>
inline MyDeque<T, CHUNKMAXSIZE>& MyDeque<T, CHUNKMAXSIZE>::operator=(MyDeque&& other) noexcept
{
	if (this == &other) return *this;

	clear();
	for (int i = 0; i < vecRingBuffer.size(); ++i) {
		delete vecRingBuffer[i];
	}
	vecRingBuffer.clear();

	vecRingBuffer = std::move(other.vecRingBuffer);
	iSize			= other.iSize;
	iStartOffset	= other.iStartOffset;
	other.vecRingBuffer.clear();
	other.iSize		= 0;
	other.iStartOffset= 0;

	return *this;
}
template<typename T, int CHUNKMAXSIZE>
inline MyDeque<T, CHUNKMAXSIZE>::~MyDeque()
{
	clear();
	for (int i = 0; i < vecRingBuffer.size(); ++i) {
		delete vecRingBuffer[i];
	}
}

template<typename T, int CHUNKMAXSIZE>
inline void MyDeque<T, CHUNKMAXSIZE>::Grow()
{
	if (iSize < vecRingBuffer.size() * ELEMENTCOUNT) return;
	if (iSize == 0) {
		vecRingBuffer.push_back(new Chunk);
		return;
	}

	int iChunkOffset = (iStartOffset + iSize) % (vecRingBuffer.size() * ELEMENTCOUNT);
	iChunkOffset /= ELEMENTCOUNT;
	int iOffset = iStartOffset % ELEMENTCOUNT;
	int iOldChunkCount = vecRingBuffer.size();

	MyVector<Chunk*> m_vecTmp;
	m_vecTmp.reserve(iOldChunkCount * 2);
	for (int i = 0; i < iOldChunkCount; i++) {
		m_vecTmp.push_back(vecRingBuffer[(i + iChunkOffset) % iOldChunkCount]);
	}
	for (int i = 0; i < iOldChunkCount; i++) {
		m_vecTmp.push_back(new Chunk);
	}

	T* pDst = reinterpret_cast<T*>(m_vecTmp[iOldChunkCount]);
	T* pSrc = reinterpret_cast<T*>(vecRingBuffer[iChunkOffset]);
	for (int i = 0; i < iOffset; i++) {
		new (pDst + i) T(std::move(pSrc[i]));
		pSrc[i].~T();
	}

	vecRingBuffer = std::move(m_vecTmp);
	iStartOffset	= iOffset;
}

template<typename T, int CHUNKMAXSIZE>
inline void MyDeque<T, CHUNKMAXSIZE>::print_info()
{
	std::cout << "Size \t: "<< iSize
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
	if (iSize >= vecRingBuffer.size() * ELEMENTCOUNT) {
		Grow();
	}
	iSize++;

	iStartOffset	+= vecRingBuffer.size() * ELEMENTCOUNT - 1;
	iStartOffset	%= (vecRingBuffer.size() * ELEMENTCOUNT);
	int iChunkOffset= iStartOffset / ELEMENTCOUNT;
	int iOffset		= iStartOffset % ELEMENTCOUNT;
	Chunk* pCurChunk= vecRingBuffer[iChunkOffset];

	new (reinterpret_cast<T*>(pCurChunk) + iOffset) T(data);
}

template<typename T, int CHUNKMAXSIZE>
inline void MyDeque<T, CHUNKMAXSIZE>::push_back(const T& data)
{
	if (iSize >= vecRingBuffer.size() * ELEMENTCOUNT) {
		Grow();
	}
	int iNextOffset = (iStartOffset + iSize) % (vecRingBuffer.size() * ELEMENTCOUNT);
	int iChunkOffset= iNextOffset / ELEMENTCOUNT;
	int iOffset		= iNextOffset % ELEMENTCOUNT;
	Chunk* pCurChunk= vecRingBuffer[iChunkOffset];
	iSize++;

	new (reinterpret_cast<T*>(pCurChunk) + iOffset) T(data);
}

template<typename T, int CHUNKMAXSIZE>
inline void MyDeque<T, CHUNKMAXSIZE>::pop_front()
{
	if (empty()) { return; }
	iSize--;

	int iChunkOffset = iStartOffset / ELEMENTCOUNT;
	int iOffset		 = iStartOffset % ELEMENTCOUNT;
	Chunk* pCurChunk = vecRingBuffer[iChunkOffset];

	reinterpret_cast<T*>(pCurChunk)[iOffset].~T();

	iStartOffset += vecRingBuffer.size() * ELEMENTCOUNT + 1;
	iStartOffset %= (vecRingBuffer.size() * ELEMENTCOUNT);
}

template<typename T, int CHUNKMAXSIZE>
inline void MyDeque<T, CHUNKMAXSIZE>::pop_back()
{
	if (empty()) { return; }
	iSize--;

	int iNextOffset = (iStartOffset + iSize) % (vecRingBuffer.size() * ELEMENTCOUNT);
	int iChunkOffset= iNextOffset / ELEMENTCOUNT;
	int iOffset		= iNextOffset % ELEMENTCOUNT;
	Chunk* pCurChunk= vecRingBuffer[iChunkOffset];

	reinterpret_cast<T*>(pCurChunk)[iOffset].~T();
}

template<typename T, int CHUNKMAXSIZE>
inline void MyDeque<T, CHUNKMAXSIZE>::clear()
{
	if (empty()) { return; }
	for (int i = 0; i < iSize; i++) {
		int iNextOffset = (iStartOffset + i) % (vecRingBuffer.size() * ELEMENTCOUNT);
		int iChunkOffset = iNextOffset / ELEMENTCOUNT;
		int iOffset = iNextOffset % ELEMENTCOUNT;
		Chunk* pCurChunk = vecRingBuffer[iChunkOffset];

		reinterpret_cast<T*>(pCurChunk)[iOffset].~T();
	}

	iSize = 0;
	iStartOffset = 0;
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

	int iChunkOffset= iStartOffset / ELEMENTCOUNT;
	int iOffset		= iStartOffset % ELEMENTCOUNT;

	return reinterpret_cast<T*>(vecRingBuffer[iChunkOffset])[iOffset];
}

template<typename T, int CHUNKMAXSIZE>
const T& MyDeque<T, CHUNKMAXSIZE>::back() const {
	if (empty()) {
		throw std::out_of_range("deque is empty!");
	}

	int iEndOffset	= (iStartOffset + iSize - 1) % (vecRingBuffer.size() * ELEMENTCOUNT);
	int iChunkOffset= iEndOffset / ELEMENTCOUNT;
	int iOffset		= iEndOffset % ELEMENTCOUNT;

	return reinterpret_cast<T*>(vecRingBuffer[iChunkOffset])[iOffset];
}

template<typename T, int CHUNKMAXSIZE>
inline T& MyDeque<T, CHUNKMAXSIZE>::operator[] (int ind) {
	return const_cast<T&>(static_cast<const MyDeque&>(*this)[ind]);
}

template<typename T, int CHUNKMAXSIZE>
inline const T& MyDeque<T, CHUNKMAXSIZE>::operator[](int ind) const
{
	if (ind <0 || ind  >= iSize) {
		throw std::out_of_range("Index out of range!");
	}

	int iEndOffset	= (iStartOffset + ind) % (vecRingBuffer.size() * ELEMENTCOUNT);
	int iChunkOffset= iEndOffset / ELEMENTCOUNT;
	int iOffset		= iEndOffset % ELEMENTCOUNT;

	return reinterpret_cast<T*>(vecRingBuffer[iChunkOffset])[iOffset];
}
