#pragma once
#include <string>
#include "../../data_structures/Vector.h"
using std::string;

// ==================== CITIZEN STATE ENUM ====================
enum class CitizenState {
    IDLE_HOME,          // At home, doing nothing specific
    SLEEPING,           // At home, sleeping (night time)
    WORKING,            // At workplace
    COMMUTING,          // On a vehicle (bus, rickshaw)
    WAITING_FOR_BUS,    // At a stop, waiting for transport
    WAITING_FOR_RIDE,   // Not at a stop, needs rickshaw pickup
    WALKING,            // Moving between nodes on foot
    SHOPPING,           // At mall/shop
    EATING,             // At restaurant
    AT_SCHOOL,          // Student at school
    AT_HOSPITAL,        // Patient at hospital
    EMERGENCY           // Emergency state (ambulance needed)
};

// ==================== NEEDS SYSTEM ====================
struct CitizenNeeds {
    double hunger;      // 0-100, increases over time (100 = starving)
    double energy;      // 0-100, decreases over time (0 = exhausted)
    double social;      // 0-100, decays over time (0 = lonely)
    double health;      // 0-100, can decrease (0 = critical)
    double wallet;      // Cash on hand (PKR)
    
    CitizenNeeds() 
        : hunger(0.0), energy(100.0), social(50.0), health(100.0), wallet(1000.0) {}
    
    // Decay needs over time (call each simulation tick)
    void decay(double deltaTime) {
        hunger += 0.1 * deltaTime;      // Gets hungry over time
        energy -= 0.05 * deltaTime;     // Gets tired over time
        social -= 0.02 * deltaTime;     // Gets lonely over time
        
        // Clamp values
        if (hunger > 100.0) hunger = 100.0;
        if (hunger < 0.0) hunger = 0.0;
        if (energy > 100.0) energy = 100.0;
        if (energy < 0.0) energy = 0.0;
        if (social > 100.0) social = 100.0;
        if (social < 0.0) social = 0.0;
        if (health > 100.0) health = 100.0;
        if (health < 0.0) health = 0.0;
    }
    
    // Replenish needs
    void eat() { hunger -= 30.0; if (hunger < 0) hunger = 0; wallet -= 200; }
    void sleep() { energy = 100.0; }
    void socialize() { social += 20.0; if (social > 100) social = 100; }
    void heal() { health += 30.0; if (health > 100) health = 100; }
    
    // Priority checks for AI
    bool isHungry() const { return hunger > 60.0; }
    bool isCriticallyHungry() const { return hunger > 80.0; }
    bool isTired() const { return energy < 30.0; }
    bool isExhausted() const { return energy < 10.0; }
    bool isLonely() const { return social < 30.0; }
    bool isSick() const { return health < 50.0; }
    bool isCritical() const { return health < 20.0; }
    bool canAfford(double amount) const { return wallet >= amount; }
};

// ==================== MOVEMENT/PATHING DATA ====================
struct CitizenPath {
    Vector<int> nodes;          // List of node IDs to traverse
    int currentIndex;           // Current position in path
    double progressOnEdge;      // 0.0 to 1.0 progress between nodes
    int destinationNodeID;      // Final destination
    string destinationType;     // Type of destination (SCHOOL, HOSPITAL, etc.)
    
    CitizenPath() : currentIndex(0), progressOnEdge(0.0), 
                    destinationNodeID(-1), destinationType("") {}
    
    void clear() {
        nodes.clear();
        currentIndex = 0;
        progressOnEdge = 0.0;
        destinationNodeID = -1;
        destinationType = "";
    }
    
    bool hasPath() const { return nodes.getSize() > 0; }
    bool isComplete() const { 
        return currentIndex >= nodes.getSize() - 1 && progressOnEdge >= 1.0;
    }
    
    int getCurrentNodeID() const {
        if (currentIndex < nodes.getSize()) return nodes[currentIndex];
        return -1;
    }
    
    int getNextNodeID() const {
        if (currentIndex + 1 < nodes.getSize()) return nodes[currentIndex + 1];
        return -1;
    }
    
    // Advance along path, returns true if reached a new node
    bool advance(double speed) {
        if (isComplete()) return false;
        
        progressOnEdge += speed;
        if (progressOnEdge >= 1.0) {
            progressOnEdge = 0.0;
            currentIndex++;
            return true;  // Reached new node
        }
        return false;
    }
};

// ==================== CITIZEN STRUCT ====================
struct Citizen {
    // Identity
    string cnic;
    string name;
    int age;
    
    // Home address
    string sector;
    int street;
    int houseNo;
    int homeNodeID;         // Graph node ID of home location
    
    // Current location
    int currentNodeID;      // Current node (or last visited node)
    double lat, lon;        // Interpolated position for rendering
    
    // State
    CitizenState state;
    string currentStatus;   // Legacy field for display
    
    // Needs System
    CitizenNeeds needs;
    
    // Pathing
    CitizenPath path;
    string currentVehicleID;    // If not empty, citizen is on this vehicle
    
    // Work/School
    int workplaceNodeID;        // Where they work (-1 if unemployed/child)
    int schoolNodeID;           // Where they study (-1 if not a student)
    string occupation;          // Job type or "Student" or "Unemployed"
    
    // Time tracking
    int lastActionTime;         // Simulation time of last state change
    
    Citizen()
        : cnic(""), name(""), age(0),
          sector(""), street(0), houseNo(0), homeNodeID(-1),
          currentNodeID(-1), lat(0), lon(0),
          state(CitizenState::IDLE_HOME), currentStatus("Home"),
          currentVehicleID(""),
          workplaceNodeID(-1), schoolNodeID(-1), occupation("Unemployed"),
          lastActionTime(0) {
    }

    Citizen(string cnic, string name, int age, string sector, int street, int houseNo)
        : cnic(cnic), name(name), age(age), 
          sector(sector), street(street), houseNo(houseNo), homeNodeID(-1),
          currentNodeID(-1), lat(0), lon(0),
          state(CitizenState::IDLE_HOME), currentStatus("Home"),
          currentVehicleID(""),
          workplaceNodeID(-1), schoolNodeID(-1), occupation("Unemployed"),
          lastActionTime(0) {
        // Determine occupation based on age
        if (age < 5) occupation = "Toddler";
        else if (age < 18) occupation = "Student";
        else if (age >= 60) occupation = "Retired";
    }

    // ==================== STATE HELPERS ====================
    bool isOnVehicle() const { return !currentVehicleID.empty(); }
    bool isWalking() const { return state == CitizenState::WALKING; }
    bool isWaiting() const { 
        return state == CitizenState::WAITING_FOR_BUS || 
               state == CitizenState::WAITING_FOR_RIDE; 
    }
    bool isAtHome() const { return state == CitizenState::IDLE_HOME || state == CitizenState::SLEEPING; }
    bool isWorking() const { return state == CitizenState::WORKING; }
    bool isStudent() const { return age >= 5 && age < 18; }
    bool isWorker() const { return age >= 18 && age < 60; }
    bool needsTransport() const { return state == CitizenState::WAITING_FOR_RIDE; }
    
    // ==================== STATUS STRING ====================
    string getStateString() const {
        switch (state) {
            case CitizenState::IDLE_HOME: return "Home";
            case CitizenState::SLEEPING: return "Sleeping";
            case CitizenState::WORKING: return "Working";
            case CitizenState::COMMUTING: return "Commuting";
            case CitizenState::WAITING_FOR_BUS: return "Waiting for Bus";
            case CitizenState::WAITING_FOR_RIDE: return "Waiting for Ride";
            case CitizenState::WALKING: return "Walking";
            case CitizenState::SHOPPING: return "Shopping";
            case CitizenState::EATING: return "Eating";
            case CitizenState::AT_SCHOOL: return "At School";
            case CitizenState::AT_HOSPITAL: return "At Hospital";
            case CitizenState::EMERGENCY: return "Emergency!";
            default: return "Unknown";
        }
    }
    
    string getThought() const {
        if (needs.isCriticallyHungry()) return "I'm starving!";
        if (needs.isHungry()) return "I should eat something...";
        if (needs.isExhausted()) return "I need to sleep...";
        if (needs.isTired()) return "Getting tired...";
        if (needs.isCritical()) return "I need a hospital!";
        if (needs.isSick()) return "I don't feel well...";
        if (needs.isLonely()) return "I should visit friends...";
        if (state == CitizenState::WAITING_FOR_BUS) return "Hope the bus comes soon...";
        if (state == CitizenState::COMMUTING) return "Almost there...";
        return "Having a nice day.";
    }

    // ==================== LEGACY GETTERS ====================
    string getCNIC() const { return cnic; }
    string getName() const { return name; }
    int getAge() const { return age; }
    string getCurrentStatus() const { return getStateString(); }
    string getSector() const { return sector; }
    int getStreet() const { return street; }
    int getHouseNo() const { return houseNo; }
    
    string getFullAddress() const {
        return sector + ", Street " + std::to_string(street) + ", House " + std::to_string(houseNo);
    }

    // ==================== LEGACY SETTERS ====================
    void setCNIC(const string& newCnic) { cnic = newCnic; }
    void setName(const string& newName) { name = newName; }
    void setAge(int newAge) { age = newAge; }
    void setCurrentStatus(const string& status) { currentStatus = status; }
    void setSector(const string& newSector) { sector = newSector; }
    void setStreet(int newStreet) { street = newStreet; }
    void setHouseNo(int newHouseNo) { houseNo = newHouseNo; }
    
    void setAddress(const string& sec, int st, int house) {
        sector = sec;
        street = st;
        houseNo = house;
    }

    bool operator==(const Citizen& other) const {
        return cnic == other.cnic;
    }
};