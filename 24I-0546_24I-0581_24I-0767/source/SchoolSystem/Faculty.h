#pragma once
#include <string>
#include "../HousingSystem/Citizen.h"

using std::string;

class Faculty {
public:
    Citizen* profile;

    string employeeID;
    string qualification;
    float salary;

    Faculty();
    Faculty(Citizen* c, string empID, string qual, float sal);

    string getName() const;
};

// ==================== Implemenation ====================


inline Faculty::Faculty()
    : profile(nullptr), employeeID(""), qualification(""), salary(0.0f) {
}

inline Faculty::Faculty(Citizen* c, string empID, string qual, float sal)
    : profile(c), employeeID(empID), qualification(qual), salary(sal) {
    if (c) {
        c->currentStatus = "Teacher";
    }
}

inline string Faculty::getName() const {
    return profile ? profile->name : "Unknown";
}