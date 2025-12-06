#pragma once
#include <string>
#include "CustomSTL.h"
#include "Location.h"
#include "Shop.h"

using std::string;

class Mall {
public:
    string id;
    string name;
    Location location;
    Vector<Shop*> shops; 

    Mall() : id(""), name(""), location(), shops() {}

    Mall(string id, string name, string sector)
        : id(id), name(name), location(sector, 0.0, 0.0), shops() {
    }

    ~Mall() {
        for (int i = 0; i < shops.getSize(); i++) {
            delete shops[i];
        }
    }

    void addShop(Shop* s);
    Shop* findShop(const string& shopName);
    Shop* findShopByID(const string& shopID);
};
// Implementation

void Mall::addShop(Shop* s) {
    shops.push_back(s);
}

Shop* Mall::findShop(const string& shopName) {
    for (int i = 0; i < shops.getSize(); i++) {
        if (shops[i]->name == shopName) {
            return shops[i];
        }
    }
    return nullptr;
}

Shop* Mall::findShopByID(const string& shopID) {
    for (int i = 0; i < shops.getSize(); i++) {
        if (shops[i]->id == shopID) {
            return shops[i];
        }
    }
    return nullptr;
}