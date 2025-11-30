#pragma once
#include<string>
using std::string;
class Faculty {
	public:
	string name;
	string cnic;
	string qualification;
	float salary;

	// Rule of three
	Faculty() : name(""), cnic(""), qualification(""), salary(0.0f) {}
	Faculty(string name, string cnic, string qualification, float salary)
		: name(name), cnic(cnic), qualification(qualification), salary(salary) {
	}
	Faculty(const Faculty& other)
		: name(other.name), cnic(other.cnic), qualification(other.qualification), salary(other.salary) {
	}
	Faculty& operator=(const Faculty& other) {
		if (this != &other) {
			name = other.name;
			cnic = other.cnic;
			qualification = other.qualification;
			salary = other.salary;
		}
		return *this;
	}
	~Faculty() {}
};