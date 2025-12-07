#pragma once
#include <string>
#include "../../utils/ModuleUtils.h"
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

		bool operator==(const Student& other) const {
			return rollNumber == other.rollNumber;
		}

		// ==================== GETTERS ====================
		string getRollNumber() const { return rollNumber; }
		Citizen* getProfile() const { return profile; }
		
		int getAge() const {
			return profile ? profile->age : 0;
		}
		
		string getName() const {
			return profile ? profile->name : "";
		}
		
		string getCNIC() const {
			return profile ? profile->cnic : "";
		}
		
		string getSector() const {
			return profile ? profile->sector : "";
		}
		
		int getStreet() const {
			return profile ? profile->street : 0;
		}
		
		int getHouseNo() const {
			return profile ? profile->houseNo : 0;
		}
		
		string getCurrentStatus() const {
			return profile ? profile->currentStatus : "";
		}
		
		string getFullAddress() const {
			return profile ? profile->getFullAddress() : "";
		}

		// ==================== SETTERS ====================
		void setRollNumber(const string& roll) { rollNumber = roll; }
		void setProfile(Citizen* citizen) { profile = citizen; }
		
		void setCurrentStatus(const string& status) {
			if (profile) profile->currentStatus = status;
		}
};