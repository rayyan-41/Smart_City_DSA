#pragma once
#include <string>
#include "../../data_structures/CustomSTL.h"
#include "../../utils/ModuleUtils.h"
#include "Patient.h"
#include "Doctor.h"

using std::string;

class Hospital {
public:
    string id;
    string name;
    string sector;

    int totalBeds;
    Vector<Patient> admittedPatients;
    Vector<Doctor> doctors;
    Vector<string> specializations;

    // QUEUE (Min-Heap / Priority Queue)
    PriorityQueue<Patient> emergencyRoom;

    Location location;
    string graphNodeID;

    Hospital() : id(""), name(""), sector(""), totalBeds(0), location(), graphNodeID("") {}

    Hospital(string id, string name, string sector, int beds, string graphNodeID = "", double x = 0.0, double y = 0.0)
        : id(id), name(name), sector(sector),
        totalBeds(beds),
        location(sector, x, y), graphNodeID(graphNodeID) {
    }

    // ==================== GETTERS ====================
    string getId() const { return id; }
    string getName() const { return name; }
    string getSector() const { return sector; }
    int getTotalBeds() const { return totalBeds; }
    int getOccupiedBeds() const { return admittedPatients.getSize(); }
    int getAvailableBeds() const { return totalBeds - admittedPatients.getSize(); }
    int getERQueueSize() const { return emergencyRoom.size(); }
    int getDoctorCount() const { return doctors.getSize(); }
    int getSpecializationCount() const { return specializations.getSize(); }
    string getGraphNodeID() const { return graphNodeID; }
    double getLatitude() const { return location.coord.x; }
    double getLongitude() const { return location.coord.y; }
    const Location& getLocation() const { return location; }
    const Vector<Patient>& getAdmittedPatients() const { return admittedPatients; }
    const Vector<Doctor>& getDoctors() const { return doctors; }
    const Vector<string>& getSpecializations() const { return specializations; }
    
    double getOccupancyRate() const {
        if (totalBeds == 0) return 0.0;
        return (double)admittedPatients.getSize() / totalBeds * 100.0;
    }
    
    bool isAtCapacity() const { return getAvailableBeds() <= 0; }
    bool hasEmergencyQueue() const { return !emergencyRoom.empty(); }

    // ==================== SETTERS ====================
    void setId(const string& newId) { id = newId; }
    void setName(const string& newName) { name = newName; }
    void setSector(const string& newSector) { sector = newSector; location.sector = newSector; }
    void setTotalBeds(int beds) { totalBeds = beds; }
    void setGraphNodeID(const string& nodeID) { graphNodeID = nodeID; }
    void setCoordinates(double lat, double lon) { location.coord.x = lat; location.coord.y = lon; }
    void setLocation(const Location& loc) { location = loc; }

    // ==================== Implementation ====================

    bool admitPatient(const Patient& p) {
        if (getAvailableBeds() > 0) {
            admittedPatients.push_back(p);
            return true;
        }
        else {
            emergencyRoom.push(p);
            return false;
        }
    }

    void processAmbulanceArrival(const Vector<Patient>& victims) {
        for (int i = 0; i < victims.getSize(); i++) {
            admitPatient(victims[i]);
        }
    }

    bool dischargePatient() {
        if (admittedPatients.getSize() > 0) {
            Patient p = admittedPatients[0];
            if (p.profile) p.profile->currentStatus = "Home";
            for (int j = 0; j < admittedPatients.getSize() - 1; j++) {
                admittedPatients[j] = admittedPatients[j + 1];
            }
            admittedPatients.pop_back();
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
                if (p.profile) p.profile->currentStatus = "Home";
                for (int j = i; j < admittedPatients.getSize() - 1; j++) {
                    admittedPatients[j] = admittedPatients[j + 1];
                }
                admittedPatients.pop_back();
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

    void addDoctor(const Doctor& doc) {
        doctors.push_back(doc);
        if (!hasSpecialization(doc.specialization)) {
            addSpecialization(doc.specialization);
        }
    }

    bool removeDoctor(const string& docID) {
        for (int i = 0; i < doctors.getSize(); i++) {
            if (doctors[i].doctorID == docID) {
                doctors.erase(i);
                return true;
            }
        }
        return false;
    }
};