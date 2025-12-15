#pragma once
#include <string>
#include "../../data_structures/CustomSTL.h"
#include "../../utils/Location.h"
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

    // ==================== GETTERS ====================
    string getId() const { return id; }
    string getName() const { return name; }
    string getSector() const { return location.sector; }
    double getLatitude() const { return location.coord.x; }
    double getLongitude() const { return location.coord.y; }
    const Location& getLocation() const { return location; }
    int getShopCount() const { return shops.getSize(); }
    const Vector<Shop*>& getShops() const { return shops; }
    
    int getTotalProductCount() const {
        int total = 0;
        for (int i = 0; i < shops.getSize(); i++) {
            total += shops[i]->getProductCount();
        }
        return total;
    }
    
    Vector<string> getCategories() const {
        Vector<string> categories;
        for (int i = 0; i < shops.getSize(); i++) {
            string cat = shops[i]->getCategory();
            bool found = false;
            for (int j = 0; j < categories.getSize(); j++) {
                if (categories[j] == cat) { found = true; break; }
            }
            if (!found) categories.push_back(cat);
        }
        return categories;
    }

    // ==================== SETTERS ====================
    void setId(const string& newId) { id = newId; }
    void setName(const string& newName) { name = newName; }
    void setSector(const string& sector) { location.sector = sector; }
    void setCoordinates(double lat, double lon) { location.coord.x = lat; location.coord.y = lon; }
    void setLocation(const Location& loc) { location = loc; }

    // ==================== OPERATIONS ====================
    void addShop(Shop* s);
    bool removeShop(const string& shopID);
    Shop* findShop(const string& shopName);
    Shop* findShopByID(const string& shopID);
    Shop* getShop(int index) const {
        if (index >= 0 && index < shops.getSize()) return shops[index];
        return nullptr;
    }
};

// ==================== Implemenation ====================

inline void Mall::addShop(Shop* s) {
    shops.push_back(s);
}

bool Mall::removeShop(const string& shopID) {
    for (int i = 0; i < shops.getSize(); i++) {
        if (shops[i]->getId() == shopID) {
            
            Shop* temp = shops[i];
            shops.erase(i);
            delete temp; 
            return true;
        }
    }
    return false;
}

inline Shop* Mall::findShop(const string& shopName) {
    for (int i = 0; i < shops.getSize(); i++) {
        if (shops[i]->name == shopName) {
            return shops[i];
        }
    }
    return nullptr;
}

inline Shop* Mall::findShopByID(const string& shopID) {
    for (int i = 0; i < shops.getSize(); i++) {
        if (shops[i]->id == shopID) {
            return shops[i];
        }
    }
    return nullptr;
}