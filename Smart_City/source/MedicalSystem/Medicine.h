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

    // ==================== GETTERS ====================
    string getName() const { return name; }
    string getFormula() const { return formula; }
    float getPrice() const { return price; }

    // ==================== SETTERS ====================
    void setName(const string& n) { name = n; }
    void setFormula(const string& f) { formula = f; }
    void setPrice(float p) { price = p; }

    bool operator==(const Medicine& other) const {
        return name == other.name && formula == other.formula;
    }
};