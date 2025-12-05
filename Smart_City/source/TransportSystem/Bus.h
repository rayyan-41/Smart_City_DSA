#pragma once
#include <string>
#include "customSTL.h"

using std::string;

class Bus {
public:
    string busNo;           // Bus number (e.g., B101)
    string company;         // Bus company name
    string currentStop;     // Current stop ID
    Vector<int> route;      // Vector of node IDs representing the route
    double totalDistance;   // Total route distance in km
    
    // Route stop information
    string startStopID;     // Starting stop database ID
    string endStopID;       // Ending stop database ID
    
    Bus();
    Bus(const string& busNo, const string& company, const string& currentStop);
    Bus(const Bus& other);
    Bus& operator=(const Bus& other);
    ~Bus();
    
    void setRoute(const Vector<int>& newRoute, double distance);
    void setStops(const string& start, const string& end);
    int getStopCount() const;
    bool isOnRoute(int nodeID) const;
};

// ---------------- IMPLEMENTATION ----------------

Bus::Bus()
    : busNo(""),
      company(""),
      currentStop(""),
      route(),
      totalDistance(0.0),
      startStopID(""),
      endStopID("") {
}

Bus::Bus(const string& busNo, const string& company, const string& currentStop)
    : busNo(busNo),
      company(company),
      currentStop(currentStop),
      route(),
      totalDistance(0.0),
      startStopID(""),
      endStopID("") {
}

Bus::Bus(const Bus& other)
    : busNo(other.busNo),
      company(other.company),
      currentStop(other.currentStop),
      route(other.route),
      totalDistance(other.totalDistance),
      startStopID(other.startStopID),
      endStopID(other.endStopID) {
}

Bus& Bus::operator=(const Bus& other) {
    if (this != &other) {
        busNo = other.busNo;
        company = other.company;
        currentStop = other.currentStop;
        route = other.route;
        totalDistance = other.totalDistance;
        startStopID = other.startStopID;
        endStopID = other.endStopID;
    }
    return *this;
}

Bus::~Bus() {
    // Vector handles its own cleanup
}

void Bus::setRoute(const Vector<int>& newRoute, double distance) {
    route = newRoute;
    totalDistance = distance;
}

void Bus::setStops(const string& start, const string& end) {
    startStopID = start;
    endStopID = end;
}

int Bus::getStopCount() const {
    return route.getSize();
}

bool Bus::isOnRoute(int nodeID) const {
    for (int i = 0; i < route.getSize(); i++) {
        if (route[i] == nodeID) {
            return true;
        }
    }
    return false;
}
