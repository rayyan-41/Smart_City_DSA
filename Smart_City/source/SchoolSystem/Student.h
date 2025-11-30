#pragma once
#include <string>
using std::string;

class Student {
public:
    string name;
    string cnic;
    int age;

	Student() : name(""), cnic(""), age(0) {}
	Student(string name, string cnic, int age) : name(name), cnic(cnic), age(age) {}
	Student(const Student& other) : name(other.name), cnic(other.cnic), age(other.age) {}
	Student& operator=(const Student& other) {
		if (this != &other) {
			name = other.name;
			cnic = other.cnic;
			age = other.age;
		}
		return *this;
	}
	~Student() {}
};