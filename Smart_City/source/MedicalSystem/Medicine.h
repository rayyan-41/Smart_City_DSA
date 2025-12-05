#pragma once
#include <string>
using std::string;

struct Medicine {
    string name;
    string formula;
    float price;

    Medicine() : name(""), formula(""), price(0.0f) {}

    Medicine(string name, string formula, float price)
        : name(name), formula(formula), price(price) {
    }

    // Equality check for search/hashing collisions
    bool operator==(const Medicine& other) const {
        return name == other.name && formula == other.formula;
    }
};