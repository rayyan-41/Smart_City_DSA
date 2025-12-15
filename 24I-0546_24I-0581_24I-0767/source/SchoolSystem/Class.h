#pragma once
#include <string>
#include "Student.h"
#include "../../data_structures/CustomSTL.h"

using std::string;

class Class {
public:
    int classNumber;
    Vector<Student*> students;

    Class();
    Class(int classNumber);
    Class(const Class& other);
    Class& operator=(const Class& other);
    ~Class();

    // ==================== GETTERS ====================
    int getClassNumber() const { return classNumber; }
    int getStudentCount() const { return students.getSize(); }
    const Vector<Student*>& getStudents() const { return students; }
    
    Student* getStudent(int index) const {
        if (index >= 0 && index < students.getSize()) return students[index];
        return nullptr;
    }
    
    Student* findStudentByCNIC(const string& cnic) const {
        for (int i = 0; i < students.getSize(); i++) {
            if (students[i]->getCNIC() == cnic) return students[i];
        }
        return nullptr;
    }
    
    Student* findStudentByRollNo(const string& rollNo) const {
        for (int i = 0; i < students.getSize(); i++) {
            if (students[i]->rollNumber == rollNo) return students[i];
        }
        return nullptr;
    }
    
    bool isEmpty() const { return students.getSize() == 0; }

    // ==================== SETTERS ====================
    void setClassNumber(int num) { classNumber = num; }

    // ==================== OPERATIONS ====================
    bool addStudent(Student* student);
    bool removeStudent(const string& cnic);
};

// ==================== Implemenation ====================


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
     for (int i = 0; i < students.getSize(); i++) {
        delete students[i];
    }
}

inline bool Class::addStudent(Student* student) {
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
            //  remove Vector 
            for (int j = i; j < students.getSize() - 1; j++) {
                students[j] = students[j + 1];
            }
            students.pop_back();
            return true;
        }
    }
    return false;
}