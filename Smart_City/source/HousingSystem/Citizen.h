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
        currentStatus("Home"), street(0), houseNo(0) {
    }

    Citizen(string cnic, string name, int age, string sector, int street, int houseNo)
        : cnic(cnic), name(name), age(age), sector(sector), street(street), houseNo(houseNo),
             currentStatus("Home") {
    }

    bool operator==(const Citizen& other) const {
        return cnic == other.cnic;
    }
};