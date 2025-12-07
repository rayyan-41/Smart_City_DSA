#pragma once
#include <stdexcept>
#include "Vector.h"

namespace re {

    template <typename T>
    class BST {
    private:
        struct Node {
            T data;
            Node* left;
            Node* right;

            Node(const T& value)
                : data(value), left(nullptr), right(nullptr) {}
        };

        Node* root;
        int nodeCount;

        Node* copyTree(Node* other) {
            if (!other) return nullptr;
            Node* node = new Node(other->data);
            node->left  = copyTree(other->left);
            node->right = copyTree(other->right);
            return node;
        }

        void destroySubtree(Node* node) {
            if (!node) return;
            destroySubtree(node->left);
            destroySubtree(node->right);
            delete node;
        }

        Node* insertNode(Node* node, const T& value) {
            if (!node) {
                nodeCount++;
                return new Node(value);
            }
            if (value < node->data) {
                node->left = insertNode(node->left, value);
            } else if (value > node->data) {
                node->right = insertNode(node->right, value);
            } else {
            }
            return node;
        }

        Node* findMin(Node* node) const {
            if (!node) return nullptr;
            while (node->left) node = node->left;
            return node;
        }

        Node* removeNode(Node* node, const T& value, bool& removed) {
            if (!node) return nullptr;

            if (value < node->data) {
                node->left = removeNode(node->left, value, removed);
            } else if (value > node->data) {
                node->right = removeNode(node->right, value, removed);
            } else {
                removed = true;
                if (!node->left && !node->right) {
                    delete node;
                    return nullptr;
                } else if (!node->left) {
                    Node* temp = node->right;
                    delete node;
                    return temp;
                } else if (!node->right) {
                    Node* temp = node->left;
                    delete node;
                    return temp;
                } else {
                    Node* succ = findMin(node->right);
                    node->data = succ->data;
                    bool dummy = false;
                    node->right = removeNode(node->right, succ->data, dummy);
                }
            }
            return node;
        }

        int height(Node* node) const {
            if (!node) return 0;
            int hl = height(node->left);
            int hr = height(node->right);
            return (hl > hr ? hl : hr) + 1;
        }

        bool contains(Node* node, const T& value) const {
            while (node) {
                if (value < node->data) node = node->left;
                else if (value > node->data) node = node->right;
                else return true;
            }
            return false;
        }

        void getInOrder(Node* node, Vector<T>& result) const {
            if (!node) return;
            getInOrder(node->left, result);
            result.push_back(node->data);
            getInOrder(node->right, result);
        }

        void getPreOrder(Node* node, Vector<T>& result) const {
            if (!node) return;
            result.push_back(node->data);
            getPreOrder(node->left, result);
            getPreOrder(node->right, result);
        }

        void getPostOrder(Node* node, Vector<T>& result) const {
            if (!node) return;
            getPostOrder(node->left, result);
            getPostOrder(node->right, result);
            result.push_back(node->data);
        }

        void getLevelOrder(Node* node, Vector<T>& result) const {
            if (!node) return;
            Vector<Node*> queue;
            queue.push_back(node);
            int idx = 0;
            while (idx < queue.getSize()) {
                Node* current = queue[idx++];
                result.push_back(current->data);
                if (current->left) queue.push_back(current->left);
                if (current->right) queue.push_back(current->right);
            }
        }

    public:
        BST() : root(nullptr), nodeCount(0) {}

        BST(const BST& other) : root(nullptr), nodeCount(0) {
            if (other.root) {
                root = copyTree(other.root);
                nodeCount = other.nodeCount;
            }
        }

        BST& operator=(const BST& other) {
            if (this == &other) return *this;
            clear();
            if (other.root) {
                root = copyTree(other.root);
                nodeCount = other.nodeCount;
            }
            return *this;
        }

        ~BST() {
            clear();
        }

        bool empty() const { return nodeCount == 0; }
        int size()  const { return nodeCount; }

        void insert(const T& value) {
            root = insertNode(root, value);
        }

        void remove(const T& value) {
            bool removed = false;
            root = removeNode(root, value, removed);
            if (!removed) {
                throw std::out_of_range("Value not found");
            }
            nodeCount--;
        }

        bool contains(const T& value) const {
            return contains(root, value);
        }

        int height() const {
            return height(root);
        }

        void clear() {
            destroySubtree(root);
            root = nullptr;
            nodeCount = 0;
        }

        Vector<T> getInOrder() const {
            Vector<T> result;
            getInOrder(root, result);
            return result;
        }

        Vector<T> getPreOrder() const {
            Vector<T> result;
            getPreOrder(root, result);
            return result;
        }

        Vector<T> getPostOrder() const {
            Vector<T> result;
            getPostOrder(root, result);
            return result;
        }

        Vector<T> getLevelOrder() const {
            Vector<T> result;
            getLevelOrder(root, result);
            return result;
        }
    };

}

