#pragma once
#include <iostream>
#include <stdexcept>


    template <typename T>
    class LinkedList {
    private:
        struct Node {
            T data;
            Node* prev;
            Node* next;

            Node(const T& value)
                : data(value), prev(nullptr), next(nullptr) {
            }
        };

        Node* head;
        Node* tail;
        int   m_size;

    public:
        LinkedList() : head(nullptr), tail(nullptr), m_size(0) {}

        LinkedList(const LinkedList& other) : head(nullptr), tail(nullptr), m_size(0) {
            Node* curr = other.head;
            while (curr) {
                push_back(curr->data);
                curr = curr->next;
            }
        }

        LinkedList& operator=(const LinkedList& other) {
            if (this == &other)
                return *this;

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

        void push_front(const T& value) {
            Node* n = new Node(value);
            n->next = head;
            if (head)
                head->prev = n;
            head = n;

            if (m_size == 0)
                tail = head;

            ++m_size;
        }

        void push_back(const T& value) {
            Node* n = new Node(value);
            n->prev = tail;
            if (tail)
                tail->next = n;
            tail = n;

            if (m_size == 0)
                head = tail;

            ++m_size;
        }

        void pop_front() {
            if (m_size == 0)
                return;

            Node* temp = head;
            head = head->next;
            if (head)
                head->prev = nullptr;
            else
                tail = nullptr;

            delete temp;
            --m_size;
        }

        void pop_back() {
            if (m_size == 0)
                return;

            Node* temp = tail;
            tail = tail->prev;
            if (tail)
                tail->next = nullptr;
            else
                head = nullptr;

            delete temp;
            --m_size;
        }

        T& front() {
            if (m_size == 0)
                throw std::out_of_range("List is empty");
            return head->data;
        }

        const T& front() const {
            if (m_size == 0)
                throw std::out_of_range("List is empty");
            return head->data;
        }

        T& back() {
            if (m_size == 0)
                throw std::out_of_range("List is empty");
            return tail->data;
        }

        const T& back() const {
            if (m_size == 0)
                throw std::out_of_range("List is empty");
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

        T& operator[](int index) {
            if (index < 0 || index >= m_size)
                throw std::out_of_range("Index out of range");
            return nodeAt(index)->data;
        }

        const T& operator[](int index) const {
            if (index < 0 || index >= m_size)
                throw std::out_of_range("Index out of range");
            return nodeAt(index)->data;
        }

        void insert(int index, const T& value) {
            if (index < 0 || index > m_size)
                throw std::out_of_range("Index out of range");

            if (index == 0) {
                push_front(value);
            }
            else if (index == m_size) {
                push_back(value);
            }
            else {
                Node* curr = nodeAt(index);
                Node* prevNode = curr->prev;

                Node* n = new Node(value);
                n->prev = prevNode;
                n->next = curr;

                prevNode->next = n;
                curr->prev = n;

                ++m_size;
            }
        }

        void erase(int index) {
            if (index < 0 || index >= m_size)
                throw std::out_of_range("Index out of range");

            if (index == 0) {
                pop_front();
            }
            else if (index == m_size - 1) {
                pop_back();
            }
            else {
                Node* curr = nodeAt(index);
                Node* prevNode = curr->prev;
                Node* nextNode = curr->next;

                prevNode->next = nextNode;
                nextNode->prev = prevNode;

                delete curr;
                --m_size;
            }
        }

        int getSize() const { return m_size; }
        int size()   const { return m_size; }
        bool empty() const { return m_size == 0; }

        void clear() {
            Node* curr = head;
            while (curr) {
                Node* next = curr->next;
                delete curr;
                curr = next;
            }
            head = tail = nullptr;
            m_size = 0;
        }

        void swap(LinkedList& other) {
            Node* tempHead = head;
            Node* tempTail = tail;
            int   tempSize = m_size;

            head = other.head;
            tail = other.tail;
            m_size = other.m_size;

            other.head = tempHead;
            other.tail = tempTail;
            other.m_size = tempSize;
        }

        int find(const T& value) const {
            Node* curr = head;
            for (int i = 0; i < m_size; ++i) {
                if (curr->data == value)
                    return i;
                curr = curr->next;
            }
            return -1;
        }

        bool contains(const T& value) const {
            return find(value) != -1;
        }

        void remove(const T& value) {
            Node* curr = head;
            while (curr) {
                if (curr->data == value) {
                    Node* toDelete = curr;
                    curr = curr->next;

                    if (toDelete == head) {
                        pop_front();
                    }
                    else if (toDelete == tail) {
                        pop_back();
                    }
                    else {
                        toDelete->prev->next = toDelete->next;
                        toDelete->next->prev = toDelete->prev;
                        delete toDelete;
                        --m_size;
                    }
                }
                else {
                    curr = curr->next;
                }
            }
        }

    private:
        Node* nodeAt(int index) {
            if (index < m_size / 2) {
                Node* curr = head;
                for (int i = 0; i < index; ++i)
                    curr = curr->next;
                return curr;
            }
            else {
                Node* curr = tail;
                for (int i = m_size - 1; i > index; --i)
                    curr = curr->prev;
                return curr;
            }
        }

        const Node* nodeAt(int index) const {
            if (index < m_size / 2) {
                const Node* curr = head;
                for (int i = 0; i < index; ++i)
                    curr = curr->next;
                return curr;
            }
            else {
                const Node* curr = tail;
                for (int i = m_size - 1; i > index; --i)
                    curr = curr->prev;
                return curr;
            }
        }
    };

    // ===================== CIRCULAR DOUBLY LINKED LIST =====================

    template <typename T>
    class CircularList {
    private:
        struct Node {
            T data;
            Node* prev;
            Node* next;

            Node(const T& value)
                : data(value), prev(nullptr), next(nullptr) {
            }
        };

        Node* head;
        int   m_size;

    public:
        CircularList() : head(nullptr), m_size(0) {}

        CircularList(const CircularList& other) : head(nullptr), m_size(0) {
            if (other.m_size == 0)
                return;

            Node* curr = other.head;
            for (int i = 0; i < other.m_size; ++i) {
                push_back(curr->data);
                curr = curr->next;
            }
        }

        CircularList& operator=(const CircularList& other) {
            if (this == &other)
                return *this;

            clear();
            if (other.m_size == 0)
                return *this;

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

        void push_front(const T& value) {
            Node* n = new Node(value);

            if (m_size == 0) {
                n->next = n;
                n->prev = n;
                head = n;
            }
            else {
                Node* tail = head->prev;

                n->next = head;
                n->prev = tail;

                tail->next = n;
                head->prev = n;

                head = n;
            }
            ++m_size;
        }

        void push_back(const T& value) {
            Node* n = new Node(value);

            if (m_size == 0) {
                n->next = n;
                n->prev = n;
                head = n;
            }
            else {
                Node* tail = head->prev;

                n->next = head;
                n->prev = tail;

                tail->next = n;
                head->prev = n;
            }
            ++m_size;
        }

        void pop_front() {
            if (m_size == 0)
                return;

            if (m_size == 1) {
                delete head;
                head = nullptr;
                m_size = 0;
                return;
            }

            Node* tail = head->prev;
            Node* oldHead = head;
            Node* newHead = head->next;

            tail->next = newHead;
            newHead->prev = tail;

            head = newHead;
            delete oldHead;
            --m_size;
        }

        void pop_back() {
            if (m_size == 0)
                return;

            if (m_size == 1) {
                delete head;
                head = nullptr;
                m_size = 0;
                return;
            }

            Node* tail = head->prev;
            Node* newTail = tail->prev;

            newTail->next = head;
            head->prev = newTail;

            delete tail;
            --m_size;
        }

        T& front() {
            if (m_size == 0)
                throw std::out_of_range("List is empty");
            return head->data;
        }

        const T& front() const {
            if (m_size == 0)
                throw std::out_of_range("List is empty");
            return head->data;
        }

        T& back() {
            if (m_size == 0)
                throw std::out_of_range("List is empty");
            return head->prev->data;
        }

        const T& back() const {
            if (m_size == 0)
                throw std::out_of_range("List is empty");
            return head->prev->data;
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

        T& operator[](int index) {
            if (index < 0 || index >= m_size)
                throw std::out_of_range("Index out of range");
            return nodeAt(index)->data;
        }

        const T& operator[](int index) const {
            if (index < 0 || index >= m_size)
                throw std::out_of_range("Index out of range");
            return nodeAt(index)->data;
        }

        void insert(int index, const T& value) {
            if (index < 0 || index > m_size)
                throw std::out_of_range("Index out of range");

            if (index == 0) {
                push_front(value);
            }
            else if (index == m_size) {
                push_back(value);
            }
            else {
                Node* curr = nodeAt(index);
                Node* prevNode = curr->prev;

                Node* n = new Node(value);
                n->prev = prevNode;
                n->next = curr;

                prevNode->next = n;
                curr->prev = n;

                ++m_size;
            }
        }

        void erase(int index) {
            if (index < 0 || index >= m_size)
                throw std::out_of_range("Index out of range");

            if (index == 0) {
                pop_front();
            }
            else if (index == m_size - 1) {
                pop_back();
            }
            else {
                Node* curr = nodeAt(index);
                Node* prevNode = curr->prev;
                Node* nextNode = curr->next;

                prevNode->next = nextNode;
                nextNode->prev = prevNode;

                delete curr;
                --m_size;
            }
        }

        int  getSize() const { return m_size; }
        int  size()   const { return m_size; }
        bool empty()  const { return m_size == 0; }

        void clear() {
            if (m_size == 0) {
                head = nullptr;
                return;
            }

            Node* curr = head;
            for (int i = 0; i < m_size; ++i) {
                Node* next = curr->next;
                delete curr;
                curr = next;
            }

            head = nullptr;
            m_size = 0;
        }

        void swap(CircularList& other) {
            Node* tempHead = head;
            int   tempSize = m_size;

            head = other.head;
            m_size = other.m_size;

            other.head = tempHead;
            other.m_size = tempSize;
        }

        int find(const T& value) const {
            if (m_size == 0)
                return -1;

            Node* curr = head;
            for (int i = 0; i < m_size; ++i) {
                if (curr->data == value)
                    return i;
                curr = curr->next;
            }
            return -1;
        }

        bool contains(const T& value) const {
            return find(value) != -1;
        }

        void remove(const T& value) {
            if (m_size == 0)
                return;

            int   originalSize = m_size;
            Node* curr = head;

            for (int i = 0; i < originalSize; ++i) {
                Node* next = curr->next;

                if (curr->data == value) {
                    if (m_size == 1) {
                        delete head;
                        head = nullptr;
                        m_size = 0;
                        return;
                    }

                    Node* prevNode = curr->prev;
                    Node* nextNode = curr->next;

                    prevNode->next = nextNode;
                    nextNode->prev = prevNode;

                    if (curr == head) {
                        head = nextNode;
                    }

                    delete curr;
                    --m_size;
                    curr = nextNode;

                    if (m_size == 0) {
                        head = nullptr;
                        return;
                    }
                }
                else {
                    curr = next;
                }
            }
        }

    private:
        Node* nodeAt(int index) {
            if (index <= m_size / 2) {
                Node* curr = head;
                for (int i = 0; i < index; ++i)
                    curr = curr->next;
                return curr;
            }
            else {
                Node* curr = head->prev; // tail
                for (int i = m_size - 1; i > index; --i)
                    curr = curr->prev;
                return curr;
            }
        }

        const Node* nodeAt(int index) const {
            if (index <= m_size / 2) {
                const Node* curr = head;
                for (int i = 0; i < index; ++i)
                    curr = curr->next;
                return curr;
            }
            else {
                const Node* curr = head->prev; // tail
                for (int i = m_size - 1; i > index; --i)
                    curr = curr->prev;
                return curr;
            }
        }
    };

