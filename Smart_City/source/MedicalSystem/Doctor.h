#pragma once
#include <string>
#include "ModuleUtils.h"
#include "..//HousingSystem/Citizen.h"

using std::string;

class Doctor {
private:
	Citizen* citizen;
public:
	string doctorID;
	string specialization;
	
	// ==================== SETTERS ====================

	Doctor()
		: citizen(nullptr), doctorID(""), specialization("") {
	}
	Doctor(Citizen* citizen, const string& specialization)
		: citizen(citizen), specialization(specialization) {
		doctorID = IDGenerator::generateDoctorID();
	}

	// ==================== GETTERS ====================

	Citizen* getCitizen() const {
		return citizen;
	}
};