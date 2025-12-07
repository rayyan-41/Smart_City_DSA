/*
 * ============================================================================
 * MEDICAL MANAGER - Healthcare System Controller
 * ============================================================================
 * 
 * PURPOSE:
 * Manages all healthcare facilities in the Smart City, including hospitals
 * with emergency queues and pharmacies with medicine inventory. Provides
 * O(1) medicine lookups by name and formula, priority-based emergency
 * patient admission, and cross-hospital patient tracking.
 * 
 * DATA STRUCTURES USED:
 *   - Vector<Hospital*>: All hospitals for iteration
 *   - Vector<Pharmacy*>: All pharmacies for iteration
 *   - HashTable<string, Hospital*>: O(1) hospital by ID
 *   - HashTable<string, Pharmacy*>: O(1) pharmacy by ID
 *   - HashTable<string, Vector<Pharmacy*>>: O(1) pharmacies by medicine name
 *   - HashTable<string, Vector<Pharmacy*>>: O(1) pharmacies by formula
 *   - Priority Queue (in Hospital): Emergency patient queue
 * 
 * ============================================================================
 */

#pragma once
#include <fstream>
#include <string>
#include "../../data_structures/CustomSTL.h"
#include "Hospital.h"
#include "Pharmacy.h"

using std::string;
using std::ifstream;
using std::endl;

class MedicalManager {
public:
    Vector<Hospital*> hospitals;
    Vector<Pharmacy*> pharmacies;
    HashTable<string, Hospital*> hospitalLookup;
    HashTable<string, Pharmacy*> pharmacyIdLookup;
    HashTable<string, Vector<Pharmacy*>> medicineLookup;
    HashTable<string, Vector<Pharmacy*>> formulaLookup;

    MedicalManager();
    ~MedicalManager();

    bool loadHospitals(const string& filename);
    bool loadPharmacies(const string& filename);
    // ==================== GETTERS ====================

    Hospital* findHospitalByID(const string& id) const;
    Vector<Pharmacy*> findMedicine(const string& medName) const;
    Vector<Pharmacy*> findMedicineByFormula(const string& formula) const;
    Hospital* findPatientRecord(const string& patientID) const;
    bool processEmergency(const string& hospitalID, const Patient& p);

    // ==================== SETTERS ====================

    bool addPatient(const string& hospitalID, const Patient& p);
    bool addPatient(const string& hospitalID, Citizen* citizen, string disease, int severity);
    bool removePatient(const string& hospitalID, const string& patientID);
    bool addDoctor(const string& hospitalID, Citizen* citizen, const string& specialization);
    bool removeDoctor(const string& hospitalID, const string& doctorID);

private:
    string trim(const string& s) const;
};

// ==================== Implemenation ====================


inline MedicalManager::MedicalManager()
    : hospitalLookup(53), pharmacyIdLookup(53), medicineLookup(200), formulaLookup(100) {}

inline MedicalManager::~MedicalManager() {
    for (int i = 0; i < hospitals.getSize(); i++) delete hospitals[i];
    for (int i = 0; i < pharmacies.getSize(); i++) delete pharmacies[i];
}

inline bool MedicalManager::loadHospitals(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) return false;
    string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        string fields[5];
        int idx = 0; string cur = "";
        for (int i = 0; i < (int)line.size(); i++) {
            if (line[i] == ',' && idx < 4) { fields[idx++] = trim(cur); cur.clear(); }
            else cur += line[i];
        }
        fields[idx] = trim(cur);
        string id = fields[0]; string name = fields[1]; string sector = fields[2];
        int beds = 0; if (!fields[3].empty()) beds = std::stoi(fields[3]);
        string specsRaw = fields[4];
        Hospital* h = new Hospital(id, name, sector, beds);
        if (!specsRaw.empty() && specsRaw.front() == '"') specsRaw = specsRaw.substr(1, specsRaw.size() - 2);
        string curSpec = "";
        for (char c : specsRaw) {
            if (c == ',') { h->addSpecialization(trim(curSpec)); curSpec.clear(); }
            else curSpec += c;
        }
        if (!trim(curSpec).empty()) h->addSpecialization(trim(curSpec));
        hospitals.push_back(h);
        hospitalLookup.insert(id, h);
    }
    return true;
}

inline bool MedicalManager::loadPharmacies(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) return false;
    string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        string fields[6];
        int idx = 0; string cur = "";
        for (int i = 0; i < (int)line.size(); i++) {
            if (line[i] == ',' && idx < 5) { fields[idx++] = trim(cur); cur.clear(); }
            else cur += line[i];
        }
        fields[idx] = trim(cur);
        string pID = fields[0]; string pName = fields[1]; string pSector = fields[2];
        string medName = fields[3]; string medFormula = fields[4];
        float price = 0.0f; if (!fields[5].empty()) price = std::stof(fields[5]);
        Pharmacy* p = nullptr;
        Pharmacy** existing = pharmacyIdLookup.get(pID);
        if (existing) p = *existing;
        else {
            p = new Pharmacy(pID, pName, pSector);
            pharmacies.push_back(p);
            pharmacyIdLookup.insert(pID, p);
        }
        Medicine m(medName, medFormula, price);
        p->addMedicine(m);
        Vector<Pharmacy*>* sellers = medicineLookup.get(medName);
        if (sellers) {
            bool found = false;
            for (int i = 0; i < sellers->getSize(); i++) if (sellers->at(i)->id == p->id) found = true;
            if (!found) sellers->push_back(p);
        } else {
            Vector<Pharmacy*> list; list.push_back(p);
            medicineLookup.insert(medName, list);
        }
        Vector<Pharmacy*>* fSellers = formulaLookup.get(medFormula);
        if (fSellers) {
            bool found = false;
            for (int i = 0; i < fSellers->getSize(); i++) if (fSellers->at(i)->id == p->id) found = true;
            if (!found) fSellers->push_back(p);
        } else {
            Vector<Pharmacy*> list; list.push_back(p);
            formulaLookup.insert(medFormula, list);
        }
    }
    return true;
}

inline Hospital* MedicalManager::findHospitalByID(const string& id) const {
    Hospital** h = hospitalLookup.get(id);
    return h ? *h : nullptr;
}

inline Vector<Pharmacy*> MedicalManager::findMedicine(const string& medName) const {
    Vector<Pharmacy*>* result = medicineLookup.get(medName);
    return result ? *result : Vector<Pharmacy*>();
}

inline Vector<Pharmacy*> MedicalManager::findMedicineByFormula(const string& formula) const {
    Vector<Pharmacy*>* result = formulaLookup.get(formula);
    return result ? *result : Vector<Pharmacy*>();
}

inline Hospital* MedicalManager::findPatientRecord(const string& patientID) const {
    for (int i = 0; i < hospitals.getSize(); i++) {
        Hospital* h = hospitals[i];
        if (h->findPatient(patientID) != nullptr) {
            return h;
        }
    }
    return nullptr;
}

inline bool MedicalManager::processEmergency(const string& hospitalID, const Patient& p) {
    Hospital* h = findHospitalByID(hospitalID);
    if (h) return h->admitPatient(p);
    return false;
}

inline bool MedicalManager::addPatient(const string& hospitalID, const Patient& p) {
    Hospital* h = findHospitalByID(hospitalID);
    if (h) {
        bool admittedToBed = h->admitPatient(p);
        return true; 
    }
    return false; 
}

inline bool MedicalManager::addPatient(const string& hospitalID, Citizen* citizen, string disease, int severity) {
    Hospital* h = findHospitalByID(hospitalID);
    if (h) {
        Patient p(citizen, disease, severity);

        h->admitPatient(p);
        return true;
    }
    return false; 
}

inline bool MedicalManager::removePatient(const string& hospitalID, const string& patientID) {
    Hospital* h = findHospitalByID(hospitalID);
    if (h) {
       
        return h->dischargePatient(patientID);
    }
    return false; 
}


inline bool MedicalManager::addDoctor(const string& hospitalID, Citizen* citizen, const string& specialization) {
    Hospital* h = findHospitalByID(hospitalID); // O(1) 
    if (h) {
        Doctor newDoc(citizen, specialization);

        h->addDoctor(newDoc);

        if (citizen) {
            citizen->currentStatus = "Doctor";
        }
        return true;
    }
    return false; 
}

inline bool MedicalManager::removeDoctor(const string& hospitalID, const string& doctorID) {
    Hospital* h = findHospitalByID(hospitalID);
    if (h) {
        return h->removeDoctor(doctorID);
    }
    return false; 
}

inline string MedicalManager::trim(const string& s) const {
    int start = 0, end = (int)s.size() - 1;
    while (start <= end && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r' || s[start] == '"')) start++;
    while (end >= start && (s[end] == ' ' || s[end] == '\t' || s[end] == '\r' || s[end] == '"')) end--;
    return (start > end) ? "" : s.substr(start, end - start + 1);
}