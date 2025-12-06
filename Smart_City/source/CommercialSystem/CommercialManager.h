#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include "CustomSTL.h"
#include "ModuleUtils.h" 
#include "Mall.h"
#include "Shop.h"
#include "Product.h"

using std::string;
using std::ifstream;

class CommercialManager {
public:
    Vector<Mall*> malls;
    HashTable<string, Mall*> mallLookup;
    HashTable<string, Vector<Shop*>> productLookup;
    HashTable<string, Vector<Shop*>> categoryLookup;

    CommercialManager() : malls(), mallLookup(), productLookup() {}

    ~CommercialManager() {
        for (int i = 0; i < malls.getSize(); i++) {
            delete malls[i];
        }
    }

    string trim(const string& s) const;
    int parseInt(const string& s);
    void loadMalls(const string& filename);
    void loadShops(const string& filename);
    Vector<Shop*> findShopsSellingProduct(const string& productName);
    Vector<Shop*> findShopsByCategory(const string& category);
  
};

// Implementation


string CommercialManager::trim(const string& s) const {
    int start = 0, end = (int)s.size() - 1;
    while (start <= end && (s[start] == ' ' || s[start] == '\t' || s[start] == '\"' || s[start] == '\r')) start++;
    while (end >= start && (s[end] == ' ' || s[end] == '\t' || s[end] == '\"' || s[end] == '\r')) end--;
    if (start > end) return "";
    return s.substr(start, end - start + 1);
}

int CommercialManager::parseInt(const string& s) {
    if (s.empty()) return -1;

    int res = 0;
    for (int i = 0; i < s.length(); i++) {
        if (s[i] >= '0' && s[i] <= '9') {
            res = res * 10 + (s[i] - '0');
        }
        else {
            return -1;
        }
    }
    return res;
}

Vector<Shop*> CommercialManager::findShopsSellingProduct(const string& productName) {
        Vector<Shop*>* result = productLookup.get(productName);
        if (result != nullptr) {
            return *result;
        }
        return Vector<Shop*>();
    }

Vector<Shop*> CommercialManager::findShopsByCategory(const string& category) {
    Vector<Shop*>* result = categoryLookup.get(category);
    if (result != nullptr) return *result;
    return Vector<Shop*>();
}

void CommercialManager::loadMalls(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) return;

    string line;
    std::getline(file, line);

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        string fields[3];
        int idx = 0;
        string cur = "";

        for (size_t i = 0; i < line.size(); i++) {
            char c = line[i];
            if (c == ',') {
                if (idx < 2) {
                    fields[idx++] = trim(cur);
                    cur.clear();
                }
            }
            else {
                cur += c;
            }
        }
        fields[idx] = trim(cur);

        if (idx < 2) continue;

        string id = fields[0];
        string name = fields[1];
        string sector = fields[2];

        if (id.empty() || name.empty()) continue;
        if (mallLookup.contains(id)) continue;

        Mall* newMall = new Mall(id, name, sector);
        malls.push_back(newMall);
        mallLookup.insert(id, newMall);
    }
    file.close();
}

void CommercialManager:: loadShops(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) return;

    string line;
    std::getline(file, line);

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        string fields[6];
        int idx = 0;
        string cur = "";

        for (size_t i = 0; i < line.size(); i++) {
            char c = line[i];
            if (c == ',') {
                if (idx < 5) {
                    fields[idx++] = trim(cur);
                    cur.clear();
                }
            }
            else {
                cur += c;
            }
        }
        fields[idx] = trim(cur);

        if (idx < 5) continue;

        string shopID = fields[0];
        string mallID = fields[1];
        string shopName = fields[2];
        string category = fields[3];
        string prodName = fields[4];
        string priceStr = fields[5];

        if (shopID.empty() || mallID.empty()) continue;

        int price = parseInt(priceStr);
        if (price == -1) {

            continue;
        }

        // 3. Validation: Parent Mall Existence
        Mall** mallPtr = mallLookup.get(mallID);
        if (mallPtr == nullptr) {
            continue;
        }
        Mall* mall = *mallPtr;

        // 4. Find or Create Shop
        Shop* shop = mall->findShopByID(shopID);
        if (shop == nullptr) {
            shop = new Shop(shopID, shopName, category);
            mall->addShop(shop);
        }

        // 5. Add Product
        Product p(prodName, category, price);
        shop->addProduct(p);

        // 6. Update Product Lookup
        Vector<Shop*>* existingShops = productLookup.get(prodName);
        if (existingShops != nullptr) {
            bool alreadyAdded = false;
            for (int i = 0; i < existingShops->getSize(); i++) {
                if ((*existingShops)[i]->id == shop->id) { alreadyAdded = true; break; }
            }
            if (!alreadyAdded) existingShops->push_back(shop);
        }
        else {
            Vector<Shop*> newShopList;
            newShopList.push_back(shop);
            productLookup.insert(prodName, newShopList);
        }


        if (!category.empty()) {
            Vector<Shop*>* catEntry = categoryLookup.get(category);
            if (catEntry != nullptr) {
                bool exists = false;
                for (int i = 0; i < catEntry->getSize(); i++) {
                    if ((*catEntry)[i]->id == shop->id) { exists = true; break; }
                }
                if (!exists) catEntry->push_back(shop);
            }
            else {
                Vector<Shop*> newList;
                newList.push_back(shop);
                categoryLookup.insert(category, newList);
            }
        }
    }
    file.close();
}