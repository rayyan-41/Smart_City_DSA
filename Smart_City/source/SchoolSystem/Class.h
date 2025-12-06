#pragma once
#include <string>
#include "Student.h"
#include "CustomSTL.h"

using std::string;

// Class owns the students but students will be created outside and passed here
class Class {
public:
    int classNumber;
    Vector<Student*> students; // Renamed from enrolledStudents to match your preference

    // Rule of three
    Class();
    Class(int classNumber);
    Class(const Class& other);
    Class& operator=(const Class& other);
    ~Class();

    int getStudentCount() const;

    bool addStudent(Student* student);
    bool removeStudent(const string& cnic);
};

// ==========================================
// IMPLEMENTATION
// ==========================================

inline Class::Class() : classNumber(1), students() {}

inline Class::Class(int classNumber) : classNumber(classNumber), students() {}

inline Class::Class(const Class& other) : classNumber(other.classNumber), students(other.students) {}

inline Class& Class::operator=(const Class& other) {
    if (this != &other) {
        classNumber = other.classNumber;
        students = other.students;
    }
    return *this;
}

inline Class::~Class() {
    // Students are deleted here because the Class "owns" the Student objects (wrappers)
    // The underlying Citizens are owned by PopulationManager, so deleting Student* is safe.
    for (int i = 0; i < students.getSize(); i++) {
        delete students[i];
    }
}

inline int Class::getStudentCount() const {
    return students.getSize();
}

inline bool Class::addStudent(Student* student) {
    // Check for duplicates using CNIC via getCNIC() helper
    for (int i = 0; i < students.getSize(); i++) {
        if (students[i]->getCNIC() == student->getCNIC()) {
            return false;
        }
    }
    students.push_back(student);
    return true;
}

inline bool Class::removeStudent(const string& cnic) {
    for (int i = 0; i < students.getSize(); i++) {
        if (students[i]->getCNIC() == cnic) {
            delete students[i];
            // Manual remove logic for Vector (shift left)
            for (int j = i; j < students.getSize() - 1; j++) {
                students[j] = students[j + 1];
            }
            students.pop_back();
            return true;
        }
    }
    return false;
}