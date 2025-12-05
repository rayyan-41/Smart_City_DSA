#pragma once
#include <string>
#include <fstream>
#include "customSTL.h"
#include "Bus.h"

using std::string;
using std::ifstream;

class CityGraph; // Forward declaration

class BusManager {
private:
    CityGraph* cityGraph; // Pointer to city graph for route calculation

public:
    Vector<Bus*> buses;
    
    // Hash Table for O(1) Lookup by Bus Number
    HashTable<string, Bus*> busLookup;
    
    // Hash Table for buses by company
    HashTable<string, Vector<Bus*>> companyLookup;

    BusManager();
    ~BusManager();
    
    BusManager(const BusManager& other) = delete;
    BusManager& operator=(const BusManager& other) = delete;
    
    // Set city graph reference
    void setCityGraph(CityGraph* graph);
    
    // ------- Bus Management API -------
    Bus* createBus(const string& busNo, const string& company, const string& currentStop);
    void addBus(Bus* bus);
    
    // Register bus route using Dijkstra
    void registerBusRoute(const string& busNo, const string& startStopDB, const string& endStopDB);
    
    // ------- Lookup API -------
    Bus* findBusByNumber(const string& busNo) const;
    Vector<Bus*> findBusesByCompany(const string& company) const;
    Vector<Bus*> findBusesAtStop(const string& stopID) const;
    
    int getBusCount() const { return buses.getSize(); }
    Bus* getBus(int index) const { 
        if (index >= 0 && index < buses.getSize()) return buses[index];
        return nullptr;
    }
    
    // ------- CSV Loading -------
    bool loadFromCSV(const string& filename, bool hasHeader = true);
    
private:
    string trim(const string& s) const;
    Vector<string> parseRoute(const string& routeStr) const;
};

// ================= IMPLEMENTATION =================

BusManager::BusManager() : cityGraph(nullptr), buses(), busLookup(101), companyLookup(53) {}

BusManager::~BusManager() {
    for (int i = 0; i < buses.getSize(); i++) {
        delete buses[i];
    }
}

void BusManager::setCityGraph(CityGraph* graph) {
    cityGraph = graph;
}

Bus* BusManager::createBus(const string& busNo, const string& company, const string& currentStop) {
    Bus* b = new Bus(busNo, company, currentStop);
    buses.push_back(b);
    busLookup.insert(busNo, b);
    
    // Add to company lookup
    Vector<Bus*>* existingList = companyLookup.get(company);
    if (existingList != nullptr) {
        existingList->push_back(b);
    } else {
        Vector<Bus*> newList;
        newList.push_back(b);
        companyLookup.insert(company, newList);
    }
    
    return b;
}

void BusManager::addBus(Bus* bus) {
    if (bus) {
        buses.push_back(bus);
        busLookup.insert(bus->busNo, bus);
        
        // Add to company lookup
        Vector<Bus*>* existingList = companyLookup.get(bus->company);
        if (existingList != nullptr) {
            existingList->push_back(bus);
        } else {
            Vector<Bus*> newList;
            newList.push_back(bus);
            companyLookup.insert(bus->company, newList);
        }
    }
}

void BusManager::registerBusRoute(const string& busNo, const string& startStopDB, const string& endStopDB) {
    // Implementation will be in SmartCity.h where we have access to CityGraph methods
    // This is just a placeholder
}

Bus* BusManager::findBusByNumber(const string& busNo) const {
    Bus** result = busLookup.get(busNo);
    if (result != nullptr) {
        return *result;
    }
    return nullptr;
}

Vector<Bus*> BusManager::findBusesByCompany(const string& company) const {
    Vector<Bus*>* result = companyLookup.get(company);
    if (result != nullptr) {
        return *result;
    }
    return Vector<Bus*>();
}

Vector<Bus*> BusManager::findBusesAtStop(const string& stopID) const {
    Vector<Bus*> result;
    for (int i = 0; i < buses.getSize(); i++) {
        if (buses[i]->currentStop == stopID) {
            result.push_back(buses[i]);
        }
    }
    return result;
}

bool BusManager::loadFromCSV(const string& filename, bool hasHeader) {
    ifstream file(filename);
    if (!file.is_open()) return false;
    
    string line;
    if (hasHeader) std::getline(file, line);
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        string fields[4];
        int idx = 0;
        string cur = "";
        
        for (int i = 0; i < (int)line.size(); i++) {
            char c = line[i];
            if (c == ',' && idx < 3) {
                fields[idx++] = trim(cur);
                cur.clear();
            } else {
                cur += c;
            }
        }
        fields[idx] = trim(cur);
        
        string busNo = trim(fields[0]);
        string company = trim(fields[1]);
        string currentStop = trim(fields[2]);
        string routeStr = trim(fields[3]);
        
        // Create bus
        Bus* bus = createBus(busNo, company, currentStop);
        
        // Parse route stops (will calculate actual path later)
        Vector<string> routeStops = parseRoute(routeStr);
        
        // Store start and end stops for later route calculation
        if (routeStops.getSize() >= 2) {
            bus->setStops(routeStops[0], routeStops[routeStops.getSize() - 1]);
        }
    }
    
    file.close();
    return true;
}

string BusManager::trim(const string& s) const {
    int start = 0, end = (int)s.size() - 1;
    
    while (start <= end &&
           (s[start] == ' ' || s[start] == '\t' ||
            s[start] == '\r' || s[start] == '"'))
        start++;
    
    while (end >= start &&
           (s[end] == ' ' || s[end] == '\t' ||
            s[end] == '\r' || s[end] == '"'))
        end--;
    
    return (start > end) ? "" : s.substr(start, end - start + 1);
}

Vector<string> BusManager::parseRoute(const string& routeStr) const {
    Vector<string> stops;
    string current = "";
    
    for (int i = 0; i < (int)routeStr.size(); i++) {
        if (routeStr[i] == '>') {
            string stop = trim(current);
            if (!stop.empty()) {
                stops.push_back(stop);
            }
            current = "";
        } else {
            current += routeStr[i];
        }
    }
    
    // Add last stop
    string stop = trim(current);
    if (!stop.empty()) {
        stops.push_back(stop);
    }
    
    return stops;
}
