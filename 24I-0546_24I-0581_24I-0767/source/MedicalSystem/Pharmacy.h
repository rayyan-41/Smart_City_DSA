#pragma once
#include <string>
#include "../../data_structures/CustomSTL.h" 
#include "../../utils/ModuleUtils.h"
#include "Medicine.h"

using std::string;

class Pharmacy {
public:
    string id;
    string name;
    string sector;

    Vector<Medicine> inventory;

    Location location;
    string graphNodeID; 

    Pharmacy() : id(""), name(""), sector(""), location(), graphNodeID("") {}

    Pharmacy(string id, string name, string sector, string graphNodeID = "", double x = 0.0, double y = 0.0)
        : id(id), name(name), sector(sector), location(sector, x, y), graphNodeID(graphNodeID) {
    }

    // ==================== GETTERS ====================
    string getId() const { return id; }
    string getName() const { return name; }
    string getSector() const { return sector; }
    string getGraphNodeID() const { return graphNodeID; }
    double getLatitude() const { return location.coord.x; }
    double getLongitude() const { return location.coord.y; }
    const Location& getLocation() const { return location; }
    int getMedicineCount() const { return inventory.getSize(); }
    const Vector<Medicine>& getInventory() const { return inventory; }
    
    const Medicine* getMedicine(int index) const {
        if (index >= 0 && index < inventory.getSize()) return &inventory[index];
        return nullptr;
    }
    
    const Medicine* getMedicineByName(const string& medName) const {
        for (int i = 0; i < inventory.getSize(); i++) {
            if (inventory[i].name == medName) return &inventory[i];
        }
        return nullptr;
    }
    
    double getTotalInventoryValue() const {
        double total = 0.0;
        for (int i = 0; i < inventory.getSize(); i++) {
            total += inventory[i].price;
        }
        return total;
    }
    
    Vector<string> getAvailableFormulas() const {
        Vector<string> formulas;
        for (int i = 0; i < inventory.getSize(); i++) {
            bool found = false;
            for (int j = 0; j < formulas.getSize(); j++) {
                if (formulas[j] == inventory[i].formula) { found = true; break; }
            }
            if (!found) formulas.push_back(inventory[i].formula);
        }
        return formulas;
    }

    // ==================== SETTERS ====================
    void setId(const string& newId) { id = newId; }
    void setName(const string& newName) { name = newName; }
    void setSector(const string& newSector) { sector = newSector; location.sector = newSector; }
    void setGraphNodeID(const string& nodeID) { graphNodeID = nodeID; }
    void setCoordinates(double lat, double lon) { location.coord.x = lat; location.coord.y = lon; }
    void setLocation(const Location& loc) { location = loc; }

    // ==================== Implemenation ====================
    void addMedicine(const Medicine& med) {

        for (int i = 0; i < inventory.getSize(); i++) {
            if(inventory[i].name == med.name) {
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
    
    bool hasMedicineByFormula(const string& formula) const {
        for (int i = 0; i < inventory.getSize(); i++) {
            if (inventory[i].formula == formula) return true;
        }
        return false;
    }

    float getPrice(const string& medName) const {
        for (int i = 0; i < inventory.getSize(); i++) {
            if (inventory[i].name == medName) return inventory[i].price;
        }
        return -1.0f;
    }
    
    bool removeMedicine(const string& medName) {
        for (int i = 0; i < inventory.getSize(); i++) {
            if (inventory[i].name == medName) {
                inventory.erase(i);
                return true;
            }
        }
        return false;
    }
};