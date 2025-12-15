#pragma once
#include <string>
#include <stdexcept>
#include <iostream>

template <typename K, typename V>
struct HashNode {
    K key;
    V value;
    HashNode* next;

    HashNode(const K& k, const V& v) : key(k), value(v), next(nullptr) {}
};

template <typename K, typename V>
class HashTable {
private:
    HashNode<K, V>** table; 
    int capacity;          
    int size;               


    unsigned long hashFunction(const std::string& key) const {
        unsigned long hash = 0;
        unsigned long p = 31;     
        unsigned long m = capacity;
        unsigned long p_pow = 1;

        for (char c : key) {
            hash = (hash + (c * p_pow)) % m;
            p_pow = (p_pow * p) % m;
        }
        return hash;
    }

    unsigned long hashFunction(int key) const {
        return key % capacity;
    }

public:
    HashTable(int cap = 101) : capacity(cap), size(0) {
        table = new HashNode<K, V>* [capacity];
        for (int i = 0; i < capacity; i++) {
            table[i] = nullptr;
        }
    }

    ~HashTable() {
        clear();
        delete[] table;
    }


    void insert(const K& key, const V& value) {
        unsigned long index = hashFunction(key);

        HashNode<K, V>* current = table[index];

        while (current != nullptr) {
            if (current->key == key) {
                current->value = value;
                return;
            }
            current = current->next;
        }

        HashNode<K, V>* newNode = new HashNode<K, V>(key, value);
        newNode->next = table[index];
        table[index] = newNode;
        size++;
    }

    V* get(const K& key) const {
        unsigned long index = hashFunction(key);
        HashNode<K, V>* current = table[index];

        while (current != nullptr) {
            if (current->key == key) {
                return &(current->value);
            }
            current = current->next;
        }
        return nullptr;
    }

    bool contains(const K& key) const {
        return get(key) != nullptr;
    }

    bool remove(const K& key) {
        unsigned long index = hashFunction(key);
        HashNode<K, V>* current = table[index];
        HashNode<K, V>* prev = nullptr;

        while (current != nullptr) {
            if (current->key == key) {
                if (prev == nullptr) {
                    table[index] = current->next;
                }
                else {
                    prev->next = current->next;
                }
                delete current;
                size--;
                return true;
            }
            prev = current;
            current = current->next;
        }
        return false;
    }

    void clear() {
        for (int i = 0; i < capacity; i++) {
            HashNode<K, V>* current = table[i];
            while (current != nullptr) {
                HashNode<K, V>* toDelete = current;
                current = current->next;
                delete toDelete;
            }
            table[i] = nullptr;
        }
        size = 0;
    }

    int getSize() const { return size; }

    bool isEmpty() const { return size == 0; }
};
