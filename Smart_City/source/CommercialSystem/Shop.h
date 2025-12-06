#pragma once
#include <string>
#include "CustomSTL.h" 
#include "Product.h"

using std::string;

class Shop {
public:
    string id;
    string name;
    string category;
    Vector<Product> inventory;

    Shop() : id(""), name(""), category(""), inventory() {}

    Shop(string id, string name, string category)
        : id(id), name(name), category(category), inventory() {
    }

    Shop(const Shop& other): id(other.id), name(other.name), category(other.category), inventory(other.inventory) {}

    Shop& operator=(const Shop& other) {
        if (this != &other) {
            id = other.id;
            name = other.name;
            category = other.category;
            inventory = other.inventory;
        }
        return *this;
    }

    ~Shop() {}

    void addProduct(const Product& p);
    bool hasProduct(const string& productName);
};
// Implementation
void Shop::addProduct(const Product& p) {
    inventory.push_back(p);
}

bool Shop::hasProduct(const string& productName) {
    for (int i = 0; i < inventory.getSize(); i++) {
        if (inventory[i].name == productName) {
            return true;
        }
    }
    return false;
}