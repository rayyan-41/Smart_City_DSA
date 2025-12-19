#pragma once
#include <string>
#include "../../data_structures/CustomSTL.h"
#include "Citizen.h"

using std::string;

\
class House {
public:
    int houseNumber;
    Vector<Citizen*> residents; 

    House(int num) : houseNumber(num) {}
    ~House() {}

    // ==================== GETTERS ====================
    int getHouseNumber() const { return houseNumber; }
    int getPopulation() const { return residents.getSize(); }
    const Vector<Citizen*>& getResidents() const { return residents; }
    
    Citizen* getResident(int index) const {
        if (index >= 0 && index < residents.getSize()) return residents[index];
        return nullptr;
    }
    
    Citizen* findResident(const string& cnic) const {
        for (int i = 0; i < residents.getSize(); i++) {
            if (residents[i]->cnic == cnic) return residents[i];
        }
        return nullptr;
    }
    
    bool isEmpty() const { return residents.getSize() == 0; }

    // ==================== SETTERS ====================
    void setHouseNumber(int num) { houseNumber = num; }

    // ==================== OPERATIONS ====================
    void addResident(Citizen* c) {
        residents.push_back(c);
        c->houseNo = houseNumber;
    }
    
    bool removeResident(const string& cnic) {
        for (int i = 0; i < residents.getSize(); i++) {
            if (residents[i]->cnic == cnic) {
                residents.erase(i);
                return true;
            }
        }
        return false;
    }
};

class Street {
public:
    int streetNumber;
    Vector<House*> houses; 

    Street(int num) : streetNumber(num) {}

    ~Street() {
        for (int i = 0; i < houses.getSize(); i++) delete houses[i];
    }

    // ==================== GETTERS ====================
    int getStreetNumber() const { return streetNumber; }
    int getHouseCount() const { return houses.getSize(); }
    const Vector<House*>& getHouses() const { return houses; }
    
    House* getHouse(int index) const {
        if (index >= 0 && index < houses.getSize()) return houses[index];
        return nullptr;
    }
    
    int getPopulation() const {
        int total = 0;
        for (int i = 0; i < houses.getSize(); i++) {
            total += houses[i]->getPopulation();
        }
        return total;
    }

    // ==================== SETTERS ====================
    void setStreetNumber(int num) { streetNumber = num; }

    // ==================== OPERATIONS ====================
    House* findOrCreateHouse(int houseNo) {
        for (int i = 0; i < houses.getSize(); i++) {
            if (houses[i]->houseNumber == houseNo) return houses[i];
        }
        House* h = new House(houseNo);
        houses.push_back(h);
        return h;
    }

    House* findHouse(int houseNo) const {
        for (int i = 0; i < houses.getSize(); i++) {
            if (houses[i]->houseNumber == houseNo) return houses[i];
        }
        return nullptr;
    }
};

class Sector {
public:
    string name; 
    string graphNodeID;

    Vector<Street*> streets; 

    Sector(string n) : name(n), graphNodeID("") {}

    ~Sector() {
        for (int i = 0; i < streets.getSize(); i++) delete streets[i];
    }

    // ==================== GETTERS ====================
    string getName() const { return name; }
    string getGraphNodeID() const { return graphNodeID; }
    int getStreetCount() const { return streets.getSize(); }
    const Vector<Street*>& getStreets() const { return streets; }
    
    Street* getStreet(int index) const {
        if (index >= 0 && index < streets.getSize()) return streets[index];
        return nullptr;
    }
    
    int getHouseCount() const {
        int total = 0;
        for (int i = 0; i < streets.getSize(); i++) {
            total += streets[i]->getHouseCount();
        }
        return total;
    }
    
    int getPopulation() const {
        int total = 0;
        for (int i = 0; i < streets.getSize(); i++) {
            total += streets[i]->getPopulation();
        }
        return total;
    }

    // ==================== SETTERS ====================
    void setName(const string& n) { name = n; }
    void setGraphNode(const string& nodeID) { graphNodeID = nodeID; }

    // ==================== OPERATIONS ====================
    Street* findOrCreateStreet(int streetNo) {
        for (int i = 0; i < streets.getSize(); i++) {
            if (streets[i]->streetNumber == streetNo) return streets[i];
        }
        Street* s = new Street(streetNo);
        streets.push_back(s);
        return s;
    }
    
    Street* findStreet(int streetNo) const {
        for (int i = 0; i < streets.getSize(); i++) {
            if (streets[i]->streetNumber == streetNo) return streets[i];
        }
        return nullptr;
    }
};