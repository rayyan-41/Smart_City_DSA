#pragma once
#include <string>
#include "../HousingSystem/Citizen.h" 
#include "../../utils/ModuleUtils.h"

using std::string;

struct Patient {
    Citizen* profile;

    string id; 
    string disease;

    int severity;

    Location emergencyLocation;

    Patient() : profile(nullptr), id(IDGenerator::generatePatientID()), disease(""), severity(10) {}

    Patient(Citizen* c, string dis, int sev)
        : profile(c), id(IDGenerator::generatePatientID()), disease(dis), severity(sev) {
        if (c) {
            c->currentStatus = "Hospitalized"; 
        }
    }

    // ==================== GETTERS ====================
    string getId() const { return id; }
    Citizen* getProfile() const { return profile; }
    string getDisease() const { return disease; }
    int getSeverity() const { return severity; }
    const Location& getEmergencyLocation() const { return emergencyLocation; }
    
    string getName() const { return profile ? profile->name : "Unknown"; }
    string getCNIC() const { return profile ? profile->cnic : ""; }
    int getAge() const { return profile ? profile->age : 0; }
    string getSector() const { return profile ? profile->sector : ""; }
    
    string getSeverityString() const {
        if (severity <= 2) return "Critical";
        if (severity <= 4) return "Serious";
        if (severity <= 6) return "Moderate";
        if (severity <= 8) return "Minor";
        return "Stable";
    }
    
    bool isCritical() const { return severity <= 2; }
    bool isSerious() const { return severity <= 4; }

    // ==================== SETTERS ====================
    void setId(const string& newId) { id = newId; }
    void setProfile(Citizen* c) { profile = c; }
    void setDisease(const string& dis) { disease = dis; }
    void setSeverity(int sev) { severity = sev; }
    void setEmergencyLocation(const Location& loc) { emergencyLocation = loc; }
    
    void updateStatus(const string& status) {
        if (profile) profile->currentStatus = status;
    }

    bool operator<(const Patient& other) const {
        return severity > other.severity; // Critical (1) > Stable (10)
    }

    bool operator>(const Patient& other) const {
        return severity < other.severity;
    }

    bool operator==(const Patient& other) const {
        return id == other.id;
    }
};