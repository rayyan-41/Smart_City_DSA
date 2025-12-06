/*
 * ============================================================================
 * CIRCULAR QUEUE - For Passenger Waiting Simulation
 * ============================================================================
 * 
 * A circular queue implementation using circular linked list for:
 *   - Passenger queue simulation at bus stops
 *   - Round-robin scheduling of vehicles
 *   - Efficient FIFO with wrap-around
 * 
 * Rubric: Circular Queue for Passenger queue simulation (4 marks)
 * ============================================================================
 */

#pragma once
#include "LinkedLists.h"
#include <stdexcept>

template <typename T>
class CircularQueue {
private:
    CircularList<T> list;
    int maxCapacity;  // 0 = unlimited

public:
    // ==================== LIFECYCLE ====================
    
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
    
    // ==================== CAPACITY ====================
    
    bool empty() const { return list.empty(); }
    int size() const { return list.size(); }
    int getSize() const { return list.getSize(); }
    
    bool isFull() const {
        return maxCapacity > 0 && list.size() >= maxCapacity;
    }
    
    int capacity() const { return maxCapacity; }
    
    void setCapacity(int cap) { maxCapacity = cap; }
    
    // ==================== ACCESS ====================
    
    // Get front element (next to be dequeued)
    T& front() {
        if (empty()) throw std::runtime_error("CircularQueue is empty");
        return list.front();
    }
    
    const T& front() const {
        if (empty()) throw std::runtime_error("CircularQueue is empty");
        return list.front();
    }
    
    // Get back element (most recently enqueued)
    T& back() {
        if (empty()) throw std::runtime_error("CircularQueue is empty");
        return list.back();
    }
    
    const T& back() const {
        if (empty()) throw std::runtime_error("CircularQueue is empty");
        return list.back();
    }
    
    // Peek at index (0 = front)
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
    
    // ==================== MODIFIERS ====================
    
    // Enqueue - add to back
    bool enqueue(const T& value) {
        if (isFull()) return false;
        list.push_back(value);
        return true;
    }
    
    // Alias for enqueue
    bool push(const T& value) { return enqueue(value); }
    
    // Dequeue - remove from front and return
    T dequeue() {
        if (empty()) throw std::runtime_error("CircularQueue is empty");
        T value = list.front();
        list.pop_front();
        return value;
    }
    
    // Alias for dequeue (removes and returns)
    T pop() { return dequeue(); }
    
    // Pop without returning (for compatibility)
    void pop_front() {
        if (empty()) throw std::runtime_error("CircularQueue is empty");
        list.pop_front();
    }
    
    // Clear all elements
    void clear() { list.clear(); }
    
    // ==================== CIRCULAR OPERATIONS ====================
    
    // Rotate queue: move front to back (round-robin)
    void rotate() {
        list.rotate();
    }
    
    // Rotate n times
    void rotate(int n) {
        list.rotate(n);
    }
    
    // ==================== SEARCH ====================
    
    int find(const T& value) const {
        return list.find(value);
    }
    
    bool contains(const T& value) const {
        return list.contains(value);
    }
    
    void remove(const T& value) {
        list.remove(value);
    }
    
    // ==================== UTILITY ====================
    
    void swap(CircularQueue& other) {
        list.swap(other.list);
        int temp = maxCapacity;
        maxCapacity = other.maxCapacity;
        other.maxCapacity = temp;
    }
};
