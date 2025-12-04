#pragma once
#include <string>
#include <fstream>
#include "customSTL.h"
#include "School.h"
#include "Department.h"
#include "Class.h"
#include "Faculty.h"

using std::string;
using std::ifstream;

class SchoolManager {
public:
    Vector<School*> schools;

    // Hash Table for O(1) Lookup by ID
    // Key: School ID (string), Value: School Pointer (School*)
    HashTable<string, School*> schoolLookup;

    // NEW: Hash Table for O(1) Lookup by Subject [Image of Hash Table Mapping]
    // Key: Subject Name (string), Value: List of Schools offering it
    HashTable<string, Vector<School*>> subjectLookup;

    SchoolManager();
    ~SchoolManager();

    SchoolManager(const SchoolManager& other) = delete;
    SchoolManager& operator=(const SchoolManager& other) = delete;

    // ------- School Creation API -------
    School* createSchool(const string& id,
        const string& name,
        const string& sector,
        float rating,
        const string& graphNodeID = "",
        double x = 0.0,
        double y = 0.0);

    void addSchool(School* school);

    void setSchoolSubjects(School* school,
        const Vector<string>& subjects);

    void buildDepartmentsForSchool(School* school);
    void buildDepartmentsForAllSchools();

    // ------- Lookup API -------
    School* findSchoolByID(const string& id) const;
    Vector<School*> findSchoolsBySubject(const string& subject) const;

    void setGraphNodeForSchool(const string& schoolID,
        const string& graphNodeID);

    // ------- CSV Loading -------
    bool loadFromCSV(const string& filename, bool hasHeader = true);

private:
    string mapSubjectToDepartment(const string& subject) const;

    Department* findDepartmentInSchool(School* school,
        const string& deptName) const;

    Department* createDepartmentInSchool(School* school,
        const string& deptName);

    void addClassesToDepartment(Department* dept);

    string trim(const string& s) const;
};


// ================= IMPLEMENTATION =================

// Initialize Hash Tables with a decent prime number capacity
// schoolLookup: ~101 schools
// subjectLookup: ~53 unique subjects (smaller universe usually)
SchoolManager::SchoolManager() : schools(), schoolLookup(101), subjectLookup(53) {}

SchoolManager::~SchoolManager() {
    for (int i = 0; i < schools.getSize(); i++) {
        delete schools[i];
    }
    // No need to delete pointers in schoolLookup/subjectLookup explicitly 
}

School* SchoolManager::createSchool(const string& id,
    const string& name,
    const string& sector,
    float rating,
    const string& graphNodeID,
    double x,
    double y)
{
    School* s = new School(id, name, sector, rating, graphNodeID, x, y);

    schools.push_back(s);           // Store in Vector for iteration
    schoolLookup.insert(id, s);     // Store in ID Hash Table

    return s;
}

void SchoolManager::addSchool(School* school) {
    if (school) {
        schools.push_back(school);
        schoolLookup.insert(school->id, school);
    }
}

void SchoolManager::setSchoolSubjects(School* school,
    const Vector<string>& subjects) {
    if (!school) return;
    school->subjects = subjects;

    // NEW: Populate the Subject Lookup Hash Table
    for (int i = 0; i < subjects.getSize(); i++) {
        string subj = subjects[i];

        // Check if we already have a list of schools for this subject
        Vector<School*>* existingList = subjectLookup.get(subj);

        if (existingList != nullptr) {
            // Subject exists, append this school to the list
            existingList->push_back(school);
        }
        else {
            // Subject doesn't exist, create a new list and insert
            Vector<School*> newList;
            newList.push_back(school);
            subjectLookup.insert(subj, newList);
        }
    }
}

void SchoolManager::buildDepartmentsForSchool(School* school) {
    if (!school) return;
    if (school->departments.getSize() > 0) return;

    for (int i = 0; i < school->subjects.getSize(); i++) {
        const string& subj = school->subjects[i];
        string deptName = mapSubjectToDepartment(subj);

        Department* dept = findDepartmentInSchool(school, deptName);
        if (!dept) dept = createDepartmentInSchool(school, deptName);

        dept->addSubject(subj);
    }

    for (int i = 0; i < school->departments.getSize(); i++) {
        addClassesToDepartment(school->departments[i]);
    }
}

void SchoolManager::buildDepartmentsForAllSchools() {
    for (int i = 0; i < schools.getSize(); i++) {
        buildDepartmentsForSchool(schools[i]);
    }
}

// OPTIMIZED ID LOOKUP (O(1))
School* SchoolManager::findSchoolByID(const string& id) const {
    School** result = schoolLookup.get(id);
    if (result != nullptr) {
        return *result;
    }
    return nullptr;
}

// NEW: OPTIMIZED SUBJECT LOOKUP (O(1))
Vector<School*> SchoolManager::findSchoolsBySubject(const string& subject) const {
    // OLD METHOD (O(N*M)): Loop through all schools

    // NEW METHOD (O(1)): Get the list directly from Hash Table
    Vector<School*>* result = subjectLookup.get(subject);
    if (result != nullptr) {
        return *result; // Returns a copy of the vector of schools
    }

    return Vector<School*>(); // Return empty vector if subject not found
}

void SchoolManager::setGraphNodeForSchool(const string& schoolID,
    const string& graphNodeID) {
    School* s = findSchoolByID(schoolID);
    if (s) s->graphNodeID = graphNodeID;
}

bool SchoolManager::loadFromCSV(const string& filename, bool hasHeader) {
    ifstream file(filename);
    if (!file.is_open()) return false;

    string line;

    if (hasHeader) std::getline(file, line);

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        string fields[5];
        int idx = 0;
        string cur = "";

        for (int i = 0; i < (int)line.size(); i++) {
            char c = line[i];
            if (c == ',' && idx < 4) {
                fields[idx++] = trim(cur);
                cur.clear();
            }
            else {
                cur += c;
            }
        }
        fields[idx] = trim(cur);

        string id = trim(fields[0]);
        string name = trim(fields[1]);
        string sector = trim(fields[2]);
        string ratingStr = trim(fields[3]);
        string subjectsField = trim(fields[4]);

        float rating = ratingStr.empty() ? 0.0f : std::stof(ratingStr);

        if (!subjectsField.empty() &&
            subjectsField.front() == '"' &&
            subjectsField.back() == '"' &&
            subjectsField.size() >= 2)
        {
            subjectsField = subjectsField.substr(1, subjectsField.size() - 2);
        }

        Vector<string> subjects;
        string curSub = "";
        for (int i = 0; i < (int)subjectsField.size(); i++) {
            if (subjectsField[i] == ',') {
                string t = trim(curSub);
                if (!t.empty()) subjects.push_back(t);
                curSub.clear();
            }
            else {
                curSub += subjectsField[i];
            }
        }
        string t = trim(curSub);
        if (!t.empty()) subjects.push_back(t);

        // This calls createSchool -> then setSchoolSubjects
        // which now auto-populates BOTH Hash Tables.
        School* s = createSchool(id, name, sector, rating);
        setSchoolSubjects(s, subjects);
        buildDepartmentsForSchool(s);
    }

    file.close();
    return true;
}


// ---------------- Private Helpers ----------------

string SchoolManager::mapSubjectToDepartment(const string& subject) const {
    if (subject == "English" || subject == "Urdu" ||
        subject == "Islamiat" || subject == "Arabic")
        return "Arts";

    if (subject == "Math" || subject == "Mathematics" ||
        subject == "Physics" || subject == "Chemistry" ||
        subject == "Chem" || subject == "Biology" || subject == "Bio")
        return "Science";

    if (subject == "CS" || subject == "Computer Science" ||
        subject == "AI" || subject == "Artificial Intelligence" ||
        subject == "Robotics")
        return "Computing";

    return "General";
}

Department* SchoolManager::findDepartmentInSchool(School* school,
    const string& deptName) const {
    for (int i = 0; i < school->departments.getSize(); i++) {
        if (school->departments[i]->name == deptName) return school->departments[i];
    }
    return nullptr;
}

Department* SchoolManager::createDepartmentInSchool(School* school,
    const string& deptName) {
    Department* d = new Department(deptName);
    school->departments.push_back(d);
    return d;
}

void SchoolManager::addClassesToDepartment(Department* dept) {
    for (int level = 1; level <= 10; level++) {
        Class* c = new Class(level);
        dept->addClass(c);
    }
}

string SchoolManager::trim(const string& s) const {
    int start = 0, end = (int)s.size() - 1;

    while (start <= end &&
        (s[start] == ' ' || s[start] == '\t' ||
            s[start] == '\r' || s[start] == '"'))
        start++;

    while (end >= start &&
        (s[end] == ' ' || s[end] == '\t' ||
            s[end] == '\r' || s[end] == '"'))
        end--;

    return (start > end) ? "" : s.substr(start, end - start + 1);
}