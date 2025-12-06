#pragma once
#include <string>
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

    // ==================== GETTERS ====================
    string getId() const { return id; }
    string getName() const { return name; }
    string getCategory() const { return category; }
    int getProductCount() const { return inventory.getSize(); }
    const Vector<Product>& getInventory() const { return inventory; }
    
    // Get product by index
    const Product* getProduct(int index) const {
        if (index >= 0 && index < inventory.getSize()) return &inventory[index];
        return nullptr;
    }
    
    // Get product by name
    const Product* getProductByName(const string& productName) const {
        for (int i = 0; i < inventory.getSize(); i++) {
            if (inventory[i].name == productName) return &inventory[i];
        }
        return nullptr;
    }
    
    // Get total inventory value
    double getTotalInventoryValue() const {
        double total = 0.0;
        for (int i = 0; i < inventory.getSize(); i++) {
            total += inventory[i].price;
        }
        return total;
    }

    // ==================== SETTERS ====================
    void setId(const string& newId) { id = newId; }
    void setName(const string& newName) { name = newName; }
    void setCategory(const string& newCategory) { category = newCategory; }

    // ==================== PRODUCT OPERATIONS ====================
    void addProduct(const Product& p);
    bool hasProduct(const string& productName);
    bool removeProduct(const string& productName);
};

// Implementation
void Shop::addProduct(const Product& p) {
    inventory.push_back(p);
}

    for (int i = 0; i < inventory.getSize(); i++) {
        if (inventory[i].name == productName) {
            inventory.erase(i);
            return true;
        }
    }
    return false;
}