/*
 * ============================================================================
 * SCHOOL BUS - School-to-School Transport Vehicle
 * ============================================================================
 * 
 * Extends Vehicle base class for school bus transport.
 * Features:
 *   - Routes between schools in same/adjacent sectors
 *   - Student pickup and dropoff management
 *   - School schedule integration
 *   - Priority for own sector and adjacent sectors
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

// ============================================================================
// STUDENT PASSENGER - Represents a student on a school bus
// ============================================================================
struct StudentPassenger {
    string studentCNIC;
    string studentName;
    string pickupSchoolID;      // School where student boards
    string dropoffSchoolID;     // Destination school
    int pickupNodeID;
    int dropoffNodeID;
    
    StudentPassenger() 
        : studentCNIC(""), studentName(""), pickupSchoolID(""), dropoffSchoolID(""),
          pickupNodeID(-1), dropoffNodeID(-1) {}
    
    StudentPassenger(const string& cnic, const string& name, 
                    const string& pickup, const string& dropoff,
                    int pickupNode, int dropoffNode)
        : studentCNIC(cnic), studentName(name), 
          pickupSchoolID(pickup), dropoffSchoolID(dropoff),
          pickupNodeID(pickupNode), dropoffNodeID(dropoffNode) {}
    
    bool operator==(const StudentPassenger& other) const {
        return studentCNIC == other.studentCNIC;
    }
};

// ============================================================================
// SCHOOL BUS STATUS - String constants
// ============================================================================
namespace SchoolBusStatus {
    const string AVAILABLE = "AVAILABLE";
    const string EN_ROUTE_PICKUP = "EN_ROUTE_PICKUP";
    const string AT_SCHOOL = "AT_SCHOOL";
    const string LOADING = "LOADING";
    const string EN_ROUTE_DROPOFF = "EN_ROUTE_DROPOFF";
    const string UNLOADING = "UNLOADING";
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
    string currentSchoolID;         // Current school (if at one)
    
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
    
    // Sector priority
    Vector<string> prioritySectors; // Own sector + adjacent sectors

public:
    // ==================== LIFECYCLE ====================
    
    SchoolBus()
        : Vehicle("", VehicleType::SCHOOL_BUS, 40),  // 40 student capacity
          busID(""), assignedSchoolID(""), assignedSchoolNodeID(-1),
          schoolBusStatus(SchoolBusStatus::AVAILABLE),
          waitingStudents(50), onboardStudents(),
          morningPickupTime("07:30"), afternoonDropoffTime("14:00"),
          totalStudentsTransported(0), tripsCompleted(0), totalDistanceCovered(0.0) {
        speed = 35.0;  // School buses are slower for safety
    }
    
    SchoolBus(const string& id, const string& schoolID, int schoolNodeID, const string& sector)
        : Vehicle(id, VehicleType::SCHOOL_BUS, 40),
          busID(id), assignedSchoolID(schoolID), assignedSchoolNodeID(schoolNodeID),
          schoolBusStatus(SchoolBusStatus::AVAILABLE),
          waitingStudents(50), onboardStudents(),
          morningPickupTime("07:30"), afternoonDropoffTime("14:00"),
          totalStudentsTransported(0), tripsCompleted(0), totalDistanceCovered(0.0) {
        currentNodeID = schoolNodeID;
        homeSector = sector;
        homeNodeID = schoolNodeID;
        speed = 35.0;
        
        // Set priority sectors (own + adjacent)
        setPrioritySectors(sector);
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
    const Vector<string>& getPrioritySectors() const { return prioritySectors; }
    const Vector<string>& getSchoolStops() const { return schoolStops; }
    
    bool isAvailable() const { return schoolBusStatus == SchoolBusStatus::AVAILABLE; }
    
    // ==================== SETTERS ====================
    
    void setSchoolBusStatus(const string& s) { 
        schoolBusStatus = s;
        // Map to base vehicle status
        if (s == SchoolBusStatus::AVAILABLE) {
            status = VehicleStatus::IDLE;
        } else if (s == SchoolBusStatus::EN_ROUTE_PICKUP || 
                   s == SchoolBusStatus::EN_ROUTE_DROPOFF ||
                   s == SchoolBusStatus::RETURNING) {
            status = VehicleStatus::EN_ROUTE;
        } else if (s == SchoolBusStatus::AT_SCHOOL || 
                   s == SchoolBusStatus::LOADING ||
                   s == SchoolBusStatus::UNLOADING) {
            status = VehicleStatus::AT_STOP;
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
    
    // Set priority sectors based on home sector
    void setPrioritySectors(const string& homeSector) {
        prioritySectors.clear();
        prioritySectors.push_back(homeSector);
        
        // Add adjacent sectors based on Islamabad grid pattern
        // E.g., G-10 is adjacent to G-9, G-11, F-10, H-10
        char series = homeSector[0];
        int number = 0;
        
        // Parse sector number
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
    
    // Check if a sector is in priority list
    bool isSectorInPriority(const string& sector) const {
        for (int i = 0; i < prioritySectors.getSize(); ++i) {
            if (prioritySectors[i] == sector) return true;
        }
        return false;
    }
    
    // ==================== SCHOOL ROUTE MANAGEMENT ====================
    
    // Add a school to the route
    void addSchoolToRoute(const string& schoolID) {
        schoolStops.push_back(schoolID);
    }
    
    // Clear school stops
    void clearSchoolStops() {
        schoolStops.clear();
    }
    
    // Set route between schools
    void setSchoolRoute(const Vector<int>& routeNodes, const Vector<string>& schoolIDs, double distance) {
        setRouteSimple(routeNodes, distance);
        schoolStops.clear();
        for (int i = 0; i < schoolIDs.getSize(); ++i) {
            schoolStops.push_back(schoolIDs[i]);
        }
    }
    
    // ==================== STUDENT OPERATIONS ====================
    
    // Add student to waiting queue at school
    bool addWaitingStudent(const StudentPassenger& student) {
        return waitingStudents.enqueue(student);
    }
    
    // Board students at current school
    int boardStudents() {
        if (schoolBusStatus != SchoolBusStatus::LOADING &&
            schoolBusStatus != SchoolBusStatus::AT_SCHOOL) {
            return 0;
        }
        
        int boarded = 0;
        setSchoolBusStatus(SchoolBusStatus::LOADING);
        
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
        if (currentSchoolID.empty()) return 0;
        
        int dropped = 0;
        Vector<StudentPassenger> remaining;
        
        setSchoolBusStatus(SchoolBusStatus::UNLOADING);
        
        for (int i = 0; i < onboardStudents.getSize(); ++i) {
            if (onboardStudents[i].dropoffSchoolID == currentSchoolID) {
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
    
    // Process arrival at a school
    void processSchoolArrival(const string& schoolID) {
        currentSchoolID = schoolID;
        setSchoolBusStatus(SchoolBusStatus::AT_SCHOOL);
        
        // First drop off, then pick up
        dropoffStudents();
        boardStudents();
    }
    
    // ==================== TRIP MANAGEMENT ====================
    
    // Start morning pickup route
    void startMorningRoute() {
        setSchoolBusStatus(SchoolBusStatus::EN_ROUTE_PICKUP);
        status = VehicleStatus::EN_ROUTE;
    }
    
    // Start afternoon dropoff route
    void startAfternoonRoute() {
        setSchoolBusStatus(SchoolBusStatus::EN_ROUTE_DROPOFF);
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
    
    // Override moveToNextStop
    bool moveToNextStop() override {
        if (Vehicle::moveToNextStop()) {
            return true;
        }
        
        // End of route - complete trip
        completeTrip();
        return false;
    }
};
