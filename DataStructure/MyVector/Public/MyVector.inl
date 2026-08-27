#include "MyVector.h"
template<typename T>
MyVector<T>::MyVector() {
    _size       = 0;
    _capacity   = 1;
    _data       = static_cast<T*>(malloc(sizeof(T) * _capacity));
}

template<typename T>
MyVector<T>::MyVector(int N, const T& val) {
    _size       = N;
    _capacity   = N;
    _data       = static_cast<T*>(malloc(sizeof(T) * _capacity));
    for (int i = 0; i < _size; i++) new (_data + i) T(val);
}

template<typename T>
inline MyVector<T>::MyVector(const MyVector& other)
{
    _size       = other._size;
    _capacity   = other._capacity;
    _data       = static_cast<T*>(malloc(sizeof(T) * _capacity));
    if constexpr (std::is_trivially_copyable_v<T>)
        memcpy(_data, other._data, sizeof(T) * _size);
    else
        for (int i = 0; i < _size; i++) new (_data + i) T(other._data[i]);
}

template<typename T>
MyVector<T>& MyVector<T>::operator=(const MyVector<T>& other)
{
    if (this == &other) return *this;
    for (int i = 0; i < _size; i++) _data[i].~T();
    free(_data);

    _size       = other._size;
    _capacity   = other._capacity;
    _data       = static_cast<T*>(malloc(sizeof(T) * _capacity));
    if constexpr (std::is_trivially_copyable_v<T>)
        memcpy(_data, other._data, sizeof(T) * _size);
    else 
        for (int i = 0; i < _size; i++) new (_data + i) T(other._data[i]);

    return *this;
}

template<typename T>
MyVector<T>::MyVector(MyVector<T>&& other) noexcept
{
    _size           = other._size;
    _capacity       = other._capacity;
    _data           = other._data;
    other._size     = 0;
    other._capacity = 0;
	other._data     = nullptr;
}

template<typename T>
MyVector<T>& MyVector<T>::operator=(MyVector<T>&& other) noexcept
{
    if (this == &other) return *this;
    for (int i = 0; i < _size; i++) _data[i].~T();
    free(_data);

    _size           = other._size;
    _capacity       = other._capacity;
    _data           = other._data;
    other._size     = 0;
    other._capacity = 0;
	other._data     = nullptr;

    return *this;
}

template<typename T>
MyVector<T>::~MyVector() {
    for (int i = 0; i < _size; i++) _data[i].~T();
    free(_data);
}

//member access 
template<typename T>
int MyVector<T>::size()     { return _size;     }
template<typename T>
int MyVector<T>::capacity() { return _capacity; }
template<typename T>
T* MyVector<T>::data()      { return _data;     }

//capacity modification 
template<typename T>
void MyVector<T>::resize(int newsize) {
    if (_capacity < newsize) { reserve(newsize); }
    for (int i = _size; i < newsize; i++) new (_data + i) T();
    for (int i = newsize; i < _size; i++) _data[i].~T();

    _size = newsize;
}

template<typename T>
void MyVector<T>::resize(int newsize, const T& val) {
    if (_capacity < newsize) { reserve(newsize); }
    for (int i = _size; i < newsize; i++) new (_data + i) T(val);
    for (int i = newsize; i < _size; i++) _data[i].~T();
    _size = newsize;
}

template<typename T>
void MyVector<T>::clear() {
    for (int i = 0; i < _size; i++) _data[i].~T();
    _size = 0; 
}

template<typename T>
bool MyVector<T>::empty() { return (_size == 0); }

template<typename T>
void MyVector<T>::reserve(int newcapacity) {
    if (_capacity >= newcapacity)  return;
    
    T* newData = static_cast<T*>(malloc(sizeof(T) * newcapacity));
    if constexpr (std::is_trivially_copyable_v<T>) {
        memcpy(newData, _data, sizeof(T) * _size);
    }
    else {
        for (int i = 0; i < _size; i++) new (newData + i) T(std::move_if_noexcept(_data[i]));
        for (int i = 0; i < _size; i++) _data[i].~T();
    }
    free(_data);

    _data       = newData;
    _capacity   = newcapacity;
}

template<typename T>
void MyVector<T>::shrink_to_fit() {
    if (_capacity <= _size) return;
    
    T* newData = static_cast<T*>(malloc(sizeof(T) * _size));
    if constexpr (std::is_trivially_copyable_v<T>) {
        memcpy(newData, _data, sizeof(T) * _size);
    }
    else {
        for (int i = 0; i < _size; i++) new (newData + i) T(std::move_if_noexcept(_data[i]));
        for (int i = 0; i < _size; i++) _data[i].~T();
    }
    free(_data);

    _data       = newData;
    _capacity   = _size;
}

//element manipulation 
template<typename T>
void MyVector<T>::insert(int pos, const T& val) {
    if (pos >= _size) return;
    if (_size >= _capacity) { reserve(_capacity * 2); }

    if constexpr (std::is_trivially_copyable_v<T>) {
        memmove(_data + pos + 1, _data + pos, sizeof(T) * (_size - pos));
    }
    else {
        new (_data + _size) T(std::move(_data[_size - 1]));
		for (int i = _size-1; i > pos; i--) _data[i] = std::move(_data[i - 1]);
		_data[pos].~T();
    }
    new (_data + pos) T(val);
    ++_size;
}

template<typename T>
void MyVector<T>::erase(int pos) {
    if (pos < 0 || pos >= _size) return;

    if constexpr (std::is_trivially_copyable_v<T>) {
        memmove(_data + pos, _data + pos + 1, sizeof(T) * (_size - pos - 1));
    }
    else {
        for (int i = pos; i < _size - 1; i++) _data[i] = std::move(_data[i + 1]);
    }
    _data[_size - 1].~T();
    _size--;
}

template<typename T>
void MyVector<T>::pop_back() {
    if (!empty())
        _data[--_size].~T();
}

template<typename T>
void MyVector<T>::push_back(const T& val) {
    if (_size >= _capacity) { reserve(std::max(int(_capacity * _growthFactor), 2)); }
    new (_data + _size++) T(val);
}


template<typename T>
inline void MyVector<T>::print_info()
{
    std::cout << "Size \t: " << _size << "\tCapacity \t: " << _capacity << "\n";
}

template<typename T>
inline void MyVector<T>::print_elements()
{
    std::cout << "elements \t: ";
    for (size_t i = 0; i < _size; i++) std::cout << _data[i] << "\t";
    std::cout << "\n";
}

//element access 
template<typename T>
T& MyVector<T>::operator[] (int ind) {
    return _data[ind];
}

template<typename T>
T& MyVector<T>::front() {
    return _data[0];
}

template<typename T>
T& MyVector<T>::back() {
    return _data[_size - 1];
}

//utility 
template<typename T>
void MyVector<T>::linear_push_back(const T& val) {
    if (_size >= _capacity) { reserve(_capacity + 10); }
    new (_data + _size++) T(val);
}
template<typename T>
void MyVector<T>::set_growth_factor(double growth_factor) {
    _growthFactor = growth_factor;
}

template<typename T>
double MyVector<T>::_growthFactor = 2;
