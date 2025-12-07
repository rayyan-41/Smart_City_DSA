#pragma once
#include "LinkedLists.h"
#include <stdexcept>

template <typename T>
class Stack {
private:
    LinkedList<T> list;

public:
    Stack() : list() {}

    Stack(const Stack& other) : list(other.list) {}

    Stack& operator=(const Stack& other) {
        if (this != &other)
            list = other.list;
        return *this;
    }

    ~Stack() = default;

    void push(const T& value) {
        list.push_front(value);
    }

    void pop() {
        if (empty())
            throw std::runtime_error("Stack is empty");
        list.pop_front();
    }

    T& top() {
        return list.front();
    }

    const T& top() const {
        return list.front();
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

    void swap(Stack& other) {
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
