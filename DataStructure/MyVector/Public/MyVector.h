#pragma once

#include "MyContainer.h"

template<typename T>
class MyVector : public MyContainer {
public:
    MyVector();
    MyVector(int N, const T& val = T());
    MyVector(const MyVector&);
    MyVector& operator=(const MyVector&);
    MyVector(MyVector&&) noexcept;
    MyVector& operator=(MyVector&&) noexcept;
    ~MyVector();

    //member access 
    int size();
    int capacity();
    T*  data();

    //capacity modification 
    void resize(int newsize);
    void resize(int newsize, const T& val);
    void clear();
    bool empty();
    void reserve(int newcapacity);
    void shrink_to_fit();

    //element manipulation 
    void insert(int pos, const T& val);
    void erase(int pos);
    void pop_back();
    void push_back(const T& val);

    //element access 
    T& operator[] (int ind);
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
