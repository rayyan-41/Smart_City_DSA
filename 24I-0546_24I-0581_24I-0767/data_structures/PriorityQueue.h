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
            if (!(data[parent] < data[index]))
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
            int largest = index;

            if (left < n && data[largest] < data[left])
                largest = left;
            if (right < n && data[largest] < data[right])
                largest = right;

            if (largest == index)
                break;

            T temp = data[index];
            data[index] = data[largest];
            data[largest] = temp;

            index = largest;
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