#pragma once

#include "MyVector.h"

template<typename T, typename Comp = std::less<T>>
class MyPriorityQueue : public MyContainer {
public:
    MyPriorityQueue();
    MyPriorityQueue(const MyPriorityQueue& other);
    MyPriorityQueue& operator=(const MyPriorityQueue& other);
    MyPriorityQueue(MyPriorityQueue&& other)            noexcept;
    MyPriorityQueue& operator=(MyPriorityQueue&& other) noexcept;
    ~MyPriorityQueue();
public:
    struct FPQHandle {
        uint32_t     iHandleID   = 0;
        uint32_t     iGeneration = 0;
    };
private:
    struct FPQHandleSlot {
        int          iHeapIdx    = 0;
        uint32_t     iGeneration = 0;
        bool         bAlive      = false;
    };
    struct FPQNode   {
        T           value;
        uint32_t    iHandleID = 0;
    };
public:
    //MemberAccess
    const T&    top() const;
    const T&    get     (FPQHandle handle) const;
    bool        valid   (FPQHandle handle) const;

    //Capacity
    bool    empty() const;
    int     size()  const;

    //Modifiers
    void        clear();
    void        pop();

    FPQHandle   push(const T& value);

    void        update  (FPQHandle handle, const T& value);
    void        erase   (FPQHandle handle);
private:
    void SwapNode(int iParentIdx, int iCurIdx);
private:
    MyVector<FPQNode>       _nodes;
    MyVector<FPQHandleSlot> _handles;
    MyVector<uint32_t>      _freeIDs;

    Comp _comp;

    // MyContainer을(를) 통해 상속됨
    void print_info() override;
    void print_elements() override;

};

#include "MyPriorityQueue.inl"
