#pragma once
#include <stdexcept>


template <typename T>
class Vector {
    T* data;
    int _size;
    int capacity;

public:
    Vector(int s = 0) : data(nullptr), _size(0), capacity(s) {
        if (s < 0)
            throw std::invalid_argument("Invalid size");

        if (capacity > 0)
            data = new T[capacity];
    }

    Vector(const Vector& other)
        : data(nullptr), _size(other._size), capacity(other.capacity) {
        if (capacity > 0) {
            data = new T[capacity];
            for (int i = 0; i < _size; i++)
                data[i] = other.data[i];
        }
	}

    Vector& operator=(const Vector& other) {
        if (this == &other)
            return *this;
        delete[] data;
        _size = other._size;
        capacity = other.capacity;
        data = nullptr;
        if (capacity > 0) {
            data = new T[capacity];
            for (int i = 0; i < _size; i++)
                data[i] = other.data[i];
        }
        return *this;
	}

    ~Vector() {
        delete[] data;
    }

    void push_back(const T& obj) {
        if (_size == capacity) {
            int newCap = (capacity == 0) ? 1 : capacity * 2;
            reallocate(newCap);
        }
        data[_size++] = obj;
    }

    void push_front(const T& obj) {
        if (_size == capacity) {
            int newCap = (capacity == 0) ? 1 : capacity * 2;
            reallocate(newCap);
        }
        for (int i = _size; i > 0; i--)
            data[i] = data[i - 1];
        data[0] = obj;
        _size++;
	}

    T& at(int index) {
        if (index < 0 || index >= _size)
            throw std::out_of_range("Index out of range");
        return data[index];
    }

    const T& at(int index) const {
        if (index < 0 || index >= _size)
            throw std::out_of_range("Index out of range");
        return data[index];
    }

    T& operator[](int index) {
        if (index < 0 || index >= _size)
            throw std::out_of_range("Index out of range");
        return data[index];
    }

    const T& operator[](int index) const {
        if (index < 0 || index >= _size)
            throw std::out_of_range("Index out of range");
        return data[index];
    }

    T& front() {
        if (_size == 0)
            throw std::out_of_range("Vector is empty");
        return data[0];
    }

    const T& front() const {
        if (_size == 0)
            throw std::out_of_range("Vector is empty");
        return data[0];
    }

    T& back() {
        if (_size == 0)
            throw std::out_of_range("Vector is empty");
        return data[_size - 1];
    }

    const T& back() const {
        if (_size == 0)
            throw std::out_of_range("Vector is empty");
        return data[_size - 1];
    }

    void pop_back() {
        if (_size == 0)
            return;
        _size--;
        shrinkCheck();
    }

    void pop_front() {
        if (_size == 0)
            return;
        for (int i = 0; i < _size - 1; i++)
            data[i] = data[i + 1];
        _size--;
        shrinkCheck();
	}

    void reserve(int newCap) {
        if (newCap > capacity)
            reallocate(newCap);
    }

    void resize(int newSize, const T& defVal = T()) {
        if (newSize < 0)
            throw std::invalid_argument("Invalid size");
        if (newSize < _size) {
            _size = newSize;
            shrinkCheck();  
        }
        else if (newSize > _size) { 
            if (newSize > capacity)
                reallocate(newSize);
            for (int i = _size; i < newSize; i++)
                data[i] = defVal;
            _size = newSize;
        }
    }
    bool empty() const { return _size == 0; }

    void clear() {
        _size = 0;
        shrinkCheck();
    }

	// Begin and end for range-based for loops
	T* begin() { return data; }
	T* end() { return data + _size; }

    void swap(Vector& other) {
        T* tempData = data;
        int tempSize = _size;
        int tempCapacity = capacity;
        data = other.data;
        _size = other._size;
        capacity = other.capacity;
        other.data = tempData;
    }

    int find(const T& value) const {
        for (int i = 0; i < _size; ++i) {
            if (data[i] == value)
                return i;
        }
        return -1;
    }

    bool contains(const T& value) const {
        return find(value) != -1;
    }

    void remove(const T& value) {
        int idx = find(value);
        if (idx == -1) return;
        for (int i = idx; i < _size - 1; ++i) {
            data[i] = data[i + 1];
        }
        --_size;
        shrinkCheck();
    }

    void erase(int index) {
        if (index < 0 || index >= _size)
            throw std::out_of_range("Index out of range");
        for (int i = index; i < _size - 1; i++)
            data[i] = data[i + 1];
        _size--;
        shrinkCheck();
	}

    int getSize() const { return _size; }
    int getCapacity() const { return capacity; }

private:
    void reallocate(int newCap) {
        T* newData = new T[newCap];
        for (int i = 0; i < _size; i++)
            newData[i] = data[i];
        delete[] data;
        data = newData;
        capacity = newCap;
	}

    void shrinkCheck() {
        if (capacity > 4 && _size <= capacity / 3) {
            int newCap = capacity / 2;
            if (newCap < 1) newCap = 1;
            reallocate(newCap);
        }
    }
};

