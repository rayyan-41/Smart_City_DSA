/*
 * ============================================================================
 * SMART CITY MANAGEMENT SYSTEM - Core Wrapper Class
 * ============================================================================
 * 
 * This class serves as the central hub connecting all city subsystems:
 *   - CityGraph: Graph-based infrastructure (Adjacency List + Dijkstra)
 *   - SchoolManager: Education system (School Tree hierarchy)
 *   - TransportManager: Full transport system (Buses, School Buses, Ambulances)
 *   - PopulationManager: Housing system (N-ary Tree: Sector->Street->House->Citizen)
 *   - MedicalManager: Healthcare system (Hospital + Pharmacy with Priority Queue)
 *   - CommercialManager: Commercial system (Mall->Shop->Product with Hash lookup)
 * 
 * Data Structures Used:
 *   - Graph (Adjacency List) with weighted edges
 *   - Dijkstra's Shortest Path Algorithm
 *   - N-ary Tree for Population/Housing hierarchy
 *   - School Tree (School -> Department -> Class) 3-level hierarchy
 *   - Hash Tables with separate chaining for O(1) lookups
 *   - Priority Queue (Min-Heap) for Hospital ER and Patient Transfer Dispatch
 *   - Stack for Travel History
 *   - Circular Queue for Passenger simulation at bus stops
 *   - Singly Linked List for Bus/SchoolBus/Ambulance route management
 * 
 * Authors: Rayyan Ahmad Sultan, Omar Abdullah Khan Niazi, Aryan Ali Khan
 * ============================================================================
 */

#pragma once

#include "CityGraph.h"

#include "../SchoolSystem/SchoolManager.h"
#include "../TransportSystem/TransportManager.h"
#include "../HousingSystem/PopulationManager.h"
#include "../MedicalSystem/MedicalManager.h"
#include "../CommercialSystem/CommercialManager.h"

class SmartCity {
private:
    // ========== CORE INFRASTRUCTURE ==========
    CityGraph* cityGraph;
    
    // ========== SYSTEM MANAGERS ==========
    SchoolManager* schoolManager;
    TransportManager* transportManager;
    PopulationManager* populationManager;
    MedicalManager* medicalManager;
    CommercialManager* commercialManager;
    
    // ========== SIMULATION DATA STRUCTURES ==========
    Stack<TravelRecord> travelHistory;
    int travelCounter;
    
    // ========== DATASET PATHS ==========
    string stopsCSV;
    string schoolsCSV;
    string hospitalsCSV;
    string pharmaciesCSV;
    string busesCSV;
    string populationCSV;
    string mallsCSV;
    string shopsCSV;
    string ambulancesCSV;
    
    // ========== STATE FLAGS ==========
    bool cityInitialized;

public:
    // ========== LIFECYCLE ==========
    SmartCity();
    ~SmartCity();
    
    SmartCity(const SmartCity&) = delete;
    SmartCity& operator=(const SmartCity&) = delete;
    
    // ========== CONFIGURATION ==========
    void setDatasetPaths(const string& stops, const string& schools,
                         const string& hospitals, const string& pharmacies,
                         const string& buses, const string& population = "",
                         const string& malls = "", const string& shops = "",
                         const string& ambulances = "");
    
    // ========== INITIALIZATION ==========
    bool initialize();
    bool isInitialized() const { return cityInitialized; }
    
    // ========== MANAGER ACCESSORS ==========
    CityGraph* getCityGraph() const { return cityGraph; }
    SchoolManager* getSchoolManager() const { return schoolManager; }
    TransportManager* getTransportManager() const { return transportManager; }
    PopulationManager* getPopulationManager() const { return populationManager; }
    MedicalManager* getMedicalManager() const { return medicalManager; }
    CommercialManager* getCommercialManager() const { return commercialManager; }
    
    // ========== STATISTICS & REPORTING ==========
    CityStats getCityStats() const;
    Vector<string> getSectorNames() const;
    TransportStats getTransportStats() const;
    
    // ========== GRAPH/PATHFINDING APIs ==========
    Vector<int> findShortestPath(int startID, int endID, double& outDistance);
    Vector<int> findShortestPathByName(const string& startName, const string& endName, double& outDistance);
    Vector<int> findShortestPathByDBID(const string& startDBID, const string& endDBID, double& outDistance);
    int findNearestFacility(int fromNodeID, const string& facilityType);
    int findNearestFacilityByDBID(const string& fromDBID, const string& facilityType);
    
    // ========== BUS TRANSPORT APIs ==========
    Bus* registerBus(const string& busNo, const string& company, 
                    const string& currentStop, const string& startStopID, 
                    const string& endStopID);
    Bus* findBusByNumber(const string& busNo);
    Vector<Bus*> findBusesByCompany(const string& company);
    Vector<Bus*> findBusesOnRoute(int fromNodeID, int toNodeID);
    Vector<Bus*> findBusesOnRouteByDBID(const string& fromDBID, const string& toDBID);
    bool addPassengerToStop(int stopNodeID, const string& cnic, 
                           int destinationNodeID, double fare = 50.0);
    int getWaitingPassengersAtStop(int stopNodeID);
    
    // ========== SCHOOL BUS APIs ==========
    SchoolBus* registerSchoolBus(const string& id, const string& schoolID, 
                                 int schoolNodeID, const string& sector);
    SchoolBus* findSchoolBusByID(const string& id);
    Vector<SchoolBus*> getSchoolBusesBySchool(const string& schoolID);
    Vector<SchoolBus*> getSchoolBusesInSector(const string& sector);
    SchoolBus* findSchoolBusForRoute(const string& fromSector, const string& toSector);
    
    // ========== AMBULANCE/PATIENT TRANSFER APIs ==========
    Ambulance* registerAmbulance(const string& id, const string& hospitalID, 
                                 int hospitalNodeID, const string& sector);
    Ambulance* findAmbulanceByID(const string& id);
    Vector<Ambulance*> getAmbulancesByHospital(const string& hospitalID);
    Vector<Ambulance*> getAvailableAmbulances();
    
    string requestPatientTransfer(const string& patientCNIC, const string& patientName,
                                  const string& sourceHospitalID, const string& destHospitalID,
                                  const string& priority, const string& condition);
    Ambulance* dispatchNextTransfer();
    int getPendingTransferCount();
    bool routeAmbulanceToHospital(Ambulance* amb, const string& hospitalID);
    
    // ========== POPULATION APIs ==========
    Citizen* addCitizen(const string& cnic, const string& name, int age,
                       const string& sector, int streetNo, int houseNo);
    bool removeCitizen(const string& cnic);
    Citizen* findCitizen(const string& cnic) const;
    
    // ========== EDUCATION APIs ==========
    bool enrollStudent(const string& cnic, const string& schoolID,
                      const string& deptName, int classNumber);
    Vector<School*> findSchoolsBySubject(const string& subject);
    
    // ========== MEDICAL APIs ==========
    bool admitPatient(const string& cnic, const string& hospitalID,
                     int severity, const string& condition);
    bool dischargePatient(const string& hospitalID, const string& patientID);
    Vector<Pharmacy*> findPharmaciesByMedicine(const string& medicineName);
    Vector<Pharmacy*> findPharmaciesByFormula(const string& formula);
    Hospital* findNearestAvailableHospital(int fromNodeID);
    
    // ========== COMMERCIAL APIs ==========
    Vector<Shop*> findShopsByProduct(const string& productName);
    Vector<Shop*> findShopsByCategory(const string& category);
    
    // ========== TRAVEL HISTORY (Stack) ==========
    void recordTravel(const string& cnic, int fromNode, int toNode, 
                     double distance, const string& vehicleID = "", 
                     const string& vehicleType = "WALK");
    TravelRecord getLastTravel() const;
    bool undoLastTravel();
    const Stack<TravelRecord>& getTravelHistory() const { return travelHistory; }
    int getTravelHistorySize() const { return travelHistory.size(); }
    
    // ========== SIMULATION ==========
    void simulateStep();
    void processBusArrivals();
    void processSchoolBuses();
    void updateAmbulances();
    
    // ========== SECTOR/NODE QUERIES ==========
    Vector<CityNode*> getNodesInSector(const string& sectorName) const;
    Vector<CityNode*> getSchoolsInSector(const string& sectorName) const;
    Vector<CityNode*> getHospitalsInSector(const string& sectorName) const;
    Vector<CityNode*> getPharmaciesInSector(const string& sectorName) const;
    Vector<CityNode*> getStopsInSector(const string& sectorName) const;
    
    // ========== SECTOR ADJACENCY ==========
    static Vector<string> getAdjacentSectors(const string& sector);
    static bool areSectorsAdjacent(const string& sector1, const string& sector2);
};

// ============================================================================
// IMPLEMENTATION
// ============================================================================

inline SmartCity::SmartCity() {
    cityGraph = nullptr;
    schoolManager = nullptr;
    transportManager = nullptr;
    populationManager = nullptr;
    medicalManager = nullptr;
    commercialManager = nullptr;
    cityInitialized = false;
    travelCounter = 0;
    
    stopsCSV = "dataset/stops.csv";
    schoolsCSV = "dataset/schools.csv";
    hospitalsCSV = "dataset/hospitals.csv";
    pharmaciesCSV = "dataset/pharmacies.csv";
    busesCSV = "dataset/buses.csv";
    populationCSV = "dataset/population.csv";
    mallsCSV = "dataset/malls.csv";
    shopsCSV = "dataset/shops.csv";
    ambulancesCSV = "dataset/ambulances.csv";
}

inline SmartCity::~SmartCity() {
    delete cityGraph;
    delete schoolManager;
    delete transportManager;
    delete populationManager;
    delete medicalManager;
    delete commercialManager;
}

inline void SmartCity::setDatasetPaths(const string& stops, const string& schools,
                                       const string& hospitals, const string& pharmacies,
                                       const string& buses, const string& population,
                                       const string& malls, const string& shops,
                                       const string& ambulances) {
    stopsCSV = stops;
    schoolsCSV = schools;
    hospitalsCSV = hospitals;
    pharmaciesCSV = pharmacies;
    busesCSV = buses;
    if (!population.empty()) populationCSV = population;
    if (!malls.empty()) mallsCSV = malls;
    if (!shops.empty()) shopsCSV = shops;
    if (!ambulances.empty()) ambulancesCSV = ambulances;
}

inline bool SmartCity::initialize() {
    if (cityInitialized) return true;
    
    cityGraph = new CityGraph();
    schoolManager = new SchoolManager();
    transportManager = new TransportManager();
    populationManager = new PopulationManager();
    medicalManager = new MedicalManager();
    commercialManager = new CommercialManager();
    
    cityGraph->loadStopsCSV(stopsCSV);
    
    schoolManager->loadFromCSV(schoolsCSV);
    for (int i = 0; i < schoolManager->schools.getSize(); i++) {
        School* school = schoolManager->schools[i];
        int graphID = cityGraph->addLocation(
            school->id, "", school->name, "SCHOOL",
            school->location.coord.x, school->location.coord.y
        );
        if (graphID != -1) {
            school->graphNodeID = std::to_string(graphID);
        }
    }
    
    medicalManager->loadHospitals(hospitalsCSV);
    medicalManager->loadPharmacies(pharmaciesCSV);
    cityGraph->loadBuildingsCSV(hospitalsCSV, "HOSPITAL");
    cityGraph->loadBuildingsCSV(pharmaciesCSV, "PHARMACY");
    
    transportManager->setCityGraph(cityGraph);
    transportManager->loadBusesFromCSV(busesCSV);
    transportManager->loadAmbulancesFromCSV(ambulancesCSV);
    
    for (int i = 0; i < cityGraph->getNodeCount(); i++) {
        CityNode* node = cityGraph->getNode(i);
        if (node && node->type == "STOP") {
            transportManager->initializeStopQueue(node->id, node->name, node->sector);
        }
    }
    
    populationManager->loadPopulation(populationCSV);
    commercialManager->loadMalls(mallsCSV);
    commercialManager->loadShops(shopsCSV);
    
    cityInitialized = true;
    return true;
}

// ========== STATISTICS ==========

inline CityStats SmartCity::getCityStats() const {
    CityStats stats;
    if (!cityInitialized) return stats;
    
    stats.totalNodes = cityGraph->getNodeCount();
    for (int i = 0; i < cityGraph->getNodeCount(); i++) {
        CityNode* node = cityGraph->getNode(i);
        if (node) {
            if (node->type == "STOP") stats.busStops++;
            else if (node->type == "SCHOOL") stats.schoolNodes++;
            else if (node->type == "HOSPITAL") stats.hospitalNodes++;
            else if (node->type == "PHARMACY") stats.pharmacyNodes++;
            else if (node->type == "CORNER") stats.sectorCorners++;
        }
    }
    
    stats.totalSchools = schoolManager->schools.getSize();
    stats.totalHospitals = medicalManager->hospitals.getSize();
    stats.totalPharmacies = medicalManager->pharmacies.getSize();
    stats.totalMalls = commercialManager->malls.getSize();
    
    TransportStats tStats = transportManager->getStats();
    stats.totalBuses = tStats.totalBuses;
    stats.activeBuses = tStats.activeBuses;
    stats.totalSchoolBuses = tStats.totalSchoolBuses;
    stats.activeSchoolBuses = tStats.activeSchoolBuses;
    stats.totalAmbulances = tStats.totalAmbulances;
    stats.availableAmbulances = tStats.availableAmbulances;
    stats.pendingTransfers = tStats.pendingTransfers;
    stats.totalPassengersServed = tStats.totalBusPassengers;
    stats.totalStudentsTransported = tStats.totalStudentsTransported;
    stats.totalPatientsTransported = tStats.totalTransfers;
    
    Vector<int> popStats = populationManager->getHierarchyStats();
    if (popStats.getSize() >= 4) {
        stats.totalSectors = popStats[0];
        stats.totalStreets = popStats[1];
        stats.totalHouses = popStats[2];
        stats.totalCitizens = popStats[3];
    }
    
    stats.totalTravelRecords = travelHistory.size();
    
    return stats;
}

inline Vector<string> SmartCity::getSectorNames() const {
    Vector<string> names;
    for (int i = 0; i < SECTOR_COUNT; i++) {
        names.push_back(SECTOR_GRID[i].name);
    }
    return names;
}

inline TransportStats SmartCity::getTransportStats() const {
    if (!cityInitialized) return TransportStats();
    return transportManager->getStats();
}

// ========== GRAPH/PATHFINDING ==========

inline Vector<int> SmartCity::findShortestPath(int startID, int endID, double& outDistance) {
    if (!cityInitialized) return Vector<int>();
    return cityGraph->findShortestPath(startID, endID, outDistance);
}

inline Vector<int> SmartCity::findShortestPathByName(const string& startName, 
                                                     const string& endName, 
                                                     double& outDistance) {
    if (!cityInitialized) return Vector<int>();
    int startID = cityGraph->getIDByName(startName);
    int endID = cityGraph->getIDByName(endName);
    if (startID == -1 || endID == -1) return Vector<int>();
    return cityGraph->findShortestPath(startID, endID, outDistance);
}

inline Vector<int> SmartCity::findShortestPathByDBID(const string& startDBID, 
                                                     const string& endDBID, 
                                                     double& outDistance) {
    if (!cityInitialized) return Vector<int>();
    int startID = cityGraph->getIDByDatabaseID(startDBID);
    int endID = cityGraph->getIDByDatabaseID(endDBID);
    if (startID == -1 || endID == -1) return Vector<int>();
    return cityGraph->findShortestPath(startID, endID, outDistance);
}

inline int SmartCity::findNearestFacility(int fromNodeID, const string& facilityType) {
    if (!cityInitialized) return -1;
    return cityGraph->findNearestFacility(fromNodeID, facilityType);
}

inline int SmartCity::findNearestFacilityByDBID(const string& fromDBID, const string& facilityType) {
    if (!cityInitialized) return -1;
    int fromID = cityGraph->getIDByDatabaseID(fromDBID);
    if (fromID == -1) return -1;
    return cityGraph->findNearestFacility(fromID, facilityType);
}

// ========== BUS TRANSPORT ==========

inline Bus* SmartCity::registerBus(const string& busNo, const string& company,
                                   const string& currentStop, const string& startStopID,
                                   const string& endStopID) {
    if (!cityInitialized) return nullptr;
    
    Bus* bus = transportManager->createBus(busNo, company, currentStop);
    
    int startNodeID = cityGraph->getIDByDatabaseID(startStopID);
    int endNodeID = cityGraph->getIDByDatabaseID(endStopID);
    
    if (startNodeID != -1 && endNodeID != -1) {
        double distance = 0.0;
        Vector<int> route = cityGraph->findShortestPath(startNodeID, endNodeID, distance);
        if (route.getSize() > 0) {
            transportManager->setBusRoute(busNo, route, distance, startStopID, endStopID);
        }
    }
    
    return bus;
}

inline Bus* SmartCity::findBusByNumber(const string& busNo) {
    if (!cityInitialized) return nullptr;
    return transportManager->findBusByNumber(busNo);
}

inline Vector<Bus*> SmartCity::findBusesByCompany(const string& company) {
    if (!cityInitialized) return Vector<Bus*>();
    return transportManager->findBusesByCompany(company);
}

inline Vector<Bus*> SmartCity::findBusesOnRoute(int fromNodeID, int toNodeID) {
    if (!cityInitialized) return Vector<Bus*>();
    return transportManager->findBusesOnRoute(fromNodeID, toNodeID);
}

inline Vector<Bus*> SmartCity::findBusesOnRouteByDBID(const string& fromDBID, const string& toDBID) {
    if (!cityInitialized) return Vector<Bus*>();
    int fromID = cityGraph->getIDByDatabaseID(fromDBID);
    int toID = cityGraph->getIDByDatabaseID(toDBID);
    if (fromID == -1 || toID == -1) return Vector<Bus*>();
    return transportManager->findBusesOnRoute(fromID, toID);
}

inline bool SmartCity::addPassengerToStop(int stopNodeID, const string& cnic, 
                                          int destinationNodeID, double fare) {
    if (!cityInitialized) return false;
    Passenger p(cnic, stopNodeID, destinationNodeID, fare);
    return transportManager->addPassengerToStop(stopNodeID, p);
}

inline int SmartCity::getWaitingPassengersAtStop(int stopNodeID) {
    if (!cityInitialized) return 0;
    return transportManager->getWaitingCount(stopNodeID);
}

// ========== SCHOOL BUS ==========

inline SchoolBus* SmartCity::registerSchoolBus(const string& id, const string& schoolID,
                                               int schoolNodeID, const string& sector) {
    if (!cityInitialized) return nullptr;
    return transportManager->createSchoolBus(id, schoolID, schoolNodeID, sector);
}

inline SchoolBus* SmartCity::findSchoolBusByID(const string& id) {
    if (!cityInitialized) return nullptr;
    return transportManager->findSchoolBusByID(id);
}

inline Vector<SchoolBus*> SmartCity::getSchoolBusesBySchool(const string& schoolID) {
    if (!cityInitialized) return Vector<SchoolBus*>();
    return transportManager->getSchoolBusesBySchool(schoolID);
}

inline Vector<SchoolBus*> SmartCity::getSchoolBusesInSector(const string& sector) {
    if (!cityInitialized) return Vector<SchoolBus*>();
    return transportManager->getSchoolBusesBySector(sector);
}

inline SchoolBus* SmartCity::findSchoolBusForRoute(const string& fromSector, const string& toSector) {
    if (!cityInitialized) return nullptr;
    return transportManager->findSchoolBusForRoute(fromSector, toSector);
}

// ========== AMBULANCE/PATIENT TRANSFER ==========

inline Ambulance* SmartCity::registerAmbulance(const string& id, const string& hospitalID,
                                               int hospitalNodeID, const string& sector) {
    if (!cityInitialized) return nullptr;
    return transportManager->createAmbulance(id, hospitalID, hospitalNodeID, sector);
}

inline Ambulance* SmartCity::findAmbulanceByID(const string& id) {
    if (!cityInitialized) return nullptr;
    return transportManager->findAmbulanceByID(id);
}

inline Vector<Ambulance*> SmartCity::getAmbulancesByHospital(const string& hospitalID) {
    if (!cityInitialized) return Vector<Ambulance*>();
    return transportManager->getAmbulancesByHospital(hospitalID);
}

inline Vector<Ambulance*> SmartCity::getAvailableAmbulances() {
    if (!cityInitialized) return Vector<Ambulance*>();
    return transportManager->getAvailableAmbulances();
}

inline string SmartCity::requestPatientTransfer(const string& patientCNIC, const string& patientName,
                                                const string& sourceHospitalID, const string& destHospitalID,
                                                const string& priority, const string& condition) {
    if (!cityInitialized) return "";
    
    int sourceNodeID = cityGraph->getIDByDatabaseID(sourceHospitalID);
    int destNodeID = cityGraph->getIDByDatabaseID(destHospitalID);
    
    if (sourceNodeID == -1 || destNodeID == -1) return "";
    
    CityNode* sourceNode = cityGraph->getNode(sourceNodeID);
    CityNode* destNode = cityGraph->getNode(destNodeID);
    
    string sourceSector = sourceNode ? sourceNode->sector : "";
    string destSector = destNode ? destNode->sector : "";
    
    return transportManager->requestTransfer(patientCNIC, patientName,
                                            sourceHospitalID, sourceNodeID, sourceSector,
                                            destHospitalID, destNodeID, destSector,
                                            priority, condition);
}

inline Ambulance* SmartCity::dispatchNextTransfer() {
    if (!cityInitialized) return nullptr;
    return transportManager->dispatchNextTransfer();
}

inline int SmartCity::getPendingTransferCount() {
    if (!cityInitialized) return 0;
    return transportManager->getPendingTransferCount();
}

inline bool SmartCity::routeAmbulanceToHospital(Ambulance* amb, const string& hospitalID) {
    if (!cityInitialized || !amb) return false;
    
    int hospitalNodeID = cityGraph->getIDByDatabaseID(hospitalID);
    if (hospitalNodeID == -1) return false;
    
    double distance = 0.0;
    Vector<int> route = cityGraph->findShortestPath(amb->getCurrentNodeID(), 
                                                    hospitalNodeID, distance);
    if (route.getSize() > 0) {
        amb->setRouteSimple(route, distance);
        return true;
    }
    return false;
}

// ========== POPULATION ==========

inline Citizen* SmartCity::addCitizen(const string& cnic, const string& name, int age,
                                      const string& sector, int streetNo, int houseNo) {
    if (!cityInitialized) return nullptr;
    return populationManager->addCitizen(cnic, name, age, sector, streetNo, houseNo, "");
}

inline bool SmartCity::removeCitizen(const string& cnic) {
    if (!cityInitialized) return false;
    schoolManager->removeStudentFromAllSchools(cnic);
    schoolManager->removeFacultyFromAllSchools(cnic);
    return populationManager->removeCitizen(cnic);
}

inline Citizen* SmartCity::findCitizen(const string& cnic) const {
    if (!cityInitialized) return nullptr;
    return populationManager->getCitizen(cnic);
}

// ========== EDUCATION ==========

inline bool SmartCity::enrollStudent(const string& cnic, const string& schoolID,
                                    const string& deptName, int classNumber) {
    if (!cityInitialized) return false;
    Citizen* citizen = populationManager->getCitizen(cnic);
    if (!citizen) return false;
    return schoolManager->addStudent(schoolID, deptName, citizen, classNumber);
}

inline Vector<School*> SmartCity::findSchoolsBySubject(const string& subject) {
    if (!cityInitialized) return Vector<School*>();
    return schoolManager->findSchoolsBySubject(subject);
}

// ========== MEDICAL ==========

inline bool SmartCity::admitPatient(const string& cnic, const string& hospitalID,
                                   int severity, const string& condition) {
    if (!cityInitialized) return false;
    Citizen* citizen = populationManager->getCitizen(cnic);
    if (!citizen) return false;
    Patient patient(citizen, condition, severity);
    return medicalManager->processEmergency(hospitalID, patient);
}

inline bool SmartCity::dischargePatient(const string& hospitalID, const string& patientID) {
    if (!cityInitialized) return false;
    Hospital* hospital = medicalManager->findHospitalByID(hospitalID);
    if (!hospital) return false;
    return hospital->dischargePatient(patientID);
}

inline Vector<Pharmacy*> SmartCity::findPharmaciesByMedicine(const string& medicineName) {
    if (!cityInitialized) return Vector<Pharmacy*>();
    return medicalManager->findMedicine(medicineName);
}

inline Vector<Pharmacy*> SmartCity::findPharmaciesByFormula(const string& formula) {
    if (!cityInitialized) return Vector<Pharmacy*>();
    return medicalManager->findMedicineByFormula(formula);
}

inline Hospital* SmartCity::findNearestAvailableHospital(int fromNodeID) {
    if (!cityInitialized) return nullptr;
    
    Hospital* nearest = nullptr;
    double minDistance = 1e9;
    
    for (int i = 0; i < medicalManager->hospitals.getSize(); i++) {
        Hospital* h = medicalManager->hospitals[i];
        if (h->getAvailableBeds() > 0) {
            int hospitalNodeID = cityGraph->getIDByDatabaseID(h->id);
            if (hospitalNodeID != -1) {
                double dist = 0.0;
                Vector<int> path = cityGraph->findShortestPath(fromNodeID, hospitalNodeID, dist);
                if (path.getSize() > 0 && dist < minDistance) {
                    minDistance = dist;
                    nearest = h;
                }
            }
        }
    }
    return nearest;
}

// ========== COMMERCIAL ==========

inline Vector<Shop*> SmartCity::findShopsByProduct(const string& productName) {
    if (!cityInitialized) return Vector<Shop*>();
    return commercialManager->findShopsSellingProduct(productName);
}

inline Vector<Shop*> SmartCity::findShopsByCategory(const string& category) {
    if (!cityInitialized) return Vector<Shop*>();
    return commercialManager->findShopsByCategory(category);
}

// ========== TRAVEL HISTORY ==========

inline void SmartCity::recordTravel(const string& cnic, int fromNode, int toNode, 
                                   double distance, const string& vehicleID, 
                                   const string& vehicleType) {
    string timestamp = "T" + std::to_string(++travelCounter);
    TravelRecord record(cnic, fromNode, toNode, timestamp, distance, vehicleID, vehicleType);
    travelHistory.push(record);
}

inline TravelRecord SmartCity::getLastTravel() const {
    if (travelHistory.empty()) return TravelRecord();
    return travelHistory.top();
}

inline bool SmartCity::undoLastTravel() {
    if (travelHistory.empty()) return false;
    travelHistory.pop();
    return true;
}

// ========== SIMULATION ==========

inline void SmartCity::simulateStep() {
    processBusArrivals();
    processSchoolBuses();
    updateAmbulances();
}

inline void SmartCity::processBusArrivals() {
    if (!cityInitialized) return;
    transportManager->simulateBusStep();
}

inline void SmartCity::processSchoolBuses() {
    if (!cityInitialized) return;
    transportManager->simulateSchoolBusStep();
}

inline void SmartCity::updateAmbulances() {
    if (!cityInitialized) return;
    for (int i = 0; i < transportManager->getAmbulanceCount(); i++) {
        transportManager->updateAmbulanceStatus(transportManager->getAmbulance(i));
    }
}

// ========== SECTOR QUERIES ==========

inline Vector<CityNode*> SmartCity::getNodesInSector(const string& sectorName) const {
    Vector<CityNode*> result;
    if (!cityInitialized) return result;
    for (int i = 0; i < cityGraph->getNodeCount(); i++) {
        CityNode* node = cityGraph->getNode(i);
        if (node && node->sector == sectorName) result.push_back(node);
    }
    return result;
}

inline Vector<CityNode*> SmartCity::getSchoolsInSector(const string& sectorName) const {
    Vector<CityNode*> result;
    if (!cityInitialized) return result;
    for (int i = 0; i < cityGraph->getNodeCount(); i++) {
        CityNode* node = cityGraph->getNode(i);
        if (node && node->sector == sectorName && node->type == "SCHOOL") result.push_back(node);
    }
    return result;
}

inline Vector<CityNode*> SmartCity::getHospitalsInSector(const string& sectorName) const {
    Vector<CityNode*> result;
    if (!cityInitialized) return result;
    for (int i = 0; i < cityGraph->getNodeCount(); i++) {
        CityNode* node = cityGraph->getNode(i);
        if (node && node->sector == sectorName && node->type == "HOSPITAL") result.push_back(node);
    }
    return result;
}

inline Vector<CityNode*> SmartCity::getPharmaciesInSector(const string& sectorName) const {
    Vector<CityNode*> result;
    if (!cityInitialized) return result;
    for (int i = 0; i < cityGraph->getNodeCount(); i++) {
        CityNode* node = cityGraph->getNode(i);
        if (node && node->sector == sectorName && node->type == "PHARMACY") result.push_back(node);
    }
    return result;
}

inline Vector<CityNode*> SmartCity::getStopsInSector(const string& sectorName) const {
    Vector<CityNode*> result;
    if (!cityInitialized) return result;
    for (int i = 0; i < cityGraph->getNodeCount(); i++) {
        CityNode* node = cityGraph->getNode(i);
        if (node && node->sector == sectorName && node->type == "STOP") result.push_back(node);
    }
    return result;
}

// ========== SECTOR ADJACENCY ==========

inline Vector<string> SmartCity::getAdjacentSectors(const string& sector) {
    return TransportManager::getAdjacentSectors(sector);
}

inline bool SmartCity::areSectorsAdjacent(const string& sector1, const string& sector2) {
    return TransportManager::areSectorsAdjacent(sector1, sector2);
}