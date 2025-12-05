#pragma once
#include <string>
#include "customSTL.h" 
#include "ModuleUtils.h"
#include "Medicine.h"

using std::string;

class Pharmacy {
public:
    string id;
    string name;
    string sector;

    // Inventory of medicines available at this specific branch
    Vector<Medicine> inventory;

    Location location;
    string graphNodeID; // For graph integration later

    Pharmacy() : id(""), name(""), sector(""), location(), graphNodeID("") {}

    Pharmacy(string id, string name, string sector, string graphNodeID = "", double x = 0.0, double y = 0.0)
        : id(id), name(name), sector(sector), location(sector, x, y), graphNodeID(graphNodeID) {
    }

    // Add medicine to inventory
    // This is called repeatedly when parsing rows with the same PharmacyID
    void addMedicine(const Medicine& med) {
        //Check for duplicates
		for (int i = 0; i < inventory.getSize(); i++) {
            if(inventory[i].name == med.name) {
                // Medicine already exists, update price and formula
                inventory[i].price = med.price;
                inventory[i].formula = med.formula;
                return;
            }
        }
        inventory.push_back(med);
    } 

    bool hasMedicine(const string& medName) const {
        for (int i = 0; i < inventory.getSize(); i++) {
            if (inventory[i].name == medName) return true;
        }
        return false;
    }

    // Helper to get price if medicine exists
    float getPrice(const string& medName) const {
        for (int i = 0; i < inventory.getSize(); i++) {
            if (inventory[i].name == medName) return inventory[i].price;
        }
        return -1.0f; // Not found
    }
};