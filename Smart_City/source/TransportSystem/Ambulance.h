

#pragma once
#include "Vehicle.h"
#include "../HousingSystem/Citizen.h"
#include <string>

using std::string;


namespace EmergencyPriority {
    const string CRITICAL = "CRITICAL";     
    const string HIGH = "HIGH";            
    const string MEDIUM = "MEDIUM";         
    const string LOW = "LOW";               
    const string ROUTINE = "ROUTINE";      
    
    inline int getValue(const string& priority) {
        if (priority == CRITICAL) return 1;
        if (priority == HIGH) return 2;
        if (priority == MEDIUM) return 3;
        if (priority == LOW) return 4;
        if (priority == ROUTINE) return 5;
        return 3; 
    }
}

namespace AmbulanceStatus {
    const string AVAILABLE = "AVAILABLE";          
    const string DISPATCHED = "DISPATCHED";        
    const string AT_PICKUP = "AT_PICKUP";          
    const string LOADING_PATIENT = "LOADING";      
    const string TRANSPORTING = "TRANSPORTING";     
    const string AT_DESTINATION = "AT_DESTINATION"; 
    const string UNLOADING = "UNLOADING";           
    const string RETURNING = "RETURNING";           
    const string OUT_OF_SERVICE = "OUT_OF_SERVICE"; 
}


struct PatientTransfer {
    string requestID;            
    string patientCNIC;            
    string patientName;
    
    string sourceHospitalID;
    int sourceHospitalNodeID;
    string sourceSector;
    
    string destHospitalID;
    int destHospitalNodeID;
    string destSector;
    
    string priority;              
    string condition;               
    string timestamp;
    bool isActive;
    
    PatientTransfer()
        : requestID(""), patientCNIC(""), patientName(""),
          sourceHospitalID(""), sourceHospitalNodeID(-1), sourceSector(""),
          destHospitalID(""), destHospitalNodeID(-1), destSector(""),
          priority(EmergencyPriority::MEDIUM), condition(""), 
          timestamp(""), isActive(true) {}
    
    PatientTransfer(const string& id, const string& cnic, const string& name,
                   const string& srcHosp, int srcNode, const string& srcSec,
                   const string& dstHosp, int dstNode, const string& dstSec,
                   const string& prio, const string& cond)
        : requestID(id), patientCNIC(cnic), patientName(name),
          sourceHospitalID(srcHosp), sourceHospitalNodeID(srcNode), sourceSector(srcSec),
          destHospitalID(dstHosp), destHospitalNodeID(dstNode), destSector(dstSec),
          priority(prio), condition(cond), timestamp(""), isActive(true) {}
    
    bool operator<(const PatientTransfer& other) const {
        return EmergencyPriority::getValue(priority) > EmergencyPriority::getValue(other.priority);
    }
    
    bool operator>(const PatientTransfer& other) const {
        return EmergencyPriority::getValue(priority) < EmergencyPriority::getValue(other.priority);
    }
    
    bool operator==(const PatientTransfer& other) const {
        return requestID == other.requestID;
    }
};

// ==================== Ambulance ====================
class Ambulance : public Vehicle {
private:
    string ambulanceID;             
    string baseHospitalID;          
    int baseHospitalNodeID;         
    string ambulanceStatus;         
    
    PatientTransfer* currentTransfer;
    
    bool hasALS;                   
    bool hasDefibrillator;
    bool hasOxygen;
    bool hasVentilator;
    
    int totalTransfersCompleted;
    int criticalTransfersHandled;
    double totalTransferDistance;
    
    Vector<string> prioritySectors;
    
    static int nextRequestID;

public:
    // ==================== LIFECYCLE ====================
    
    Ambulance()
        : Vehicle("", VehicleType::AMBULANCE, 1),  // 1 patient at a time
          ambulanceID(""), baseHospitalID(""), baseHospitalNodeID(-1),
          ambulanceStatus(AmbulanceStatus::AVAILABLE),
          currentTransfer(nullptr),
          hasALS(true), hasDefibrillator(true), hasOxygen(true), hasVentilator(false),
          totalTransfersCompleted(0), criticalTransfersHandled(0), 
          totalTransferDistance(0.0) {
        speed = 60.0; 
    }
    
    Ambulance(const string& id, const string& hospitalID, int hospitalNodeID, const string& sector)
        : Vehicle(id, VehicleType::AMBULANCE, 1),
          ambulanceID(id), baseHospitalID(hospitalID), baseHospitalNodeID(hospitalNodeID),
          ambulanceStatus(AmbulanceStatus::AVAILABLE),
          currentTransfer(nullptr),
          hasALS(true), hasDefibrillator(true), hasOxygen(true), hasVentilator(false),
          totalTransfersCompleted(0), criticalTransfersHandled(0), 
          totalTransferDistance(0.0) {
        currentNodeID = hospitalNodeID;
        homeSector = sector;
        homeNodeID = hospitalNodeID;
        speed = 60.0;
        
        setPrioritySectors(sector);
    }
    
    ~Ambulance() override {
        delete currentTransfer;
    }
    
    // ==================== GETTERS ====================

    
    string getAmbulanceID() const { return ambulanceID; }
    string getBaseHospitalID() const { return baseHospitalID; }
    int getBaseHospitalNodeID() const { return baseHospitalNodeID; }
    string getAmbulanceStatus() const { return ambulanceStatus; }
    PatientTransfer* getCurrentTransfer() const { return currentTransfer; }
    
    bool getHasALS() const { return hasALS; }
    bool getHasDefibrillator() const { return hasDefibrillator; }
    bool getHasOxygen() const { return hasOxygen; }
    bool getHasVentilator() const { return hasVentilator; }
    
    int getTotalTransfersCompleted() const { return totalTransfersCompleted; }
    int getCriticalTransfersHandled() const { return criticalTransfersHandled; }
    double getTotalTransferDistance() const { return totalTransferDistance; }
    const Vector<string>& getPrioritySectors() const { return prioritySectors; }
    
    bool isAvailable() const { return ambulanceStatus == AmbulanceStatus::AVAILABLE; }
    
    // ==================== SETTERS ====================
    
    void setAmbulanceStatus(const string& s) { 
        ambulanceStatus = s;
        if (s == AmbulanceStatus::AVAILABLE) {
            status = VehicleStatus::IDLE;
        } else if (s == AmbulanceStatus::DISPATCHED || 
                   s == AmbulanceStatus::TRANSPORTING ||
                   s == AmbulanceStatus::RETURNING) {
            status = VehicleStatus::EN_ROUTE;
        } else if (s == AmbulanceStatus::AT_PICKUP || 
                   s == AmbulanceStatus::AT_DESTINATION) {
            status = VehicleStatus::AT_STOP;
        } else if (s == AmbulanceStatus::LOADING_PATIENT ||
                   s == AmbulanceStatus::UNLOADING) {
            status = VehicleStatus::BOARDING;
        } else if (s == AmbulanceStatus::OUT_OF_SERVICE) {
            status = VehicleStatus::MAINTENANCE;
        }
    }
    
    void setEquipment(bool als, bool defib, bool oxygen, bool vent) {
        hasALS = als;
        hasDefibrillator = defib;
        hasOxygen = oxygen;
        hasVentilator = vent;
    }
    
    // ==================== SECTOR PRIORITY ====================
    
    void setPrioritySectors(const string& sector) {
        prioritySectors.clear();
        prioritySectors.push_back(sector);
        
        char series = sector[0];
        int number = 0;
        
        string numStr = "";
        for (int i = 2; i < (int)sector.length(); ++i) {
            numStr += sector[i];
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
    
    bool shouldHandleTransfer(const PatientTransfer& transfer) const {
        
		return isSectorInPriority(transfer.sourceSector) || 
               isSectorInPriority(transfer.destSector);
    }
    
    // ==================== TRANSFER OPERATIONS ====================
    
    void resetToBase() {
        currentNodeID = baseHospitalNodeID;
        currentSector = homeSector;
        currentOccupancy = 0;
        resetRoute();
        setAmbulanceStatus(AmbulanceStatus::AVAILABLE);
        
        if (currentTransfer) {
            delete currentTransfer;
            currentTransfer = nullptr;
        }
    }
    
    bool acceptTransfer(PatientTransfer* transfer) {
        if (!isAvailable() || transfer == nullptr) return false;
        
        currentTransfer = new PatientTransfer(*transfer);
        setAmbulanceStatus(AmbulanceStatus::DISPATCHED);
        
        if (currentTransfer->priority == EmergencyPriority::CRITICAL) {
            ++criticalTransfersHandled;
        }
        
        return true;
    }
    
    void arriveAtPickup() {
        if (ambulanceStatus != AmbulanceStatus::DISPATCHED) return;
        if (!currentTransfer) return;
        
        currentNodeID = currentTransfer->sourceHospitalNodeID;
        currentSector = currentTransfer->sourceSector;
        setAmbulanceStatus(AmbulanceStatus::AT_PICKUP);
    }
    
    bool loadPatient() {
        if (ambulanceStatus != AmbulanceStatus::AT_PICKUP) return false;
        if (currentOccupancy >= maxCapacity) return false;
        
        setAmbulanceStatus(AmbulanceStatus::LOADING_PATIENT);
        ++currentOccupancy;
        setAmbulanceStatus(AmbulanceStatus::TRANSPORTING);
        
        return true;
    }
    
    void startTransport() {
        if (ambulanceStatus != AmbulanceStatus::LOADING_PATIENT &&
            ambulanceStatus != AmbulanceStatus::AT_PICKUP) return;
        
        setAmbulanceStatus(AmbulanceStatus::TRANSPORTING);
    }
    
    void arriveAtDestination() {
        if (ambulanceStatus != AmbulanceStatus::TRANSPORTING) return;
        if (!currentTransfer) return;
        
        currentNodeID = currentTransfer->destHospitalNodeID;
        currentSector = currentTransfer->destSector;
        setAmbulanceStatus(AmbulanceStatus::AT_DESTINATION);
    }
    
    bool unloadPatient() {
        if (ambulanceStatus != AmbulanceStatus::AT_DESTINATION) return false;
        
        setAmbulanceStatus(AmbulanceStatus::UNLOADING);
        --currentOccupancy;
        ++totalTransfersCompleted;
        totalTransferDistance += distanceTraveled;
        
        return true;
    }
    
    void completeTransfer() {
        if (ambulanceStatus != AmbulanceStatus::UNLOADING &&
            ambulanceStatus != AmbulanceStatus::AT_DESTINATION) return;
        
        delete currentTransfer;
        currentTransfer = nullptr;
        
        setAmbulanceStatus(AmbulanceStatus::RETURNING);
    }
    
    void returnToBase() {
        setAmbulanceStatus(AmbulanceStatus::RETURNING);
    }
    
    void arriveAtBase() {
        currentNodeID = baseHospitalNodeID;
        currentSector = homeSector;
        resetRoute();
        setAmbulanceStatus(AmbulanceStatus::AVAILABLE);
    }
    
    void takeOutOfService() {
        setAmbulanceStatus(AmbulanceStatus::OUT_OF_SERVICE);
    }
    
    void putInService() {
        if (ambulanceStatus == AmbulanceStatus::OUT_OF_SERVICE) {
            setAmbulanceStatus(AmbulanceStatus::AVAILABLE);
        }
    }
    
    // ==================== UTILITY ====================
    
    static string generateRequestID() {
        return "XFER-" + std::to_string(++nextRequestID);
    }
    
    int getCurrentDestination() const {
        if (!currentTransfer) return baseHospitalNodeID;
        
        if (ambulanceStatus == AmbulanceStatus::DISPATCHED) {
            return currentTransfer->sourceHospitalNodeID;
        } else if (ambulanceStatus == AmbulanceStatus::TRANSPORTING) {
            return currentTransfer->destHospitalNodeID;
        } else if (ambulanceStatus == AmbulanceStatus::RETURNING) {
            return baseHospitalNodeID;
        }
        
        return currentNodeID;
    }
    
    string getStatusString() const {
        return ambulanceStatus;
    }
};

inline int Ambulance::nextRequestID = 1000;
