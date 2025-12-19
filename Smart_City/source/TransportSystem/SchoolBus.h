

#pragma once
#include "Vehicle.h"
#include "../../data_structures/CircularQueue.h"
#include <string>

using std::string;

struct StudentPassenger {
    string studentCNIC;
    string studentName;
    string pickupLocation;      
    string dropoffSchoolID;     
    int pickupNodeID;
    int dropoffNodeID;
    bool isHomePickup;        
    
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

struct PickupPoint {
    int nodeID;                 
    string sector;              
    string locationName;        
    bool isResidential;
    CircularQueue<StudentPassenger> waitingStudents;
    
    PickupPoint() 
        : nodeID(-1), sector(""), locationName(""), isResidential(true), waitingStudents(50) {}
    
    PickupPoint(int node, const string& sec, const string& name, bool residential = true)
        : nodeID(node), sector(sec), locationName(name), isResidential(residential), waitingStudents(50) {}
};


namespace SchoolBusStatus {
    const string AVAILABLE = "AVAILABLE";
    const string EN_ROUTE_HOME_PICKUP = "EN_ROUTE_HOME_PICKUP";   
    const string AT_PICKUP_POINT = "AT_PICKUP_POINT";            
    const string LOADING_STUDENTS = "LOADING_STUDENTS";           
    const string EN_ROUTE_TO_SCHOOL = "EN_ROUTE_TO_SCHOOL";       
    const string AT_SCHOOL = "AT_SCHOOL";
    const string UNLOADING = "UNLOADING";
    const string EN_ROUTE_SCHOOL_TO_SCHOOL = "EN_ROUTE_SCHOOL_TO_SCHOOL"; 
    const string RETURNING = "RETURNING";
    const string OUT_OF_SERVICE = "OUT_OF_SERVICE";
}


class SchoolBus : public Vehicle {
private:
    string busID;                   
    string assignedSchoolID;        
    int assignedSchoolNodeID;       
    string schoolBusStatus;         
    
    Vector<string> schoolStops;     
    Vector<int> pickupPointNodes;   
    string currentSchoolID;        
    int currentPickupPointIndex;   
    
    CircularQueue<StudentPassenger> waitingStudents;
    Vector<StudentPassenger> onboardStudents;
    
    string morningPickupTime;       
    string afternoonDropoffTime;   
    
    int totalStudentsTransported;
    int tripsCompleted;
    double totalDistanceCovered;
    int homePickupsCompleted;
    int schoolPickupsCompleted;
    
    Vector<string> prioritySectors;
    
    Vector<string> destinationSchools;
    Vector<int> destinationSchoolNodes;

public:
    
    SchoolBus()
        : Vehicle("", VehicleType::SCHOOL_BUS, 40),  
          busID(""), assignedSchoolID(""), assignedSchoolNodeID(-1),
          schoolBusStatus(SchoolBusStatus::AVAILABLE),
          currentPickupPointIndex(0),
          waitingStudents(50), onboardStudents(),
          morningPickupTime("07:30"), afternoonDropoffTime("14:00"),
          totalStudentsTransported(0), tripsCompleted(0), totalDistanceCovered(0.0),
          homePickupsCompleted(0), schoolPickupsCompleted(0) {
        speed = 35.0; 
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
        
        setPrioritySectors(sector);
        
        destinationSchools.push_back(schoolID);
        destinationSchoolNodes.push_back(schoolNodeID);
    }
    
    ~SchoolBus() override = default;
    
    // ==================== Getter ====================
    
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
        
        if (number > 6) {
            prioritySectors.push_back(string(1, series) + "-" + std::to_string(number - 1));
        }
        if (number < 12) {
            prioritySectors.push_back(string(1, series) + "-" + std::to_string(number + 1));
        }
        
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
    
    void addPickupPoint(int nodeID) {
        pickupPointNodes.push_back(nodeID);
    }
    
    void clearPickupPoints() {
        pickupPointNodes.clear();
        currentPickupPointIndex = 0;
    }
    
    void setPickupRoute(const Vector<int>& pickupNodes) {
        pickupPointNodes.clear();
        for (int i = 0; i < pickupNodes.getSize(); ++i) {
            pickupPointNodes.push_back(pickupNodes[i]);
        }
        currentPickupPointIndex = 0;
    }
    
    void addDestinationSchool(const string& schoolID, int nodeID) {
        destinationSchools.push_back(schoolID);
        destinationSchoolNodes.push_back(nodeID);
    }
    
    void clearDestinationSchools() {
        destinationSchools.clear();
        destinationSchoolNodes.clear();
    }
    
    int getNextPickupPointNode() const {
        if (currentPickupPointIndex < pickupPointNodes.getSize()) {
            return pickupPointNodes[currentPickupPointIndex];
        }
        return -1;
    }
    
    void advanceToNextPickupPoint() {
        ++currentPickupPointIndex;
    }
    
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
    
    bool addWaitingStudent(const StudentPassenger& student) {
        return waitingStudents.enqueue(student);
    }
    
    bool boardStudent(const StudentPassenger& student) {
        if (isFull()) return false;
        onboardStudents.push_back(student);
        ++currentOccupancy;
        return true;
    }
    
    int boardStudentsAtLocation(int locationNodeID) {
        int boarded = 0;
        setSchoolBusStatus(SchoolBusStatus::LOADING_STUDENTS);
        
        while (!waitingStudents.empty() && !isFull()) {
            StudentPassenger student = waitingStudents.dequeue();
            
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
            
            if (isOnRoute(student.dropoffNodeID)) {
                onboardStudents.push_back(student);
                ++currentOccupancy;
                ++boarded;
            }
        }
        
        return boarded;
    }
    
    int dropoffStudents() {
        if (currentSchoolID.empty() && currentNodeID == -1) return 0;
        
        int dropped = 0;
        Vector<StudentPassenger> remaining;
        
        setSchoolBusStatus(SchoolBusStatus::UNLOADING);
        
        for (int i = 0; i < onboardStudents.getSize(); ++i) {
            bool shouldDropoff = false;
            
            if (!currentSchoolID.empty() && onboardStudents[i].dropoffSchoolID == currentSchoolID) {
                shouldDropoff = true;
            }
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
    
    int dropoffAllStudents() {
        int dropped = onboardStudents.getSize();
        totalStudentsTransported += dropped;
        currentOccupancy = 0;
        onboardStudents.clear();
        return dropped;
    }
    
    void processSchoolArrival(const string& schoolID) {
        currentSchoolID = schoolID;
        setSchoolBusStatus(SchoolBusStatus::AT_SCHOOL);
        
        dropoffStudents();
        boardStudents();
    }
    
    void processPickupPointArrival(int nodeID) {
        setSchoolBusStatus(SchoolBusStatus::AT_PICKUP_POINT);
        boardStudentsAtLocation(nodeID);
    }
    
    // ==================== TRIP MANAGEMENT ====================
    
    void resetToBase() {
        currentNodeID = assignedSchoolNodeID;
        currentSchoolID = assignedSchoolID;
        currentPickupPointIndex = 0;
        currentOccupancy = 0;
        onboardStudents.clear();
        resetRoute();
        setSchoolBusStatus(SchoolBusStatus::AVAILABLE);
    }
    
    void startHomePickupRoute() {
        currentPickupPointIndex = 0;
        setSchoolBusStatus(SchoolBusStatus::EN_ROUTE_HOME_PICKUP);
        status = VehicleStatus::EN_ROUTE;
    }
    
    void startSchoolRoute() {
        setSchoolBusStatus(SchoolBusStatus::EN_ROUTE_TO_SCHOOL);
        status = VehicleStatus::EN_ROUTE;
    }
    
    void startInterSchoolRoute() {
        setSchoolBusStatus(SchoolBusStatus::EN_ROUTE_SCHOOL_TO_SCHOOL);
        status = VehicleStatus::EN_ROUTE;
    }
    
    void startAfternoonRoute() {
        setSchoolBusStatus(SchoolBusStatus::EN_ROUTE_TO_SCHOOL);
        status = VehicleStatus::EN_ROUTE;
    }
    
    void completeTrip() {
        ++tripsCompleted;
        totalDistanceCovered += distanceTraveled;
        setSchoolBusStatus(SchoolBusStatus::RETURNING);
    }
    
    void arriveAtBase() {
        currentNodeID = assignedSchoolNodeID;
        currentSchoolID = assignedSchoolID;
        currentPickupPointIndex = 0;
        resetRoute();
        setSchoolBusStatus(SchoolBusStatus::AVAILABLE);
    }
    
    void takeOutOfService() {
        setSchoolBusStatus(SchoolBusStatus::OUT_OF_SERVICE);
    }
    
    void putInService() {
        if (schoolBusStatus == SchoolBusStatus::OUT_OF_SERVICE) {
            setSchoolBusStatus(SchoolBusStatus::AVAILABLE);
        }
    }
    
    // ==================== SIMULATION STEP ====================
    
    void simulateStep() {
        switch (schoolBusStatus[0]) {
            case 'A': 
                if (schoolBusStatus == SchoolBusStatus::AVAILABLE) {
                } else if (schoolBusStatus == SchoolBusStatus::AT_PICKUP_POINT) {
                    boardStudentsAtLocation(currentNodeID);
                    
                    if (isFull() || allPickupsComplete()) {
                        startSchoolRoute();
                    } else {
                        advanceToNextPickupPoint();
                        setSchoolBusStatus(SchoolBusStatus::EN_ROUTE_HOME_PICKUP);
                    }
                } else if (schoolBusStatus == SchoolBusStatus::AT_SCHOOL) {
                    dropoffStudents();
                    
                    if (isEmpty()) {
                        completeTrip();
                    }
                }
                break;
                
            case 'E':
                if (moveToNextStop()) {
                } else {
                    if (schoolBusStatus == SchoolBusStatus::EN_ROUTE_HOME_PICKUP) {
                        setSchoolBusStatus(SchoolBusStatus::AT_PICKUP_POINT);
                    } else if (schoolBusStatus == SchoolBusStatus::EN_ROUTE_TO_SCHOOL) {
                        setSchoolBusStatus(SchoolBusStatus::AT_SCHOOL);
                    } else if (schoolBusStatus == SchoolBusStatus::EN_ROUTE_SCHOOL_TO_SCHOOL) {
                        setSchoolBusStatus(SchoolBusStatus::AT_SCHOOL);
                    }
                }
                break;
                
            case 'L': 
                boardStudents();
                break;
                
            case 'U': 
                dropoffStudents();
                if (isEmpty()) {
                    completeTrip();
                }
                break;
                
            case 'R': 
                if (moveToNextStop()) {
                } else {
                    arriveAtBase();
                }
                break;
                
            case 'O': 
                break;
        }
    }
    
    bool moveToNextStop() override {
        if (Vehicle::moveToNextStop()) {
            return true;
        }
        
        return false;
    }
};
