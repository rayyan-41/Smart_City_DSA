
#pragma once
#include <fstream>
#include <string>
#include <iostream>
#include "../../data_structures/CustomSTL.h"
#include "HousingHierarchy.h" 
#include "../../utils/ID_Generator.h"

using std::string;
using std::ifstream;
using std::cout;
using std::endl;

class PopulationManager {
public:
    // N-ary Tree 
    Vector<Sector*> sectors;

    Vector<Citizen*> masterList;
    HashTable<string, Citizen*> cnicLookup;

    PopulationManager();
    ~PopulationManager();

    
    bool loadPopulation(const string& filename);

    Citizen* addCitizen(string cnic, string name, int age,
        string secName, int stNo, int hNo, string job);

    bool removeCitizen(const string& cnic);

    // TREE OPERATIONS
    Sector* findOrCreateSector(const string& name);
	Sector* findSector(const string& name) const;

    // ==================== GETTERS ====================

    Citizen* getCitizen(const string& cnic) const;
	Vector<int> getHierarchyStats() const;
    Vector<int> getSectorStats(const string& sectorName) const;
    Vector<House*> getHousesInSector(const string& sectorName) const;
    Vector<Citizen*> getCitizensInSector(const string& sectorName) const;

private:
    string trim(const string& s) const;
};

// ==================== Implemenation ====================

inline PopulationManager::PopulationManager() : cnicLookup(1000) {}

inline PopulationManager::~PopulationManager() {
    for (int i = 0; i < sectors.getSize(); i++) delete sectors[i];
    for (int i = 0; i < masterList.getSize(); i++) delete masterList[i];
}

inline bool PopulationManager::loadPopulation(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) return false;

    string line;
    std::getline(file, line); 

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        string fields[7];
        int idx = 0;
        string cur = "";

        for (int i = 0; i < (int)line.length(); i++) {
            char c = line[i];
            if (c == ',' && idx < 6) {
                fields[idx++] = trim(cur);
                cur.clear();
            }
            else cur += c;
        }
        fields[idx] = trim(cur);

        string cnic = fields[0];
        string name = fields[1];
        int age = 0;
        string secName = fields[3];
        int stNo = 0; int hNo = 0;
        string job = fields[6];

        if (!fields[2].empty()) age = std::stoi(fields[2]);
        if (!fields[4].empty()) stNo = std::stoi(fields[4]);
        if (!fields[5].empty()) hNo = std::stoi(fields[5]);

        addCitizen(cnic, name, age, secName, stNo, hNo, job);
    }
    file.close();
    return true;
}

inline Citizen* PopulationManager::addCitizen(string cnic, string name, int age,
    string secName, int stNo, int hNo, string job) {

    if (cnic.empty()) cnic = IDGenerator::generateCNIC();

    Citizen* c = new Citizen(cnic, name, age, secName, stNo, hNo);
    masterList.push_back(c);
    cnicLookup.insert(cnic, c);

    Sector* sec = findOrCreateSector(secName);
    Street* st = sec->findOrCreateStreet(stNo);
    House* house = st->findOrCreateHouse(hNo);

    house->addResident(c);
    return c;
}


inline bool PopulationManager::removeCitizen(const string& cnic) {
    Citizen* c = getCitizen(cnic);
    if (!c) return false; 
  
    Sector* sec = findSector(c->sector);
	if (!sec) return false;
    Street* st = sec->findStreet(c->street);
	if (!st) return false; 
    House* house = st->findHouse(c->houseNo);
	if (!house) return false; 
    

    house->residents.remove(c);

    // Remove Hash
    cnicLookup.remove(cnic);

    masterList.remove(c);

    delete c;

    return true;
}

// ==================== tree ====================
inline Sector* PopulationManager::findOrCreateSector(const string& name) {
    for (int i = 0; i < sectors.getSize(); i++) {
        if (sectors[i]->name == name) return sectors[i];
    }
    Sector* s = new Sector(name);
    s->setGraphNode(name);
    sectors.push_back(s);
    return s;
}

inline Sector* PopulationManager::findSector(const string& name) const {
    for (int i = 0; i < sectors.getSize(); i++) {
        if (sectors[i]->name == name) return sectors[i];
    }
    return nullptr;
}

// ==================== Operations ====================
inline Citizen* PopulationManager::getCitizen(const string& cnic) const {
    Citizen** c = cnicLookup.get(cnic);
    return c ? *c : nullptr;
}

inline Vector<int> PopulationManager::getHierarchyStats() const {
    int sectorCount = sectors.getSize();
    int streetCount = 0;
    int houseCount = 0;
    int citizenCount = masterList.getSize();
    for (int i = 0; i < sectors.getSize(); i++) {
        Sector* sec = sectors[i];
        streetCount += sec->streets.getSize();
        for (int j = 0; j < sec->streets.getSize(); j++) {
            Street* st = sec->streets[j];
            houseCount += st->houses.getSize();
        }
    }
    Vector<int> stats;
    stats.push_back(sectorCount);
    stats.push_back(streetCount);
    stats.push_back(houseCount);
    stats.push_back(citizenCount);
    return stats;
}

inline Vector<int> PopulationManager::getSectorStats(const string& sectorName) const {
    Vector<int> stats;
    int streetCount = 0;
    int houseCount = 0;
    int citizenCount = 0;
    
    Sector* sec = findSector(sectorName);
    if (sec) {
        streetCount = sec->streets.getSize();
        for (int j = 0; j < sec->streets.getSize(); j++) {
            Street* st = sec->streets[j];
            houseCount += st->houses.getSize();
            for (int k = 0; k < st->houses.getSize(); k++) {
                citizenCount += st->houses[k]->residents.getSize();
            }
        }
    }
    
    stats.push_back(streetCount);
    stats.push_back(houseCount);
    stats.push_back(citizenCount);
    return stats;
}

inline Vector<House*> PopulationManager::getHousesInSector(const string& sectorName) const {
    Vector<House*> houses;
    Sector* sec = findSector(sectorName);
    if (sec) {
        for (int j = 0; j < sec->streets.getSize(); j++) {
            Street* st = sec->streets[j];
            for (int k = 0; k < st->houses.getSize(); k++) {
                houses.push_back(st->houses[k]);
            }
        }
    }
    return houses;
}

inline Vector<Citizen*> PopulationManager::getCitizensInSector(const string& sectorName) const {
    Vector<Citizen*> citizens;
    Sector* sec = findSector(sectorName);
    if (sec) {
        for (int j = 0; j < sec->streets.getSize(); j++) {
            Street* st = sec->streets[j];
            for (int k = 0; k < st->houses.getSize(); k++) {
                House* h = st->houses[k];
                for (int m = 0; m < h->residents.getSize(); m++) {
                    citizens.push_back(h->residents[m]);
                }
            }
        }
    }
    return citizens;
}

inline string PopulationManager::trim(const string& s) const {
    int start = 0, end = (int)s.size() - 1;
    while (start <= end && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r' || s[start] == '"')) start++;
    while (end >= start && (s[end] == ' ' || s[end] == '\t' || s[end] == '\r' || s[end] == '"')) end--;
    return (start > end) ? "" : s.substr(start, end - start + 1);
}
