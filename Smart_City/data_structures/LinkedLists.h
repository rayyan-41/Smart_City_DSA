/*
 * ============================================================================
 * SINGLY LINKED LIST - For Bus Route Management & General Use
 * ============================================================================
 * 
 * A singly linked list implementation used for:
 *   - Bus route stop management (sequential traversal)
 *   - Route node tracking
 *   - Forward-only iteration patterns
 *   - Stack and Queue underlying storage
 * 
 * Rubric: Singly Linked List for Bus route management (4 marks)
 * ============================================================================
 */

#pragma once
#include <stdexcept>

template <typename T>
class LinkedList {
public:
    // Public node structure for external iteration
    struct Node {
        T data;
        Node* next;
        
        Node(const T& value) : data(value), next(nullptr) {}
    };

private:
    Node* head;
    Node* tail;
    int m_size;

public:
    // ==================== LIFECYCLE ====================
    
    LinkedList() : head(nullptr), tail(nullptr), m_size(0) {}
    
    LinkedList(const LinkedList& other) : head(nullptr), tail(nullptr), m_size(0) {
        Node* curr = other.head;
        while (curr) {
            push_back(curr->data);
            curr = curr->next;
        }
    }
    
    LinkedList& operator=(const LinkedList& other) {
        if (this == &other) return *this;
        
        clear();
        Node* curr = other.head;
        while (curr) {
            push_back(curr->data);
            curr = curr->next;
        }
        return *this;
    }
    
    ~LinkedList() {
        clear();
    }
    
    // ==================== ACCESSORS ====================
    
    Node* getHead() const { return head; }
    Node* getTail() const { return tail; }
    int size() const { return m_size; }
    int getSize() const { return m_size; }
    bool empty() const { return m_size == 0; }
    
    T& front() {
        if (empty()) throw std::out_of_range("List is empty");
        return head->data;
    }
    
    const T& front() const {
        if (empty()) throw std::out_of_range("List is empty");
        return head->data;
    }
    
    T& back() {
        if (empty()) throw std::out_of_range("List is empty");
        return tail->data;
    }
    
    const T& back() const {
        if (empty()) throw std::out_of_range("List is empty");
        return tail->data;
    }
    
    T& at(int index) {
        if (index < 0 || index >= m_size)
            throw std::out_of_range("Index out of range");
        return nodeAt(index)->data;
    }
    
    const T& at(int index) const {
        if (index < 0 || index >= m_size)
            throw std::out_of_range("Index out of range");
        return nodeAt(index)->data;
    }
    
    T& operator[](int index) { return at(index); }
    const T& operator[](int index) const { return at(index); }
    
    // ==================== MODIFIERS ====================
    
    // Add to front - O(1)
    void push_front(const T& value) {
        Node* newNode = new Node(value);
        newNode->next = head;
        head = newNode;
        
        if (m_size == 0) {
            tail = head;
        }
        ++m_size;
    }
    
    // Add to back - O(1) with tail pointer
    void push_back(const T& value) {
        Node* newNode = new Node(value);
        
        if (m_size == 0) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        ++m_size;
    }
    
    // Remove from front - O(1)
    void pop_front() {
        if (empty()) return;
        
        Node* temp = head;
        head = head->next;
        delete temp;
        --m_size;
        
        if (m_size == 0) {
            tail = nullptr;
        }
    }
    
    // Remove from back - O(n) since we need to find previous node
    void pop_back() {
        if (empty()) return;
        
        if (m_size == 1) {
            delete head;
            head = tail = nullptr;
            m_size = 0;
            return;
        }
        
        // Find second to last node
        Node* curr = head;
        while (curr->next != tail) {
            curr = curr->next;
        }
        
        delete tail;
        tail = curr;
        tail->next = nullptr;
        --m_size;
    }
    
    // Insert at index - O(n)
    void insert(int index, const T& value) {
        if (index < 0 || index > m_size)
            throw std::out_of_range("Index out of range");
        
        if (index == 0) {
            push_front(value);
            return;
        }
        
        if (index == m_size) {
            push_back(value);
            return;
        }
        
        Node* prev = nodeAt(index - 1);
        Node* newNode = new Node(value);
        newNode->next = prev->next;
        prev->next = newNode;
        ++m_size;
    }
    
    // Erase at index - O(n)
    void erase(int index) {
        if (index < 0 || index >= m_size)
            throw std::out_of_range("Index out of range");
        
        if (index == 0) {
            pop_front();
            return;
        }
        
        Node* prev = nodeAt(index - 1);
        Node* toDelete = prev->next;
        prev->next = toDelete->next;
        
        if (toDelete == tail) {
            tail = prev;
        }
        
        delete toDelete;
        --m_size;
    }
    
    // Clear all elements
    void clear() {
        while (head) {
            Node* temp = head;
            head = head->next;
            delete temp;
        }
        head = tail = nullptr;
        m_size = 0;
    }
    
    // Swap contents with another list
    void swap(LinkedList& other) {
        Node* tempHead = head;
        Node* tempTail = tail;
        int tempSize = m_size;
        
        head = other.head;
        tail = other.tail;
        m_size = other.m_size;
        
        other.head = tempHead;
        other.tail = tempTail;
        other.m_size = tempSize;
    }
    
    // ==================== SEARCH ====================
    
    int find(const T& value) const {
        Node* curr = head;
        int index = 0;
        while (curr) {
            if (curr->data == value) return index;
            curr = curr->next;
            ++index;
        }
        return -1;
    }
    
    bool contains(const T& value) const {
        return find(value) != -1;
    }
    
    // Remove first occurrence of value
    void remove(const T& value) {
        if (empty()) return;
        
        // Special case: head contains value
        if (head->data == value) {
            pop_front();
            return;
        }
        
        Node* curr = head;
        while (curr->next) {
            if (curr->next->data == value) {
                Node* toDelete = curr->next;
                curr->next = toDelete->next;
                
                if (toDelete == tail) {
                    tail = curr;
                }
                
                delete toDelete;
                --m_size;
                return;
            }
            curr = curr->next;
        }
    }
    
    // ==================== ROUTE-SPECIFIC OPERATIONS ====================
    
    // Get node at specific position (for route traversal)
    Node* getNodeAt(int index) {
        return nodeAt(index);
    }
    
    // Reverse the list (for reverse route)
    void reverse() {
        if (m_size <= 1) return;
        
        Node* prev = nullptr;
        Node* curr = head;
        tail = head;
        
        while (curr) {
            Node* next = curr->next;
            curr->next = prev;
            prev = curr;
            curr = next;
        }
        
        head = prev;
    }
    
    // Get sublist from index start to end (inclusive)
    LinkedList<T> sublist(int start, int end) const {
        LinkedList<T> result;
        
        if (start < 0 || end >= m_size || start > end) {
            return result;
        }
        
        Node* curr = nodeAt(start);
        for (int i = start; i <= end && curr; ++i) {
            result.push_back(curr->data);
            curr = curr->next;
        }
        
        return result;
    }

private:
    Node* nodeAt(int index) const {
        Node* curr = head;
        for (int i = 0; i < index; ++i) {
            curr = curr->next;
        }
        return curr;
    }
};

// ============================================================================
// CIRCULAR LINKED LIST - For Circular Queue and Round-Robin Scheduling
// ============================================================================

template <typename T>
class CircularList {
public:
    struct Node {
        T data;
        Node* next;
        
        Node(const T& value) : data(value), next(nullptr) {}
    };

private:
    Node* head;
    Node* tail;  // Points to last node, tail->next = head
    int m_size;

public:
    CircularList() : head(nullptr), tail(nullptr), m_size(0) {}
    
    CircularList(const CircularList& other) : head(nullptr), tail(nullptr), m_size(0) {
        if (other.m_size == 0) return;
        
        Node* curr = other.head;
        for (int i = 0; i < other.m_size; ++i) {
            push_back(curr->data);
            curr = curr->next;
        }
    }
    
    CircularList& operator=(const CircularList& other) {
        if (this == &other) return *this;
        
        clear();
        if (other.m_size == 0) return *this;
        
        Node* curr = other.head;
        for (int i = 0; i < other.m_size; ++i) {
            push_back(curr->data);
            curr = curr->next;
        }
        return *this;
    }
    
    ~CircularList() {
        clear();
    }
    
    // ==================== ACCESSORS ====================
    
    Node* getHead() const { return head; }
    Node* getTail() const { return tail; }
    int size() const { return m_size; }
    int getSize() const { return m_size; }
    bool empty() const { return m_size == 0; }
    
    T& front() {
        if (empty()) throw std::out_of_range("List is empty");
        return head->data;
    }
    
    const T& front() const {
        if (empty()) throw std::out_of_range("List is empty");
        return head->data;
    }
    
    T& back() {
        if (empty()) throw std::out_of_range("List is empty");
        return tail->data;
    }
    
    const T& back() const {
        if (empty()) throw std::out_of_range("List is empty");
        return tail->data;
    }
    
    T& at(int index) {
        if (index < 0 || index >= m_size)
            throw std::out_of_range("Index out of range");
        return nodeAt(index)->data;
    }
    
    const T& at(int index) const {
        if (index < 0 || index >= m_size)
            throw std::out_of_range("Index out of range");
        return nodeAt(index)->data;
    }
    
    T& operator[](int index) { return at(index); }
    const T& operator[](int index) const { return at(index); }
    
    // ==================== MODIFIERS ====================
    
    void push_front(const T& value) {
        Node* newNode = new Node(value);
        
        if (m_size == 0) {
            head = tail = newNode;
            newNode->next = newNode;  // Points to itself
        } else {
            newNode->next = head;
            head = newNode;
            tail->next = head;  // Maintain circular link
        }
        ++m_size;
    }
    
    void push_back(const T& value) {
        Node* newNode = new Node(value);
        
        if (m_size == 0) {
            head = tail = newNode;
            newNode->next = newNode;
        } else {
            newNode->next = head;
            tail->next = newNode;
            tail = newNode;
        }
        ++m_size;
    }
    
    void pop_front() {
        if (empty()) return;
        
        if (m_size == 1) {
            delete head;
            head = tail = nullptr;
            m_size = 0;
            return;
        }
        
        Node* temp = head;
        head = head->next;
        tail->next = head;
        delete temp;
        --m_size;
    }
    
    void pop_back() {
        if (empty()) return;
        
        if (m_size == 1) {
            delete head;
            head = tail = nullptr;
            m_size = 0;
            return;
        }
        
        // Find second to last node
        Node* curr = head;
        while (curr->next != tail) {
            curr = curr->next;
        }
        
        delete tail;
        tail = curr;
        tail->next = head;
        --m_size;
    }
    
    void insert(int index, const T& value) {
        if (index < 0 || index > m_size)
            throw std::out_of_range("Index out of range");
        
        if (index == 0) {
            push_front(value);
            return;
        }
        
        if (index == m_size) {
            push_back(value);
            return;
        }
        
        Node* prev = nodeAt(index - 1);
        Node* newNode = new Node(value);
        newNode->next = prev->next;
        prev->next = newNode;
        ++m_size;
    }
    
    void erase(int index) {
        if (index < 0 || index >= m_size)
            throw std::out_of_range("Index out of range");
        
        if (index == 0) {
            pop_front();
            return;
        }
        
        if (index == m_size - 1) {
            pop_back();
            return;
        }
        
        Node* prev = nodeAt(index - 1);
        Node* toDelete = prev->next;
        prev->next = toDelete->next;
        delete toDelete;
        --m_size;
    }
    
    void clear() {
        if (m_size == 0) {
            head = tail = nullptr;
            return;
        }
        
        Node* curr = head;
        for (int i = 0; i < m_size; ++i) {
            Node* next = curr->next;
            delete curr;
            curr = next;
        }
        
        head = tail = nullptr;
        m_size = 0;
    }
    
    void swap(CircularList& other) {
        Node* tempHead = head;
        Node* tempTail = tail;
        int tempSize = m_size;
        
        head = other.head;
        tail = other.tail;
        m_size = other.m_size;
        
        other.head = tempHead;
        other.tail = tempTail;
        other.m_size = tempSize;
    }
    
    // ==================== SEARCH ====================
    
    int find(const T& value) const {
        if (m_size == 0) return -1;
        
        Node* curr = head;
        for (int i = 0; i < m_size; ++i) {
            if (curr->data == value) return i;
            curr = curr->next;
        }
        return -1;
    }
    
    bool contains(const T& value) const {
        return find(value) != -1;
    }
    
    void remove(const T& value) {
        if (empty()) return;
        
        // Check head
        if (head->data == value) {
            pop_front();
            return;
        }
        
        Node* curr = head;
        for (int i = 0; i < m_size - 1; ++i) {
            if (curr->next->data == value) {
                Node* toDelete = curr->next;
                
                if (toDelete == tail) {
                    tail = curr;
                }
                
                curr->next = toDelete->next;
                delete toDelete;
                --m_size;
                return;
            }
            curr = curr->next;
        }
    }
    
    // ==================== CIRCULAR OPERATIONS ====================
    
    // Rotate: move head to next (round-robin)
    void rotate() {
        if (m_size <= 1) return;
        tail = head;
        head = head->next;
    }
    
    // Rotate n times
    void rotate(int n) {
        if (m_size <= 1) return;
        n = n % m_size;
        for (int i = 0; i < n; ++i) {
            rotate();
        }
    }

private:
    Node* nodeAt(int index) const {
        Node* curr = head;
        for (int i = 0; i < index; ++i) {
            curr = curr->next;
        }
        return curr;
    }
};

