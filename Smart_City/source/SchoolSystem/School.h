#pragma once
#include <string>
#include "CustomSTL.h"
#include "Department.h"
#include "ModuleUtils.h"
#include "Student.h" 

using std::string;

// School owns Departments, which own Classes & Faculty.
class School {
public:
    string id;
    string name;
    float rating;

    Location location;     // sector + coordinates
    string graphNodeID;    // ID in the city graph

    Vector<string> subjects;
    Vector<Department*> departments;

    // SIMULATION: Tracking who is currently physically inside the school
    Vector<Student*> currentStudents;

    // Constructors
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

    // Administrative Methods
    void addDepartment(Department* d);
    bool addStudentToDepartment(const string& deptName, Student* student, int classNumber);
    bool removeStudent(const string& cnic) {
        for (int i = 0; i < departments.getSize(); i++) {
            if (departments[i]->removeStudent(cnic)) {
                return true;
            }
        }
		return false;
    }

    // NEW: Add Faculty to Department
    bool addFacultyToDepartment(const string& deptName, Faculty* faculty);

    int getDepartmentCount() const;
    Department* findDepartment(const string& deptName);

    // Simulation Methods (Arrivals/Departures)
    void processArrival(Student* s);
    void processDeparture(Student* s);
    bool isStudentPresent(const string& rollNo) const;
};

// ==========================================
// IMPLEMENTATION
// ==========================================

inline School::School()
    : id(""), name(""), rating(0.0f), location(), graphNodeID(""), subjects(), departments() {
}

inline School::School(const string& id, const string& name, const string& sector, float rating,
    const string& graphNodeID, double x, double y)
    : id(id), name(name), rating(rating), location(sector, x, y), graphNodeID(graphNodeID), subjects(), departments() {
}

inline School::School(const School& other)
    : id(other.id), name(other.name), rating(other.rating), location(other.location),
    graphNodeID(other.graphNodeID), subjects(other.subjects), departments(other.departments) {
    // Note: We typically don't copy currentStudents (simulation state) unless creating a snapshot
}

inline School& School::operator=(const School& other) {
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

inline School::~School() {
    for (int i = 0; i < departments.getSize(); i++) {
        delete departments[i];
    }
}

inline void School::addDepartment(Department* d) {
    departments.push_back(d);
}

inline bool School::addStudentToDepartment(const string& deptName, Student* student, int classNumber) {
    Department* dept = findDepartment(deptName);
    if (dept) {
        // Find the specific class within the department
        // Note: This assumes Department has a way to find a class or add directly.
        // If Department.h doesn't have addStudent, we need to iterate its classes here.

        for (int i = 0; i < dept->classes.getSize(); i++) {
            if (dept->classes[i]->classNumber == classNumber) {
                return dept->classes[i]->addStudent(student);
            }
        }
        // If class doesn't exist, you might want to create it or return false
        return false;
    }
    return false;
}

inline bool School::addFacultyToDepartment(const string& deptName, Faculty* faculty) {
    Department* dept = findDepartment(deptName);
    if (dept) {
        dept->addFaculty(faculty);
        return true;
    }
    return false;
}

inline int School::getDepartmentCount() const {
    return departments.getSize();
}

inline Department* School::findDepartment(const string& deptName) {
    for (int i = 0; i < departments.getSize(); i++) {
        if (departments[i]->name == deptName) {
            return departments[i];
        }
    }
    return nullptr;
}

// --- Simulation Logic ---

inline void School::processArrival(Student* s) {
    // Check if already present to avoid duplicates
    if (!isStudentPresent(s->rollNumber)) {
        currentStudents.push_back(s);
        // Update the Citizen's status via the profile pointer
        if (s->profile) s->profile->currentStatus = "At School: " + name;
    }
}

inline void School::processDeparture(Student* s) {
    for (int i = 0; i < currentStudents.getSize(); i++) {
        if (currentStudents[i]->rollNumber == s->rollNumber) {
            // Remove from vector (shift remaining)
            for (int j = i; j < currentStudents.getSize() - 1; j++) {
                currentStudents[j] = currentStudents[j + 1];
            }
            currentStudents.pop_back();

            if (s->profile) s->profile->currentStatus = "Home"; // Or "Traveling"
            return;
        }
    }
}

inline bool School::isStudentPresent(const string& rollNo) const {
    for (int i = 0; i < currentStudents.getSize(); i++) {
        if (currentStudents[i]->rollNumber == rollNo) return true;
    }
    return false;
}