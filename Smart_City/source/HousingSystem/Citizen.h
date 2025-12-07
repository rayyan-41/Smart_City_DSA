#pragma once
#include <string>
using std::string;

struct Citizen {
    string cnic;
    string name;
    int age;
    string currentStatus;

    // Home Address (Tree Traversal Links)
    string sector;
    int street;
    int houseNo;

    Citizen()
        : cnic(""), name(""), age(0),
        currentStatus("Home"), sector(""), street(0), houseNo(0) {
    }

    Citizen(string cnic, string name, int age, string sector, int street, int houseNo)
        : cnic(cnic), name(name), age(age), sector(sector), street(street), houseNo(houseNo),
             currentStatus("Home") {
    }

    // ==================== GETTERS ====================
    string getCNIC() const { return cnic; }
    string getName() const { return name; }
    int getAge() const { return age; }
    string getCurrentStatus() const { return currentStatus; }
    string getSector() const { return sector; }
    int getStreet() const { return street; }
    int getHouseNo() const { return houseNo; }
    
    // Full address for display
    string getFullAddress() const {
        return sector + ", Street " + std::to_string(street) + ", House " + std::to_string(houseNo);
    }
    
    // Check status helpers
    bool isAtHome() const { return currentStatus == "Home"; }
    bool isAtSchool() const { return currentStatus.find("School") != string::npos; }
    bool isAtHospital() const { return currentStatus.find("Hospital") != string::npos; }
    bool isTraveling() const { return currentStatus == "Traveling"; }

    // ==================== SETTERS ====================
    void setCNIC(const string& newCnic) { cnic = newCnic; }
    void setName(const string& newName) { name = newName; }
    void setAge(int newAge) { age = newAge; }
    void setCurrentStatus(const string& status) { currentStatus = status; }
    void setSector(const string& newSector) { sector = newSector; }
    void setStreet(int newStreet) { street = newStreet; }
    void setHouseNo(int newHouseNo) { houseNo = newHouseNo; }
    
    // Set full address at once
    void setAddress(const string& sec, int st, int house) {
        sector = sec;
        street = st;
        houseNo = house;
    }

    bool operator==(const Citizen& other) const {
        return cnic == other.cnic;
    }
};