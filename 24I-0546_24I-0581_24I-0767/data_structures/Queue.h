#pragma once
#include "LinkedLists.h"
#include <stdexcept>

template <typename T>
class Queue {
private:
    LinkedList<T> list;

public:
    Queue() : list() {}

    Queue(const Queue& other) : list(other.list) {}

    Queue& operator=(const Queue& other) {
        if (this != &other)
            list = other.list;
        return *this;
    }

    ~Queue() = default;

    void push(const T& value) {
        list.push_back(value);
    }

    void pop() {
        if (empty())
            throw std::runtime_error("Queue is empty");
        list.pop_front();
    }

    T& front() {
        return list.front();
    }

    const T& front() const {
        return list.front();
    }

    T& back() {
        return list.back();
    }

    const T& back() const {
        return list.back();
    }

    bool empty() const {
        return list.empty();
    }

    int size() const {
        return list.getSize();
    }

    void clear() {
        list.clear();
    }

    void swap(Queue& other) {
        list.swap(other.list);
    }

    int find(const T& value) const {
        return list.find(value);
    }

    bool contains(const T& value) const {
        return list.contains(value);
    }

    void remove(const T& value) {
        list.remove(value);
    }

    LinkedList<T>& getList() {
        return list;
    }

    const LinkedList<T>& getList() const {
        return list;
    }
};

