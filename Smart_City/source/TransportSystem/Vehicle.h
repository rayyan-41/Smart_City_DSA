#pragma once
#include <string>
#include "../../data_structures/LinkedLists.h"
#include "../../data_structures/Vector.h"

using std::string;


namespace VehicleStatus {
    const string IDLE = "IDLE";
    const string EN_ROUTE = "EN_ROUTE";
    const string AT_STOP = "AT_STOP";
    const string EMERGENCY = "EMERGENCY";
    const string MAINTENANCE = "MAINTENANCE";
    const string BOARDING = "BOARDING";
    const string RETURNING = "RETURNING";
    const string STUCK_IN_TRAFFIC = "STUCK_IN_TRAFFIC";
    const string PICKING_UP = "PICKING_UP";          // Rickshaw picking up passenger
    const string DROPPING_OFF = "DROPPING_OFF";      // Rickshaw dropping off passenger
}


namespace VehicleType {
    const string BUS = "BUS";
    const string SCHOOL_BUS = "SCHOOL_BUS";
    const string AMBULANCE = "AMBULANCE";
    const string RICKSHAW = "RICKSHAW";              // Feeder vehicle
}


struct RouteNode {
    int graphNodeID;            
    string stopName;           
    string sector;              
    double distanceFromPrev;   
    double cumulativeDistance; 
    bool isScheduledStop;      
    
    RouteNode() 
        : graphNodeID(-1), stopName(""), sector(""), distanceFromPrev(0.0), 
          cumulativeDistance(0.0), isScheduledStop(true) {}
    
    RouteNode(int nodeID, const string& name, const string& sec, double dist = 0.0, bool scheduled = true)
        : graphNodeID(nodeID), stopName(name), sector(sec), distanceFromPrev(dist),
          cumulativeDistance(0.0), isScheduledStop(scheduled) {}
    
    bool operator==(const RouteNode& other) const {
        return graphNodeID == other.graphNodeID;
    }
};


class Vehicle {
protected:
    string vehicleID;           
    string vehicleType;         
    string status;             
    
    LinkedList<RouteNode> route;
    int currentRouteIndex;     
    
    int currentNodeID;         
    string currentStopName;     
    string currentSector;      
    
    string homeSector;          
    int homeNodeID;            
    
    double totalDistance;       
    double distanceTraveled;   
    double speed;             
    
    int maxCapacity;           
    int currentOccupancy;      

    // ==================== SPATIAL AWARENESS (Phase 2) ====================
    int nextNodeID;             // The node the vehicle is trying to reach
    double progressOnEdge;      // 0.0 to 1.0, position on current road segment
    bool isStuck;               // True if road is at capacity, vehicle must wait
    int waitingTicks;           // How long the vehicle has been stuck
    
    // Rendering position (interpolated)
    double renderLat, renderLon;

    // Passenger tracking for rickshaws
    Vector<string> passengerCNICs;  // List of passenger CNICs on this vehicle

public:
    
    Vehicle() 
        : vehicleID(""), vehicleType(VehicleType::BUS), status(VehicleStatus::IDLE),
          currentRouteIndex(0), currentNodeID(-1), currentStopName(""), currentSector(""),
          homeSector(""), homeNodeID(-1),
          totalDistance(0.0), distanceTraveled(0.0), speed(40.0),
          maxCapacity(0), currentOccupancy(0),
          nextNodeID(-1), progressOnEdge(0.0), isStuck(false), waitingTicks(0),
          renderLat(0.0), renderLon(0.0) {}
    
    Vehicle(const string& id, const string& type, int capacity)
        : vehicleID(id), vehicleType(type), status(VehicleStatus::IDLE),
          currentRouteIndex(0), currentNodeID(-1), currentStopName(""), currentSector(""),
          homeSector(""), homeNodeID(-1),
          totalDistance(0.0), distanceTraveled(0.0), speed(40.0),
          maxCapacity(capacity), currentOccupancy(0),
          nextNodeID(-1), progressOnEdge(0.0), isStuck(false), waitingTicks(0),
          renderLat(0.0), renderLon(0.0) {}
    
    virtual ~Vehicle() = default;
    
    // ==================== SPATIAL GETTERS ====================
    int getNextNodeID() const { return nextNodeID; }
    double getProgressOnEdge() const { return progressOnEdge; }
    bool getIsStuck() const { return isStuck; }
    int getWaitingTicks() const { return waitingTicks; }
    double getRenderLat() const { return renderLat; }
    double getRenderLon() const { return renderLon; }
    const Vector<string>& getPassengerCNICs() const { return passengerCNICs; }
    
    // ==================== SPATIAL SETTERS ====================
    void setNextNodeID(int nodeID) { nextNodeID = nodeID; }
    void setProgressOnEdge(double progress) { progressOnEdge = progress; }
    void setIsStuck(bool stuck) { 
        isStuck = stuck; 
        if (stuck) {
            waitingTicks++;
            status = VehicleStatus::STUCK_IN_TRAFFIC;
        } else {
            waitingTicks = 0;
            if (status == VehicleStatus::STUCK_IN_TRAFFIC) {
                status = VehicleStatus::EN_ROUTE;
            }
        }
    }
    void setRenderPosition(double lat, double lon) { renderLat = lat; renderLon = lon; }
    
    // ==================== PASSENGER MANAGEMENT ====================
    bool addPassenger(const string& cnic) {
        if (currentOccupancy < maxCapacity) {
            passengerCNICs.push_back(cnic);
            currentOccupancy++;
            return true;
        }
        return false;
    }
    
    bool removePassenger(const string& cnic) {
        for (int i = 0; i < passengerCNICs.getSize(); i++) {
            if (passengerCNICs[i] == cnic) {
                passengerCNICs.erase(i);
                currentOccupancy--;
                return true;
            }
        }
        return false;
    }
    
    bool hasPassenger(const string& cnic) const {
        for (int i = 0; i < passengerCNICs.getSize(); i++) {
            if (passengerCNICs[i] == cnic) return true;
        }
        return false;
    }
    
    void clearPassengers() {
        passengerCNICs.clear();
        currentOccupancy = 0;
    }
    
    // ==================== ORIGINAL GETTERS ====================
    
    string getID() const { return vehicleID; }
    string getType() const { return vehicleType; }
    string getStatus() const { return status; }
    int getCurrentNodeID() const { return currentNodeID; }
    string getCurrentStopName() const { return currentStopName; }
    string getCurrentSector() const { return currentSector; }
    string getHomeSector() const { return homeSector; }
    int getHomeNodeID() const { return homeNodeID; }
    double getTotalDistance() const { return totalDistance; }
    double getDistanceTraveled() const { return distanceTraveled; }
    double getSpeed() const { return speed; }
    int getMaxCapacity() const { return maxCapacity; }
    int getCurrentOccupancy() const { return currentOccupancy; }
    int getAvailableCapacity() const { return maxCapacity - currentOccupancy; }
    int getRouteLength() const { return route.size(); }
    int getCurrentRouteIndex() const { return currentRouteIndex; }
    
    const LinkedList<RouteNode>& getRoute() const { return route; }
    
    // ==================== SETTERS ====================
    
    void setStatus(const string& s) { status = s; }
    void setSpeed(double s) { speed = s; }
    void setHomeSector(const string& sector) { homeSector = sector; }
    void setHomeNode(int nodeID) { homeNodeID = nodeID; }
    
    void setCurrentLocation(int nodeID, const string& name, const string& sector) {
        currentNodeID = nodeID;
        currentStopName = name;
        currentSector = sector;
    }
    
    // ==================== ROUTE MANAGEMENT ====================
    
    virtual void setRoute(const Vector<int>& nodeIDs, const Vector<string>& names, 
                         const Vector<string>& sectors, const Vector<double>& distances) {
        route.clear();
        totalDistance = 0.0;
        
        for (int i = 0; i < nodeIDs.getSize(); ++i) {
            double dist = (i > 0 && i < distances.getSize()) ? distances[i] : 0.0;
            string sec = (i < sectors.getSize()) ? sectors[i] : "";
            string name = (i < names.getSize()) ? names[i] : "";
            
            RouteNode node(nodeIDs[i], name, sec, dist);
            totalDistance += dist;
            node.cumulativeDistance = totalDistance;
            route.push_back(node);
        }
        
        currentRouteIndex = 0;
        progressOnEdge = 0.0;
        if (route.size() > 0) {
            currentNodeID = route.front().graphNodeID;
            currentStopName = route.front().stopName;
            currentSector = route.front().sector;
            if (route.size() > 1) {
                nextNodeID = route.at(1).graphNodeID;
            }
        }
    }
    
    void setRouteSimple(const Vector<int>& nodeIDs, double totalDist) {
        route.clear();
        for (int i = 0; i < nodeIDs.getSize(); ++i) {
            RouteNode node(nodeIDs[i], "", "", 0.0);
            route.push_back(node);
        }
        totalDistance = totalDist;
        currentRouteIndex = 0;
        progressOnEdge = 0.0;
        if (route.size() > 0) {
            currentNodeID = route.front().graphNodeID;
            if (route.size() > 1) {
                nextNodeID = route.at(1).graphNodeID;
            }
        }
    }
    
    RouteNode* getCurrentRouteNode() {
        if (currentRouteIndex < route.size()) {
            return &route.at(currentRouteIndex);
        }
        return nullptr;
    }
    
    RouteNode* getNextRouteNode() {
        if (currentRouteIndex + 1 < route.size()) {
            return &route.at(currentRouteIndex + 1);
        }
        return nullptr;
    }
    
    bool isOnRoute(int nodeID) const {
        auto* curr = route.getHead();
        while (curr) {
            if (curr->data.graphNodeID == nodeID) return true;
            curr = curr->next;
        }
        return false;
    }
    
    int getRoutePosition(int nodeID) const {
        auto* curr = route.getHead();
        int pos = 0;
        while (curr) {
            if (curr->data.graphNodeID == nodeID) return pos;
            curr = curr->next;
            ++pos;
        }
        return -1;
    }
    
    Vector<int> getRouteVector() const {
        Vector<int> result;
        auto* curr = route.getHead();
        while (curr) {
            result.push_back(curr->data.graphNodeID);
            curr = curr->next;
        }
        return result;
    }
    
    // ==================== SIMULATION ====================
    
    virtual bool moveToNextStop() {
        if (currentRouteIndex + 1 >= route.size()) {
            return false;
        }
        
        ++currentRouteIndex;
        RouteNode& next = route.at(currentRouteIndex);
        distanceTraveled = next.cumulativeDistance;
        currentNodeID = next.graphNodeID;
        currentStopName = next.stopName;
        currentSector = next.sector;
        progressOnEdge = 0.0;
        
        // Update next node
        if (currentRouteIndex + 1 < route.size()) {
            nextNodeID = route.at(currentRouteIndex + 1).graphNodeID;
        } else {
            nextNodeID = -1;
        }
        
        return true;
    }
    
    virtual void resetRoute() {
        currentRouteIndex = 0;
        distanceTraveled = 0.0;
        progressOnEdge = 0.0;
        isStuck = false;
        waitingTicks = 0;
        if (route.size() > 0) {
            currentNodeID = route.front().graphNodeID;
            currentStopName = route.front().stopName;
            currentSector = route.front().sector;
            if (route.size() > 1) {
                nextNodeID = route.at(1).graphNodeID;
            }
        }
    }
    
    bool isAtRouteEnd() const {
        return currentRouteIndex >= route.size() - 1;
    }
    
    // Legacy occupancy methods
    virtual bool addOccupant() {
        if (currentOccupancy < maxCapacity) {
            ++currentOccupancy;
            return true;
        }
        return false;
    }
    
    virtual bool removeOccupant() {
        if (currentOccupancy > 0) {
            --currentOccupancy;
            return true;
        }
        return false;
    }
    
    void clearOccupancy() { currentOccupancy = 0; passengerCNICs.clear(); }
    bool isFull() const { return currentOccupancy >= maxCapacity; }
    bool isEmpty() const { return currentOccupancy == 0; }
};
