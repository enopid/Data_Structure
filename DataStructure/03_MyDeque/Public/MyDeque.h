#pragma once
#include <cstddef>
#include <cstring>
#include<iostream>
#include<deque>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include "MyVector.h"

template<typename T, int CHUNKMAXSIZE = 1024>
class MyDeque : public MyContainer {
    static constexpr int ELEMENTSIZE    = sizeof(T);
    static constexpr int ELEMENTCOUNT   = CHUNKMAXSIZE / sizeof(T) > 0
                                        ? CHUNKMAXSIZE / sizeof(T)
                                        : 1;
    static constexpr int CHUNKSIZE      = ELEMENTSIZE * ELEMENTCOUNT;
private:
    struct Chunk {
        alignas(T) std::byte data[sizeof(T) * ELEMENTCOUNT];
    };
public:
    MyDeque();
    MyDeque(const MyDeque& other);
    MyDeque& operator=(const MyDeque& other);
    MyDeque(MyDeque&& other)            noexcept;
    MyDeque& operator=(MyDeque&& other) noexcept;
    ~MyDeque();

    //member access 
    void        push_front(const T& data);
    void        push_back(const T& data);
    void        pop_front();
    void        pop_back();
    void        clear();

    T& operator[] (int ind);
    const T& operator[] (int ind) const;

    T&          front();
    T&          back();
    const T&    front()		const;
    const T&    back()		const;

    bool	    empty()		const { return m_iSize == 0; };
    int         size()      const { return m_iSize; };
private:
    void        Grow();
private:
    MyVector<Chunk*> m_vecRingBuffer;
    int m_iSize         = 0;
    int m_iStartOffset  = 0;

    virtual void print_info()       override;
    virtual void print_elements()   override;
};

#include "MyDeque.inl"
