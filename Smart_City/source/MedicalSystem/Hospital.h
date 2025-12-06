#pragma once
#include <string>
#include "customSTL.h"
#include "ModuleUtils.h"
#include "Patient.h"
#include "Doctor.h"
#include "../HousingSystem/PopulationManager.h"

using std::string;


class Hospital {
public:
    string id;
    string name;
    string sector;

    int totalBeds;
    // Stores actual Patient objects (which contain Citizen* pointers)
    Vector<Patient> admittedPatients;

    Vector<Doctor> doctors;
    Vector<string> specializations;

    // EMERGENCY ROOM (ER) QUEUE (Min-Heap / Priority Queue)
    // Most critical patients bubble to the top
    PriorityQueue<Patient> emergencyRoom;

    Location location;
    string graphNodeID;

    Hospital() : id(""), name(""), sector(""), totalBeds(0), location(), graphNodeID("") {}

    Hospital(string id, string name, string sector, int beds, string graphNodeID = "", double x = 0.0, double y = 0.0)
        : id(id), name(name), sector(sector),
        totalBeds(beds),
        location(sector, x, y), graphNodeID(graphNodeID) {
    }

    // --- Core Hospital Logic ---

    int getAvailableBeds() const {
        return totalBeds - admittedPatients.getSize();
    }

    // Attempts to admit a patient
    bool admitPatient(const Patient& p) {
        if (getAvailableBeds() > 0) {
            admittedPatients.push_back(p);
            return true; // Admitted to bed
        }
        else {
            emergencyRoom.push(p);
            return false; // Queued in ER
        }
    }

    // Batch Arrival (e.g., from an Ambulance/Bus)
    void processAmbulanceArrival(const Vector<Patient>& victims) {
        for (int i = 0; i < victims.getSize(); i++) {
            admitPatient(victims[i]);
        }
    }

    // --- Disaster Simulation Logic ---

   // Calculates death rate (40-60%) for CRITICAL patients (Severity 1)
   // Removes them from Hospital AND Population Manager
    //void simulateDisasterCasualties(PopulationManager* popMgr) {
    //    if (!popMgr) return;

    //    // 1. Identify Critical Patients (Severity == 1)
    //    Vector<Patient> criticalList;
    //    for (int i = 0; i < admittedPatients.getSize(); i++) {
    //        if (admittedPatients[i].severity == 1) {
    //            criticalList.push_back(admittedPatients[i]);
    //        }
    //    }

    //    if (criticalList.getSize() == 0) return;

    //    // 2. Calculate Casualties (Random 40-60%)
    //    int percent = 40 + (rand() % 21); // 40 to 60
    //    int deathCount = (criticalList.getSize() * percent) / 100;
    //    if (deathCount == 0 && criticalList.getSize() > 0) deathCount = 1;

    //    // 3. Process Deaths
    //    // We collect the CNICs first. We CANNOT delete from PopulationManager yet,
    //    // because we need the Citizen pointers to stay valid while we remove them from admittedPatients.
    //    Vector<string> deceasedCNICs;

    //    for (int k = 0; k < deathCount; k++) {
    //        Patient deceased = criticalList[k];
    //        // std::cout << " [Alert] Patient " << deceased.getName() << " (Severity: 1) has passed away." << std::endl;

    //        // Store CNIC for later deletion from Population System
    //        string cnic = deceased.getCNIC();
    //        if (!cnic.empty()) deceasedCNICs.push_back(cnic);
    //    }

    //    // 4. Update Hospital Records (Remove deceased from admittedPatients)
    //    // We rebuild the list, excluding those who died.
    //    Vector<Patient> survivors;
    //    for (int i = 0; i < admittedPatients.getSize(); i++) {
    //        bool isDeceased = false;
    //        string currentCNIC = admittedPatients[i].getCNIC();

    //        for (int j = 0; j < deceasedCNICs.getSize(); j++) {
    //            if (currentCNIC == deceasedCNICs[j]) {
    //                isDeceased = true;
    //                break;
    //            }
    //        }

    //        if (!isDeceased) {
    //            survivors.push_back(admittedPatients[i]);
    //        }
    //    }

    //    // Replace admitted list with survivors
    //    // (Manual copy because we don't have a simple vector assignment overload setup for this scenario in context)
    //    admittedPatients.clear();
    //    for (int i = 0; i < survivors.getSize(); i++) {
    //        admittedPatients.push_back(survivors[i]);
    //    }

    //    // 5. FINAL STEP: Remove from Population Manager
    //    // Now that Hospital no longer holds pointers to them, it is safe to delete the memory.
    //    for (int i = 0; i < deceasedCNICs.getSize(); i++) {
    //        popMgr->removeCitizen(deceasedCNICs[i]);
    //    }
    //}

    // Discharge
    bool dischargePatient() {
        if (admittedPatients.getSize() > 0) {
            // First in First Out discharge
            Patient p = admittedPatients[0];
            if (p.profile) p.profile->currentStatus = "Home"; // Recovered

            // Shift Logic
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

    bool dischargePatient(const string& pID) {
        for (int i = 0; i < admittedPatients.getSize(); i++) {
            if (admittedPatients[i].id == pID) {
                Patient p = admittedPatients[i];
                if (p.profile) p.profile->currentStatus = "Home"; // Recovered
                // Shift Logic
                for (int j = i; j < admittedPatients.getSize() - 1; j++) {
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
        }
        return false;
	}

	//bool killPatient(const string& pID, PopulationManager* popMgr) {
 //       for (int i = 0; i < admittedPatients.getSize(); i++) {
 //           if (admittedPatients[i].id == pID) {
 //               Patient p = admittedPatients[i];
 //               string cnic = p.getCNIC();
 //               // Shift Logic
 //               for (int j = i; j < admittedPatients.getSize() - 1; j++) {
 //                   admittedPatients[j] = admittedPatients[j + 1];
 //               }
 //               admittedPatients.pop_back();
 //               // Auto-Admit from ER
 //               if (!emergencyRoom.empty()) {
 //                   Patient next = emergencyRoom.top();
 //                   emergencyRoom.pop();
 //                   admittedPatients.push_back(next);
 //               }
 //               // Remove from Population Manager
 //               if (popMgr && !cnic.empty()) {
 //                   popMgr->removeCitizen(cnic);
 //               }
 //               return true;
 //           }
 //       }
 //       return false;
 //   }

    Vector<Patient> getAdmittedPatients() const {
        return admittedPatients;
	}

    Patient* findPatient(const string& pID) {
        for (int i = 0; i < admittedPatients.getSize(); i++) {
            if (admittedPatients[i].id == pID) return &admittedPatients[i];
        }
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