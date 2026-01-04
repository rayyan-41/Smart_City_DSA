#pragma once
#ifndef CITY_MANAGEMENT_H
#define CITY_MANAGEMENT_H

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include "../../SmartCity.h"

using std::string;
using std::cout;
using std::cin;
using std::endl;

class CityManagement {
private:
    SmartCity* city;
    int idCounters[12];

    enum IDType {
        SCHOOL_ID = 0,
        BUS_ID = 1,
        FACULTY_ID = 2,
        CITIZEN_ID = 3,
        AMBULANCE_ID = 4,
        ROUTE_ID = 5,
        HOSPITAL_ID = 6,
        PHARMACY_ID = 7,
        SCHOOL_BUS_ID = 8,
        DEPARTMENT_ID = 9,
        STUDENT_ID = 10,
        STOP_ID = 11,
        MALL_ID = 12,
        SHOP_ID = 13
    };

    string generateID(IDType type);

public:
    CityManagement(SmartCity* smartCity);
    ~CityManagement() = default;

    // ==================== SCHOOL MANAGEMENT ====================
    string addSchool(const string& name, const string& sector, float rating,
        const Vector<string>& departments, const Vector<string>& subjects);

    bool removeSchool(const string& schoolID);


    bool addDepartmentToSchool(const string& schoolID, const string& deptName);


    bool removeDepartmentFromSchool(const string& schoolID, const string& deptName);


    bool addClassToDepartment(const string& schoolID, const string& deptName, int classNumber);

    // ==================== FACULTY MANAGEMENT ====================


    string addNewFaculty(const string& name, const string& cnic, const string& qualification,
        const string& schoolID, const string& deptName, double salary);


    string hireCitizenAsFaculty(const string& citizenCNIC, const string& schoolID,
        const string& deptName, const string& qualification,
        double salary);


    bool removeFaculty(const string& schoolID, const string& deptName, const string& employeeID);


    Vector<Citizen*> getUnemployedCitizens();

    // ==================== BUS & ROUTE MANAGEMENT ====================


    string registerNewBus(const string& company, const string& startStopDBID,
        const string& endStopDBID);


    bool removeBus(const string& busNo);

    struct RouteInfo {
        Vector<int> path;
        double distance;
        int stopCount;
        string startName;
        string endName;
        bool valid;

        RouteInfo() : distance(0), stopCount(0), valid(false) {}
    };

    RouteInfo calculateRoute(const string& startStopDBID, const string& endStopDBID);
    RouteInfo calculateRouteByName(const string& startName, const string& endName);


    string registerSchoolBus(const string& schoolID, const string& sector);


    bool removeSchoolBus(const string& busID);




    string addHospital(const string& name, const string& sector, int beds,
        const Vector<string>& specializations);



    bool removeHospital(const string& hospitalID);

    bool admitPatient(const string& cnic, const string& hospitalID, int severity, const string& disease) {
        PopulationManager* pm = city->getPopulationManager();
        Citizen* citizen = pm->getCitizen(cnic);
        if (!citizen) return false;
        MedicalManager* mm = city->getMedicalManager();
        Patient p(citizen, disease, severity);
        return mm->addPatient(hospitalID, p);

    }

    bool addSpecializationToHospital(const string& hospitalID, const string& specialization);


    Vector<Hospital*> getAllHospitals();


    Vector<Hospital*> getHospitalsInSector(const string& sector);

    string addPharmacy(const string& name, const string& sector);


    bool removePharmacy(const string& pharmacyID);

    bool addMedicineToPharmacy(const string& pharmacyID, const string& medName,
        const string& formula, float price);


    Vector<Pharmacy*> getAllPharmacies();

    // ==================== AMBULANCE MANAGEMENT ====================


    string registerAmbulance(const string& hospitalID, const string& sector);


    bool removeAmbulance(const string& ambulanceID);


    Vector<Ambulance*> getAllAmbulances();


    Vector<Ambulance*> getAvailableAmbulances();

    // ==================== CITIZEN MANAGEMENT ====================


    string addCitizen(const string& name, int age, const string& sector,
        int streetNo, int houseNo);


    bool removeCitizen(const string& cnic);

    bool enrollStudent(const string& citizenCNIC, const string& schoolID,
        const string& deptName, int classNumber);


    bool removeStudent(const string& schoolID, const string& studentCNIC);


    Vector<Citizen*> getCitizensInSector(const string& sector);


    int getTotalCitizenCount();

    // ==================== LOCATION MANAGEMENT ====================


    int addBusStop(const string& name, const string& sector, double lat, double lon);


    int addBusStopInSector(const string& name, const string& sector);


    bool addRoad(int node1ID, int node2ID);


    bool removeRoad(int node1ID, int node2ID);

    // ==================== QUERY METHODS ====================


    Vector<School*> getSchoolsInSector(const string& sector);

    Vector<School*> getAllSchools();

    Vector<Bus*> getAllBuses();


    Vector<SchoolBus*> getAllSchoolBuses();

    Vector<CityNode*> getAllStops();


    Vector<CityNode*> getStopsInSector(const string& sector);


    struct SchoolDetails {
        string id;
        string name;
        string sector;
        float rating;
        int departmentCount;
        int totalStudents;
        int totalFaculty;
        Vector<string> departments;
        Vector<string> subjects;
    };

    SchoolDetails getSchoolDetails(const string& schoolID);


    struct BusDetails {
        string busNo;
        string company;
        string startStop;
        string endStop;
        int routeLength;
        double routeDistance;
        int currentPassengers;
        int totalPassengersServed;
    };

    BusDetails getBusDetails(const string& busNo);


    struct HospitalDetails {
        string id;
        string name;
        string sector;
        int totalBeds;
        int availableBeds;
        int admittedPatients;
        Vector<string> specializations;
    };

    HospitalDetails getHospitalDetails(const string& hospitalID);


    struct CityManagementStats {
        int totalSchools;
        int totalHospitals;
        int totalPharmacies;
        int totalBuses;
        int totalSchoolBuses;
        int totalAmbulances;
        int totalCitizens;
        int totalStops;
        int totalRoads;
    };

    CityManagementStats getManagementStats();

    // ==================== COMMERCIAL MANAGEMENT ====================

    string addMall(const string& name, const string& sector);


    bool removeMall(const string& mallID);


    string addShop(const string& mallID, const string& name, const string& category);
    bool removeShop(const string& mallID, const string& shopID);

    Vector<Mall*> getAllMalls();


private:
    string generateCNIC();
    Vector<Citizen*> collectCitizensFromSector(const string& sector);
};

// ============================================================================
// IMPLEMENTATION
// ============================================================================

inline CityManagement::CityManagement(SmartCity* smartCity) : city(smartCity) {
    for (int i = 0; i < 14; i++) {
        idCounters[i] = 1000;
    }
}

inline string CityManagement::generateID(IDType type) {
    string prefix;
    switch (type) {
    case SCHOOL_ID: prefix = "SCH"; break;
    case BUS_ID: prefix = "BUS"; break;
    case FACULTY_ID: prefix = "FAC"; break;
    case CITIZEN_ID: prefix = "CIT"; break;
    case AMBULANCE_ID: prefix = "AMB"; break;
    case ROUTE_ID: prefix = "RTE"; break;
    case HOSPITAL_ID: prefix = "HOS"; break;
    case PHARMACY_ID: prefix = "PHR"; break;
    case SCHOOL_BUS_ID: prefix = "SBU"; break;
    case DEPARTMENT_ID: prefix = "DPT"; break;
    case STUDENT_ID: prefix = "STU"; break;
    case STOP_ID: prefix = "STP"; break;
    case MALL_ID: prefix = "MALL"; break;
    case SHOP_ID: prefix = "SHOP"; break;
    }
    return prefix + "-" + std::to_string(++idCounters[type]);
}

inline string CityManagement::generateCNIC() {
    static int cnicCounter = 10000;
    return "35201-" + std::to_string(++cnicCounter) + "-1";
}

inline Vector<Citizen*> CityManagement::collectCitizensFromSector(const string& sector) {
    Vector<Citizen*> result;
    if (!city || !city->isInitialized()) return result;

    PopulationManager* pm = city->getPopulationManager();
    Sector* sec = pm->findSector(sector);
    if (!sec) return result;

    for (int s = 0; s < sec->streets.getSize(); s++) {
        Street* street = sec->streets[s];
        for (int h = 0; h < street->houses.getSize(); h++) {
            House* house = street->houses[h];
            for (int r = 0; r < house->residents.getSize(); r++) {
                result.push_back(house->residents[r]);
            }
        }
    }
    return result;
}

// ==================== SCHOOL MANAGEMENT ====================

inline string CityManagement::addSchool(const string& name, const string& sector, float rating,
    const Vector<string>& departments,
    const Vector<string>& subjects) {
    if (!city || !city->isInitialized()) return "";

    string schoolID = generateID(SCHOOL_ID);

    double lat = 0.0, lon = 0.0;
    GeometryUtils::generateCoords(sector, lat, lon);

    SchoolManager* sm = city->getSchoolManager();
    School* school = sm->createSchool(schoolID, name, sector, rating, "", lat, lon);

    if (!school) return "";

    CityGraph* graph = city->getCityGraph();
    int graphNodeID = graph->addLocation(schoolID, "", name, "SCHOOL", lat, lon);
    school->graphNodeID = std::to_string(graphNodeID);

    sm->setSchoolSubjects(school, subjects);

    for (int i = 0; i < departments.getSize(); i++) {
        Department* dept = new Department(departments[i]);
        school->addDepartment(dept);
        for (int level = 1; level <= 10; level++) {
            Class* c = new Class(level);
            dept->addClass(c);
        }
    }

    return schoolID;
}

inline bool CityManagement::removeSchool(const string& schoolID) {
    if (!city || !city->isInitialized()) return false;

    SchoolManager* sm = city->getSchoolManager();
    School* school = sm->findSchoolByID(schoolID);

    if (!school) return false;

    for (int i = 0; i < sm->schools.getSize(); i++) {
        if (sm->schools[i]->id == schoolID) {
            delete sm->schools[i];
            sm->schools.erase(i);
            break;
        }
    }

    return true;
}

inline bool CityManagement::addDepartmentToSchool(const string& schoolID, const string& deptName) {
    if (!city || !city->isInitialized()) return false;

    School* school = city->getSchoolManager()->findSchoolByID(schoolID);
    if (!school) return false;

    if (school->findDepartment(deptName)) return false;

    Department* dept = new Department(deptName);
    school->addDepartment(dept);

    for (int level = 1; level <= 10; level++) {
        Class* c = new Class(level);
        dept->addClass(c);
    }

    return true;
}

inline bool CityManagement::removeDepartmentFromSchool(const string& schoolID, const string& deptName) {
    if (!city || !city->isInitialized()) return false;

    School* school = city->getSchoolManager()->findSchoolByID(schoolID);
    if (!school) return false;

    for (int i = 0; i < school->departments.getSize(); i++) {
        if (school->departments[i]->name == deptName) {
            delete school->departments[i];
            school->departments.erase(i);
            return true;
        }
    }
    return false;
}

inline bool CityManagement::addClassToDepartment(const string& schoolID, const string& deptName,
    int classNumber) {
    if (!city || !city->isInitialized()) return false;

    School* school = city->getSchoolManager()->findSchoolByID(schoolID);
    if (!school) return false;

    Department* dept = school->findDepartment(deptName);
    if (!dept) return false;

    for (int i = 0; i < dept->classes.getSize(); i++) {
        if (dept->classes[i]->classNumber == classNumber) return false;
    }

    Class* c = new Class(classNumber);
    dept->addClass(c);
    return true;
}

// ==================== COMMERCIAL MANAGEMENT IMPLEMENTATION ====================

inline string CityManagement::addMall(const string& name, const string& sector) {
    if (!city || !city->isInitialized()) return "";

    string mallID = generateID(MALL_ID);

    Mall* mall = new Mall(mallID, name, sector);

    city->getCommercialManager()->addMall(mall);

    double lat = 0.0, lon = 0.0;
    GeometryUtils::generateCoords(sector, lat, lon);
    city->getCityGraph()->addLocation(mallID, "", name, "MALL", lat, lon);

    return mallID;
}

inline bool CityManagement::removeMall(const string& mallID) {
    if (!city || !city->isInitialized()) return false;

    return city->getCommercialManager()->removeMall(mallID);
}

inline string CityManagement::addShop(const string& mallID, const string& name, const string& category) {
    if (!city || !city->isInitialized()) return "";

    Mall* mall = city->getCommercialManager()->mallLookup.get(mallID)
        ? *city->getCommercialManager()->mallLookup.get(mallID)
        : nullptr;

    if (!mall) return "";

    string shopID = generateID(SHOP_ID);

    Shop* shop = new Shop(shopID, name, category);

    mall->addShop(shop);

    Vector<Shop*>* catList = city->getCommercialManager()->categoryLookup.get(category);
    if (catList) {
        catList->push_back(shop);
    }
    else {
        Vector<Shop*> newList;
        newList.push_back(shop);
        city->getCommercialManager()->categoryLookup.insert(category, newList);
    }

    return shopID;
}

inline bool CityManagement::removeShop(const string& mallID, const string& shopID) {
    if (!city || !city->isInitialized()) return false;

    return city->getCommercialManager()->removeShop(mallID, shopID);
}

inline Vector<Mall*> CityManagement::getAllMalls() {
    if (!city || !city->isInitialized()) return Vector<Mall*>();
    return city->getCommercialManager()->malls;
}

// ==================== FACULTY MANAGEMENT ====================

inline string CityManagement::addNewFaculty(const string& name, const string& cnic,
    const string& qualification,
    const string& schoolID, const string& deptName,
    double salary) {
    if (!city || !city->isInitialized()) return "";

    string employeeID = generateID(FACULTY_ID);

    Citizen* citizenProfile = city->findCitizen(cnic);
    if (!citizenProfile) {
        citizenProfile = city->addCitizen(cnic, name, 30, "G-9", 1, 1);
        if (!citizenProfile) return "";
    }

    Faculty* faculty = new Faculty(citizenProfile, employeeID, qualification, (float)salary);

    if (!city->getSchoolManager()->addFacultyToSchoolDepartment(schoolID, deptName, faculty)) {
        delete faculty;
        return "";
    }

    return employeeID;
}

inline string CityManagement::hireCitizenAsFaculty(const string& citizenCNIC, const string& schoolID,
    const string& deptName, const string& qualification,
    double salary) {
    if (!city || !city->isInitialized()) return "";

    Citizen* citizen = city->findCitizen(citizenCNIC);
    if (!citizen) return "";

    if (citizen->currentStatus.find("Employed") != string::npos ||
        citizen->currentStatus.find("Faculty") != string::npos ||
        citizen->currentStatus.find("Teacher") != string::npos) {
        return "";
    }

    string employeeID = generateID(FACULTY_ID);

    Faculty* faculty = new Faculty(citizen, employeeID, qualification, (float)salary);

    if (!city->getSchoolManager()->addFacultyToSchoolDepartment(schoolID, deptName, faculty)) {
        delete faculty;
        return "";
    }


    return employeeID;
}

inline bool CityManagement::removeFaculty(const string& schoolID, const string& deptName,
    const string& employeeID) {
    if (!city || !city->isInitialized()) return false;
    return city->getSchoolManager()->removeFacultyFromSchoolDepartment(schoolID, deptName, employeeID);
}

inline Vector<Citizen*> CityManagement::getUnemployedCitizens() {
    Vector<Citizen*> unemployed;
    if (!city || !city->isInitialized()) return unemployed;

    PopulationManager* pm = city->getPopulationManager();

    for (int i = 0; i < SECTOR_COUNT; i++) {
        Vector<Citizen*> sectorCitizens = collectCitizensFromSector(SECTOR_GRID[i].name);
        for (int j = 0; j < sectorCitizens.getSize(); j++) {
            Citizen* c = sectorCitizens[j];
            if (c && c->age >= 18 &&
                c->currentStatus.find("Employed") == string::npos &&
                c->currentStatus.find("Faculty") == string::npos &&
                c->currentStatus.find("Teacher") == string::npos &&
                c->currentStatus.find("Student") == string::npos) {
                unemployed.push_back(c);
            }
        }
    }

    return unemployed;
}

// ==================== BUS & ROUTE MANAGEMENT ====================

inline string CityManagement::registerNewBus(const string& company, const string& startStopDBID,
    const string& endStopDBID) {
    if (!city || !city->isInitialized()) return "";

    string busNo = generateID(BUS_ID);

    CityGraph* graph = city->getCityGraph();
    int startID = graph->getIDByDatabaseID(startStopDBID);
    int endID = graph->getIDByDatabaseID(endStopDBID);

    if (startID == -1 || endID == -1) return "";

    CityNode* startNode = graph->getNode(startID);
    string currentStop = startNode ? startNode->name : "";

    Bus* bus = city->registerBus(busNo, company, currentStop, startStopDBID, endStopDBID);

    if (!bus) return "";

    return busNo;
}

inline bool CityManagement::removeBus(const string& busNo) {
    if (!city || !city->isInitialized()) return false;

    TransportManager* tm = city->getTransportManager();
    Bus* bus = tm->findBusByNumber(busNo);
    if (bus) {
        bus->setStatus(VehicleStatus::MAINTENANCE);
        return true;
    }
    return false;
}

inline CityManagement::RouteInfo CityManagement::calculateRoute(const string& startStopDBID,
    const string& endStopDBID) {
    RouteInfo info;
    if (!city || !city->isInitialized()) return info;

    CityGraph* graph = city->getCityGraph();
    int startID = graph->getIDByDatabaseID(startStopDBID);
    int endID = graph->getIDByDatabaseID(endStopDBID);

    if (startID == -1 || endID == -1) return info;

    CityNode* startNode = graph->getNode(startID);
    CityNode* endNode = graph->getNode(endID);

    info.path = graph->findShortestPath(startID, endID, info.distance);
    info.stopCount = info.path.getSize();
    info.startName = startNode ? startNode->name : "";
    info.endName = endNode ? endNode->name : "";
    info.valid = info.path.getSize() > 0;

    return info;
}

inline CityManagement::RouteInfo CityManagement::calculateRouteByName(const string& startName,
    const string& endName) {
    RouteInfo info;
    if (!city || !city->isInitialized()) return info;

    CityGraph* graph = city->getCityGraph();
    int startID = graph->getIDByName(startName);
    int endID = graph->getIDByName(endName);

    if (startID == -1 || endID == -1) return info;

    info.path = graph->findShortestPath(startID, endID, info.distance);
    info.stopCount = info.path.getSize();
    info.startName = startName;
    info.endName = endName;
    info.valid = info.path.getSize() > 0;

    return info;
}

inline string CityManagement::registerSchoolBus(const string& schoolID, const string& sector) {
    if (!city || !city->isInitialized()) return "";

    SchoolManager* sm = city->getSchoolManager();
    School* school = sm->findSchoolByID(schoolID);
    if (!school) return "";

    string busID = generateID(SCHOOL_BUS_ID);
    int schoolNodeID = -1;

    if (!school->graphNodeID.empty()) {
        schoolNodeID = std::stoi(school->graphNodeID);
    }

    SchoolBus* bus = city->registerSchoolBus(busID, schoolID, schoolNodeID, sector);
    return bus ? busID : "";
}

inline bool CityManagement::removeSchoolBus(const string& busID) {
    if (!city || !city->isInitialized()) return false;

    SchoolBus* bus = city->findSchoolBusByID(busID);
    if (bus) {
        bus->setSchoolBusStatus(SchoolBusStatus::OUT_OF_SERVICE);
        return true;
    }
    return false;
}

// ==================== HOSPITAL MANAGEMENT ====================

inline string CityManagement::addHospital(const string& name, const string& sector, int beds,
    const Vector<string>& specializations) {
    if (!city || !city->isInitialized()) return "";

    string hospitalID = generateID(HOSPITAL_ID);

    Hospital* hospital = new Hospital(hospitalID, name, sector, beds);

    for (int i = 0; i < specializations.getSize(); i++) {
        hospital->addSpecialization(specializations[i]);
    }

    MedicalManager* mm = city->getMedicalManager();
    mm->hospitals.push_back(hospital);
    mm->hospitalLookup.insert(hospitalID, hospital);

    double lat = 0.0, lon = 0.0;
    GeometryUtils::generateCoords(sector, lat, lon);
    city->getCityGraph()->addLocation(hospitalID, "", name, "HOSPITAL", lat, lon);

    return hospitalID;
}

inline bool CityManagement::removeHospital(const string& hospitalID) {
    if (!city || !city->isInitialized()) return false;

    MedicalManager* mm = city->getMedicalManager();

    for (int i = 0; i < mm->hospitals.getSize(); i++) {
        if (mm->hospitals[i]->id == hospitalID) {
            delete mm->hospitals[i];
            mm->hospitals.erase(i);
            return true;
        }
    }
    return false;
}

inline bool CityManagement::addSpecializationToHospital(const string& hospitalID, const string& specialization) {
    if (!city || !city->isInitialized()) return false;

    Hospital* hospital = city->getMedicalManager()->findHospitalByID(hospitalID);
    if (!hospital) return false;

    hospital->addSpecialization(specialization);
    return true;
}

inline Vector<Hospital*> CityManagement::getAllHospitals() {
    if (!city || !city->isInitialized()) return Vector<Hospital*>();
    return city->getMedicalManager()->hospitals;
}

inline Vector<Hospital*> CityManagement::getHospitalsInSector(const string& sector) {
    Vector<Hospital*> result;
    if (!city || !city->isInitialized()) return result;

    MedicalManager* mm = city->getMedicalManager();
    for (int i = 0; i < mm->hospitals.getSize(); i++) {
        if (mm->hospitals[i]->sector == sector) {
            result.push_back(mm->hospitals[i]);
        }
    }
    return result;
}

// ==================== PHARMACY MANAGEMENT ====================

inline string CityManagement::addPharmacy(const string& name, const string& sector) {
    if (!city || !city->isInitialized()) return "";

    string pharmacyID = generateID(PHARMACY_ID);

    Pharmacy* pharmacy = new Pharmacy(pharmacyID, name, sector);

    MedicalManager* mm = city->getMedicalManager();
    mm->pharmacies.push_back(pharmacy);
    mm->pharmacyIdLookup.insert(pharmacyID, pharmacy);

    double lat = 0.0, lon = 0.0;
    GeometryUtils::generateCoords(sector, lat, lon);
    city->getCityGraph()->addLocation(pharmacyID, "", name, "PHARMACY", lat, lon);

    return pharmacyID;
}

inline bool CityManagement::removePharmacy(const string& pharmacyID) {
    if (!city || !city->isInitialized()) return false;

    MedicalManager* mm = city->getMedicalManager();

    for (int i = 0; i < mm->pharmacies.getSize(); i++) {
        if (mm->pharmacies[i]->id == pharmacyID) {
            delete mm->pharmacies[i];
            mm->pharmacies.erase(i);
            return true;
        }
    }
    return false;
}

inline bool CityManagement::addMedicineToPharmacy(const string& pharmacyID, const string& medName,
    const string& formula, float price) {
    if (!city || !city->isInitialized()) return false;

    MedicalManager* mm = city->getMedicalManager();
    Pharmacy** pPtr = mm->pharmacyIdLookup.get(pharmacyID);
    if (!pPtr || !(*pPtr)) return false;

    Pharmacy* pharmacy = *pPtr;
    Medicine med(medName, formula, price);
    pharmacy->addMedicine(med);

    Vector<Pharmacy*>* sellers = mm->medicineLookup.get(medName);
    if (sellers) {
        bool found = false;
        for (int i = 0; i < sellers->getSize(); i++) {
            if (sellers->at(i)->id == pharmacyID) { found = true; break; }
        }
        if (!found) sellers->push_back(pharmacy);
    }
    else {
        Vector<Pharmacy*> list; list.push_back(pharmacy);
        mm->medicineLookup.insert(medName, list);
    }

    return true;
}

inline Vector<Pharmacy*> CityManagement::getAllPharmacies() {
    if (!city || !city->isInitialized()) return Vector<Pharmacy*>();
    return city->getMedicalManager()->pharmacies;
}

// ==================== AMBULANCE MANAGEMENT ====================

inline string CityManagement::registerAmbulance(const string& hospitalID, const string& sector) {
    if (!city || !city->isInitialized()) return "";

    string ambID = generateID(AMBULANCE_ID);

    CityGraph* graph = city->getCityGraph();
    int hospitalNodeID = graph->getIDByDatabaseID(hospitalID);

    Ambulance* amb = city->registerAmbulance(ambID, hospitalID, hospitalNodeID, sector);
    return amb ? ambID : "";
}

inline bool CityManagement::removeAmbulance(const string& ambulanceID) {
    if (!city || !city->isInitialized()) return false;

    Ambulance* amb = city->findAmbulanceByID(ambulanceID);
    if (amb) {
        amb->setAmbulanceStatus(AmbulanceStatus::OUT_OF_SERVICE);
        return true;
    }
    return false;
}

inline Vector<Ambulance*> CityManagement::getAllAmbulances() {
    if (!city || !city->isInitialized()) return Vector<Ambulance*>();
    return city->getTransportManager()->getAllAmbulances();
}

inline Vector<Ambulance*> CityManagement::getAvailableAmbulances() {
    if (!city || !city->isInitialized()) return Vector<Ambulance*>();
    return city->getAvailableAmbulances();
}

// ==================== CITIZEN MANAGEMENT ====================

inline string CityManagement::addCitizen(const string& name, int age, const string& sector,
    int streetNo, int houseNo) {
    if (!city || !city->isInitialized()) return "";

    string cnic = generateCNIC();

    Citizen* citizen = city->addCitizen(cnic, name, age, sector, streetNo, houseNo);
    return citizen ? cnic : "";
}

inline bool CityManagement::removeCitizen(const string& cnic) {
    if (!city || !city->isInitialized()) return false;
    return city->removeCitizen(cnic);
}

inline bool CityManagement::enrollStudent(const string& citizenCNIC, const string& schoolID,
    const string& deptName, int classNumber) {
    if (!city || !city->isInitialized()) return false;
    return city->enrollStudent(citizenCNIC, schoolID, deptName, classNumber);
}

inline bool CityManagement::removeStudent(const string& schoolID, const string& studentCNIC) {
    if (!city || !city->isInitialized()) return false;
    return city->getSchoolManager()->removeStudent(schoolID, studentCNIC);
}

inline Vector<Citizen*> CityManagement::getCitizensInSector(const string& sector) {
    return collectCitizensFromSector(sector);
}

inline int CityManagement::getTotalCitizenCount() {
    if (!city || !city->isInitialized()) return 0;

    PopulationManager* pm = city->getPopulationManager();
    return pm->masterList.getSize();
}

// ==================== LOCATION MANAGEMENT ====================

inline int CityManagement::addBusStop(const string& name, const string& sector,
    double lat, double lon) {
    if (!city || !city->isInitialized()) return -1;

    CityGraph* graph = city->getCityGraph();
    string dbID = generateID(STOP_ID);

    int nodeID = graph->addLocation(dbID, dbID, name, "STOP", lat, lon);

    if (nodeID != -1) {
        city->getTransportManager()->initializeStopQueue(nodeID, name, sector);
    }

    return nodeID;
}

inline int CityManagement::addBusStopInSector(const string& name, const string& sector) {
    if (!city || !city->isInitialized()) return -1;

    double lat = 0.0, lon = 0.0;
    GeometryUtils::generateCoords(sector, lat, lon);

    return addBusStop(name, sector, lat, lon);
}

inline bool CityManagement::addRoad(int node1ID, int node2ID) {
    if (!city || !city->isInitialized()) return false;

    CityGraph* graph = city->getCityGraph();
    graph->addRoad(node1ID, node2ID);
    return true;
}

inline bool CityManagement::removeRoad(int node1ID, int node2ID) {

    if (!city || !city->isInitialized()) return false;
    return false;
}

// ==================== QUERY METHODS ====================

inline Vector<School*> CityManagement::getSchoolsInSector(const string& sector) {
    Vector<School*> result;
    if (!city || !city->isInitialized()) return result;

    SchoolManager* sm = city->getSchoolManager();
    for (int i = 0; i < sm->schools.getSize(); i++) {
        if (sm->schools[i]->getSector() == sector) {
            result.push_back(sm->schools[i]);
        }
    }
    return result;
}

inline Vector<School*> CityManagement::getAllSchools() {
    if (!city || !city->isInitialized()) return Vector<School*>();
    return city->getSchoolManager()->schools;
}

inline Vector<Bus*> CityManagement::getAllBuses() {
    if (!city || !city->isInitialized()) return Vector<Bus*>();
    return city->getTransportManager()->getAllBuses();
}

inline Vector<SchoolBus*> CityManagement::getAllSchoolBuses() {
    if (!city || !city->isInitialized()) return Vector<SchoolBus*>();
    return city->getTransportManager()->getAllSchoolBuses();
}

inline Vector<CityNode*> CityManagement::getAllStops() {
    Vector<CityNode*> stops;
    if (!city || !city->isInitialized()) return stops;

    CityGraph* graph = city->getCityGraph();
    for (int i = 0; i < graph->getNodeCount(); i++) {
        CityNode* node = graph->getNode(i);
        if (node && node->type == "STOP") {
            stops.push_back(node);
        }
    }
    return stops;
}

inline Vector<CityNode*> CityManagement::getStopsInSector(const string& sector) {
    Vector<CityNode*> stops;
    if (!city || !city->isInitialized()) return stops;

    CityGraph* graph = city->getCityGraph();
    for (int i = 0; i < graph->getNodeCount(); i++) {
        CityNode* node = graph->getNode(i);
        if (node && node->type == "STOP" && node->sector == sector) {
            stops.push_back(node);
        }
    }
    return stops;
}

inline CityManagement::SchoolDetails CityManagement::getSchoolDetails(const string& schoolID) {
    SchoolDetails details;
    if (!city || !city->isInitialized()) return details;

    School* school = city->getSchoolManager()->findSchoolByID(schoolID);
    if (!school) return details;

    details.id = school->id;
    details.name = school->name;
    details.sector = school->getSector();
    details.rating = school->rating;
    details.departmentCount = school->getDepartmentCount();
    details.totalStudents = school->getTotalEnrolledStudents();
    details.totalFaculty = school->getTotalFaculty();
    details.subjects = school->subjects;

    for (int i = 0; i < school->departments.getSize(); i++) {
        details.departments.push_back(school->departments[i]->name);
    }

    return details;
}

inline CityManagement::BusDetails CityManagement::getBusDetails(const string& busNo) {
    BusDetails details;
    if (!city || !city->isInitialized()) return details;

    Bus* bus = city->findBusByNumber(busNo);
    if (!bus) return details;

    details.busNo = bus->getBusNo();
    details.company = bus->getCompany();
    details.startStop = bus->getStartStopID();
    details.endStop = bus->getEndStopID();
    details.routeLength = bus->getStopCount();
    details.routeDistance = bus->getDistanceTraveled();
    details.currentPassengers = bus->getOnboardCount();
    details.totalPassengersServed = bus->getTotalPassengersServed();

    return details;
}

inline CityManagement::HospitalDetails CityManagement::getHospitalDetails(const string& hospitalID) {
    HospitalDetails details;
    if (!city || !city->isInitialized()) return details;

    Hospital* hospital = city->getMedicalManager()->findHospitalByID(hospitalID);
    if (!hospital) return details;

    details.id = hospital->id;
    details.name = hospital->name;
    details.sector = hospital->sector;
    details.totalBeds = hospital->getTotalBeds();
    details.availableBeds = hospital->getAvailableBeds();
    details.admittedPatients = hospital->getOccupiedBeds();
    details.specializations = hospital->getSpecializations();

    return details;
}

inline CityManagement::CityManagementStats CityManagement::getManagementStats() {
    CityManagementStats stats;
    if (!city || !city->isInitialized()) return stats;

    stats.totalSchools = city->getSchoolManager()->schools.getSize();
    stats.totalHospitals = city->getMedicalManager()->hospitals.getSize();
    stats.totalPharmacies = city->getMedicalManager()->pharmacies.getSize();
    stats.totalBuses = city->getTransportManager()->getBusCount();
    stats.totalSchoolBuses = city->getTransportManager()->getSchoolBusCount();
    stats.totalAmbulances = city->getTransportManager()->getAmbulanceCount();
    stats.totalCitizens = getTotalCitizenCount();
    stats.totalStops = getAllStops().getSize();

    int edgeCount = 0;
    CityGraph* graph = city->getCityGraph();
    for (int i = 0; i < graph->getNodeCount(); i++) {
        CityNode* node = graph->getNode(i);
        if (node) {
            edgeCount += node->getRoads().size();
        }
    }
    stats.totalRoads = edgeCount / 2;

    return stats;
}

#endif // CITY_MANAGEMENT_H

