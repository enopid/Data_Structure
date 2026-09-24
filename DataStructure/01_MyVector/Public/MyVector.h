#pragma once

#include "MyContainer.h"

#include <type_traits>
#include <utility>

namespace myvector_detail {

template <typename T, typename = void>
struct is_ostreamable : std::false_type {};

template <typename T>
struct is_ostreamable<T, std::void_t<decltype(std::declval<std::ostream&>()
                                              << std::declval<const T&>())>>
    : std::true_type {};

} // namespace myvector_detail

template<typename T>
class MyVector : public MyContainer {
public:
    MyVector();
    MyVector(int N, const T& val = T());
    MyVector(const MyVector& other);
    MyVector& operator=(const MyVector& other);
    MyVector(MyVector&& other)            noexcept;
    MyVector& operator=(MyVector&& other) noexcept;
    ~MyVector();

    //member access 
    int size() const;
    int capacity() const;
    T*  data();

    //capacity modification 
    void resize(int newsize);
    void resize(int newsize, const T& val);
    void clear();
    bool empty() const;
    void reserve(int newcapacity);
    void shrink_to_fit();

    //element manipulation 
    void insert(int pos, const T& val);
    void erase(int pos);
    void pop_back();
    void push_back(const T& val);

    //element access 
    T& operator[] (int ind);
    const T& operator[] (int ind) const;
    T& front();
    T& back();

    //For Debug
    static void set_growth_factor(double growth_factor);
    void linear_push_back(const T& val);

    virtual void print_info()       override;
    virtual void print_elements()   override;
private:
    int _size;
    int _capacity;
    static double _growthFactor;
    T* _data;
};

#include "MyVector.inl"
