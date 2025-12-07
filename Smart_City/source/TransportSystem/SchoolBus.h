/*
 * ============================================================================
 * SCHOOL BUS - School-to-School & Home-to-School Transport Vehicle
 * ============================================================================
 * 
 * Extends Vehicle base class for school bus transport.
 * Features:
 *   - Routes between schools in same/adjacent sectors
 *   - Home pickup routes - picks up students from residential areas
 *   - Student pickup and dropoff management
 *   - School schedule integration
 *   - Priority for own sector and adjacent sectors
 *   - Fills up at homes until capacity, then goes to schools
 * 
 * Rubric:
 *   - Transport Module with school routes (5 marks)
 *   - Graph usage for route calculation (4 marks)
 * ============================================================================
 */

#pragma once
#include "Vehicle.h"
#include "../../data_structures/CircularQueue.h"
#include <string>

using std::string;

struct StudentPassenger {
    string studentCNIC;
    string studentName;
    string pickupLocation;      
    string dropoffSchoolID;     // Destination school
    int pickupNodeID;
    int dropoffNodeID;
    bool isHomePickup;          // True if picking up from home, false if from school
    
    StudentPassenger() 
        : studentCNIC(""), studentName(""), pickupLocation(""), dropoffSchoolID(""),
          pickupNodeID(-1), dropoffNodeID(-1), isHomePickup(false) {}
    
    StudentPassenger(const string& cnic, const string& name, 
                    const string& pickup, const string& dropoff,
                    int pickupNode, int dropoffNode, bool fromHome = false)
        : studentCNIC(cnic), studentName(name), 
          pickupLocation(pickup), dropoffSchoolID(dropoff),
          pickupNodeID(pickupNode), dropoffNodeID(dropoffNode),
          isHomePickup(fromHome) {}
    
    bool operator==(const StudentPassenger& other) const {
        return studentCNIC == other.studentCNIC;
    }
};

// ============================================================================
// PICKUP POINT - A location where students wait to be picked up
// ============================================================================
struct PickupPoint {
    int nodeID;                 // Graph node ID
    string sector;              // Sector name
    string locationName;        // Human-readable name
    bool isResidential;         // True for home pickup, false for school
    CircularQueue<StudentPassenger> waitingStudents;
    
    PickupPoint() 
        : nodeID(-1), sector(""), locationName(""), isResidential(true), waitingStudents(50) {}
    
    PickupPoint(int node, const string& sec, const string& name, bool residential = true)
        : nodeID(node), sector(sec), locationName(name), isResidential(residential), waitingStudents(50) {}
};

// ============================================================================
// SCHOOL BUS STATUS - String constants
// ============================================================================
namespace SchoolBusStatus {
    const string AVAILABLE = "AVAILABLE";
    const string EN_ROUTE_HOME_PICKUP = "EN_ROUTE_HOME_PICKUP";   // Going to homes
    const string AT_PICKUP_POINT = "AT_PICKUP_POINT";             // At a home/stop
    const string LOADING_STUDENTS = "LOADING_STUDENTS";           // Loading students
    const string EN_ROUTE_TO_SCHOOL = "EN_ROUTE_TO_SCHOOL";       // Going to school
    const string AT_SCHOOL = "AT_SCHOOL";
    const string UNLOADING = "UNLOADING";
    const string EN_ROUTE_SCHOOL_TO_SCHOOL = "EN_ROUTE_SCHOOL_TO_SCHOOL";  // Inter-school
    const string RETURNING = "RETURNING";
    const string OUT_OF_SERVICE = "OUT_OF_SERVICE";
}

// ============================================================================
// SCHOOL BUS CLASS
// ============================================================================
class SchoolBus : public Vehicle {
private:
    // School bus specific attributes
    string busID;                   // e.g., "SB-01"
    string assignedSchoolID;        // Primary school this bus serves
    int assignedSchoolNodeID;       // Graph node of assigned school
    string schoolBusStatus;         // Detailed status
    
    // Route information
    Vector<string> schoolStops;     // List of school IDs on route
    Vector<int> pickupPointNodes;   // List of pickup point node IDs (homes/stops)
    string currentSchoolID;         // Current school (if at one)
    int currentPickupPointIndex;    // Current index in pickup route
    
    // Passenger management
    CircularQueue<StudentPassenger> waitingStudents;
    Vector<StudentPassenger> onboardStudents;
    
    // Schedule
    string morningPickupTime;       // e.g., "07:30"
    string afternoonDropoffTime;    // e.g., "14:00"
    
    // Statistics
    int totalStudentsTransported;
    int tripsCompleted;
    double totalDistanceCovered;
    int homePickupsCompleted;
    int schoolPickupsCompleted;
    
    // Sector priority
    Vector<string> prioritySectors; // Own sector + adjacent sectors
    
    // Destination schools for current route
    Vector<string> destinationSchools;
    Vector<int> destinationSchoolNodes;

public:
    // ==================== LIFECYCLE ====================
    
    SchoolBus()
        : Vehicle("", VehicleType::SCHOOL_BUS, 40),  // 40 student capacity
          busID(""), assignedSchoolID(""), assignedSchoolNodeID(-1),
          schoolBusStatus(SchoolBusStatus::AVAILABLE),
          currentPickupPointIndex(0),
          waitingStudents(50), onboardStudents(),
          morningPickupTime("07:30"), afternoonDropoffTime("14:00"),
          totalStudentsTransported(0), tripsCompleted(0), totalDistanceCovered(0.0),
          homePickupsCompleted(0), schoolPickupsCompleted(0) {
        speed = 35.0;  // School buses are slower for safety
    }
    
    SchoolBus(const string& id, const string& schoolID, int schoolNodeID, const string& sector)
        : Vehicle(id, VehicleType::SCHOOL_BUS, 40),
          busID(id), assignedSchoolID(schoolID), assignedSchoolNodeID(schoolNodeID),
          schoolBusStatus(SchoolBusStatus::AVAILABLE),
          currentPickupPointIndex(0),
          waitingStudents(50), onboardStudents(),
          morningPickupTime("07:30"), afternoonDropoffTime("14:00"),
          totalStudentsTransported(0), tripsCompleted(0), totalDistanceCovered(0.0),
          homePickupsCompleted(0), schoolPickupsCompleted(0) {
        currentNodeID = schoolNodeID;
        homeSector = sector;
        homeNodeID = schoolNodeID;
        speed = 35.0;
        
        // Set priority sectors (own + adjacent)
        setPrioritySectors(sector);
        
        // Add assigned school as default destination
        destinationSchools.push_back(schoolID);
        destinationSchoolNodes.push_back(schoolNodeID);
    }
    
    ~SchoolBus() override = default;
    
    // ==================== ACCESSORS ====================
    
    string getBusID() const { return busID; }
    string getAssignedSchoolID() const { return assignedSchoolID; }
    int getAssignedSchoolNodeID() const { return assignedSchoolNodeID; }
    string getSchoolBusStatus() const { return schoolBusStatus; }
    string getCurrentSchoolID() const { return currentSchoolID; }
    string getMorningPickupTime() const { return morningPickupTime; }
    string getAfternoonDropoffTime() const { return afternoonDropoffTime; }
    int getTotalStudentsTransported() const { return totalStudentsTransported; }
    int getTripsCompleted() const { return tripsCompleted; }
    double getTotalDistanceCovered() const { return totalDistanceCovered; }
    int getWaitingStudentCount() const { return waitingStudents.size(); }
    int getOnboardStudentCount() const { return onboardStudents.getSize(); }
    int getHomePickupsCompleted() const { return homePickupsCompleted; }
    int getSchoolPickupsCompleted() const { return schoolPickupsCompleted; }
    const Vector<string>& getPrioritySectors() const { return prioritySectors; }
    const Vector<string>& getSchoolStops() const { return schoolStops; }
    const Vector<int>& getPickupPointNodes() const { return pickupPointNodes; }
    const Vector<string>& getDestinationSchools() const { return destinationSchools; }
    
    bool isAvailable() const { return schoolBusStatus == SchoolBusStatus::AVAILABLE; }
    bool isPickingUpFromHomes() const { 
        return schoolBusStatus == SchoolBusStatus::EN_ROUTE_HOME_PICKUP || 
               schoolBusStatus == SchoolBusStatus::AT_PICKUP_POINT ||
               schoolBusStatus == SchoolBusStatus::LOADING_STUDENTS;
    }
    bool isEnRouteToSchool() const { return schoolBusStatus == SchoolBusStatus::EN_ROUTE_TO_SCHOOL; }
    
    // ==================== SETTERS ====================
    
    void setSchoolBusStatus(const string& s) { 
        schoolBusStatus = s;
        // Map to base vehicle status
        if (s == SchoolBusStatus::AVAILABLE) {
            status = VehicleStatus::IDLE;
        } else if (s == SchoolBusStatus::EN_ROUTE_HOME_PICKUP || 
                   s == SchoolBusStatus::EN_ROUTE_TO_SCHOOL ||
                   s == SchoolBusStatus::EN_ROUTE_SCHOOL_TO_SCHOOL ||
                   s == SchoolBusStatus::RETURNING) {
            status = VehicleStatus::EN_ROUTE;
        } else if (s == SchoolBusStatus::AT_SCHOOL || 
                   s == SchoolBusStatus::AT_PICKUP_POINT) {
            status = VehicleStatus::AT_STOP;
        } else if (s == SchoolBusStatus::LOADING_STUDENTS ||
                   s == SchoolBusStatus::UNLOADING) {
            status = VehicleStatus::BOARDING;
        } else if (s == SchoolBusStatus::OUT_OF_SERVICE) {
            status = VehicleStatus::MAINTENANCE;
        }
    }
    
    void setSchedule(const string& morning, const string& afternoon) {
        morningPickupTime = morning;
        afternoonDropoffTime = afternoon;
    }
    
    void setCurrentSchool(const string& schoolID) {
        currentSchoolID = schoolID;
    }
    
    // ==================== SECTOR PRIORITY ====================
    
    void setPrioritySectors(const string& homeSector) {
        prioritySectors.clear();
        prioritySectors.push_back(homeSector);
        
        char series = homeSector[0];
        int number = 0;
        
        string numStr = "";
        for (int i = 2; i < (int)homeSector.length(); ++i) {
            numStr += homeSector[i];
        }
        if (!numStr.empty()) {
            number = std::stoi(numStr);
        }
        
        // Add same series, adjacent numbers
        if (number > 6) {
            prioritySectors.push_back(string(1, series) + "-" + std::to_string(number - 1));
        }
        if (number < 12) {
            prioritySectors.push_back(string(1, series) + "-" + std::to_string(number + 1));
        }
        
        // Add adjacent series, same number
        if (series > 'E') {
            prioritySectors.push_back(string(1, series - 1) + "-" + std::to_string(number));
        }
        if (series < 'I') {
            prioritySectors.push_back(string(1, series + 1) + "-" + std::to_string(number));
        }
    }
    
    bool isSectorInPriority(const string& sector) const {
        for (int i = 0; i < prioritySectors.getSize(); ++i) {
            if (prioritySectors[i] == sector) return true;
        }
        return false;
    }
    
    // ==================== PICKUP ROUTE MANAGEMENT ====================
    
    // Add a pickup point (home/residential stop) to the route
    void addPickupPoint(int nodeID) {
        pickupPointNodes.push_back(nodeID);
    }
    
    // Clear all pickup points
    void clearPickupPoints() {
        pickupPointNodes.clear();
        currentPickupPointIndex = 0;
    }
    
    // Set pickup route from vector of node IDs
    void setPickupRoute(const Vector<int>& pickupNodes) {
        pickupPointNodes.clear();
        for (int i = 0; i < pickupNodes.getSize(); ++i) {
            pickupPointNodes.push_back(pickupNodes[i]);
        }
        currentPickupPointIndex = 0;
    }
    
    // Add a destination school
    void addDestinationSchool(const string& schoolID, int nodeID) {
        destinationSchools.push_back(schoolID);
        destinationSchoolNodes.push_back(nodeID);
    }
    
    // Clear destination schools
    void clearDestinationSchools() {
        destinationSchools.clear();
        destinationSchoolNodes.clear();
    }
    
    // Get next pickup point node ID
    int getNextPickupPointNode() const {
        if (currentPickupPointIndex < pickupPointNodes.getSize()) {
            return pickupPointNodes[currentPickupPointIndex];
        }
        return -1;
    }
    
    // Move to next pickup point
    void advanceToNextPickupPoint() {
        ++currentPickupPointIndex;
    }
    
    // Check if all pickup points have been visited
    bool allPickupsComplete() const {
        return currentPickupPointIndex >= pickupPointNodes.getSize();
    }
    
    // ==================== SCHOOL ROUTE MANAGEMENT ====================
    
    void addSchoolToRoute(const string& schoolID) {
        schoolStops.push_back(schoolID);
    }
    
    void clearSchoolStops() {
        schoolStops.clear();
    }
    
    void setSchoolRoute(const Vector<int>& routeNodes, const Vector<string>& schoolIDs, double distance) {
        setRouteSimple(routeNodes, distance);
        schoolStops.clear();
        for (int i = 0; i < schoolIDs.getSize(); ++i) {
            schoolStops.push_back(schoolIDs[i]);
        }
    }
    
    // ==================== STUDENT OPERATIONS ====================
    
    // Add student to waiting queue
    bool addWaitingStudent(const StudentPassenger& student) {
        return waitingStudents.enqueue(student);
    }
    
    // Board a single student (if capacity allows)
    bool boardStudent(const StudentPassenger& student) {
        if (isFull()) return false;
        onboardStudents.push_back(student);
        ++currentOccupancy;
        return true;
    }
    
    // Board students from waiting queue at current location
    int boardStudentsAtLocation(int locationNodeID) {
        int boarded = 0;
        setSchoolBusStatus(SchoolBusStatus::LOADING_STUDENTS);
        
        while (!waitingStudents.empty() && !isFull()) {
            StudentPassenger student = waitingStudents.dequeue();
            
            // Check if this student should board at this location
            if (student.pickupNodeID == locationNodeID || student.pickupNodeID == -1) {
                onboardStudents.push_back(student);
                ++currentOccupancy;
                ++boarded;
                
                if (student.isHomePickup) {
                    ++homePickupsCompleted;
                } else {
                    ++schoolPickupsCompleted;
                }
            }
        }
        
        return boarded;
    }
    
    // Board students at current school
    int boardStudents() {
        if (schoolBusStatus != SchoolBusStatus::LOADING_STUDENTS &&
            schoolBusStatus != SchoolBusStatus::AT_SCHOOL &&
            schoolBusStatus != SchoolBusStatus::AT_PICKUP_POINT) {
            return 0;
        }
        
        int boarded = 0;
        setSchoolBusStatus(SchoolBusStatus::LOADING_STUDENTS);
        
        while (!waitingStudents.empty() && !isFull()) {
            StudentPassenger student = waitingStudents.dequeue();
            
            // Check if destination is on route
            if (isOnRoute(student.dropoffNodeID)) {
                onboardStudents.push_back(student);
                ++currentOccupancy;
                ++boarded;
            }
        }
        
        return boarded;
    }
    
    // Drop off students at current school
    int dropoffStudents() {
        if (currentSchoolID.empty() && currentNodeID == -1) return 0;
        
        int dropped = 0;
        Vector<StudentPassenger> remaining;
        
        setSchoolBusStatus(SchoolBusStatus::UNLOADING);
        
        for (int i = 0; i < onboardStudents.getSize(); ++i) {
            bool shouldDropoff = false;
            
            // Drop off if at their destination school
            if (!currentSchoolID.empty() && onboardStudents[i].dropoffSchoolID == currentSchoolID) {
                shouldDropoff = true;
            }
            // Or if at their destination node
            else if (onboardStudents[i].dropoffNodeID == currentNodeID) {
                shouldDropoff = true;
            }
            
            if (shouldDropoff) {
                ++dropped;
                ++totalStudentsTransported;
                --currentOccupancy;
            } else {
                remaining.push_back(onboardStudents[i]);
            }
        }
        
        onboardStudents = remaining;
        return dropped;
    }
    
    // Drop off all students (at destination school)
    int dropoffAllStudents() {
        int dropped = onboardStudents.getSize();
        totalStudentsTransported += dropped;
        currentOccupancy = 0;
        onboardStudents.clear();
        return dropped;
    }
    
    // Process arrival at a school
    void processSchoolArrival(const string& schoolID) {
        currentSchoolID = schoolID;
        setSchoolBusStatus(SchoolBusStatus::AT_SCHOOL);
        
        // First drop off, then pick up
        dropoffStudents();
        boardStudents();
    }
    
    // Process arrival at a pickup point (home/residential area)
    void processPickupPointArrival(int nodeID) {
        setSchoolBusStatus(SchoolBusStatus::AT_PICKUP_POINT);
        boardStudentsAtLocation(nodeID);
    }
    
    // ==================== TRIP MANAGEMENT ====================
    
    // Reset school bus to base (for simulation reset)
    void resetToBase() {
        currentNodeID = assignedSchoolNodeID;
        currentSchoolID = assignedSchoolID;
        currentPickupPointIndex = 0;
        currentOccupancy = 0;
        onboardStudents.clear();
        resetRoute();
        setSchoolBusStatus(SchoolBusStatus::AVAILABLE);
    }
    
    // Start morning home pickup route
    void startHomePickupRoute() {
        currentPickupPointIndex = 0;
        setSchoolBusStatus(SchoolBusStatus::EN_ROUTE_HOME_PICKUP);
        status = VehicleStatus::EN_ROUTE;
    }
    
    // Start route to school after pickups
    void startSchoolRoute() {
        setSchoolBusStatus(SchoolBusStatus::EN_ROUTE_TO_SCHOOL);
        status = VehicleStatus::EN_ROUTE;
    }
    
    // Start inter-school route
    void startInterSchoolRoute() {
        setSchoolBusStatus(SchoolBusStatus::EN_ROUTE_SCHOOL_TO_SCHOOL);
        status = VehicleStatus::EN_ROUTE;
    }
    
    // Start afternoon dropoff route
    void startAfternoonRoute() {
        setSchoolBusStatus(SchoolBusStatus::EN_ROUTE_TO_SCHOOL);
        status = VehicleStatus::EN_ROUTE;
    }
    
    // Complete trip and return to base
    void completeTrip() {
        ++tripsCompleted;
        totalDistanceCovered += distanceTraveled;
        setSchoolBusStatus(SchoolBusStatus::RETURNING);
    }
    
    // Arrive back at base school
    void arriveAtBase() {
        currentNodeID = assignedSchoolNodeID;
        currentSchoolID = assignedSchoolID;
        currentPickupPointIndex = 0;
        resetRoute();
        setSchoolBusStatus(SchoolBusStatus::AVAILABLE);
    }
    
    // Take out of service
    void takeOutOfService() {
        setSchoolBusStatus(SchoolBusStatus::OUT_OF_SERVICE);
    }
    
    // Put back in service
    void putInService() {
        if (schoolBusStatus == SchoolBusStatus::OUT_OF_SERVICE) {
            setSchoolBusStatus(SchoolBusStatus::AVAILABLE);
        }
    }
    
    // ==================== SIMULATION STEP ====================
    
    // Process one simulation step
    void simulateStep() {
        switch (schoolBusStatus[0]) {
            case 'A': // AVAILABLE or AT_*
                if (schoolBusStatus == SchoolBusStatus::AVAILABLE) {
                    // Do nothing, waiting for dispatch
                } else if (schoolBusStatus == SchoolBusStatus::AT_PICKUP_POINT) {
                    // Board students at pickup point
                    boardStudentsAtLocation(currentNodeID);
                    
                    // If full or no more pickup points, head to school
                    if (isFull() || allPickupsComplete()) {
                        startSchoolRoute();
                    } else {
                        advanceToNextPickupPoint();
                        setSchoolBusStatus(SchoolBusStatus::EN_ROUTE_HOME_PICKUP);
                    }
                } else if (schoolBusStatus == SchoolBusStatus::AT_SCHOOL) {
                    // Drop off students
                    dropoffStudents();
                    
                    // If empty, either continue to next school or return
                    if (isEmpty()) {
                        completeTrip();
                    }
                }
                break;
                
            case 'E': // EN_ROUTE_*
                if (moveToNextStop()) {
                    // Still moving
                } else {
                    // Arrived at destination
                    if (schoolBusStatus == SchoolBusStatus::EN_ROUTE_HOME_PICKUP) {
                        setSchoolBusStatus(SchoolBusStatus::AT_PICKUP_POINT);
                    } else if (schoolBusStatus == SchoolBusStatus::EN_ROUTE_TO_SCHOOL) {
                        setSchoolBusStatus(SchoolBusStatus::AT_SCHOOL);
                    } else if (schoolBusStatus == SchoolBusStatus::EN_ROUTE_SCHOOL_TO_SCHOOL) {
                        setSchoolBusStatus(SchoolBusStatus::AT_SCHOOL);
                    }
                }
                break;
                
            case 'L': // LOADING_STUDENTS
                // Continue loading
                boardStudents();
                break;
                
            case 'U': // UNLOADING
                // Finish unloading
                dropoffStudents();
                if (isEmpty()) {
                    completeTrip();
                }
                break;
                
            case 'R': // RETURNING
                if (moveToNextStop()) {
                    // Still returning
                } else {
                    arriveAtBase();
                }
                break;
                
            case 'O': // OUT_OF_SERVICE
                // Do nothing
                break;
        }
    }
    
    // Override moveToNextStop
    bool moveToNextStop() override {
        if (Vehicle::moveToNextStop()) {
            return true;
        }
        
        // End of route
        return false;
    }
};
