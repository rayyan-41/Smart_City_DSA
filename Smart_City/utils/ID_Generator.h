#pragma once
#include <string>

using std::string;

class IDGenerator {
public:
    // ==================== Operations ====================
    static int schoolCounter;
    static int facultyCounter;
    static int studentCounter;
    static int citizenCounter;
	static int doctorCounter;
	static int patientCounter;

    static string generateSchoolID();
    static string generateFacultyID();
    static string generateStudentID();

    static string generateCNIC();
    static string generateDoctorID();
	static string generatePatientID();  
};

// ==================== Implemenation ====================

int IDGenerator::schoolCounter = 100;
int IDGenerator::facultyCounter = 1000;
int IDGenerator::studentCounter = 5000;
int IDGenerator::citizenCounter = 1000000;
int IDGenerator::doctorCounter = 2000;
int IDGenerator::patientCounter = 3000;

string IDGenerator::generateSchoolID() {
    return "S" + std::to_string(schoolCounter++);
}

string IDGenerator::generateFacultyID() {
    return "F" + std::to_string(facultyCounter++);
}

string IDGenerator::generateStudentID() {
    return "ST" + std::to_string(studentCounter++);
}

string IDGenerator::generateCNIC() {
    int random = rand() % 10;
	string cnic = "61101-" + std::to_string(citizenCounter++) + "-" + std::to_string(random);
	return cnic;
}
string IDGenerator::generateDoctorID() {
    return "D" + std::to_string(doctorCounter++);
}
string IDGenerator::generatePatientID() {
    return "P" + std::to_string(patientCounter++);
}