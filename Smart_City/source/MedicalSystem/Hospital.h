#pragma once
#include <string>
#include "customSTL.h"
#include "ModuleUtils.h"
#include "Patient.h"

using std::string;


class Hospital {
public:
    string id;
    string name;
    string sector;

    int availableBeds;

    // NEW: actual records of patients currently in beds
    Vector<Patient> admittedPatients;

    // Specializations
    Vector<string> specializations;

    // EMERGENCY ROOM (ER) QUEUE (Min-Heap / Priority Queue)
    PriorityQueue<Patient> emergencyRoom;

    Location location;
    string graphNodeID;

    Hospital() : id(""), name(""), sector(""), availableBeds(0), location(), graphNodeID("") {}

    Hospital(string id, string name, string sector, int beds, string graphNodeID = "", double x = 0.0, double y = 0.0)
        : id(id), name(name), sector(sector),
          availableBeds(beds),
          location(sector, x, y), graphNodeID(graphNodeID) {
}

    // --- Core Hospital Logic ---

    // Property Getter for compatibility
    int getAvailableBeds() const {
        return availableBeds - admittedPatients.getSize();
    }

    // Attempts to admit a patient.
    bool admitPatient(const Patient& p) {
        if (getAvailableBeds() > 0) {
            admittedPatients.push_back(p);
            return true;
        }
 else {
            // No beds available. Add to Min-Heap.
            emergencyRoom.push(p);
            return false;
        }
    }

    // NEW: Discharge a SPECIFIC patient by ID
    bool dischargePatient(const string& patientID) {
        // 1. Find and remove from admitted list
        bool found = false;
        for (int i = 0; i < admittedPatients.getSize(); i++) {
            if (admittedPatients[i].id == patientID) {
                // Remove from vector (shift remaining)
                // Using re::Vector's remove logic manually or if implemented
                // For now, simple swap-to-end removal or shift
                for (int j = i; j < admittedPatients.getSize() - 1; j++) {
                    admittedPatients[j] = admittedPatients[j + 1];
                }
                admittedPatients.pop_back();
                found = true;
                break;
            }
        }

        if (found) {
            // 2. Auto-Admit from ER if anyone is waiting
            if (!emergencyRoom.empty()) {
                Patient next = emergencyRoom.top();
                emergencyRoom.pop();
                admittedPatients.push_back(next);
            }
            return true;
        }
        return false; // Patient not found
    }

    // Overload: Discharge the "oldest" or first patient (Simple mode)
    bool dischargePatient() {
        if (admittedPatients.getSize() > 0) {
            // Remove the first patient (FIFO for simple discharge simulation)
            // Ideally this would be specific, but for simulation flow:
            for (int j = 0; j < admittedPatients.getSize() - 1; j++) {
                admittedPatients[j] = admittedPatients[j + 1];
            }
            admittedPatients.pop_back();

            // Auto-Admit from ER
            if (!emergencyRoom.empty()) {
                Patient next = emergencyRoom.top();
                emergencyRoom.pop();
                admittedPatients.push_back(next);
            }
            return true;
        }
        return false;
    }

    // NEW: Find a patient record inside this hospital
    Patient* findPatient(const string& pID) {
        // Check Admitted
        for (int i = 0; i < admittedPatients.getSize(); i++) {
            if (admittedPatients[i].id == pID) return &admittedPatients[i];
        }
        // Note: We typically don't search the Heap (ER) randomly as it breaks heap structure,
        // but for a "Record Search", we assume they are either admitted or we can't find them easily in the heap.
        return nullptr;
    }

    void addSpecialization(const string& spec) {
        specializations.push_back(spec);
    }

    bool hasSpecialization(const string& spec) const {
        for (int i = 0; i < specializations.getSize(); i++) {
            if (specializations[i] == spec) return true;
        }
        return false;
    }
};