#pragma once
#include <string>
#include "customSTL.h"
#include "Department.h"
#include "Location.h"

using std::string;

// School owns Departments, which own Classes & Faculty.
class School {
public:
    string id;
    string name;
    float rating;

    Location location;     // sector + coordinates
    string graphNodeID;    // ID in the city graph (string now)

    Vector<string> subjects;
    Vector<Department*> departments;

    School();
    School(const string& id,
        const string& name,
        const string& sector,
        float rating,
        const string& graphNodeID = "",
        double x = 0.0,
        double y = 0.0);

    School(const School& other);
    School& operator=(const School& other);
    ~School();

    void addDepartment(Department* d);
    int getDepartmentCount() const;
    Department* findDepartment(const string& deptName);
};

// ---------------- IMPLEMENTATION ----------------

School::School()
    : id(""),
    name(""),
    rating(0.0f),
    location(),
    graphNodeID(""),
    subjects(),
    departments() {
}

School::School(const string& id,
    const string& name,
    const string& sector,
    float rating,
    const string& graphNodeID,
    double x,
    double y)
    : id(id),
    name(name),
    rating(rating),
    location(sector, x, y),
    graphNodeID(graphNodeID),
    subjects(),
    departments() {
}

School::School(const School& other)
    : id(other.id),
    name(other.name),
    rating(other.rating),
    location(other.location),
    graphNodeID(other.graphNodeID),
    subjects(other.subjects),
    departments(other.departments) {
}

School& School::operator=(const School& other) {
    if (this != &other) {
        id = other.id;
        name = other.name;
        rating = other.rating;
        location = other.location;
        graphNodeID = other.graphNodeID;
        subjects = other.subjects;
        departments = other.departments;
    }
    return *this;
}

School::~School() {
    for (int i = 0; i < departments.getSize(); i++) {
        delete departments[i];
    }
}

void School::addDepartment(Department* d) {
    departments.push_back(d);
}

int School::getDepartmentCount() const {
    return departments.getSize();
}

Department* School::findDepartment(const string& deptName) {
    for (int i = 0; i < departments.getSize(); i++) {
        if (departments[i]->name == deptName) {
            return departments[i];
        }
    }
    return nullptr;
}
