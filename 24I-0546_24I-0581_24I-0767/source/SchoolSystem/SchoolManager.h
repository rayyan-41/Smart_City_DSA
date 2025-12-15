

#pragma once
#include <string>
#include <fstream>
#include "../../data_structures/CustomSTL.h"
#include "School.h"
#include "Department.h"
#include "Class.h"
#include "Faculty.h"

using std::string;
using std::ifstream;

class SchoolManager {
public:
    Vector<School*> schools;
    HashTable<string, School*> schoolLookup;
    HashTable<string, Vector<School*>> subjectLookup;

    SchoolManager();
    ~SchoolManager();

    SchoolManager(const SchoolManager& other) = delete;
    SchoolManager& operator=(const SchoolManager& other) = delete;

    // ==================== Operations ====================

    School* createSchool(const string& id, const string& name, const string& sector,
        float rating, const string& graphNodeID = "", double x = 0.0, double y = 0.0);
    void addSchool(School* school);

    bool addStudent(const string& schoolID, const string& deptName, Citizen* studentInfo, int classNumber);
    bool removeStudent(const string& schoolID, const string& cnic);
    bool removeStudentFromAllSchools(const string& cnic);

    bool addFacultyToSchoolDepartment(const string& schoolID, const string& deptName, Faculty* faculty);
    bool removeFacultyFromSchoolDepartment(const string& schoolID, const string& deptName, const string& employeeID);   
    bool removeFacultyFromAllSchools(const string& employeeID);

    void processBusArrival(const string& schoolID, const Vector<Student*>& incomingStudents);
    void setSchoolSubjects(School* school, const Vector<string>& subjects);
    void buildDepartmentsForSchool(School* school);
    void buildDepartmentsForAllSchools();

    School* findSchoolByID(const string& id) const;
    Vector<School*> findSchoolsBySubject(const string& subject) const;
    void setGraphNodeForSchool(const string& schoolID, const string& graphNodeID);

    bool loadFromCSV(const string& filename, bool hasHeader = true);

private:
    string mapSubjectToDepartment(const string& subject) const;
    Department* findDepartmentInSchool(School* school, const string& deptName) const;
    Department* createDepartmentInSchool(School* school, const string& deptName);
    void addClassesToDepartment(Department* dept);
    string trim(const string& s) const;
};

// ================= IMPLEMENTATION =================

inline SchoolManager::SchoolManager() : schools(), schoolLookup(101), subjectLookup(53) {}

inline SchoolManager::~SchoolManager() {
    for (int i = 0; i < schools.getSize(); i++) {
        delete schools[i];
    }
}

inline School* SchoolManager::createSchool(const string& id, const string& name, const string& sector,
    float rating, const string& graphNodeID, double x, double y) {
    School* s = new School(id, name, sector, rating, graphNodeID, x, y);
    schools.push_back(s);
    schoolLookup.insert(id, s);
    return s;
}

inline void SchoolManager::addSchool(School* school) {
    if (school) {
        schools.push_back(school);
        schoolLookup.insert(school->id, school);
    }
}

inline bool SchoolManager::addStudent(const string& schoolID, const string& deptName,
    Citizen* studentInfo, int classNumber) {
    Student* student = new Student(studentInfo);
    School* s = findSchoolByID(schoolID);
    if (s) {
        return s->addStudentToDepartment(deptName, student, classNumber);
    }
    return false;
}

inline bool SchoolManager::removeStudent(const string& schoolID, const string& cnic) {
    School* s = findSchoolByID(schoolID);
    if (s) {
        return s->removeStudent(cnic);
    }
    return false;
}

inline bool SchoolManager::removeStudentFromAllSchools(const string& cnic) {
    bool removed = false;
    for (int i = 0; i < schools.getSize(); i++) {
        if (schools[i]->removeStudent(cnic)) {
            removed = true;
        }
    }
    return removed;
}

inline bool SchoolManager::addFacultyToSchoolDepartment(const string& schoolID,
    const string& deptName, Faculty* faculty) {
    School* s = findSchoolByID(schoolID);
    if (s) {
        return s->addFacultyToDepartment(deptName, faculty);
    }
    return false;
}

inline bool SchoolManager::removeFacultyFromSchoolDepartment(const string& schoolID,
    const string& deptName, const string& employeeID) {
    School* s = findSchoolByID(schoolID);
    if (s) {
        Department* dept = s->findDepartment(deptName);
        if (dept) {
            for (int i = 0; i < dept->faculty.getSize(); i++) {
                if (dept->faculty[i]->employeeID == employeeID) {
                    delete dept->faculty[i];
                    dept->faculty.erase(i);
                    return true;
                }
            }
        }
    }
    return false;
}

inline bool SchoolManager::removeFacultyFromAllSchools(const string& employeeID) {
    bool removed = false;
    for (int i = 0; i < schools.getSize(); i++) {
        School* s = schools[i];
        for (int j = 0; j < s->departments.getSize(); j++) {
            Department* dept = s->departments[j];
            for (int k = 0; k < dept->faculty.getSize(); k++) {
                if (dept->faculty[k]->employeeID == employeeID) {
                    delete dept->faculty[k];
                    dept->faculty.erase(k);
                    removed = true;
                    break;
                }
            }
        }
    }
    return removed;
}

inline void SchoolManager::processBusArrival(const string& schoolID, const Vector<Student*>& incomingStudents) {
    School* s = findSchoolByID(schoolID);
    if (s) {
        for (int i = 0; i < incomingStudents.getSize(); i++) {
            Student* student = incomingStudents.at(i);
            s->processArrival(student);
        }
    }
}

inline void SchoolManager::setSchoolSubjects(School* school, const Vector<string>& subjects) {
    if (!school) return;
    school->subjects = subjects;
    for (int i = 0; i < subjects.getSize(); i++) {
        string subj = subjects[i];
        Vector<School*>* existingList = subjectLookup.get(subj);
        if (existingList != nullptr) {
            existingList->push_back(school);
        } else {
            Vector<School*> newList;
            newList.push_back(school);
            subjectLookup.insert(subj, newList);
        }
    }
}

inline void SchoolManager::buildDepartmentsForSchool(School* school) {
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

inline void SchoolManager::buildDepartmentsForAllSchools() {
    for (int i = 0; i < schools.getSize(); i++) {
        buildDepartmentsForSchool(schools[i]);
    }
}

inline School* SchoolManager::findSchoolByID(const string& id) const {
    School** result = schoolLookup.get(id);
    if (result != nullptr) return *result;
    return nullptr;
}

inline Vector<School*> SchoolManager::findSchoolsBySubject(const string& subject) const {
    Vector<School*>* result = subjectLookup.get(subject);
    if (result != nullptr) return *result;
    return Vector<School*>();
}

inline void SchoolManager::setGraphNodeForSchool(const string& schoolID, const string& graphNodeID) {
    School* s = findSchoolByID(schoolID);
    if (s) s->graphNodeID = graphNodeID;
}

inline bool SchoolManager::loadFromCSV(const string& filename, bool hasHeader) {
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
            } else {
                cur += c;
            }
        }
        fields[idx] = trim(cur);
        string id = trim(fields[0]);
        string name = trim(fields[1]);
        string sector = trim(fields[2]);
        string ratingStr = trim(fields[3]);
        string subjectsField = trim(fields[4]);
        float rating = 0.0f;
        try { if (!ratingStr.empty()) rating = std::stof(ratingStr); }
        catch (...) {}
        if (!subjectsField.empty() && subjectsField.front() == '"' && subjectsField.back() == '"' && subjectsField.size() >= 2) {
            subjectsField = subjectsField.substr(1, subjectsField.size() - 2);
        }
        Vector<string> subjects;
        string curSub = "";
        for (int i = 0; i < (int)subjectsField.size(); i++) {
            if (subjectsField[i] == ',') {
                string t = trim(curSub);
                if (!t.empty()) subjects.push_back(t);
                curSub.clear();
            } else {
                curSub += subjectsField[i];
            }
        }
        string t = trim(curSub);
        if (!t.empty()) subjects.push_back(t);
        School* s = createSchool(id, name, sector, rating);
        setSchoolSubjects(s, subjects);
        buildDepartmentsForSchool(s);
    }
    file.close();
    return true;
}

inline string SchoolManager::mapSubjectToDepartment(const string& subject) const {
    if (subject == "English" || subject == "Urdu" || subject == "Islamiat" || subject == "Arabic")
        return "Arts";
    if (subject == "Math" || subject == "Mathematics" || subject == "Physics" || 
        subject == "Chemistry" || subject == "Chem" || subject == "Biology" || subject == "Bio")
        return "Science";
    if (subject == "CS" || subject == "Computer Science" || subject == "AI" || 
        subject == "Artificial Intelligence" || subject == "Robotics")
        return "Computing";
    return "General";
}

inline Department* SchoolManager::findDepartmentInSchool(School* school, const string& deptName) const {
    for (int i = 0; i < school->departments.getSize(); i++) {
        if (school->departments[i]->name == deptName) return school->departments[i];
    }
    return nullptr;
}

inline Department* SchoolManager::createDepartmentInSchool(School* school, const string& deptName) {
    Department* d = new Department(deptName);
    school->departments.push_back(d);
    return d;
}

inline void SchoolManager::addClassesToDepartment(Department* dept) {
    for (int level = 1; level <= 10; level++) {
        Class* c = new Class(level);
        dept->addClass(c);
    }
}

inline string SchoolManager::trim(const string& s) const {
    int start = 0, end = (int)s.size() - 1;
    while (start <= end && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r' || s[start] == '"')) start++;
    while (end >= start && (s[end] == ' ' || s[end] == '\t' || s[end] == '\r' || s[end] == '"')) end--;
    return (start > end) ? "" : s.substr(start, end - start + 1);
}