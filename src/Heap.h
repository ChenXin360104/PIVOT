#ifndef HEAP_H
#define HEAP_H

#include <vector>
#include <stdexcept>
#include <iostream>
#include"SegmentTree.h"
#include <queue>

// Element 结构
struct Element
{
    double value;
    std::vector<SegmentTreeNode *> nodePairs;

    Element(double val, std::vector<SegmentTreeNode *> &nodes)
        : value(val), nodePairs(nodes) {}

    Element(){}
};

// 比较器：小根堆（值小的优先）
struct CompareMin
{
    bool operator()(const Element &a, const Element &b) const
    {
        return a.value > b.value;
    }
};

// 比较器：大根堆（值大的优先）
struct CompareMax
{
    bool operator()(const Element &a, const Element &b) const
    {
        return a.value < b.value;
    }
};

// 封装类：MinHeap / MaxHeap（通过模板参数切换）
template <typename Comparator>
class Heap
{
public:
    void add(const Element &e)
    {
        pq.push(e);
    }

    Element top() const
    {
        return pq.top();
    }

    void pop()
    {
        pq.pop();
    }

    bool empty() const
    {
        return pq.empty();
    }

    size_t size() const
    {
        return pq.size();
    }

    std::priority_queue<Element, std::vector<Element>, Comparator> pq;
};


#endif // HEAP_H
