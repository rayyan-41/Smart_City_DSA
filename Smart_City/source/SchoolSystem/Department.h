#pragma once
#include "customSTL.h"
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

    // Add operations
    void addClass(Class* c);
    void addFaculty(Faculty* f);
    void addSubject(const string& s);

    int getClassCount() const;
    int getFacultyCount() const;
};

// Implementation
Department::Department() : name(""), classes(), faculty(), subjects() {}

Department::Department(const string& name)
    : name(name), classes(), faculty(), subjects() {
}

Department::Department(const Department& other)
    : name(other.name), classes(other.classes),
    faculty(other.faculty), subjects(other.subjects) {
}

Department& Department::operator=(const Department& other) {
    if (this != &other) {
        name = other.name;
        classes = other.classes;
        faculty = other.faculty;
        subjects = other.subjects;
    }
    return *this;
}

Department::~Department() {
    for (int i = 0; i < classes.getSize(); i++) {
        delete classes[i];
    }
    for (int i = 0; i < faculty.getSize(); i++) {
        delete faculty[i];
    }
}

// Add operations
void Department::addClass(Class* c) {
    classes.push_back(c);
}

void Department::addFaculty(Faculty* f) {
    faculty.push_back(f);
}

void Department::addSubject(const string& s) {
    subjects.push_back(s);
}

// Utility
int Department::getClassCount() const {
    return classes.getSize();
}

int Department::getFacultyCount() const {
    return faculty.getSize();
}

