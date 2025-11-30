#pragma once
#include <string>
#include <fstream>
#include "custom_STL.h"
#include "School.h"
#include "Department.h"
#include "Class.h"
#include "Faculty.h"

using std::string;
using std::ifstream;
using re::Vector;

class SchoolManager {
public:
    Vector<School*> schools;   

    SchoolManager();
    ~SchoolManager();

    // Disable copying to avoid double deletes.
    SchoolManager(const SchoolManager& other) = delete;
    SchoolManager& operator=(const SchoolManager& other) = delete;

    // ----------- Core API -----------

    School* createSchool(const string& id,
        const string& name,
        const string& sector,
        float rating,
        int graphNodeID = -1);

    void addSchool(School* school);

    void setSchoolSubjects(School* school, const Vector<string>& subjects);

    void buildDepartmentsForSchool(School* school);
    void buildDepartmentsForAllSchools();

    // ----------- Lookup API -----------

    School* findSchoolByID(const string& id) const;
    Vector<School*> findSchoolsBySubject(const string& subject) const;

    // Graph integration later
    void setGraphNodeForSchool(const string& schoolID, int graphNodeID);

    // ----------- CSV Loading -----------

    bool loadFromCSV(const string& filename, bool hasHeader = true);

private:
    // Map subject string -> department name
    string mapSubjectToDepartment(const string& subject) const;

    Department* findDepartmentInSchool(School* school,
        const string& deptName) const;

    Department* createDepartmentInSchool(School* school,
        const string& deptName);

    void addClassesToDepartment(Department* dept);

    // small helper to trim spaces/tabs from both ends
    string trim(const string& s) const;
};


// ========== IMPLEMENTATION ==========

SchoolManager::SchoolManager() : schools() {}

SchoolManager::~SchoolManager() {
    for (int i = 0; i < schools.getSize(); i++) {
        delete schools[i];
    }
}

School* SchoolManager::createSchool(const string& id,
    const string& name,
    const string& sector,
    float rating,
    int graphNodeID) {
    School* s = new School(id, name, sector, rating, graphNodeID);
    schools.push_back(s);
    return s;
}

void SchoolManager::addSchool(School* school) {
    if (school != nullptr) {
        schools.push_back(school);
    }
}

void SchoolManager::setSchoolSubjects(School* school,
    const Vector<string>& subjects) {
    if (!school) return;
    school->subjects = subjects;
}

void SchoolManager::buildDepartmentsForSchool(School* school) {
    if (!school) return;

    if (school->departments.getSize() > 0) {
        return; 
    }

    // 1) Map subjects to departments
    for (int i = 0; i < school->subjects.getSize(); i++) {
        const string& subj = school->subjects[i];
        string deptName = mapSubjectToDepartment(subj);

        Department* dept = findDepartmentInSchool(school, deptName);
        if (dept == nullptr) {
            dept = createDepartmentInSchool(school, deptName);
        }

        dept->addSubject(subj);
    }

    // 2) Give each department classes 1–10
    for (int d = 0; d < school->departments.getSize(); d++) {
        addClassesToDepartment(school->departments[d]);
    }
}

void SchoolManager::buildDepartmentsForAllSchools() {
    for (int i = 0; i < schools.getSize(); i++) {
        buildDepartmentsForSchool(schools[i]);
    }
}

School* SchoolManager::findSchoolByID(const string& id) const {
    for (int i = 0; i < schools.getSize(); i++) {
        if (schools[i]->id == id) {
            return schools[i];
        }
    }
    return nullptr;
}

Vector<School*> SchoolManager::findSchoolsBySubject(const string& subject) const {
    Vector<School*> result;

    for (int i = 0; i < schools.getSize(); i++) {
        School* s = schools[i];
        for (int j = 0; j < s->subjects.getSize(); j++) {
            if (s->subjects[j] == subject) {
                result.push_back(s);
                break;
            }
        }
    }
    return result;
}

void SchoolManager::setGraphNodeForSchool(const string& schoolID,
    int graphNodeID) {
    School* s = findSchoolByID(schoolID);
    if (s) {
        s->graphNodeID = graphNodeID;
    }
}


// ---------- CSV LOADING ----------

bool SchoolManager::loadFromCSV(const string& filename, bool hasHeader) {
    ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    string line;

    // Skip header if present
    if (hasHeader && std::getline(file, line)) {
        // ignored
    }

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        // We expect: ID,Name,Sector,Rating,Subjects...
        // First 4 commas split the first 4 columns.
        string fields[5];
        int fieldIndex = 0;
        string current = "";

        for (int i = 0; i < (int)line.size(); i++) {
            char c = line[i];

            if (c == ',' && fieldIndex < 4) {
                fields[fieldIndex] = trim(current);
                current.clear();
                fieldIndex++;
            }
            else {
                current += c;
            }
        }

        if (fieldIndex <= 4) {
            fields[fieldIndex] = trim(current);
        }

        string id = trim(fields[0]);
        string name = trim(fields[1]);
        string sector = trim(fields[2]);
        string ratingStr = trim(fields[3]);
        string subjectsField = trim(fields[4]);

        if (!subjectsField.empty() &&
            subjectsField.front() == '"' &&
            subjectsField.back() == '"' &&
            subjectsField.size() >= 2) {
            subjectsField = subjectsField.substr(1, subjectsField.size() - 2);
        }

        if (id.empty() || name.empty()) {
            continue; 
        }

        float rating = 0.0f;
        if (!ratingStr.empty()) {
            rating = std::stof(ratingStr);
        }

        Vector<string> subjects;
        string subj = "";
        for (int i = 0; i < (int)subjectsField.size(); i++) {
            char c = subjectsField[i];
            if (c == ',') {
                string trimmed = trim(subj);
                if (!trimmed.empty()) {
                    subjects.push_back(trimmed);
                }
                subj.clear();
            }
            else {
                subj += c;
            }
        }
        string trimmed = trim(subj);
        if (!trimmed.empty()) {
            subjects.push_back(trimmed);
        }

        School* s = createSchool(id, name, sector, rating, -1);
        setSchoolSubjects(s, subjects);
        buildDepartmentsForSchool(s);
    }

    file.close();
    return true;
}


// ---------- Private helpers ----------

string SchoolManager::mapSubjectToDepartment(const string& subject) const {
    // Arts department
    if (subject == "English" ||
        subject == "Urdu" ||
        subject == "Islamiat" ||
        subject == "Arabic") {
        return "Arts";
    }

    // Science department
    if (subject == "Math" ||
        subject == "Mathematics" ||
        subject == "Physics" ||
        subject == "Chemistry" ||
        subject == "Chem" ||
        subject == "Biology" ||
        subject == "Bio") {
        return "Science";
    }

    // Computing
    if (subject == "CS" ||
        subject == "Computer Science" ||
        subject == "AI" ||
        subject == "Artificial Intelligence" ||
        subject == "Robotics") {
        return "Computing";
    }

    return "General";
}

Department* SchoolManager::findDepartmentInSchool(School* school,
    const string& deptName) const {
    if (!school) return nullptr;

    for (int i = 0; i < school->departments.getSize(); i++) {
        if (school->departments[i]->name == deptName) {
            return school->departments[i];
        }
    }
    return nullptr;
}

Department* SchoolManager::createDepartmentInSchool(School* school,
    const string& deptName) {
    if (!school) return nullptr;

    Department* d = new Department(deptName);
    school->departments.push_back(d);
    return d;
}

void SchoolManager::addClassesToDepartment(Department* dept) {
    if (!dept) return;

    for (int level = 1; level <= 10; level++) {
        Class* c = new Class(level);
        dept->addClass(c);
    }
}

string SchoolManager::trim(const string& s) const {
    int start = 0;
    int end = (int)s.size() - 1;

    while (start <= end &&
        (s[start] == ' ' || s[start] == '\t' || s[start] == '\r')) {
        start++;
    }

    while (end >= start &&
        (s[end] == ' ' || s[end] == '\t' || s[end] == '\r')) {
        end--;
    }

    if (start > end) {
        return "";
    }

    return s.substr(start, end - start + 1);
}
