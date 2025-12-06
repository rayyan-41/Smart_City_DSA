#pragma once
#include <string>
#include <iostream>

using std::string;

struct Product {
    string name;
    string category;
    int price;

    Product() : name(""), category(""), price(0) {}

    Product(string n, string c, int p)
        : name(n), category(c), price(p) {
    }

    bool operator==(const Product& other) const {
        return name == other.name && category == other.category;
    }
};