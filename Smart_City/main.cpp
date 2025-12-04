#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include "source/SchoolSystem/SchoolManager.h"

using std::cout;
using std::endl;
using std::string;
using std::ofstream;

// --- ANSI Colors for Terminal UI ---
const string GREEN = "\033[1;32m";
const string RED = "\033[1;31m";
const string CYAN = "\033[1;36m";
const string YELLOW = "\033[1;33m";
const string RESET = "\033[0m";

// --- Helper to print status ---
void printPass(const string& testName) {
    cout << GREEN << "[PASS] " << RESET << testName << endl;
}

void printFail(const string& testName, const string& reason = "") {
    cout << RED << "[FAIL] " << RESET << testName;
    if (!reason.empty()) cout << " -> " << reason;
    cout << endl;
}

// --- Helper to Generate Dummy CSV ---
void createDummyCSV(const string& filename) {
    ofstream file(filename);
    file << "SchoolID,Name,Sector,Rating,Subjects\n"; // Header
    file << "S01,City School,G-10,4.5,\"Math, Physics, English\"\n";
    file << "S02,Allied School,F-8,4.0,\"CS, Math, Urdu\"\n";
    file << "S03,Beaconhouse,F-6,4.7,\"Bio, Chem, English\"\n";
    file << "S04,Iqra Academy,G-9,4.2,\"Islamiat, Arabic\"\n";
    file << "S05,Roots Millennium,F-7,4.8,\"AI, Robotics, Math\"\n";
    file.close();
    cout << YELLOW << "-> Generated test file: " << filename << RESET << endl;
}

// ================= TEST SUITES =================

void test_csv_loading(SchoolManager& mgr) {
    cout << CYAN << "\n--- Test 1: CSV Loading & Parsing ---" << RESET << endl;

    // Create and load
    createDummyCSV("schools_test.csv");
    bool loaded = mgr.loadFromCSV("schools_test.csv");

    if (loaded) printPass("File loaded successfully");
    else printFail("File loading failed");

    // Check Vector Size
    if (mgr.schools.getSize() == 5) printPass("Correct number of schools loaded (5)");
    else printFail("School count mismatch", "Expected 5, Got " + std::to_string(mgr.schools.getSize()));
}

void test_id_lookup(SchoolManager& mgr) {
    cout << CYAN << "\n--- Test 2: ID Hash Table Lookup (O(1)) ---" << RESET << endl;

    // 1. Valid Lookup
    School* s1 = mgr.findSchoolByID("S01");
    if (s1 && s1->name == "City School") printPass("Found S01 (City School)");
    else printFail("Lookup S01 failed");

    School* s5 = mgr.findSchoolByID("S05");
    if (s5 && s5->name == "Roots Millennium") printPass("Found S05 (Roots Millennium)");
    else printFail("Lookup S05 failed");

    // 2. Invalid Lookup
    School* ghost = mgr.findSchoolByID("S999");
    if (ghost == nullptr) printPass("Correctly returned nullptr for invalid ID");
    else printFail("Found non-existent ID S999");
}

void test_subject_lookup(SchoolManager& mgr) {
    cout << CYAN << "\n--- Test 3: Subject Hash Table Lookup (O(1)) ---" << RESET << endl;

    // 1. Subject with multiple schools (Math is in S01, S02, S05)
    Vector<School*> mathSchools = mgr.findSchoolsBySubject("Math");
    if (mathSchools.getSize() == 3) {
        printPass("Found 3 schools for 'Math'");
        // Verify specifically
        bool foundS01 = false, foundS05 = false;
        for (int i = 0; i < mathSchools.getSize(); i++) {
            if (mathSchools[i]->id == "S01") foundS01 = true;
            if (mathSchools[i]->id == "S05") foundS05 = true;
        }
        if (foundS01 && foundS05) printPass("Verified list contains S01 and S05");
        else printFail("List contents incorrect for Math");
    }
    else {
        printFail("Math count mismatch", "Expected 3, Got " + std::to_string(mathSchools.getSize()));
    }

    // 2. Subject with one school (Robotics is in S05 only)
    Vector<School*> roboSchools = mgr.findSchoolsBySubject("Robotics");
    if (roboSchools.getSize() == 1 && roboSchools[0]->id == "S05")
        printPass("Found unique school for 'Robotics'");
    else
        printFail("Robotics lookup failed");

    // 3. Subject with NO schools
    Vector<School*> chefSchools = mgr.findSchoolsBySubject("Culinary Arts");
    if (chefSchools.getSize() == 0) printPass("Correctly returned empty list for unknown subject");
    else printFail("Found schools for non-existent subject");
}

void test_hierarchy_structure(SchoolManager& mgr) {
    cout << CYAN << "\n--- Test 4: Tree Structure (School -> Dept -> Class) ---" << RESET << endl;

    // Analyze S01 (Subjects: Math, Physics, English)
    // Departments should be: Science (for Math/Phy) and Arts (for English)
    School* s01 = mgr.findSchoolByID("S01");
    if (!s01) { printFail("Critical: S01 missing for hierarchy test"); return; }

    // Check Department Count
    // Math & Physics -> map to "Science"
    // English -> maps to "Arts"
    // Should have 2 departments created (Science, Arts)
    // Note: Depends on if duplicate departments are merged or unique. 
    // Your Manager Logic: finds existing department, if not creates. So should be unique.

    if (s01->departments.getSize() >= 2) printPass("Departments created (Science & Arts)");
    else printFail("Department count low", "Expected >= 2, Got " + std::to_string(s01->departments.getSize()));

    // Drill down into Science
    Department* scienceDept = s01->findDepartment("Science");
    if (scienceDept) {
        printPass("Found 'Science' Department");

        // Check Classes (1 to 10)
        if (scienceDept->classes.getSize() == 10) printPass("Science Department has 10 Classes");
        else printFail("Class count mismatch in Science");

        // Check Subjects inside Dept
        bool hasMath = false;
        for (int i = 0; i < scienceDept->subjects.getSize(); i++) {
            if (scienceDept->subjects[i] == "Math") hasMath = true;
        }
        if (hasMath) printPass("'Math' is correctly assigned to Science Dept");
        else printFail("'Math' missing from Science Dept");

    }
    else {
        printFail("'Science' Department missing in S01");
    }
}

void test_manual_insertion(SchoolManager& mgr) {
    cout << CYAN << "\n--- Test 5: Manual Insertion & Live Update ---" << RESET << endl;

    // Manually add a new school
    School* newSchool = mgr.createSchool("S99", "Future High", "H-12", 5.0);
    Vector<string> newSubs;
    newSubs.push_back("AI");
    newSubs.push_back("Quantum Physics");

    // Set subjects (should trigger Hash Table update)
    mgr.setSchoolSubjects(newSchool, newSubs);
    mgr.buildDepartmentsForSchool(newSchool); // Build tree

    // Verify ID Lookup
    if (mgr.findSchoolByID("S99") != nullptr) printPass("Manual Add: ID Lookup Success");
    else printFail("Manual Add: ID Lookup Failed");

    // Verify Subject Lookup (AI should now have 2 schools: S05 and S99)
    Vector<School*> aiSchools = mgr.findSchoolsBySubject("AI");
    if (aiSchools.getSize() == 2) printPass("Manual Add: Subject Hash Table Updated (AI count = 2)");
    else printFail("Manual Add: Subject Update Failed", "Expected 2, Got " + std::to_string(aiSchools.getSize()));
}

int main() {
    cout << "========================================" << endl;
    cout << "   SCHOOL SYSTEM INTEGRATION TESTS      " << endl;
    cout << "========================================" << endl;

    SchoolManager manager;

    test_csv_loading(manager);
    test_id_lookup(manager);
    test_subject_lookup(manager);
    test_hierarchy_structure(manager);
    test_manual_insertion(manager);

    cout << "\n========================================" << endl;
    cout << "   ALL TESTS COMPLETED                  " << endl;
    cout << "========================================" << endl;

    return 0;
}