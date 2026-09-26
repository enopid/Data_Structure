#include "MyVector.h"
template<typename T>
MyVector<T>::MyVector() {
    iSize       = 0;
    iCapacity   = 1;
    pData       = static_cast<T*>(malloc(sizeof(T) * iCapacity));
}

template<typename T>
MyVector<T>::MyVector(int N, const T& val) {
    iSize       = N;
    iCapacity   = N;
    pData       = static_cast<T*>(malloc(sizeof(T) * iCapacity));
    for (int i = 0; i < iSize; i++) new (pData + i) T(val);
}

template<typename T>
inline MyVector<T>::MyVector(const MyVector& other)
{
    iSize       = other.iSize;
    iCapacity   = other.iCapacity;
    pData       = static_cast<T*>(malloc(sizeof(T) * iCapacity));
    if constexpr (std::is_trivially_copyable_v<T>)
        memcpy(pData, other.pData, sizeof(T) * iSize);
    else
        for (int i = 0; i < iSize; i++) new (pData + i) T(other.pData[i]);
}

template<typename T>
MyVector<T>& MyVector<T>::operator=(const MyVector<T>& other)
{
    if (this == &other) return *this;
    for (int i = 0; i < iSize; i++) pData[i].~T();
    free(pData);

    iSize       = other.iSize;
    iCapacity   = other.iCapacity;
    pData       = static_cast<T*>(malloc(sizeof(T) * iCapacity));
    if constexpr (std::is_trivially_copyable_v<T>)
        memcpy(pData, other.pData, sizeof(T) * iSize);
    else 
        for (int i = 0; i < iSize; i++) new (pData + i) T(other.pData[i]);

    return *this;
}

template<typename T>
MyVector<T>::MyVector(MyVector<T>&& other) noexcept
{
    iSize           = other.iSize;
    iCapacity       = other.iCapacity;
    pData           = other.pData;
    other.iSize     = 0;
    other.iCapacity = 0;
	other.pData     = nullptr;
}

template<typename T>
MyVector<T>& MyVector<T>::operator=(MyVector<T>&& other) noexcept
{
    if (this == &other) return *this;
    for (int i = 0; i < iSize; i++) pData[i].~T();
    free(pData);

    iSize           = other.iSize;
    iCapacity       = other.iCapacity;
    pData           = other.pData;
    other.iSize     = 0;
    other.iCapacity = 0;
	other.pData     = nullptr;

    return *this;
}

template<typename T>
MyVector<T>::~MyVector() {
    for (int i = 0; i < iSize; i++) pData[i].~T();
    free(pData);
}

//member access 
template<typename T>
int MyVector<T>::size()     const { return iSize;     }
template<typename T>
int MyVector<T>::capacity() const { return iCapacity; }
template<typename T>
T* MyVector<T>::data()      { return pData;     }

//capacity modification 
template<typename T>
void MyVector<T>::resize(int newsize) {
    if (iCapacity < newsize) { reserve(newsize); }
    for (int i = iSize; i < newsize; i++) new (pData + i) T();
    for (int i = newsize; i < iSize; i++) pData[i].~T();

    iSize = newsize;
}

template<typename T>
void MyVector<T>::resize(int newSize, const T& val) {
    if (iCapacity < newSize) { reserve(newSize); }
    for (int i = iSize; i < newSize; i++) new (pData + i) T(val);
    for (int i = newSize; i < iSize; i++) pData[i].~T();
    iSize = newSize;
}

template<typename T>
void MyVector<T>::clear() {
    for (int i = 0; i < iSize; i++) pData[i].~T();
    iSize = 0;
}

template<typename T>
bool MyVector<T>::empty() const { return (iSize == 0); }

template<typename T>
void MyVector<T>::reserve(int newCapacity) {
    if (iCapacity >= newCapacity)  return;
    
    T* _pNewData = static_cast<T*>(malloc(sizeof(T) * newCapacity));
    if constexpr (std::is_trivially_copyable_v<T>) {
        memcpy(_pNewData, pData, sizeof(T) * iSize);
    }
    else {
        for (int i = 0; i < iSize; i++) new (_pNewData + i) T(std::move_if_noexcept(pData[i]));
        for (int i = 0; i < iSize; i++) pData[i].~T();
    }
    free(pData);

    pData       = _pNewData;
    iCapacity   = newCapacity;
}

template<typename T>
void MyVector<T>::shrink_to_fit() {
    if (iCapacity <= iSize) return;
    
    T* _pNewData = static_cast<T*>(malloc(sizeof(T) * iSize));
    if constexpr (std::is_trivially_copyable_v<T>) {
        memcpy(_pNewData, pData, sizeof(T) * iSize);
    }
    else {
        for (int i = 0; i < iSize; i++) new (_pNewData + i) T(std::move_if_noexcept(pData[i]));
        for (int i = 0; i < iSize; i++) pData[i].~T();
    }
    free(pData);

    pData       = _pNewData;
    iCapacity   = iSize;
}

//element manipulation 
template<typename T>
void MyVector<T>::insert(int pos, const T& val) {
    if (pos >= iSize) return;
    if (iSize >= iCapacity) { reserve(std::max(int(iCapacity * fGrowthFactor), 2)); }

    if constexpr (std::is_trivially_copyable_v<T>) {
        memmove(pData + pos + 1, pData + pos, sizeof(T) * (iSize - pos));
    }
    else {
        new (pData + iSize) T(std::move(pData[iSize - 1]));
		for (int i = iSize-1; i > pos; i--) pData[i] = std::move(pData[i - 1]);
		pData[pos].~T();
    }
    new (pData + pos) T(val);
    ++iSize;
}

template<typename T>
void MyVector<T>::erase(int pos) {
    if (pos < 0 || pos >= iSize) return;

    if constexpr (std::is_trivially_copyable_v<T>) {
        memmove(pData + pos, pData + pos + 1, sizeof(T) * (iSize - pos - 1));
    }
    else {
        for (int i = pos; i < iSize - 1; i++) pData[i] = std::move(pData[i + 1]);
    }
    pData[iSize - 1].~T();
    iSize--;
}

template<typename T>
void MyVector<T>::pop_back() {
    if (!empty())
        pData[--iSize].~T();
}

template<typename T>
void MyVector<T>::push_back(const T& val) {
    if (iSize >= iCapacity) { reserve(std::max(int(iCapacity * fGrowthFactor), 2)); }
    new (pData + iSize++) T(val);
}


template<typename T>
inline void MyVector<T>::print_info()
{
    std::cout << "Size \t: " << iSize << "\tCapacity \t: " << iCapacity << "\n";
}

template<typename T>
inline void MyVector<T>::print_elements()
{
    if constexpr (myvector_detail::is_ostreamable<T>::value)
    {
        std::cout << "elements \t: ";
        for (size_t i = 0; i < iSize; i++) std::cout << pData[i] << "\t";
        std::cout << "\n";
    }
}

//element access 
template<typename T>
T& MyVector<T>::operator[] (int ind) {
    return pData[ind];
}

template<typename T>
inline const T& MyVector<T>::operator[](int ind) const
{
    return pData[ind];
}

template<typename T>
T& MyVector<T>::front() {
    return pData[0];
}

template<typename T>
T& MyVector<T>::back() {
    return pData[iSize - 1];
}

//utility 
template<typename T>
void MyVector<T>::linear_push_back(const T& val) {
    if (iSize >= iCapacity) { reserve(iCapacity + 10); }
    new (pData + iSize++) T(val);
}
template<typename T>
void MyVector<T>::set_growth_factor(double growth_factor) {
    fGrowthFactor = growth_factor;
}

template<typename T>
double MyVector<T>::fGrowthFactor = 1.5;
