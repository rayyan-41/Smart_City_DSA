#pragma once
#include <string>
#include "customSTL.h"
#include "Student.h"
using std::string;

// Class owns the students but students will be created outside and passed here
class Class {
public:
	int classNumber;
	Vector<Student*> students;

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

// Implementation
Class::Class() : classNumber(1), students() {}
Class::Class(int classNumber) : classNumber(classNumber), students() {}
Class::Class(const Class& other) : classNumber(other.classNumber), students(other.students) {}
Class& Class::operator=(const Class& other) {
	if (this != &other) {
		classNumber = other.classNumber;
		students = other.students;
	}
	return *this;
}
Class::~Class() {
	// Students are delete here
	for(int i = 0; i < students.getSize(); i++) {
		delete students[i];
	}
}
int Class::getStudentCount() const {
	return students.getSize();
}
bool Class::addStudent(Student* student) {
	for (int i = 0; i < students.getSize(); i++) {
		if (students[i]->cnic == student->cnic) {
			return false;
		}
	}
	students.push_back(student);
	return true;
}
bool Class::removeStudent(const string& cnic) {
	for (int i = 0; i < students.getSize(); i++) {
		if (students[i]->cnic == cnic) {
			delete students[i]; 
			for (int j = i; j < students.getSize() - 1; j++) {
				students[j] = students[j + 1];
			}
			students.pop_back();
			return true; 
		}
	}
	return false; 
}
