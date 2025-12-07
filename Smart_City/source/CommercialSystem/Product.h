#pragma once
#include <string>

using std::string;

struct Product {
    string name;
    string category;
    int price;

    Product() : name(""), category(""), price(0) {}

    Product(string n, string c, int p)
        : name(n), category(c), price(p) {
    }

    
    // ==================== GETTERS ====================
    string getName() const { return name; }
    string getCategory() const { return category; }
    int getPrice() const { return price; }

    // ==================== SETTERS ====================
    void setName(const string& n) { name = n; }
    void setCategory(const string& c) { category = c; }
    void setPrice(int p) { price = p; }

    bool operator==(const Product& other) const {
        return name == other.name && category == other.category;
    }
};