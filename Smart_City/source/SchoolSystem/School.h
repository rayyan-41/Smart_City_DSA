#pragma once
#include <string>
#include "custom_STL.h"
#include "Department.h"

using std::string;
using re::Vector;

// School owns its Departments (and deletes them in destructor)
// Departments in turn own Classes and Faculty.
class School {
public:
    string id;           
    string name;         
    string sector;       
    float rating;        
    int graphNodeID;    

    Vector<string> subjects;      
    Vector<Department*> departments; 

    School();
    School(const string& id, const string& name, const string& sector, float rating, int graphNodeID = -1);

    School(const School& other);
    School& operator=(const School& other);
    ~School();

    // Department management (to be used by SchoolManager)
    void addDepartment(Department* d);
    int getDepartmentCount() const;
    Department* findDepartment(const string& deptName);
};

// --- Implementation ---

School::School()
    : id(""),
    name(""),
    sector(""),
    rating(0.0f),
    graphNodeID(-1),
    subjects(),
    departments() {
}

School::School(const string& id, const string& name, const string& sector, float rating, int graphNodeID) : id(id), name(name), sector(sector), 
       rating(rating), graphNodeID(graphNodeID), subjects(), departments() {}

School::School(const School& other)
    : id(other.id),
    name(other.name),
    sector(other.sector),
    rating(other.rating),
    graphNodeID(other.graphNodeID),
    subjects(other.subjects),
    departments(other.departments) {
}

School& School::operator=(const School& other) {
    if (this != &other) {
        id = other.id;
        name = other.name;
        sector = other.sector;
        rating = other.rating;
        graphNodeID = other.graphNodeID;
        subjects = other.subjects;
        departments = other.departments;
    }
    return *this;
}

School::~School() {
    // School owns Departments
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
