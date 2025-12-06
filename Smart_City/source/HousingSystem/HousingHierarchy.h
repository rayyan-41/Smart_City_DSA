#pragma once
#include <string>
#include "customSTL.h"
#include "Citizen.h"

using std::string;
// ==========================================
// LEVEL 3: HOUSE (Leaf Container)
// ==========================================
class House {
public:
    int houseNumber;
    Vector<Citizen*> residents; // The leaves of the tree

    House(int num) : houseNumber(num) {}

    // We do NOT delete Citizens here. 
    // PopulationManager owns the Citizen memory (Master List).
    ~House() {}

    void addResident(Citizen* c) {
        residents.push_back(c);
        // Link the citizen back to this house logic if needed
        c->houseNo = houseNumber;
    }

    int getPopulation() const {
        return residents.getSize();
    }
};

// ==========================================
// LEVEL 2: STREET (Intermediate Node)
// ==========================================
class Street {
public:
    int streetNumber;
    Vector<House*> houses; // Children

    Street(int num) : streetNumber(num) {}

    ~Street() {
        for (int i = 0; i < houses.getSize(); i++) delete houses[i];
    }

    House* findOrCreateHouse(int houseNo) {
        // Linear search is efficient here (Street rarely has >50 houses)
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

// ==========================================
// LEVEL 1: SECTOR (Top Level Node)
// ==========================================
class Sector {
public:
    string name; // e.g., "G-10"

    // GRAPH INTEGRATION:
    // This ID maps to a Node in your CityMap (Adjacency List).
    // Example: "G-10" -> Node 5 (which is G-10 Markaz Bus Stop)
    string graphNodeID;

    Vector<Street*> streets; // Children

    Sector(string n) : name(n), graphNodeID("") {}

    ~Sector() {
        for (int i = 0; i < streets.getSize(); i++) delete streets[i];
    }

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

    // Helper to link to Graph after loading
    void setGraphNode(const string& nodeID) {
        graphNodeID = nodeID;
    }
};