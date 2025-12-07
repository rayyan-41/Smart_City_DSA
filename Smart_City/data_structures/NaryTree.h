#pragma once
#include "Vector.h"
#include <stdexcept>

template <typename T>
class NaryTree {
public:
    struct Node {
        T data;
        Vector<Node*> children; 
        Node* parent;

        Node(const T& value, Node* p = nullptr)
            : data(value), children(), parent(p) {
        }
    };

private:
    Node* root;
    int nodeCount;

public:
    NaryTree() : root(nullptr), nodeCount(0) {}

    NaryTree(const NaryTree& other) : root(nullptr), nodeCount(0) {
        if (other.root) {
            root = copySubtree(other.root, nullptr);
            nodeCount = other.nodeCount;
        }
    }

    NaryTree& operator=(const NaryTree& other) {
        if (this == &other)
            return *this;

        clear();

        if (other.root) {
            root = copySubtree(other.root, nullptr);
            nodeCount = other.nodeCount;
        }

        return *this;
    }

    ~NaryTree() {
        clear();
    }

    bool empty() const { return root == nullptr; }
    int size() const { return nodeCount; }

    Node* getRoot() { return root; }
    const Node* getRoot() const { return root; }

    Node* setRoot(const T& value) {
        if (root != nullptr)
            throw std::logic_error("Root already exists");
        root = new Node(value, nullptr);
        nodeCount = 1;
        return root;
    }

    Node* addChild(Node* parent, const T& value) {
        if (parent == nullptr)
            throw std::invalid_argument("Parent cannot be null");

        Node* child = new Node(value, parent);
        parent->children.push_back(child);
        ++nodeCount;
        return child;
    }

    Node* getChild(Node* parent, int index) {
        if (parent == nullptr)
            throw std::invalid_argument("Parent cannot be null");
        int n = parent->children.getSize();
        if (index < 0 || index >= n)
            throw std::out_of_range("Child index out of range");
        return parent->children[index];
    }

    const Node* getChild(const Node* parent, int index) const {
        if (parent == nullptr)
            throw std::invalid_argument("Parent cannot be null");
        int n = parent->children.getSize();
        if (index < 0 || index >= n)
            throw std::out_of_range("Child index out of range");
        return parent->children[index];
    }

    int childCount(const Node* parent) const {
        if (parent == nullptr)
            throw std::invalid_argument("Parent cannot be null");
        return parent->children.getSize();
    }

    void removeSubtree(Node* node) {
        if (node == nullptr)
            return;

        if (node == root) {
            int removed = deleteSubtree(root);
            root = nullptr;
            nodeCount -= removed; 
            if (nodeCount < 0) nodeCount = 0; 
        }
        else {
            Node* parent = node->parent;
            if (parent != nullptr) {
                int n = parent->children.getSize();
                int index = -1;
                for (int i = 0; i < n; ++i) {
                    if (parent->children[i] == node) {
                        index = i;
                        break;
                    }
                }
                if (index != -1) {
                    for (int i = index; i < n - 1; ++i) {
                        parent->children[i] = parent->children[i + 1];
                    }
                    parent->children.pop_back();
                }
            }
            int removed = deleteSubtree(node);
            nodeCount -= removed;
            if (nodeCount < 0) nodeCount = 0;
        }
    }

    void clear() {
        if (root) {
            deleteSubtree(root);
            root = nullptr;
            nodeCount = 0;
        }
    }

private:
    Node* copySubtree(const Node* otherNode, Node* parent) {
        if (!otherNode)
            return nullptr;

        Node* newNode = new Node(otherNode->data, parent);

        int n = otherNode->children.getSize();
        for (int i = 0; i < n; ++i) {
            Node* childCopy = copySubtree(otherNode->children[i], newNode);
            newNode->children.push_back(childCopy);
        }

        return newNode;
    }

    int deleteSubtree(Node* node) {
        if (!node)
            return 0;

        int removed = 1;

        int n = node->children.getSize();
        for (int i = 0; i < n; ++i) {
            removed += deleteSubtree(node->children[i]);
        }

        delete node;
        return removed;
    }
};

