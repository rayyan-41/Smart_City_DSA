#pragma once
#include "../../data_structures/CustomSTL.h"
#include "Class.h"
#include "Faculty.h"

using std::string;

class Department {
public:
    string name;  
    Vector<Class*> classes;        
    Vector<Faculty*> faculty;        
    Vector<string> subjects;         

    Department();
    Department(const string& name);
    Department(const Department& other);
    Department& operator=(const Department& other);
    ~Department();

    // ==================== GETTERS ====================
    string getName() const { return name; }
    int getClassCount() const { return classes.getSize(); }
    int getFacultyCount() const { return faculty.getSize(); }
    int getSubjectCount() const { return subjects.getSize(); }
    const Vector<Class*>& getClasses() const { return classes; }
    const Vector<Faculty*>& getFaculty() const { return faculty; }
    const Vector<string>& getSubjects() const { return subjects; }
    
    Class* getClass(int index) const {
        if (index >= 0 && index < classes.getSize()) return classes[index];
        return nullptr;
    }
    
    Class* getClassByNumber(int classNumber) const {
        for (int i = 0; i < classes.getSize(); i++) {
            if (classes[i]->classNumber == classNumber) return classes[i];
        }
        return nullptr;
    }
    
    Faculty* getFacultyMember(int index) const {
        if (index >= 0 && index < faculty.getSize()) return faculty[index];
        return nullptr;
    }
    
    int getTotalStudentCount() const {
        int total = 0;
        for (int i = 0; i < classes.getSize(); i++) {
            total += classes[i]->getStudentCount();
        }
        return total;
    }

    // ==================== SETTERS ====================
    void setName(const string& newName) { name = newName; }

    // ==================== OPERATIONS ====================
    void addClass(Class* c);
    void addFaculty(Faculty* f);
    void addSubject(const string& s);
    bool addStudent(Student* student, int classNumber);
    bool removeStudent(const string& cnic);
    bool removeFaculty(const string& employeeID);
    bool hasSubject(const string& subject) const;
};

// ==================== Implemenation ====================
inline Department::Department() : name(""), classes(), faculty(), subjects() {}

inline Department::Department(const string& name)
    : name(name), classes(), faculty(), subjects() {
}

inline Department::Department(const Department& other)
    : name(other.name), classes(other.classes),
    faculty(other.faculty), subjects(other.subjects) {
}

inline Department& Department::operator=(const Department& other) {
    if (this != &other) {
        name = other.name;
        classes = other.classes;
        faculty = other.faculty;
        subjects = other.subjects;
    }
    return *this;
}

inline Department::~Department() {
    for (int i = 0; i < classes.getSize(); i++) {
        delete classes[i];
    }
    for (int i = 0; i < faculty.getSize(); i++) {
        delete faculty[i];
    }
}

inline void Department::addClass(Class* c) {
    classes.push_back(c);
}

inline void Department::addFaculty(Faculty* f) {
    faculty.push_back(f);
}

inline void Department::addSubject(const string& s) {
    subjects.push_back(s);
}

inline bool Department::addStudent(Student* student, int classNumber) {
    for (int i = 0; i < classes.getSize(); i++) {
        if (classes[i]->classNumber == classNumber) {
            classes[i]->addStudent(student);
            return true;
        }
    }
    return false;
}

inline bool Department::removeStudent(const string& cnic) {
    for (int i = 0; i < classes.getSize(); i++) {
        if (classes[i]->removeStudent(cnic)) {
            return true;
        }
    }
    return false;
}

inline bool Department::removeFaculty(const string& employeeID) {
    for (int i = 0; i < faculty.getSize(); i++) {
        if (faculty[i]->employeeID == employeeID) {
            delete faculty[i];
            faculty.erase(i);
            return true;
        }
    }
    return false;
}

inline bool Department::hasSubject(const string& subject) const {
    for (int i = 0; i < subjects.getSize(); i++) {
        if (subjects[i] == subject) return true;
    }
    return false;
}

