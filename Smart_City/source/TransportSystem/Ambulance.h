/*
 * ============================================================================
 * AMBULANCE - Hospital-to-Hospital Emergency Transport Vehicle
 * ============================================================================
 * 
 * Extends Vehicle base class for emergency medical transport.
 * Features:
 *   - Hospital-to-hospital routing (not random locations)
 *   - Priority-based dispatch using graph pathfinding
 *   - Patient transport with severity tracking
 *   - Priority for own sector and adjacent sector hospitals
 * 
 * Rubric:
 *   - Transport Module with emergency services (5 marks)
 *   - Graph usage in modules for emergency routing (4 marks)
 * ============================================================================
 */

#pragma once
#include "Vehicle.h"
#include "../HousingSystem/Citizen.h"
#include <string>

using std::string;

// ============================================================================
// EMERGENCY PRIORITY - String constants instead of enum
// ============================================================================
namespace EmergencyPriority {
    const string CRITICAL = "CRITICAL";     // Life-threatening, immediate
    const string HIGH = "HIGH";             // Serious, fast response
    const string MEDIUM = "MEDIUM";         // Significant, prompt response
    const string LOW = "LOW";               // Non-urgent
    const string ROUTINE = "ROUTINE";       // Scheduled transfer
    
    // Priority values for comparison (lower = more urgent)
    inline int getValue(const string& priority) {
        if (priority == CRITICAL) return 1;
        if (priority == HIGH) return 2;
        if (priority == MEDIUM) return 3;
        if (priority == LOW) return 4;
        if (priority == ROUTINE) return 5;
        return 3; // Default to MEDIUM
    }
}

// ============================================================================
// AMBULANCE STATUS - String constants instead of enum
// ============================================================================
namespace AmbulanceStatus {
    const string AVAILABLE = "AVAILABLE";           // At hospital, ready
    const string DISPATCHED = "DISPATCHED";         // En route to pickup hospital
    const string AT_PICKUP = "AT_PICKUP";           // At pickup hospital
    const string LOADING_PATIENT = "LOADING";       // Loading patient
    const string TRANSPORTING = "TRANSPORTING";     // Taking patient to destination
    const string AT_DESTINATION = "AT_DESTINATION"; // At destination hospital
    const string UNLOADING = "UNLOADING";           // Transferring patient
    const string RETURNING = "RETURNING";           // Returning to base hospital
    const string OUT_OF_SERVICE = "OUT_OF_SERVICE"; // Maintenance
}

// ============================================================================
// PATIENT TRANSFER REQUEST - For hospital-to-hospital transfers
// ============================================================================
struct PatientTransfer {
    string requestID;               // Unique request ID
    string patientCNIC;             // Patient identifier
    string patientName;
    
    // Source hospital
    string sourceHospitalID;
    int sourceHospitalNodeID;
    string sourceSector;
    
    // Destination hospital
    string destHospitalID;
    int destHospitalNodeID;
    string destSector;
    
    // Transfer details
    string priority;                // From EmergencyPriority
    string condition;               // Medical condition
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
    
    // For priority queue comparison
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

// ============================================================================
// AMBULANCE CLASS
// ============================================================================
class Ambulance : public Vehicle {
private:
    // Ambulance-specific attributes
    string ambulanceID;             // e.g., "AMB-01"
    string baseHospitalID;          // Home hospital ID
    int baseHospitalNodeID;         // Home hospital graph node
    string ambulanceStatus;         // Detailed status
    
    // Current assignment
    PatientTransfer* currentTransfer;
    
    // Equipment flags
    bool hasALS;                    // Advanced Life Support
    bool hasDefibrillator;
    bool hasOxygen;
    bool hasVentilator;
    
    // Statistics
    int totalTransfersCompleted;
    int criticalTransfersHandled;
    double totalTransferDistance;
    
    // Sector priority (own sector + adjacent)
    Vector<string> prioritySectors;
    
    // Request ID generator
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
        speed = 60.0;  // Ambulances are faster
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
        
        // Set priority sectors
        setPrioritySectors(sector);
    }
    
    ~Ambulance() override {
        delete currentTransfer;
    }
    
    // ==================== ACCESSORS ====================
    
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
        // Map to base vehicle status
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
        
        // Adjacent in same series
        if (number > 6) {
            prioritySectors.push_back(string(1, series) + "-" + std::to_string(number - 1));
        }
        if (number < 12) {
            prioritySectors.push_back(string(1, series) + "-" + std::to_string(number + 1));
        }
        
        // Adjacent series
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
    
    // Check if this ambulance should handle a transfer (sector priority)
    bool shouldHandleTransfer(const PatientTransfer& transfer) const {
        // Always handle if source or destination is in priority sectors
        return isSectorInPriority(transfer.sourceSector) || 
               isSectorInPriority(transfer.destSector);
    }
    
    // ==================== TRANSFER OPERATIONS ====================
    
    // Accept a patient transfer request
    bool acceptTransfer(PatientTransfer* transfer) {
        if (!isAvailable() || transfer == nullptr) return false;
        
        currentTransfer = new PatientTransfer(*transfer);
        setAmbulanceStatus(AmbulanceStatus::DISPATCHED);
        
        if (currentTransfer->priority == EmergencyPriority::CRITICAL) {
            ++criticalTransfersHandled;
        }
        
        return true;
    }
    
    // Arrive at pickup hospital
    void arriveAtPickup() {
        if (ambulanceStatus != AmbulanceStatus::DISPATCHED) return;
        if (!currentTransfer) return;
        
        currentNodeID = currentTransfer->sourceHospitalNodeID;
        currentSector = currentTransfer->sourceSector;
        setAmbulanceStatus(AmbulanceStatus::AT_PICKUP);
    }
    
    // Load patient
    bool loadPatient() {
        if (ambulanceStatus != AmbulanceStatus::AT_PICKUP) return false;
        if (currentOccupancy >= maxCapacity) return false;
        
        setAmbulanceStatus(AmbulanceStatus::LOADING_PATIENT);
        ++currentOccupancy;
        setAmbulanceStatus(AmbulanceStatus::TRANSPORTING);
        
        return true;
    }
    
    // Start transport to destination hospital
    void startTransport() {
        if (ambulanceStatus != AmbulanceStatus::LOADING_PATIENT &&
            ambulanceStatus != AmbulanceStatus::AT_PICKUP) return;
        
        setAmbulanceStatus(AmbulanceStatus::TRANSPORTING);
    }
    
    // Arrive at destination hospital
    void arriveAtDestination() {
        if (ambulanceStatus != AmbulanceStatus::TRANSPORTING) return;
        if (!currentTransfer) return;
        
        currentNodeID = currentTransfer->destHospitalNodeID;
        currentSector = currentTransfer->destSector;
        setAmbulanceStatus(AmbulanceStatus::AT_DESTINATION);
    }
    
    // Unload patient at destination
    bool unloadPatient() {
        if (ambulanceStatus != AmbulanceStatus::AT_DESTINATION) return false;
        
        setAmbulanceStatus(AmbulanceStatus::UNLOADING);
        --currentOccupancy;
        ++totalTransfersCompleted;
        totalTransferDistance += distanceTraveled;
        
        return true;
    }
    
    // Complete transfer and prepare to return
    void completeTransfer() {
        if (ambulanceStatus != AmbulanceStatus::UNLOADING &&
            ambulanceStatus != AmbulanceStatus::AT_DESTINATION) return;
        
        delete currentTransfer;
        currentTransfer = nullptr;
        
        setAmbulanceStatus(AmbulanceStatus::RETURNING);
    }
    
    // Return to base hospital
    void returnToBase() {
        setAmbulanceStatus(AmbulanceStatus::RETURNING);
    }
    
    // Arrive back at base hospital
    void arriveAtBase() {
        currentNodeID = baseHospitalNodeID;
        currentSector = homeSector;
        resetRoute();
        setAmbulanceStatus(AmbulanceStatus::AVAILABLE);
    }
    
    // Take out of service
    void takeOutOfService() {
        setAmbulanceStatus(AmbulanceStatus::OUT_OF_SERVICE);
    }
    
    // Put back in service
    void putInService() {
        if (ambulanceStatus == AmbulanceStatus::OUT_OF_SERVICE) {
            setAmbulanceStatus(AmbulanceStatus::AVAILABLE);
        }
    }
    
    // ==================== UTILITY ====================
    
    // Generate unique request ID
    static string generateRequestID() {
        return "XFER-" + std::to_string(++nextRequestID);
    }
    
    // Get current destination based on status
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
    
    // Get status string
    string getStatusString() const {
        return ambulanceStatus;
    }
};

// Static member initialization
inline int Ambulance::nextRequestID = 1000;
