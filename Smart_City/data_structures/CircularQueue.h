#pragma once
#include "LinkedLists.h"
#include <stdexcept>

template <typename T>
class CircularQueue {
private:
    CircularList<T> list;
    int maxCapacity;  

public:
  
    
    CircularQueue() : list(), maxCapacity(0) {}
    
    explicit CircularQueue(int capacity) : list(), maxCapacity(capacity) {}
    
    CircularQueue(const CircularQueue& other) 
        : list(other.list), maxCapacity(other.maxCapacity) {}
    
    CircularQueue& operator=(const CircularQueue& other) {
        if (this != &other) {
            list = other.list;
            maxCapacity = other.maxCapacity;
        }
        return *this;
    }
    
    ~CircularQueue() = default;
    
    
    bool empty() const { return list.empty(); }
    int size() const { return list.size(); }
    int getSize() const { return list.getSize(); }
    
    bool isFull() const {
        return maxCapacity > 0 && list.size() >= maxCapacity;
    }
    
    int capacity() const { return maxCapacity; }
    
    void setCapacity(int cap) { maxCapacity = cap; }
    
    
    T& front() {
        if (empty()) throw std::runtime_error("CircularQueue is empty");
        return list.front();
    }
    
    const T& front() const {
        if (empty()) throw std::runtime_error("CircularQueue is empty");
        return list.front();
    }
    
    T& back() {
        if (empty()) throw std::runtime_error("CircularQueue is empty");
        return list.back();
    }
    
    const T& back() const {
        if (empty()) throw std::runtime_error("CircularQueue is empty");
        return list.back();
    }
    
    T& at(int index) {
        if (index < 0 || index >= list.size())
            throw std::out_of_range("Index out of range");
        return list.at(index);
    }
    
    const T& at(int index) const {
        if (index < 0 || index >= list.size())
            throw std::out_of_range("Index out of range");
        return list.at(index);
    }
    
    
    bool enqueue(const T& value) {
        if (isFull()) return false;
        list.push_back(value);
        return true;
    }
    
    bool push(const T& value) { return enqueue(value); }
    
    T dequeue() {
        if (empty()) throw std::runtime_error("CircularQueue is empty");
        T value = list.front();
        list.pop_front();
        return value;
    }
    
    T pop() { return dequeue(); }
    
    void pop_front() {
        if (empty()) throw std::runtime_error("CircularQueue is empty");
        list.pop_front();
    }
    
    void clear() { list.clear(); }
    
    
    void rotate() {
        list.rotate();
    }
    
    void rotate(int n) {
        list.rotate(n);
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
    
    
    void swap(CircularQueue& other) {
        list.swap(other.list);
        int temp = maxCapacity;
        maxCapacity = other.maxCapacity;
        other.maxCapacity = temp;
    }
};
