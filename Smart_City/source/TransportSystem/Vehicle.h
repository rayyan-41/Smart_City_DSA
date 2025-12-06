/*
 * ============================================================================
 * VEHICLE BASE CLASS - Abstract Base for All Transport Vehicles
 * ============================================================================
 * 
 * Base class providing common functionality for:
 *   - Buses (public transport)
 *   - School Buses (school-to-school transport)
 *   - Ambulances (hospital-to-hospital emergency transport)
 * 
 * Uses Singly Linked List for route management.
 * 
 * Rubric:
 *   - Singly Linked List for Bus route management (4 marks)
 *   - Transport Module (5 marks)
 * ============================================================================
 */

#pragma once
#include <string>
#include "../../data_structures/LinkedLists.h"
#include "../../data_structures/Vector.h"

using std::string;

// ============================================================================
// VEHICLE STATUS - String constants instead of enums
// ============================================================================
namespace VehicleStatus {
    const string IDLE = "IDLE";
    const string EN_ROUTE = "EN_ROUTE";
    const string AT_STOP = "AT_STOP";
    const string EMERGENCY = "EMERGENCY";
    const string MAINTENANCE = "MAINTENANCE";
    const string BOARDING = "BOARDING";
    const string RETURNING = "RETURNING";
}

// ============================================================================
// VEHICLE TYPE - String constants instead of enums
// ============================================================================
namespace VehicleType {
    const string BUS = "BUS";
    const string SCHOOL_BUS = "SCHOOL_BUS";
    const string AMBULANCE = "AMBULANCE";
}

// ============================================================================
// ROUTE NODE - Represents a stop on the vehicle's route
// ============================================================================
struct RouteNode {
    int graphNodeID;            // ID in CityGraph
    string stopName;            // Human-readable name
    string sector;              // Sector this node belongs to
    double distanceFromPrev;    // Distance from previous stop (km)
    double cumulativeDistance;  // Total distance from start
    bool isScheduledStop;       // Whether vehicle actually stops here
    
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

// ============================================================================
// VEHICLE - Abstract Base Class
// ============================================================================
class Vehicle {
protected:
    string vehicleID;           // Unique identifier (e.g., "B101", "AMB-01", "SB-01")
    string vehicleType;         // Type of vehicle (from VehicleType namespace)
    string status;              // Current status (from VehicleStatus namespace)
    
    // Route management using Singly Linked List
    LinkedList<RouteNode> route;
    int currentRouteIndex;      // Current position in route
    
    // Location tracking
    int currentNodeID;          // Current graph node ID
    string currentStopName;     // Current stop name
    string currentSector;       // Current sector
    
    // Home base
    string homeSector;          // Primary sector this vehicle serves
    int homeNodeID;             // Home base node ID
    
    // Metrics
    double totalDistance;       // Total route distance (km)
    double distanceTraveled;    // Distance traveled so far
    double speed;               // Average speed (km/h)
    
    // Capacity
    int maxCapacity;            // Maximum passengers/patients
    int currentOccupancy;       // Current count

public:
    // ==================== LIFECYCLE ====================
    
    Vehicle() 
        : vehicleID(""), vehicleType(VehicleType::BUS), status(VehicleStatus::IDLE),
          currentRouteIndex(0), currentNodeID(-1), currentStopName(""), currentSector(""),
          homeSector(""), homeNodeID(-1),
          totalDistance(0.0), distanceTraveled(0.0), speed(40.0),
          maxCapacity(0), currentOccupancy(0) {}
    
    Vehicle(const string& id, const string& type, int capacity)
        : vehicleID(id), vehicleType(type), status(VehicleStatus::IDLE),
          currentRouteIndex(0), currentNodeID(-1), currentStopName(""), currentSector(""),
          homeSector(""), homeNodeID(-1),
          totalDistance(0.0), distanceTraveled(0.0), speed(40.0),
          maxCapacity(capacity), currentOccupancy(0) {}
    
    virtual ~Vehicle() = default;
    
    // ==================== ACCESSORS ====================
    
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
    
    // Set route from vector of node IDs with details
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
        if (route.size() > 0) {
            currentNodeID = route.front().graphNodeID;
            currentStopName = route.front().stopName;
            currentSector = route.front().sector;
        }
    }
    
    // Simple route set (just node IDs)
    void setRouteSimple(const Vector<int>& nodeIDs, double totalDist) {
        route.clear();
        for (int i = 0; i < nodeIDs.getSize(); ++i) {
            RouteNode node(nodeIDs[i], "", "", 0.0);
            route.push_back(node);
        }
        totalDistance = totalDist;
        currentRouteIndex = 0;
        if (route.size() > 0) {
            currentNodeID = route.front().graphNodeID;
        }
    }
    
    // Get current route node
    RouteNode* getCurrentRouteNode() {
        if (currentRouteIndex < route.size()) {
            return &route.at(currentRouteIndex);
        }
        return nullptr;
    }
    
    // Get next route node
    RouteNode* getNextRouteNode() {
        if (currentRouteIndex + 1 < route.size()) {
            return &route.at(currentRouteIndex + 1);
        }
        return nullptr;
    }
    
    // Check if a node is on this vehicle's route
    bool isOnRoute(int nodeID) const {
        auto* curr = route.getHead();
        while (curr) {
            if (curr->data.graphNodeID == nodeID) return true;
            curr = curr->next;
        }
        return false;
    }
    
    // Get position of node in route (-1 if not found)
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
    
    // Get route as Vector<int> for compatibility
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
    
    // Move to next stop on route
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
        
        return true;
    }
    
    // Reset to start of route
    virtual void resetRoute() {
        currentRouteIndex = 0;
        distanceTraveled = 0.0;
        if (route.size() > 0) {
            currentNodeID = route.front().graphNodeID;
            currentStopName = route.front().stopName;
            currentSector = route.front().sector;
        }
    }
    
    // Check if at end of route
    bool isAtRouteEnd() const {
        return currentRouteIndex >= route.size() - 1;
    }
    
    // ==================== CAPACITY MANAGEMENT ====================
    
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
    
    void clearOccupancy() { currentOccupancy = 0; }
    bool isFull() const { return currentOccupancy >= maxCapacity; }
    bool isEmpty() const { return currentOccupancy == 0; }
};
