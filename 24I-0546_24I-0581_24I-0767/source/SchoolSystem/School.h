#pragma once
#include <string>
#include "../../data_structures/CustomSTL.h"
#include "Department.h"
#include "../../utils/ModuleUtils.h"
#include "Student.h" 

using std::string;

class School {
public:
    string id;
    string name;
    float rating;

    Location location;     
    string graphNodeID;    

    Vector<string> subjects;
    Vector<Department*> departments;

    Vector<Student*> currentStudents;

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

    // ==================== GETTERS ====================
    string getId() const { return id; }
    string getName() const { return name; }
    float getRating() const { return rating; }
    string getSector() const { return location.sector; }
    double getLatitude() const { return location.coord.x; }
    double getLongitude() const { return location.coord.y; }
    string getGraphNodeID() const { return graphNodeID; }
    int getDepartmentCount() const;
    int getSubjectCount() const { return subjects.getSize(); }
    int getCurrentStudentCount() const { return currentStudents.getSize(); }
    const Vector<string>& getSubjects() const { return subjects; }
    const Vector<Department*>& getDepartments() const { return departments; }
    const Vector<Student*>& getCurrentStudents() const { return currentStudents; }
    const Location& getLocation() const { return location; }

    // ==================== SETTERS ====================
    void setId(const string& newId) { id = newId; }
    void setName(const string& newName) { name = newName; }
    void setRating(float newRating) { rating = newRating; }
    void setSector(const string& sector) { location.sector = sector; }
    void setCoordinates(double lat, double lon) { location.coord.x = lat; location.coord.y = lon; }
    void setGraphNodeID(const string& nodeID) { graphNodeID = nodeID; }
    void setLocation(const Location& loc) { location = loc; }

    // ==================== OPERATIONS ====================
       
    void addDepartment(Department* d);
    bool addStudentToDepartment(const string& deptName, Student* student, int classNumber);
    bool removeStudent(const string& cnic);
    bool addFacultyToDepartment(const string& deptName, Faculty* faculty);
    Department* findDepartment(const string& deptName);

    void processArrival(Student* s);
    void processDeparture(Student* s);
    bool isStudentPresent(const string& rollNo) const;

    int getTotalEnrolledStudents() const;
    int getTotalFaculty() const;
};

// ==================== Implemenation ====================


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
        for (int i = 0; i < dept->classes.getSize(); i++) {
            if (dept->classes[i]->classNumber == classNumber) {
                return dept->classes[i]->addStudent(student);
            }
        }
        return false;
    }
    return false;
}

inline bool School::removeStudent(const string& cnic) {
    for (int i = 0; i < departments.getSize(); i++) {
        if (departments[i]->removeStudent(cnic)) {
            return true;
        }
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

inline int School::getTotalEnrolledStudents() const {
    int total = 0;
    for (int i = 0; i < departments.getSize(); i++) {
        for (int j = 0; j < departments[i]->classes.getSize(); j++) {
            total += departments[i]->classes[j]->students.getSize();
        }
    }
    return total;
}

inline int School::getTotalFaculty() const {
    int total = 0;
    for (int i = 0; i < departments.getSize(); i++) {
        total += departments[i]->faculty.getSize();
    }
    return total;
}

// --- Simu.lation  ---

inline void School::processArrival(Student* s) {
    if (!isStudentPresent(s->rollNumber)) {
        currentStudents.push_back(s);
        if (s->profile) s->profile->currentStatus = "At School: " + name;
    }
}

inline void School::processDeparture(Student* s) {
    for (int i = 0; i < currentStudents.getSize(); i++) {
        if (currentStudents[i]->rollNumber == s->rollNumber) {
            for (int j = i; j < currentStudents.getSize() - 1; j++) {
                currentStudents[j] = currentStudents[j + 1];
            }
            currentStudents.pop_back();
            if (s->profile) s->profile->currentStatus = "Home";
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