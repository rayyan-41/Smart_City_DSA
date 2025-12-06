#pragma once
#include <fstream>
#include <string>
#include "CustomSTL.h"
#include "Hospital.h"
#include "Pharmacy.h"

using std::string;
using std::ifstream;
using std::endl;

class MedicalManager {
public:
    Vector<Hospital*> hospitals;
    Vector<Pharmacy*> pharmacies;

    // Lookup Tables
    HashTable<string, Hospital*> hospitalLookup;
    HashTable<string, Pharmacy*> pharmacyIdLookup;

    // Existing: Name -> Pharmacies
    HashTable<string, Vector<Pharmacy*>> medicineLookup;

    // NEW: Formula -> Pharmacies (e.g., "Paracetamol" -> List of Pharmacies)
    HashTable<string, Vector<Pharmacy*>> formulaLookup;

    MedicalManager();
    ~MedicalManager();

    bool loadHospitals(const string& filename);
    bool loadPharmacies(const string& filename);

    Hospital* findHospitalByID(const string& id) const;
    Vector<Pharmacy*> findMedicine(const string& medName) const;

    // NEW: Find by Formula
    Vector<Pharmacy*> findMedicineByFormula(const string& formula) const;

    // NEW: Locate a patient in ANY hospital
    // Returns a pointer to the Hospital they are in, or nullptr
    Hospital* findPatientRecord(const string& patientID) const;

    bool processEmergency(const string& hospitalID, const Patient& p);

private:
    string trim(const string& s) const;
};

// ==========================================
// IMPLEMENTATION
// ==========================================

inline MedicalManager::MedicalManager()
    : hospitalLookup(53), pharmacyIdLookup(53), medicineLookup(200), formulaLookup(100) {
}

inline MedicalManager::~MedicalManager() {
    for (int i = 0; i < hospitals.getSize(); i++) delete hospitals[i];
    for (int i = 0; i < pharmacies.getSize(); i++) delete pharmacies[i];
}

// ---------------- CSV Loading ----------------

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

        // 1. Index Name
        Vector<Pharmacy*>* sellers = medicineLookup.get(medName);
        if (sellers) {
            bool found = false;
            for (int i = 0; i < sellers->getSize(); i++) if (sellers->at(i)->id == p->id) found = true;
            if (!found) sellers->push_back(p);
        }
        else {
            Vector<Pharmacy*> list; list.push_back(p);
            medicineLookup.insert(medName, list);
        }

        // 2. Index Formula (NEW)
        Vector<Pharmacy*>* fSellers = formulaLookup.get(medFormula);
        if (fSellers) {
            bool found = false;
            for (int i = 0; i < fSellers->getSize(); i++) if (fSellers->at(i)->id == p->id) found = true;
            if (!found) fSellers->push_back(p);
        }
        else {
            Vector<Pharmacy*> list; list.push_back(p);
            formulaLookup.insert(medFormula, list);
        }
    }
    return true;
}

// ---------------- Queries ----------------

inline Hospital* MedicalManager::findHospitalByID(const string& id) const {
    Hospital** h = hospitalLookup.get(id);
    return h ? *h : nullptr;
}

inline Vector<Pharmacy*> MedicalManager::findMedicine(const string& medName) const {
    Vector<Pharmacy*>* result = medicineLookup.get(medName);
    return result ? *result : Vector<Pharmacy*>();
}

// NEW: Search by Formula
inline Vector<Pharmacy*> MedicalManager::findMedicineByFormula(const string& formula) const {
    Vector<Pharmacy*>* result = formulaLookup.get(formula);
    return result ? *result : Vector<Pharmacy*>();
}

// NEW: Find Patient in ANY hospital
inline Hospital* MedicalManager::findPatientRecord(const string& patientID) const {
    // Iterate through all hospitals (This is O(Hospitals * Patients_per_Hospital))
    // Not O(1), but acceptable since Hospital count is low (<100).
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

inline string MedicalManager::trim(const string& s) const {
    int start = 0, end = (int)s.size() - 1;
    while (start <= end && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r' || s[start] == '"')) start++;
    while (end >= start && (s[end] == ' ' || s[end] == '\t' || s[end] == '\r' || s[end] == '"')) end--;
    return (start > end) ? "" : s.substr(start, end - start + 1);
}