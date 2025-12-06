#pragma once
#include <string>
#include "ModuleUtils.h"
#include "../HousingSystem/Citizen.h"
using std::string;

class Student {
	public:
		Citizen* profile;
		string rollNumber;

		Student()
			: profile(nullptr), rollNumber(IDGenerator::generateStudentID()) {
		}
		Student(Citizen* citizen)
			: profile(citizen), rollNumber(IDGenerator::generateStudentID()) {
		}

		// Equality check based on Student ID
		bool operator==(const Student& other) const {
			return rollNumber == other.rollNumber;
		}

		// Getters
		int getAge() const {
			return profile ? profile->age : 0;
		}
		string getName() const {
			return profile ? profile->name : "";
		}
		string getCNIC() const {
			return profile ? profile->cnic : "";
		}
};