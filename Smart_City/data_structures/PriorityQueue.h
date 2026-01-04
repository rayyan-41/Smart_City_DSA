#pragma once
#include <stdexcept>
#include "Vector.h"

template <typename T>
class PriorityQueue {
private:
    Vector<T> data;

    void heapifyUp(int index) {
        while (index > 0) {
            int parent = (index - 1) / 2;

            if (!(data[index] < data[parent]))
                break;

            T temp = data[parent];
            data[parent] = data[index];
            data[index] = temp;

            index = parent;
        }
    }

    void heapifyDown(int index) {
        int n = data.getSize();
        while (true) {
            int left = 2 * index + 1;
            int right = 2 * index + 2;
            int smallest = index;

            if (left < n && data[left] < data[smallest])
                smallest = left;
            if (right < n && data[right] < data[smallest])
                smallest = right;

            if (smallest == index)
                break;

            T temp = data[index];
            data[index] = data[smallest];
            data[smallest] = temp;

            index = smallest;
        }
    }

public:
    PriorityQueue() : data() {}

    PriorityQueue(const PriorityQueue& other)
        : data(other.data) {
    }

    PriorityQueue& operator=(const PriorityQueue& other) {
        if (this != &other)
            data = other.data;
        return *this;
    }

    ~PriorityQueue() = default;

    bool empty() const {
        return data.empty();
    }

    int size() const {
        return data.getSize();
    }

    void clear() {
        data.clear();
    }

    void swap(PriorityQueue& other) {
        Vector<T> temp = data;
        data = other.data;
        other.data = temp;
    }

    void push(const T& value) {
        data.push_back(value);
        heapifyUp(data.getSize() - 1);
    }

    void pop() {
        if (empty())
            throw std::runtime_error("PriorityQueue is empty");

        int lastIndex = data.getSize() - 1;
        data[0] = data[lastIndex];
        data.pop_back();

        if (!data.empty())
            heapifyDown(0);
    }

    T& top() {
        if (empty())
            throw std::out_of_range("PriorityQueue is empty");
        return data[0];
    }

    const T& top() const {
        if (empty())
            throw std::out_of_range("PriorityQueue is empty");
        return data[0];
    }

    Vector<T>& getVector() {
        return data;
    }

    const Vector<T>& getVector() const {
        return data;
    }
};
