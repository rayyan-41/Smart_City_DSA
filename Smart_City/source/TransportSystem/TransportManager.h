#pragma once
#include <string>
#include <fstream>
#include "Bus.h"
#include "SchoolBus.h"
#include "Ambulance.h"
#include "../../data_structures/CustomSTL.h"
#include "../../data_structures/CircularQueue.h"
#include "../../data_structures/PriorityQueue.h"

using std::string;
using std::ifstream;

class CityGraph;


struct BusStopQueue {
    int stopNodeID;
    string stopName;
    string sector;
    CircularQueue<Passenger> waitingPassengers;
    
    BusStopQueue() : stopNodeID(-1), stopName(""), sector(""), waitingPassengers(200) {}
    BusStopQueue(int nodeID, const string& name, const string& sec) 
        : stopNodeID(nodeID), stopName(name), sector(sec), waitingPassengers(200) {}
};


struct TransportStats {
    int totalBuses;
    int activeBuses;
    int totalBusPassengers;
    double totalBusFares;
    int totalBusTrips;
    
    int totalSchoolBuses;
    int activeSchoolBuses;
    int totalStudentsTransported;
    int schoolBusTrips;
    
    int totalAmbulances;
    int availableAmbulances;
    int totalTransfers;
    int pendingTransfers;
    int criticalTransfers;
    
    int totalWaitingPassengers;
    
    TransportStats() 
        : totalBuses(0), activeBuses(0), totalBusPassengers(0),
          totalBusFares(0.0), totalBusTrips(0),
          totalSchoolBuses(0), activeSchoolBuses(0), 
          totalStudentsTransported(0), schoolBusTrips(0),
          totalAmbulances(0), availableAmbulances(0), 
          totalTransfers(0), pendingTransfers(0), criticalTransfers(0),
          totalWaitingPassengers(0) {}
};

class TransportManager {
private:
    CityGraph* cityGraph;
    
    // ========== BUS MANAGEMENT ==========
    Vector<Bus*> buses;
    HashTable<string, Bus*> busLookup;
    HashTable<string, Vector<Bus*>> companyLookup;
    HashTable<int, Vector<Bus*>> stopLookup;
    
    // ========== SCHOOL BUS MANAGEMENT ==========
    Vector<SchoolBus*> schoolBuses;
    HashTable<string, SchoolBus*> schoolBusLookup;
    HashTable<string, Vector<SchoolBus*>> schoolLookup;
    HashTable<string, Vector<SchoolBus*>> sectorSchoolBusLookup;
    
    HashTable<int, PickupPoint*> pickupPoints;
    HashTable<string, Vector<int>> sectorPickupPoints;
    
    // ========== AMBULANCE MANAGEMENT ==========
    Vector<Ambulance*> ambulances;
    HashTable<string, Ambulance*> ambulanceLookup;
    HashTable<string, Vector<Ambulance*>> hospitalAmbulanceLookup;
    HashTable<string, Vector<Ambulance*>> sectorAmbulanceLookup;
    
    PriorityQueue<PatientTransfer> transferQueue;
    Vector<PatientTransfer> activeTransfers;
    
    // ========== RICKSHAW MANAGEMENT (Feeder Vehicles) ==========
    Vector<Vehicle*> rickshaws;
    HashTable<string, Vector<Vehicle*>> sectorRickshawLookup;
    int rickshawIDCounter;
    
    HashTable<int, BusStopQueue*> stopQueues;
    
    int simulationStep;
    
    int totalTransferRequests;
    int transferIDCounter;

public:
    
    TransportManager();
    ~TransportManager();
    
    TransportManager(const TransportManager&) = delete;
    TransportManager& operator=(const TransportManager&) = delete;
    
    
    void setCityGraph(CityGraph* graph) { cityGraph = graph; }
    CityGraph* getCityGraph() const { return cityGraph; }
    
    // ==================== BUS MANAGEMENT ====================
    
    Bus* createBus(const string& busNo, const string& company, const string& currentStop);
    bool setBusRoute(const string& busNo, const Vector<int>& route, 
                    double distance, const string& startStopID, const string& endStopID);
    Bus* findBusByNumber(const string& busNo) const;
    Vector<Bus*> findBusesByCompany(const string& company) const;
    Vector<Bus*> findBusesAtStop(int stopNodeID) const;
    Vector<Bus*> findBusesOnRoute(int fromNodeID, int toNodeID) const;
    
    int getBusCount() const { return buses.getSize(); }
    Bus* getBus(int index) const;
    const Vector<Bus*>& getAllBuses() const { return buses; }
    
    // ==================== SCHOOL BUS MANAGEMENT ====================
    
    SchoolBus* createSchoolBus(const string& id, const string& schoolID, 
                               int schoolNodeID, const string& sector);
    SchoolBus* findSchoolBusByID(const string& id) const;
    Vector<SchoolBus*> getSchoolBusesBySchool(const string& schoolID) const;
    Vector<SchoolBus*> getSchoolBusesBySector(const string& sector) const;
    Vector<SchoolBus*> getAvailableSchoolBuses() const;
    SchoolBus* findSchoolBusForRoute(const string& fromSector, const string& toSector) const;
    
    int getSchoolBusCount() const { return schoolBuses.getSize(); }
    SchoolBus* getSchoolBus(int index) const;
    const Vector<SchoolBus*>& getAllSchoolBuses() const { return schoolBuses; }
    
    // ========== SCHOOL BUS PICKUP POINT MANAGEMENT ==========
    
    void createPickupPoint(int nodeID, const string& sector, const string& locationName, bool isResidential = true);
    bool addStudentToPickupPoint(int nodeID, const StudentPassenger& student);
    PickupPoint* getPickupPoint(int nodeID) const;
    Vector<int> getPickupPointsInSector(const string& sector) const;
    bool setupSchoolBusHomeRoute(const string& busID, const Vector<int>& pickupNodes,
                                  int schoolNodeID, const string& schoolID);
    bool dispatchSchoolBusForHomePickup(const string& busID);
    int getStudentsWaitingAtPickup(int nodeID) const;
    
    // ==================== AMBULANCE MANAGEMENT ====================
    
    Ambulance* createAmbulance(const string& id, const string& hospitalID, 
                               int hospitalNodeID, const string& sector);
    Ambulance* findAmbulanceByID(const string& id) const;
    Vector<Ambulance*> getAmbulancesByHospital(const string& hospitalID) const;
    Vector<Ambulance*> getAmbulancesBySector(const string& sector) const;
    Vector<Ambulance*> getAvailableAmbulances() const;
    Ambulance* findAmbulanceForTransfer(const string& sourceSector, const string& destSector) const;
    
    int getAmbulanceCount() const { return ambulances.getSize(); }
    Ambulance* getAmbulance(int index) const;
    const Vector<Ambulance*>& getAllAmbulances() const { return ambulances; }
    
    // ==================== RICKSHAW MANAGEMENT ====================
    
    Vehicle* spawnRickshaw(const string& sector, int startNodeID);
    void spawnRickshaws(int count);
    Vehicle* findAvailableRickshaw(int nearNodeID, const string& sector) const;
    bool dispatchRickshaw(Vehicle* rickshaw, int pickupNodeID, int destNodeID, const string& passengerCNIC);
    void simulateRickshawStep();
    int getRickshawCount() const { return rickshaws.getSize(); }
    const Vector<Vehicle*>& getAllRickshaws() const { return rickshaws; }
    
    // ==================== PATIENT TRANSFER ====================
    
    string requestTransfer(const string& patientCNIC, const string& patientName,
                          const string& sourceHospitalID, int sourceNodeID, const string& sourceSector,
                          const string& destHospitalID, int destNodeID, const string& destSector,
                          const string& priority, const string& condition);
    Ambulance* dispatchNextTransfer();
    bool dispatchAmbulance(const string& ambulanceID, const string& requestID);
    int getPendingTransferCount() const { return transferQueue.size(); }
    PatientTransfer* peekNextTransfer();
    
    // ==================== PASSENGER QUEUE MANAGEMENT ====================
    
    void initializeStopQueue(int stopNodeID, const string& stopName, const string& sector);
    bool addPassengerToStop(int stopNodeID, const Passenger& passenger);
    int getWaitingCount(int stopNodeID) const;
    BusStopQueue* getStopQueue(int stopNodeID) const;
    void processBusArrival(Bus* bus, int stopNodeID);
    
    // ==================== SIMULATION ====================
    
    void runSimulationStep();
    void runSimulation() { runSimulationStep(); }
    void runSimulationSteps(int steps);
    void runSimulation(int steps) { runSimulationSteps(steps); }
    int getSimulationStep() const { return simulationStep; }
    int getSimulationTick() const { return simulationStep; }
    void resetSimulation();
    void startSimulation() { simulationRunning = true; }
    void stopSimulation() { simulationRunning = false; }
    bool isSimulationRunning() const { return simulationRunning; }

    void simulateBusStep();
    void simulateSchoolBusStep();
    void simulateAmbulanceStep();
    void processSchoolBusPickup(SchoolBus* sb, int pickupNodeID);
    void processSchoolBusSchoolArrival(SchoolBus* sb, const string& schoolID, int schoolNodeID);
   
    TransportStats getStats() const;
    
    // ==================== CSV LOADING ====================
    
    bool loadBusesFromCSV(const string& filename, bool hasHeader = true);
    bool loadAmbulancesFromCSV(const string& filename, bool hasHeader = true);
    bool loadSchoolBusesFromCSV(const string& filename, bool hasHeader = true);
    
    static Vector<string> getAdjacentSectors(const string& sector);
    static bool areSectorsAdjacent(const string& sector1, const string& sector2);

private:
    string trim(const string& s) const;
    Vector<string> parseRoute(const string& routeStr) const;
    
    bool simulationRunning;
};


inline TransportManager::TransportManager() 
    : cityGraph(nullptr), buses(), busLookup(101), companyLookup(53),
      stopLookup(201), schoolBuses(), schoolBusLookup(53), schoolLookup(53),
      sectorSchoolBusLookup(53), pickupPoints(201), sectorPickupPoints(53),
      ambulances(), ambulanceLookup(53), 
      hospitalAmbulanceLookup(53), sectorAmbulanceLookup(53),
      transferQueue(), activeTransfers(), 
      rickshaws(), sectorRickshawLookup(53), rickshawIDCounter(0),
      stopQueues(201),
      simulationStep(0), simulationRunning(false),
      totalTransferRequests(0), transferIDCounter(1000) {}

inline TransportManager::~TransportManager() {
    for (int i = 0; i < buses.getSize(); ++i) delete buses[i];
    for (int i = 0; i < schoolBuses.getSize(); ++i) delete schoolBuses[i];
    for (int i = 0; i < ambulances.getSize(); ++i) delete ambulances[i];
    for (int i = 0; i < rickshaws.getSize(); ++i) delete rickshaws[i];
}


inline Vector<string> TransportManager::getAdjacentSectors(const string& sector) {
    Vector<string> adjacent;
    
    if (sector.empty() || sector.length() < 3) return adjacent;
    
    char series = sector[0];
    int number = 0;
    
    string numStr = "";
    for (int i = 2; i < (int)sector.length(); ++i) {
        numStr += sector[i];
    }
    
    try {
        if (!numStr.empty()) {
            number = std::stoi(numStr);
        }
    } catch (...) {
        return adjacent;
    }
    
    if (number > 6) {
        adjacent.push_back(string(1, series) + "-" + std::to_string(number - 1));
    }
    if (number < 12) {
        adjacent.push_back(string(1, series) + "-" + std::to_string(number + 1));
    }
    if (series > 'E') {
        adjacent.push_back(string(1, series - 1) + "-" + std::to_string(number));
    }
    if (series < 'I') {
        adjacent.push_back(string(1, series + 1) + "-" + std::to_string(number));
    }
    
    return adjacent;
}

inline bool TransportManager::areSectorsAdjacent(const string& sector1, const string& sector2) {
    if (sector1 == sector2) return true;
    
    Vector<string> adjacent = getAdjacentSectors(sector1);
    for (int i = 0; i < adjacent.getSize(); ++i) {
        if (adjacent[i] == sector2) return true;
    }
    return false;
}

// ==================== BUS MANAGEMENT ====================

inline Bus* TransportManager::createBus(const string& busNo, const string& company,
                                        const string& currentStop) {
    Bus* bus = new Bus(busNo, company, currentStop);
    buses.push_back(bus);
    busLookup.insert(busNo, bus);
    
    Vector<Bus*>* existingList = companyLookup.get(company);
    if (existingList) {
        existingList->push_back(bus);
    } else {
        Vector<Bus*> newList;
        newList.push_back(bus);
        companyLookup.insert(company, newList);
    }
    
    return bus;
}

inline bool TransportManager::setBusRoute(const string& busNo, const Vector<int>& route,
                                          double distance, const string& startStopID,
                                          const string& endStopID) {
    Bus* bus = findBusByNumber(busNo);
    if (!bus) return false;
    
    bus->setRoute(route, distance);
    bus->setStops(startStopID, endStopID);
    
    for (int i = 0; i < route.getSize(); ++i) {
        int stopID = route[i];
        Vector<Bus*>* busesAtStop = stopLookup.get(stopID);
        if (busesAtStop) {
            bool found = false;
            for (int j = 0; j < busesAtStop->getSize(); ++j) {
                if ((*busesAtStop)[j]->getBusNo() == busNo) {
                    found = true;
                    break;
                }
            }
            if (!found) busesAtStop->push_back(bus);
        } else {
            Vector<Bus*> newList;
            newList.push_back(bus);
            stopLookup.insert(stopID, newList);
        }
    }
    
    return true;
}

inline Bus* TransportManager::findBusByNumber(const string& busNo) const {
    Bus** result = busLookup.get(busNo);
    return result ? *result : nullptr;
}

inline Vector<Bus*> TransportManager::findBusesByCompany(const string& company) const {
    Vector<Bus*>* result = companyLookup.get(company);
    return result ? *result : Vector<Bus*>();
}

inline Vector<Bus*> TransportManager::findBusesAtStop(int stopNodeID) const {
    Vector<Bus*>* result = stopLookup.get(stopNodeID);
    return result ? *result : Vector<Bus*>();
}

inline Vector<Bus*> TransportManager::findBusesOnRoute(int fromNodeID, int toNodeID) const {
    Vector<Bus*> result;
    for (int i = 0; i < buses.getSize(); ++i) {
        Bus* bus = buses[i];
        int fromPos = bus->getRoutePosition(fromNodeID);
        int toPos = bus->getRoutePosition(toNodeID);
        if (fromPos != -1 && toPos != -1 && fromPos < toPos) {
            result.push_back(bus);
        }
    }
    return result;
}

inline Bus* TransportManager::getBus(int index) const {
    if (index >= 0 && index < buses.getSize()) return buses[index];
    return nullptr;
}

// ==================== SCHOOL BUS MANAGEMENT ====================

inline SchoolBus* TransportManager::createSchoolBus(const string& id, const string& schoolID,
                                                    int schoolNodeID, const string& sector) {
    SchoolBus* sb = new SchoolBus(id, schoolID, schoolNodeID, sector);
    schoolBuses.push_back(sb);
    schoolBusLookup.insert(id, sb);
    
    Vector<SchoolBus*>* schoolList = schoolLookup.get(schoolID);
    if (schoolList) {
        schoolList->push_back(sb);
    } else {
        Vector<SchoolBus*> newList;
        newList.push_back(sb);
        schoolLookup.insert(schoolID, newList);
    }
    
    Vector<SchoolBus*>* sectorList = sectorSchoolBusLookup.get(sector);
    if (sectorList) {
        sectorList->push_back(sb);
    } else {
        Vector<SchoolBus*> newList;
        newList.push_back(sb);
        sectorSchoolBusLookup.insert(sector, newList);
    }
    
    return sb;
}

inline SchoolBus* TransportManager::findSchoolBusByID(const string& id) const {
    SchoolBus** result = schoolBusLookup.get(id);
    return result ? *result : nullptr;
}

inline Vector<SchoolBus*> TransportManager::getSchoolBusesBySchool(const string& schoolID) const {
    Vector<SchoolBus*>* result = schoolLookup.get(schoolID);
    return result ? *result : Vector<SchoolBus*>();
}

inline Vector<SchoolBus*> TransportManager::getSchoolBusesBySector(const string& sector) const {
    Vector<SchoolBus*>* result = sectorSchoolBusLookup.get(sector);
    return result ? *result : Vector<SchoolBus*>();
}

inline Vector<SchoolBus*> TransportManager::getAvailableSchoolBuses() const {
    Vector<SchoolBus*> result;
    for (int i = 0; i < schoolBuses.getSize(); ++i) {
        if (schoolBuses[i]->isAvailable()) {
            result.push_back(schoolBuses[i]);
        }
    }
    return result;
}

inline SchoolBus* TransportManager::findSchoolBusForRoute(const string& fromSector, 
                                                          const string& toSector) const {
    Vector<SchoolBus*> fromBuses = getSchoolBusesBySector(fromSector);
    for (int i = 0; i < fromBuses.getSize(); ++i) {
        if (fromBuses[i]->isAvailable() && fromBuses[i]->isSectorInPriority(toSector)) {
            return fromBuses[i];
        }
    }
    
    Vector<SchoolBus*> toBuses = getSchoolBusesBySector(toSector);
    for (int i = 0; i < toBuses.getSize(); ++i) {
        if (toBuses[i]->isAvailable() && toBuses[i]->isSectorInPriority(fromSector)) {
            return toBuses[i];
        }
    }
    
    for (int i = 0; i < schoolBuses.getSize(); ++i) {
        if (schoolBuses[i]->isAvailable() &&
            schoolBuses[i]->isSectorInPriority(fromSector) &&
            schoolBuses[i]->isSectorInPriority(toSector)) {
            return schoolBuses[i];
        }
    }
    
    return nullptr;
}

inline SchoolBus* TransportManager::getSchoolBus(int index) const {
    if (index >= 0 && index < schoolBuses.getSize()) return schoolBuses[index];
    return nullptr;
}

// ==================== SCHOOL BUS MANAGEMENT ====================

inline void TransportManager::createPickupPoint(int nodeID, const string& sector, 
                                                 const string& locationName, bool isResidential) {
    PickupPoint* pp = new PickupPoint(nodeID, sector, locationName, isResidential);
    pickupPoints.insert(nodeID, pp);
    
    Vector<int>* sectorNodes = sectorPickupPoints.get(sector);
    if (sectorNodes) {
        sectorNodes->push_back(nodeID);
    } else {
        Vector<int> newList;
        newList.push_back(nodeID);
        sectorPickupPoints.insert(sector, newList);
    }
}

inline bool TransportManager::addStudentToPickupPoint(int nodeID, const StudentPassenger& student) {
    PickupPoint** pp = pickupPoints.get(nodeID);
    if (!pp || !(*pp)) return false;
    return (*pp)->waitingStudents.enqueue(student);
}

inline PickupPoint* TransportManager::getPickupPoint(int nodeID) const {
    PickupPoint** pp = pickupPoints.get(nodeID);
    return pp ? *pp : nullptr;
}

inline Vector<int> TransportManager::getPickupPointsInSector(const string& sector) const {
    Vector<int>* nodes = sectorPickupPoints.get(sector);
    return nodes ? *nodes : Vector<int>();
}

inline bool TransportManager::setupSchoolBusHomeRoute(const string& busID, const Vector<int>& pickupNodes,
                                                       int schoolNodeID, const string& schoolID) {
    SchoolBus* sb = findSchoolBusByID(busID);
    if (!sb || !sb->isAvailable()) return false;
    
    sb->setPickupRoute(pickupNodes);
    sb->clearDestinationSchools();
    sb->addDestinationSchool(schoolID, schoolNodeID);
    
    return true;
}

inline bool TransportManager::dispatchSchoolBusForHomePickup(const string& busID) {
    SchoolBus* sb = findSchoolBusByID(busID);
    if (!sb || !sb->isAvailable()) return false;
    
    sb->startHomePickupRoute();
    return true;
}

inline int TransportManager::getStudentsWaitingAtPickup(int nodeID) const {
    PickupPoint* pp = getPickupPoint(nodeID);
    if (!pp) return 0;
    return pp->waitingStudents.size();
}

// ==================== AMBULANCE MANAGEMENT ====================

inline Ambulance* TransportManager::createAmbulance(const string& id, const string& hospitalID,
                                                    int hospitalNodeID, const string& sector) {
    Ambulance* amb = new Ambulance(id, hospitalID, hospitalNodeID, sector);
    ambulances.push_back(amb);
    ambulanceLookup.insert(id, amb);
    
    Vector<Ambulance*>* hospList = hospitalAmbulanceLookup.get(hospitalID);
    if (hospList) {
        hospList->push_back(amb);
    } else {
        Vector<Ambulance*> newList;
        newList.push_back(amb);
        hospitalAmbulanceLookup.insert(hospitalID, newList);
    }
    
    Vector<Ambulance*>* sectorList = sectorAmbulanceLookup.get(sector);
    if (sectorList) {
        sectorList->push_back(amb);
    } else {
        Vector<Ambulance*> newList;
        newList.push_back(amb);
        sectorAmbulanceLookup.insert(sector, newList);
    }
    
    return amb;
}

inline Ambulance* TransportManager::findAmbulanceByID(const string& id) const {
    Ambulance** result = ambulanceLookup.get(id);
    return result ? *result : nullptr;
}

inline Vector<Ambulance*> TransportManager::getAmbulancesByHospital(const string& hospitalID) const {
    Vector<Ambulance*>* result = hospitalAmbulanceLookup.get(hospitalID);
    return result ? *result : Vector<Ambulance*>();
}

inline Vector<Ambulance*> TransportManager::getAmbulancesBySector(const string& sector) const {
    Vector<Ambulance*>* result = sectorAmbulanceLookup.get(sector);
    return result ? *result : Vector<Ambulance*>();
}

inline Vector<Ambulance*> TransportManager::getAvailableAmbulances() const {
    Vector<Ambulance*> result;
    for (int i = 0; i < ambulances.getSize(); ++i) {
        if (ambulances[i]->isAvailable()) {
            result.push_back(ambulances[i]);
        }
    }
    return result;
}

inline Ambulance* TransportManager::findAmbulanceForTransfer(const string& sourceSector,
                                                              const string& destSector) const {
    Vector<Ambulance*> sourceAmbs = getAmbulancesBySector(sourceSector);
    for (int i = 0; i < sourceAmbs.getSize(); ++i) {
        if (sourceAmbs[i]->isAvailable() && sourceAmbs[i]->isSectorInPriority(destSector)) {
            return sourceAmbs[i];
        }
    }
    
    Vector<Ambulance*> destAmbs = getAmbulancesBySector(destSector);
    for (int i = 0; i < destAmbs.getSize(); ++i) {
        if (destAmbs[i]->isAvailable() && destAmbs[i]->isSectorInPriority(sourceSector)) {
            return destAmbs[i];
        }
    }
    
    Vector<string> adjacentToSource = getAdjacentSectors(sourceSector);
    for (int i = 0; i < adjacentToSource.getSize(); ++i) {
        Vector<Ambulance*> adjAmbs = getAmbulancesBySector(adjacentToSource[i]);
        for (int j = 0; j < adjAmbs.getSize(); ++j) {
            if (adjAmbs[j]->isAvailable()) {
                return adjAmbs[j];
            }
        }
    }
    
    Vector<Ambulance*> available = getAvailableAmbulances();
    return available.getSize() > 0 ? available[0] : nullptr;
}

inline Ambulance* TransportManager::getAmbulance(int index) const {
    if (index >= 0 && index < ambulances.getSize()) return ambulances[index];
    return nullptr;
}

// ==================== PATIENT TRANSFER DISPATCH ====================

inline string TransportManager::requestTransfer(const string& patientCNIC, const string& patientName,
                                                const string& sourceHospitalID, int sourceNodeID, 
                                                const string& sourceSector,
                                                const string& destHospitalID, int destNodeID, 
                                                const string& destSector,
                                                const string& priority, const string& condition) {
    string requestID = "XFER-" + std::to_string(++transferIDCounter);
    
    PatientTransfer transfer(requestID, patientCNIC, patientName,
                            sourceHospitalID, sourceNodeID, sourceSector,
                            destHospitalID, destNodeID, destSector,
                            priority, condition);
    
    transferQueue.push(transfer);
    ++totalTransferRequests;
    
    return requestID;
}

inline Ambulance* TransportManager::dispatchNextTransfer() {
    if (transferQueue.empty()) return nullptr;
    
    PatientTransfer transfer = transferQueue.top();
    Ambulance* amb = findAmbulanceForTransfer(transfer.sourceSector, transfer.destSector);
    
    if (amb) {
        transferQueue.pop();
        amb->acceptTransfer(&transfer);
        activeTransfers.push_back(transfer);
    }
    
    return amb;
}

inline bool TransportManager::dispatchAmbulance(const string& ambulanceID, const string& requestID) {
    Ambulance* amb = findAmbulanceByID(ambulanceID);
    if (!amb || !amb->isAvailable()) return false;
    return false;
}

inline PatientTransfer* TransportManager::peekNextTransfer() {
    if (transferQueue.empty()) return nullptr;
    return &transferQueue.top();
}

// ==================== PASSENGER QUEUE MANAGEMENT ====================

inline void TransportManager::initializeStopQueue(int stopNodeID, const string& stopName, 
                                                  const string& sector) {
    BusStopQueue* queue = new BusStopQueue(stopNodeID, stopName, sector);
    stopQueues.insert(stopNodeID, queue);
}

inline bool TransportManager::addPassengerToStop(int stopNodeID, const Passenger& passenger) {
    BusStopQueue** queuePtr = stopQueues.get(stopNodeID);
    if (!queuePtr || !(*queuePtr)) {
        initializeStopQueue(stopNodeID, "", "");
        queuePtr = stopQueues.get(stopNodeID);
    }
    return (*queuePtr)->waitingPassengers.enqueue(passenger);
}

inline int TransportManager::getWaitingCount(int stopNodeID) const {
    BusStopQueue** queuePtr = stopQueues.get(stopNodeID);
    if (queuePtr && *queuePtr) {
        return (*queuePtr)->waitingPassengers.size();
    }
    return 0;
}

inline BusStopQueue* TransportManager::getStopQueue(int stopNodeID) const {
    BusStopQueue** queuePtr = stopQueues.get(stopNodeID);
    return queuePtr ? *queuePtr : nullptr;
}

inline void TransportManager::processBusArrival(Bus* bus, int stopNodeID) {
    if (!bus) return;
    
    bus->alightPassengers();
    
    BusStopQueue* queue = getStopQueue(stopNodeID);
    if (queue) {
        while (!queue->waitingPassengers.empty() && !bus->isFull()) {
            Passenger p = queue->waitingPassengers.dequeue();
            
            int destPos = bus->getRoutePosition(p.destinationStopID);
            int currentPos = bus->getCurrentRouteIndex();
            
            if (destPos > currentPos) {
                bus->addWaitingPassenger(p);
                bus->boardWaitingPassengers();
            } else {
                queue->waitingPassengers.enqueue(p);
            }
        }
    }
}

// ==================== SIMULATION ====================

inline void TransportManager::runSimulationStep() {
    ++simulationStep;
    
    simulateBusStep();
    
    simulateSchoolBusStep();
    
    simulateAmbulanceStep();
    
    simulateRickshawStep();
    
    while (getPendingTransferCount() > 0 && getAvailableAmbulances().getSize() > 0) {
        if (!dispatchNextTransfer()) break;
    }
}

inline void TransportManager::runSimulationSteps(int steps) {
    for (int i = 0; i < steps; ++i) {
        runSimulationStep();
    }
}

inline void TransportManager::resetSimulation() {
    simulationStep = 0;
    
    for (int i = 0; i < buses.getSize(); ++i) {
        buses[i]->resetToRouteStart();
    }
    
    for (int i = 0; i < schoolBuses.getSize(); ++i) {
        schoolBuses[i]->resetToBase();
    }
    
    for (int i = 0; i < ambulances.getSize(); ++i) {
        ambulances[i]->resetToBase();
    }
}

inline void TransportManager::simulateBusStep() {
    for (int i = 0; i < buses.getSize(); ++i) {
        Bus* bus = buses[i];
        
        if (bus->isAtRouteEnd()) {
            bus->resetRoute();
            continue;
        }
        
        int currentNode = bus->getCurrentNodeID();
        int nextNode = bus->getNextNodeID();
        
        if (bus->getIsStuck()) {
            if (cityGraph && nextNode != -1) {
                if (cityGraph->tryEnterEdge(currentNode, nextNode)) {
                    bus->setIsStuck(false);
                } else {
                    bus->setIsStuck(true);
                    continue;
                }
            }
        }
        
        double progress = bus->getProgressOnEdge();
        double baseSpeed = 0.2;
        if (cityGraph && nextNode != -1) {
            double congestion = cityGraph->getEdgeCongestion(currentNode, nextNode);
            double speedMultiplier = 1.0 - 0.7 * congestion * congestion;
            if (speedMultiplier < 0.1) speedMultiplier = 0.1;
            progress += baseSpeed * speedMultiplier;
        } else {
            progress += baseSpeed;
        }
        
        bus->setProgressOnEdge(progress);
        
        if (cityGraph && currentNode >= 0 && nextNode >= 0) {
            CityNode* currNode = cityGraph->getNode(currentNode);
            CityNode* nxtNode = cityGraph->getNode(nextNode);
            if (currNode && nxtNode) {
                double t = (progress > 1.0) ? 1.0 : progress;
                bus->setRenderPosition(
                    currNode->lat + t * (nxtNode->lat - currNode->lat),
                    currNode->lon + t * (nxtNode->lon - currNode->lon));
            }
        }
        
        if (progress >= 1.0) {
            if (cityGraph && nextNode != -1) cityGraph->leaveEdge(currentNode, nextNode);
            processBusArrival(bus, nextNode);
            if (bus->moveToNextStop()) {
                int newCurrent = bus->getCurrentNodeID();
                int newNext = bus->getNextNodeID();
                if (cityGraph && newNext != -1) {
                    if (!cityGraph->tryEnterEdge(newCurrent, newNext)) {
                        bus->setIsStuck(true);
                    }
                }
            }
        }
    }
}

inline void TransportManager::simulateSchoolBusStep() {
    for (int i = 0; i < schoolBuses.getSize(); ++i) {
        SchoolBus* sb = schoolBuses[i];
        string status = sb->getSchoolBusStatus();
        
        // Handle stuck state
        if (sb->getIsStuck()) {
            int currentNode = sb->getCurrentNodeID();
            int nextNode = sb->getNextNodeID();
            if (cityGraph && nextNode != -1) {
                if (cityGraph->tryEnterEdge(currentNode, nextNode)) {
                    sb->setIsStuck(false);
                } else {
                    continue;
                }
            }
        }
        
        if (status == SchoolBusStatus::AVAILABLE) {
            Vector<int> pickups = getPickupPointsInSector(sb->getHomeSector());
            bool hasWaiting = false;
            for (int j = 0; j < pickups.getSize(); ++j) {
                if (getStudentsWaitingAtPickup(pickups[j]) > 0) {
                    hasWaiting = true;
                    break;
                }
            }
            
            if (hasWaiting && pickups.getSize() > 0) {
                sb->setPickupRoute(pickups);
                sb->startHomePickupRoute();
            }
        }
        else if (status == SchoolBusStatus::EN_ROUTE_HOME_PICKUP ||
                 status == SchoolBusStatus::EN_ROUTE_TO_SCHOOL ||
                 status == SchoolBusStatus::EN_ROUTE_SCHOOL_TO_SCHOOL ||
                 status == SchoolBusStatus::RETURNING) {
            
            double progress = sb->getProgressOnEdge();
            double baseSpeed = 0.15;
            int currentNode = sb->getCurrentNodeID();
            int nextNode = sb->getNextNodeID();
            
            if (cityGraph && nextNode != -1) {
                double congestion = cityGraph->getEdgeCongestion(currentNode, nextNode);
                double speedMultiplier = 1.0 - 0.6 * congestion * congestion;
                if (speedMultiplier < 0.15) speedMultiplier = 0.15;
                progress += baseSpeed * speedMultiplier;
            } else {
                progress += baseSpeed;
            }
            
            sb->setProgressOnEdge(progress);
            
            // Update render position
            if (cityGraph && currentNode >= 0 && nextNode >= 0) {
                CityNode* currNode = cityGraph->getNode(currentNode);
                CityNode* nxtNode = cityGraph->getNode(nextNode);
                if (currNode && nxtNode) {
                    double t = (progress > 1.0) ? 1.0 : progress;
                    sb->setRenderPosition(
                        currNode->lat + t * (nxtNode->lat - currNode->lat),
                        currNode->lon + t * (nxtNode->lon - currNode->lon));
                }
            }
            
            if (progress >= 1.0) {
                if (cityGraph && nextNode != -1) cityGraph->leaveEdge(currentNode, nextNode);
                
                if (!sb->moveToNextStop()) {
                    if (status == SchoolBusStatus::EN_ROUTE_HOME_PICKUP) {
                        int pickupNode = sb->getNextPickupPointNode();
                        if (pickupNode != -1) processSchoolBusPickup(sb, pickupNode);
                    } else if (status == SchoolBusStatus::EN_ROUTE_TO_SCHOOL ||
                               status == SchoolBusStatus::EN_ROUTE_SCHOOL_TO_SCHOOL) {
                        sb->setSchoolBusStatus(SchoolBusStatus::AT_SCHOOL);
                    } else if (status == SchoolBusStatus::RETURNING) {
                        sb->arriveAtBase();
                    }
                } else {
                    int newCurrent = sb->getCurrentNodeID();
                    int newNext = sb->getNextNodeID();
                    if (cityGraph && newNext != -1) {
                        if (!cityGraph->tryEnterEdge(newCurrent, newNext)) {
                            sb->setIsStuck(true);
                        }
                    }
                }
            }
        }
        else if (status == SchoolBusStatus::AT_PICKUP_POINT || 
                 status == SchoolBusStatus::LOADING_STUDENTS) {
            int pickupNode = sb->getCurrentNodeID();
            PickupPoint* pp = getPickupPoint(pickupNode);
            
            if (pp) {
                while (!pp->waitingStudents.empty() && !sb->isFull()) {
                    StudentPassenger student = pp->waitingStudents.dequeue();
                    sb->boardStudent(student);
                }
            }
            
            if (sb->isFull() || sb->allPickupsComplete()) {
                sb->startSchoolRoute();
            } else {
                sb->advanceToNextPickupPoint();
                sb->setSchoolBusStatus(SchoolBusStatus::EN_ROUTE_HOME_PICKUP);
            }
        }
        else if (status == SchoolBusStatus::AT_SCHOOL || 
                 status == SchoolBusStatus::UNLOADING) {
            sb->dropoffAllStudents();
            sb->completeTrip();
        }
    }
}

inline void TransportManager::simulateAmbulanceStep() {
    for (int i = 0; i < ambulances.getSize(); ++i) {
        Ambulance* amb = ambulances[i];
        string status = amb->getAmbulanceStatus();
        
        if (status == AmbulanceStatus::AVAILABLE) {
            continue;
        }
        
        // Handle stuck state
        if (amb->getIsStuck()) {
            int currentNode = amb->getCurrentNodeID();
            int nextNode = amb->getNextNodeID();
            if (cityGraph && nextNode != -1) {
                // Ambulances have priority - always try to enter
                if (cityGraph->tryEnterEdge(currentNode, nextNode)) {
                    amb->setIsStuck(false);
                } else {
                    continue;
                }
            }
        }
        
        // Update progress for moving ambulances
        if (status == AmbulanceStatus::DISPATCHED || 
            status == AmbulanceStatus::TRANSPORTING ||
            status == AmbulanceStatus::RETURNING) {
            
            double progress = amb->getProgressOnEdge();
            double baseSpeed = 0.3;  // Ambulances are faster
            int currentNode = amb->getCurrentNodeID();
            int nextNode = amb->getNextNodeID();
            
            if (cityGraph && nextNode != -1) {
                double congestion = cityGraph->getEdgeCongestion(currentNode, nextNode);
                // Ambulances are less affected by congestion (emergency)
                double speedMultiplier = 1.0 - 0.3 * congestion * congestion;
                if (speedMultiplier < 0.3) speedMultiplier = 0.3;
                progress += baseSpeed * speedMultiplier;
            } else {
                progress += baseSpeed;
            }
            
            amb->setProgressOnEdge(progress);
            
            // Update render position
            if (cityGraph && currentNode >= 0 && nextNode >= 0) {
                CityNode* currNode = cityGraph->getNode(currentNode);
                CityNode* nxtNode = cityGraph->getNode(nextNode);
                if (currNode && nxtNode) {
                    double t = (progress > 1.0) ? 1.0 : progress;
                    amb->setRenderPosition(
                        currNode->lat + t * (nxtNode->lat - currNode->lat),
                        currNode->lon + t * (nxtNode->lon - currNode->lon));
                }
            }
            
            if (progress >= 1.0) {
                if (cityGraph && nextNode != -1) cityGraph->leaveEdge(currentNode, nextNode);
                
                if (!amb->moveToNextStop()) {
                    // Reached destination
                    if (status == AmbulanceStatus::DISPATCHED) {
                        amb->arriveAtPickup();
                        amb->loadPatient();
                    } else if (status == AmbulanceStatus::TRANSPORTING) {
                        amb->arriveAtDestination();
                    } else if (status == AmbulanceStatus::RETURNING) {
                        amb->arriveAtBase();
                    }
                } else {
                    int newCurrent = amb->getCurrentNodeID();
                    int newNext = amb->getNextNodeID();
                    if (cityGraph && newNext != -1) {
                        if (!cityGraph->tryEnterEdge(newCurrent, newNext)) {
                            amb->setIsStuck(true);
                        }
                    }
                }
            }
        }
        else if (status == AmbulanceStatus::AT_PICKUP || 
                 status == AmbulanceStatus::LOADING_PATIENT) {
            amb->loadPatient();
            amb->startTransport();
        }
        else if (status == AmbulanceStatus::AT_DESTINATION || 
                 status == AmbulanceStatus::UNLOADING) {
            amb->unloadPatient();
            amb->completeTransfer();
        }
    }
}

inline void TransportManager::processSchoolBusPickup(SchoolBus* sb, int pickupNodeID) {
    if (!sb) return;
    
    sb->setSchoolBusStatus(SchoolBusStatus::AT_PICKUP_POINT);
    
    PickupPoint* pp = getPickupPoint(pickupNodeID);
    if (pp) {
        while (!pp->waitingStudents.empty() && !sb->isFull()) {
            StudentPassenger student = pp->waitingStudents.dequeue();
            sb->boardStudent(student);
        }
    }
}

inline void TransportManager::processSchoolBusSchoolArrival(SchoolBus* sb, const string& schoolID, int schoolNodeID) {
    if (!sb) return;
    
    sb->setCurrentSchool(schoolID);
    sb->setSchoolBusStatus(SchoolBusStatus::AT_SCHOOL);
    sb->dropoffStudents();
}

inline TransportStats TransportManager::getStats() const {
    TransportStats stats;
    
    stats.totalBuses = buses.getSize();
    stats.totalSchoolBuses = schoolBuses.getSize();
    stats.totalAmbulances = ambulances.getSize();
    
    for (int i = 0; i < buses.getSize(); ++i) {
        Bus* bus = buses[i];
        if (bus->getStatus() == VehicleStatus::EN_ROUTE) stats.activeBuses++;
        stats.totalBusPassengers += bus->getTotalPassengersServed();
        stats.totalBusFares += bus->getTotalFareCollected();
        stats.totalBusTrips += bus->getTripsCompleted();
    }
    
    for (int i = 0; i < schoolBuses.getSize(); ++i) {
        SchoolBus* sb = schoolBuses[i];
        if (!sb->isAvailable()) stats.activeSchoolBuses++;
        stats.totalStudentsTransported += sb->getTotalStudentsTransported();
        stats.schoolBusTrips += sb->getTripsCompleted();
    }
    
    for (int i = 0; i < ambulances.getSize(); ++i) {
        Ambulance* amb = ambulances[i];
        if (amb->isAvailable()) stats.availableAmbulances++;
        stats.totalTransfers += amb->getTotalTransfersCompleted();
        stats.criticalTransfers += amb->getCriticalTransfersHandled();
    }
    
    stats.pendingTransfers = transferQueue.size();
    
    return stats;
}

inline bool TransportManager::loadBusesFromCSV(const string& filename, bool hasHeader) {
    ifstream file(filename);
    if (!file.is_open()) return false;
    
    string line;
    if (hasHeader) std::getline(file, line);
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        string fields[4];
        int idx = 0;
        string cur = "";
        
        for (int i = 0; i < (int)line.size(); ++i) {
            char c = line[i];
            if (c == ',' && idx < 3) {
                fields[idx++] = trim(cur);
                cur.clear();
            } else {
                cur += c;
            }
        }
        fields[idx] = trim(cur);
        
        string busNo = fields[0];
        string company = fields[1];
        string currentStop = fields[2];
        string routeStr = fields[3];
        
        Bus* bus = createBus(busNo, company, currentStop);
        
        Vector<string> routeStops = parseRoute(routeStr);
        if (routeStops.getSize() >= 2) {
            bus->setStops(routeStops[0], routeStops[routeStops.getSize() - 1]);
        }
    }
    
    file.close();
    return true;
}

inline bool TransportManager::loadAmbulancesFromCSV(const string& filename, bool hasHeader) {
    ifstream file(filename);
    if (!file.is_open()) return false;
    
    string line;
    if (hasHeader) std::getline(file, line);
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        string fields[4];
        int idx = 0;
        string cur = "";
        
        for (int i = 0; i < (int)line.size(); ++i) {
            char c = line[i];
            if (c == ',' && idx < 3) {
                fields[idx++] = trim(cur);
                cur.clear();
            } else {
                cur += c;
            }
        }
        fields[idx] = trim(cur);
        
        string ambID = fields[0];
        string hospitalID = fields[1];
        int hospitalNode = 0;
        string sector = fields[3];
        
        try {
            if (!fields[2].empty()) {
                hospitalNode = std::stoi(fields[2]);
            }
        } catch (...) {
            hospitalNode = 0; 
        }
        
        if (!ambID.empty() && !hospitalID.empty() && !sector.empty()) {
            createAmbulance(ambID, hospitalID, hospitalNode, sector);
        }
    }
    
    file.close();
    return true;
}

inline bool TransportManager::loadSchoolBusesFromCSV(const string& filename, bool hasHeader) {
    ifstream file(filename);
    if (!file.is_open()) return false;
    
    string line;
    if (hasHeader) std::getline(file, line);
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        string fields[4];
        int idx = 0;
        string cur = "";
        
        for (int i = 0; i < (int)line.size(); ++i) {
            char c = line[i];
            if (c == ',' && idx < 3) {
                fields[idx++] = trim(cur);
                cur.clear();
            } else {
                cur += c;
            }
        }
        fields[idx] = trim(cur);
        
        string busID = fields[0];
        string schoolID = fields[1];
        int schoolNode = 0;
        string sector = fields[3];
        
        try {
            if (!fields[2].empty()) {
                schoolNode = std::stoi(fields[2]);
            }
        } catch (...) {
            schoolNode = 0;
        }
        
        if (!busID.empty() && !schoolID.empty() && !sector.empty()) {
            createSchoolBus(busID, schoolID, schoolNode, sector);
        }
    }
    
    file.close();
    return true;
}

inline string TransportManager::trim(const string& s) const {
    int start = 0, end = (int)s.size() - 1;
    while (start <= end && (s[start] == ' ' || s[start] == '\t' || 
           s[start] == '\r' || s[start] == '"')) start++;
    while (end >= start && (s[end] == ' ' || s[end] == '\t' || 
           s[end] == '\r' || s[end] == '"')) end--;
    return (start > end) ? "" : s.substr(start, end - start + 1);
}

inline Vector<string> TransportManager::parseRoute(const string& routeStr) const {
    Vector<string> stops;
    string current = "";
    
    for (int i = 0; i < (int)routeStr.size(); ++i) {
        if (routeStr[i] == '>') {
            string stop = trim(current);
            if (!stop.empty()) stops.push_back(stop);
            current = "";
        } else {
            current += routeStr[i];
        }
    }
    
    string stop = trim(current);
    if (!stop.empty()) stops.push_back(stop);
    
    return stops;
}

// ==================== RICKSHAW MANAGEMENT ====================

inline Vehicle* TransportManager::spawnRickshaw(const string& sector, int startNodeID) {
    string id = "RICK-" + std::to_string(++rickshawIDCounter);
    Vehicle* rickshaw = new Vehicle(id, VehicleType::RICKSHAW, 3);  // 3 passenger capacity
    rickshaw->setStatus(VehicleStatus::IDLE);
    rickshaw->setHomeSector(sector);
    rickshaw->setHomeNode(startNodeID);
    rickshaw->setCurrentLocation(startNodeID, "", sector);
    
    rickshaws.push_back(rickshaw);
    
    Vector<Vehicle*>* sectorList = sectorRickshawLookup.get(sector);
    if (sectorList) {
        sectorList->push_back(rickshaw);
    } else {
        Vector<Vehicle*> newList;
        newList.push_back(rickshaw);
        sectorRickshawLookup.insert(sector, newList);
    }
    
    return rickshaw;
}

inline void TransportManager::spawnRickshaws(int count) {
    if (!cityGraph) return;
    
    // Distribute rickshaws across sectors
    int nodeCount = cityGraph->getNodeCount();
    if (nodeCount == 0) return;
    
    for (int i = 0; i < count; ++i) {
        // Find a random corner node to spawn at
        int attempts = 0;
        while (attempts < 100) {
            int nodeIdx = rand() % nodeCount;
            CityNode* node = cityGraph->getNode(nodeIdx);
            if (node && node->type == "CORNER" && !node->sector.empty()) {
                spawnRickshaw(node->sector, node->id);
                break;
            }
            attempts++;
        }
    }
}

inline Vehicle* TransportManager::findAvailableRickshaw(int nearNodeID, const string& sector) const {
    // First check rickshaws in the same sector
    Vector<Vehicle*>* sectorList = sectorRickshawLookup.get(sector);
    if (sectorList) {
        for (int i = 0; i < sectorList->getSize(); ++i) {
            Vehicle* rick = (*sectorList)[i];
            if (rick->getStatus() == VehicleStatus::IDLE) {
                return rick;
            }
        }
    }
    
    // Check adjacent sectors
    Vector<string> adjacent = getAdjacentSectors(sector);
    for (int i = 0; i < adjacent.getSize(); ++i) {
        Vector<Vehicle*>* adjList = sectorRickshawLookup.get(adjacent[i]);
        if (adjList) {
            for (int j = 0; j < adjList->getSize(); ++j) {
                Vehicle* rick = (*adjList)[j];
                if (rick->getStatus() == VehicleStatus::IDLE) {
                    return rick;
                }
            }
        }
    }
    
    // Last resort - any available rickshaw
    for (int i = 0; i < rickshaws.getSize(); ++i) {
        if (rickshaws[i]->getStatus() == VehicleStatus::IDLE) {
            return rickshaws[i];
        }
    }
    
    return nullptr;
}

inline bool TransportManager::dispatchRickshaw(Vehicle* rickshaw, int pickupNodeID, int destNodeID, const string& passengerCNIC) {
    if (!rickshaw || !cityGraph) return false;
    if (rickshaw->getStatus() != VehicleStatus::IDLE) return false;
    
    // Calculate route from current position to pickup
    double pickupDist = 0;
    Vector<int> pickupRoute = cityGraph->findShortestPath(rickshaw->getCurrentNodeID(), pickupNodeID, pickupDist);
    
    if (pickupRoute.getSize() == 0) return false;
    
    // Store destination for later
    rickshaw->setStatus(VehicleStatus::PICKING_UP);
    rickshaw->setRouteSimple(pickupRoute, pickupDist);
    rickshaw->addPassenger(passengerCNIC);
    
    // Try to enter the first edge
    if (pickupRoute.getSize() > 1) {
        int currentNode = pickupRoute[0];
        int nextNode = pickupRoute[1];
        if (!cityGraph->tryEnterEdge(currentNode, nextNode)) {
            rickshaw->setIsStuck(true);
        }
    }
    
    return true;
}

inline void TransportManager::simulateRickshawStep() {
    for (int i = 0; i < rickshaws.getSize(); ++i) {
        Vehicle* rick = rickshaws[i];
        string status = rick->getStatus();
        
        if (status == VehicleStatus::IDLE) {
            // Roam randomly looking for passengers
            if (rand() % 20 == 0 && cityGraph) {  // 5% chance to move
                int currentNode = rick->getCurrentNodeID();
                CityNode* node = cityGraph->getNode(currentNode);
                if (node) {
                    const LinkedList<Edge>& edges = node->getRoads();
                    if (edges.size() > 0) {
                        int edgeIdx = rand() % edges.size();
                        int nextNode = edges.at(edgeIdx).destinationID;
                        
                        Vector<int> route;
                        route.push_back(currentNode);
                        route.push_back(nextNode);
                        rick->setRouteSimple(route, 0.1);
                        rick->setStatus(VehicleStatus::EN_ROUTE);
                        
                        if (!cityGraph->tryEnterEdge(currentNode, nextNode)) {
                            rick->setIsStuck(true);
                        }
                    }
                }
            }
            continue;
        }
        
        // Handle stuck state
        if (rick->getIsStuck()) {
            int currentNode = rick->getCurrentNodeID();
            int nextNode = rick->getNextNodeID();
            if (cityGraph && nextNode != -1) {
                if (cityGraph->tryEnterEdge(currentNode, nextNode)) {
                    rick->setIsStuck(false);
                } else {
                    continue;
                }
            }
        }
        
        // Moving states
        if (status == VehicleStatus::PICKING_UP || 
            status == VehicleStatus::DROPPING_OFF ||
            status == VehicleStatus::EN_ROUTE) {
            
            double progress = rick->getProgressOnEdge();
            double baseSpeed = 0.25;  // Rickshaws are medium speed
            int currentNode = rick->getCurrentNodeID();
            int nextNode = rick->getNextNodeID();
            
            if (cityGraph && nextNode != -1) {
                double congestion = cityGraph->getEdgeCongestion(currentNode, nextNode);
                double speedMultiplier = 1.0 - 0.5 * congestion * congestion;
                if (speedMultiplier < 0.2) speedMultiplier = 0.2;
                progress += baseSpeed * speedMultiplier;
            } else {
                progress += baseSpeed;
            }
            
            rick->setProgressOnEdge(progress);
            
            // Update render position
            if (cityGraph && currentNode >= 0 && nextNode >= 0) {
                CityNode* currNode = cityGraph->getNode(currentNode);
                CityNode* nxtNode = cityGraph->getNode(nextNode);
                if (currNode && nxtNode) {
                    double t = (progress > 1.0) ? 1.0 : progress;
                    rick->setRenderPosition(
                        currNode->lat + t * (nxtNode->lat - currNode->lat),
                        currNode->lon + t * (nxtNode->lon - currNode->lon));
                }
            }
            
            if (progress >= 1.0) {
                if (cityGraph && nextNode != -1) cityGraph->leaveEdge(currentNode, nextNode);
                
                if (!rick->moveToNextStop()) {
                    // Reached destination
                    if (status == VehicleStatus::PICKING_UP) {
                        // At pickup location - would load passenger and start drop-off route
                        rick->setStatus(VehicleStatus::IDLE);  // Simplified - just become idle
                        rick->clearPassengers();
                    } else if (status == VehicleStatus::DROPPING_OFF) {
                        rick->setStatus(VehicleStatus::IDLE);
                        rick->clearPassengers();
                    } else {
                        rick->setStatus(VehicleStatus::IDLE);
                    }
                } else {
                    int newCurrent = rick->getCurrentNodeID();
                    int newNext = rick->getNextNodeID();
                    if (cityGraph && newNext != -1) {
                        if (!cityGraph->tryEnterEdge(newCurrent, newNext)) {
                            rick->setIsStuck(true);
                        }
                    }
                }
            }
        }
    }
}
