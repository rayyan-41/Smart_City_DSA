#pragma once
#include <string>

using std::string;

class IDGenerator {
public:
    static int schoolCounter;
    static int facultyCounter;
    static int studentCounter;

    static string generateSchoolID();
    static string generateFacultyID();
    static string generateStudentID();
};

// Impl
int IDGenerator::schoolCounter = 100;
int IDGenerator::facultyCounter = 1000;
int IDGenerator::studentCounter = 5000;

string IDGenerator::generateSchoolID() {
    return "S" + std::to_string(schoolCounter++);
}

string IDGenerator::generateFacultyID() {
    return "F" + std::to_string(facultyCounter++);
}

string IDGenerator::generateStudentID() {
    return "S" + std::to_string(studentCounter++);
}
