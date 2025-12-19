

#pragma once
#include "Vehicle.h"
#include "../../data_structures/CircularQueue.h"
#include <string>

using std::string;

struct Passenger {
    string citizenCNIC;
    int boardingStopID;
    int destinationStopID;
    double fare;
    
    Passenger() 
        : citizenCNIC(""), boardingStopID(-1), destinationStopID(-1), fare(0.0) {}
    
    Passenger(const string& cnic, int boarding, int destination, double f = 50.0)
        : citizenCNIC(cnic), boardingStopID(boarding), destinationStopID(destination), fare(f) {}
    
    bool operator==(const Passenger& other) const {
        return citizenCNIC == other.citizenCNIC;
    }
};

class Bus : public Vehicle {
private:
    string busNo;              
    string company;             
    string routeName;         
    
    string startStopID;
    string endStopID;
    
    CircularQueue<Passenger> waitingQueue;
    Vector<Passenger> onboardPassengers;
    
    int departureIntervalMinutes;
    bool isRoundTrip;
    
    int totalPassengersServed;
    double totalFareCollected;
    int tripsCompleted;

public:
    // ==================== Constructores ====================
    
    Bus() 
        : Vehicle("", VehicleType::BUS, 50),
          busNo(""), company(""), routeName(""),
          startStopID(""), endStopID(""),
          waitingQueue(100), onboardPassengers(),
          departureIntervalMinutes(15), isRoundTrip(true),
          totalPassengersServed(0), totalFareCollected(0.0), tripsCompleted(0) {}
    
    Bus(const string& busNo, const string& company, const string& currentStop)
        : Vehicle(busNo, VehicleType::BUS, 50),
          busNo(busNo), company(company), routeName(""),
          startStopID(""), endStopID(""),
          waitingQueue(100), onboardPassengers(),
          departureIntervalMinutes(15), isRoundTrip(true),
          totalPassengersServed(0), totalFareCollected(0.0), tripsCompleted(0) {
        currentStopName = currentStop;
    }
    
    Bus(const Bus& other) 
        : Vehicle(other),
          busNo(other.busNo), company(other.company), routeName(other.routeName),
          startStopID(other.startStopID), endStopID(other.endStopID),
          waitingQueue(other.waitingQueue), onboardPassengers(other.onboardPassengers),
          departureIntervalMinutes(other.departureIntervalMinutes), 
          isRoundTrip(other.isRoundTrip),
          totalPassengersServed(other.totalPassengersServed),
          totalFareCollected(other.totalFareCollected),
          tripsCompleted(other.tripsCompleted) {}
    
    Bus& operator=(const Bus& other) {
        if (this != &other) {
            Vehicle::operator=(other);
            busNo = other.busNo;
            company = other.company;
            routeName = other.routeName;
            startStopID = other.startStopID;
            endStopID = other.endStopID;
            waitingQueue = other.waitingQueue;
            onboardPassengers = other.onboardPassengers;
            departureIntervalMinutes = other.departureIntervalMinutes;
            isRoundTrip = other.isRoundTrip;
            totalPassengersServed = other.totalPassengersServed;
            totalFareCollected = other.totalFareCollected;
            tripsCompleted = other.tripsCompleted;
        }
        return *this;
    }
    
    ~Bus() override = default;
    
    // ==================== GETTERS ====================

    string getBusNo() const { return busNo; }
    string getCompany() const { return company; }
    string getRouteName() const { return routeName; }
    string getStartStopID() const { return startStopID; }
    string getEndStopID() const { return endStopID; }
    int getDepartureInterval() const { return departureIntervalMinutes; }
    bool getIsRoundTrip() const { return isRoundTrip; }
    int getTotalPassengersServed() const { return totalPassengersServed; }
    double getTotalFareCollected() const { return totalFareCollected; }
    int getTripsCompleted() const { return tripsCompleted; }
    int getWaitingPassengerCount() const { return waitingQueue.size(); }
    int getOnboardCount() const { return onboardPassengers.getSize(); }
    
    string getCurrentStop() const { return currentStopName; }
    int getStopCount() const { return route.size(); }
    
    // ==================== SETTERS ====================
    
    void setBusNo(const string& no) { busNo = no; vehicleID = no; }
    void setCompany(const string& c) { company = c; }
    void setRouteName(const string& name) { routeName = name; }
    void setDepartureInterval(int minutes) { departureIntervalMinutes = minutes; }
    void setIsRoundTrip(bool roundTrip) { isRoundTrip = roundTrip; }
    
    void setStops(const string& start, const string& end) {
        startStopID = start;
        endStopID = end;
        routeName = start + " to " + end;
    }
    
    void setRoute(const Vector<int>& newRoute, double distance) {
        setRouteSimple(newRoute, distance);
    }
    
    // ==================== OPERATIONS ====================
    
    bool addWaitingPassenger(const Passenger& p) {
        return waitingQueue.enqueue(p);
    }
    
    int boardWaitingPassengers() {
        int boarded = 0;
        
        while (!waitingQueue.empty() && !isFull()) {
            Passenger p = waitingQueue.dequeue();
            
            int currentPos = getRoutePosition(currentNodeID);
            int destPos = getRoutePosition(p.destinationStopID);
            
            if (destPos > currentPos) {
                onboardPassengers.push_back(p);
                ++currentOccupancy;
                totalFareCollected += p.fare;
                ++boarded;
            }
        }
        
        return boarded;
    }
    
    int alightPassengers() {
        int alighted = 0;
        Vector<Passenger> remaining;
        
        for (int i = 0; i < onboardPassengers.getSize(); ++i) {
            if (onboardPassengers[i].destinationStopID == currentNodeID) {
                ++alighted;
                ++totalPassengersServed;
                --currentOccupancy;
            } else {
                remaining.push_back(onboardPassengers[i]);
            }
        }
        
        onboardPassengers = remaining;
        return alighted;
    }
    
    void processStop() {
        status = VehicleStatus::BOARDING;
        alightPassengers();   
        boardWaitingPassengers();
        status = VehicleStatus::AT_STOP;
    }
    
    // ==================== OPERATIONS ====================
    
    void resetToRouteStart() {
        resetRoute();
        tripsCompleted = 0;
        totalPassengersServed = 0;
        totalFareCollected = 0.0;
        onboardPassengers.clear();
        currentOccupancy = 0;
        status = VehicleStatus::AT_STOP;
    }
    
    void completeTrip() {
        ++tripsCompleted;
        
        if (isRoundTrip) {
            route.reverse();
            
            string temp = startStopID;
            startStopID = endStopID;
            endStopID = temp;
        }
        
        resetRoute();
    }
    
    bool moveToNextStop() override {
        if (Vehicle::moveToNextStop()) {
            return true;
        }
        
        if (isRoundTrip) {
            completeTrip();
            return true;
        }
        
        return false;
    }
};
