#pragma once
#include <string>
#include "../HousingSystem/Citizen.h" 
#include "ModuleUtils.h"

using std::string;

struct Patient {
    // POINTER to the real person in the Population System
    Citizen* profile;

    // Medical-specific data
    string id; // Internal Hospital ID (e.g., "P-101")
    string disease;

    // Severity Level: 1 (Critical) to 10 (Stable)
    int severity;

    // Graph Location (for Ambulance routing)
    Location emergencyLocation;

    // Constructors
    Patient() : profile(nullptr), id(IDGenerator::generatePatientID()), disease(""), severity(10) {}

    Patient(Citizen* c, string dis, int sev)
        : profile(c), id(IDGenerator::genetatePatientID()), disease(dis), severity(sev) {
        if (c) {
            c->currentStatus = "Hospitalized"; // Update status in Population System
        }
    }

    // ---------------------------------------------------------
    // OPERATOR OVERLOADING FOR PRIORITY QUEUE
    // ---------------------------------------------------------

    // Logic: We want Severity 1 (Critical) to be at the TOP.
    // If the underlying heap is a Max-Heap, we define 1 > 10.

    bool operator<(const Patient& other) const {
        return severity > other.severity; // Critical (1) > Stable (10)
    }

    bool operator>(const Patient& other) const {
        return severity < other.severity;
    }

    bool operator==(const Patient& other) const {
        return id == other.id;
    }

    // Helper accessors
    string getName() const { return profile ? profile->name : "Unknown"; }
    string getCNIC() const { return profile ? profile->cnic : ""; }
};