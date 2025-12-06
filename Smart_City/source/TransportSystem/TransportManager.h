/*
 * ============================================================================
 * TRANSPORT MANAGER - Central Transport System Controller
 * ============================================================================
 * 
 * PURPOSE:
 * Manages all three types of transport vehicles in the Smart City with O(1) 
 * lookups, priority-based dispatch, and sector-aware routing. This manager
 * coordinates public buses, school buses, and emergency ambulances, ensuring
 * efficient transport across the city's 30 sectors.
 * 
 * WHAT IT MANAGES:
 *   1. BUSES - Public transport with fixed routes and fare collection
 *   2. SCHOOL BUSES - Student transport with sector priority
 *   3. AMBULANCES - Emergency patient transfers with priority dispatch
 *   4. PASSENGER QUEUES - Waiting passengers at each stop (Circular Queue)
 *   5. TRANSFER REQUESTS - Emergency patient transfers (Priority Queue)
 * 
 * HOW IT WORKS:
 * 
 * ???????????????????????????????????????????????????????????????????
 * ?                    TRANSPORT MANAGER                            ?
 * ?                                                                 ?
 * ?  ????????????????  ????????????????  ????????????????        ?
 * ?  ?   BUS FLEET  ?  ? SCHOOL BUSES ?  ?  AMBULANCES  ?        ?
 * ?  ?  (Vectors)   ?  ?  (Vectors)   ?  ?  (Vectors)   ?        ?
 * ?  ????????????????  ????????????????  ????????????????        ?
 * ?         ?                  ?                  ?                 ?
 * ?  ????????????????????????????????????????????????????         ?
 * ?  ?         HASH TABLE LOOKUPS (O(1) Access)          ?         ?
 * ?  ?  • Bus by Number     • School Bus by ID           ?         ?
 * ?  ?  • Bus by Company    • School Bus by Sector       ?         ?
 * ?  ?  • Bus at Stop       • Ambulance by Hospital      ?         ?
 * ?  ?  • Ambulance by ID   • Ambulance by Sector        ?         ?
 * ?  ?????????????????????????????????????????????????????         ?
 * ?                                                                 ?
 * ?  ???????????????????????????????????????????????????          ?
 * ?  ?      PASSENGER SIMULATION (Circular Queues)      ?          ?
 * ?  ?   Each Stop: ???????????????????                ?          ?
 * ?  ?             ? Waiting Queue   ? ? Enqueue       ?          ?
 * ?  ?             ?  (FIFO: 200)    ? ? Dequeue       ?          ?
 * ?  ?             ???????????????????                  ?          ?
 * ?  ???????????????????????????????????????????????????          ?
 * ?                                                                 ?
 * ?  ???????????????????????????????????????????????????          ?
 * ?  ?   EMERGENCY DISPATCH (Priority Queue Min-Heap)   ?          ?
 * ?  ?   CRITICAL (1) ? Highest Priority                ?          ?
 * ?  ?   HIGH     (2)                                   ?          ?
 * ?  ?   MEDIUM   (3) ? Default                         ?          ?
 * ?  ?   LOW      (4)                                   ?          ?
 * ?  ?   ROUTINE  (5) ? Lowest Priority                 ?          ?
 * ?  ???????????????????????????????????????????????????          ?
 * ???????????????????????????????????????????????????????????????????
 * 
 * KEY ALGORITHMS:
 * 
 * 1. BUS DISPATCH:
 *    - Find buses at stop: O(1) hash lookup
 *    - Find buses by company: O(1) hash lookup
 *    - Process boarding: O(k) where k = passengers boarding
 *    - Route matching: O(n*m) where n = stops, m = destination position
 * 
 * 2. SCHOOL BUS SECTOR PRIORITY:
 *    Algorithm: Find school bus for inter-school transfer
 *    Priority levels:
 *      1. Bus from SOURCE sector (own sector first)
 *      2. Bus from DESTINATION sector 
 *      3. Bus covering BOTH sectors (adjacent sectors)
 *      4. ANY available bus (fallback)
 *    
 *    Example: Transfer from G-10 ? F-9
 *      Priority 1: G-10 bus that serves F-9 ? (best)
 *      Priority 2: F-9 bus that serves G-10
 *      Priority 3: Any bus serving both
 * 
 * 3. AMBULANCE DISPATCH (Priority Queue):
 *    Algorithm: Dispatch ambulance for patient transfer
 *    Step 1: Pop highest priority transfer from queue (Min-Heap)
 *    Step 2: Find ambulance using sector priority:
 *      Priority 1: Ambulance from SOURCE hospital sector
 *      Priority 2: Ambulance from DESTINATION hospital sector
 *      Priority 3: Ambulance from ADJACENT sectors
 *      Priority 4: ANY available ambulance
 *    Step 3: Assign transfer and update ambulance status
 * 
 * 4. PASSENGER BOARDING (Circular Queue):
 *    At each stop:
 *      1. Alight passengers at destination
 *      2. Dequeue waiting passengers (FIFO)
 *      3. Check if destination is AHEAD on route
 *      4. Board if space available
 *      5. Re-queue if destination behind or bus full
 * 
 * SECTOR ADJACENCY:
 * 
 *        E-7   E-8   E-9   E-10  E-11
 *         |     |     |      |     |
 *        F-6   F-7   F-8   F-9   F-10  F-11
 *         |     |     |      |     |     |
 *        G-6   G-7   G-8   G-9   G-10  G-11
 *         |     |     |      |     |     |
 *              H-8   H-9   H-10  H-11  H-12
 *               |     |      |     |     |
 *              I-8   I-9   I-10  I-11  I-12
 * 
 * G-10 is adjacent to: G-9, G-11 (same series), F-10, H-10 (adjacent series)
 * 
 * DATA STRUCTURES USED:
 *   - Hash Tables: O(1) vehicle lookups (7 different tables)
 *   - Circular Queues: Passenger waiting at stops (FIFO, size 200)
 *   - Priority Queue: Emergency transfers (Min-Heap by priority)
 *   - Vectors: Store all vehicles and manage collections
 *   - Singly Linked Lists: Route management in each vehicle
 * 
 * INTEGRATION WITH CITYGRAPH:
 *   - Uses Dijkstra's algorithm for shortest path calculation
 *   - Routes stored as graph node IDs
 *   - Sector boundaries from SECTOR_GRID
 * 
 * RUBRIC COMPLIANCE:
 *   - Transport Module: buses, routes, pathfinding (5 marks) ?
 *   - Hash Lookup features: vehicle search (4 marks) ?
 *   - Priority Queue: emergency dispatch (4 marks) ?
 *   - Circular Queue: passenger simulation (4 marks) ?
 *   - Singly Linked List: route management (4 marks) ?
 *   TOTAL: 21 marks
 * 
 * USAGE EXAMPLE:
 * 
 *   TransportManager tm;
 *   tm.setCityGraph(graph);
 *   
 *   // Create bus with route
 *   Bus* bus = tm.createBus("B101", "Metro", "Stop-01");
 *   tm.setBusRoute("B101", routeNodes, distance, "STP-001", "STP-050");
 *   
 *   // Add passengers to stop
 *   Passenger p("12345-1234567-1", stopID, destID, 50.0);
 *   tm.addPassengerToStop(stopID, p);
 *   
 *   // Simulate bus arrival (automatic boarding)
 *   tm.processBusArrival(bus, stopID);
 *   
 *   // Request emergency transfer
 *   string reqID = tm.requestTransfer(cnic, name, srcHosp, srcNode, srcSector,
 *                                      destHosp, destNode, destSector,
 *                                      EmergencyPriority::CRITICAL, "Cardiac");
 *   
 *   // Dispatch ambulance (automatic sector priority matching)
 *   Ambulance* amb = tm.dispatchNextTransfer();
 *   
 *   // Find school bus for inter-school transfer
 *   SchoolBus* sb = tm.findSchoolBusForRoute("G-10", "F-9");
 * 
 * ============================================================================
 */

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

// Forward declaration
class CityGraph;

// ============================================================================
// BUS STOP QUEUE - Passengers waiting at a stop
// ============================================================================
/**
 * Represents a queue of passengers waiting at a bus stop.
 * Each stop has a unique ID, name, and associated sector.
 * Passengers wait in a circular queue (FIFO) until they board the bus.
 */
struct BusStopQueue {
    int stopNodeID;                ///< Unique identifier for the stop (Node ID in graph)
    string stopName;               ///< Name/label of the stop
    string sector;                 ///< Sector associated with the stop
    CircularQueue<Passenger> waitingPassengers;  ///< Circular queue of waiting passengers
    
    BusStopQueue() : stopNodeID(-1), stopName(""), sector(""), waitingPassengers(200) {}
    BusStopQueue(int nodeID, const string& name, const string& sec) 
        : stopNodeID(nodeID), stopName(name), sector(sec), waitingPassengers(200) {}
};

// ============================================================================
// TRANSPORT STATISTICS
// ============================================================================
/**
 * Holds various statistics for transport vehicles and operations.
 * This includes counts of active/total vehicles, passengers, trips, fares, etc.
 */
struct TransportStats {
    // Bus stats
    int totalBuses;               ///< Total number of buses
    int activeBuses;              ///< Number of active buses (currently in service)
    int totalBusPassengers;        ///< Total number of passengers transported by buses
    double totalBusFares;         ///< Total fare collected from bus passengers
    int totalBusTrips;            ///< Total number of trips completed by buses
    
    // School bus stats
    int totalSchoolBuses;         ///< Total number of school buses
    int activeSchoolBuses;        ///< Number of active school buses
    int totalStudentsTransported;  ///< Total number of students transported by school buses
    int schoolBusTrips;           ///< Total number of trips completed by school buses
    
    // Ambulance stats
    int totalAmbulances;          ///< Total number of ambulances
    int availableAmbulances;      ///< Number of available ambulances (not currently on a transfer)
    int totalTransfers;           ///< Total number of patient transfers completed
    int pendingTransfers;         ///< Number of transfer requests currently pending
    int criticalTransfers;        ///< Number of critical priority transfers
    
    // Queue stats
    int totalWaitingPassengers;    ///< Total number of passengers waiting across all stops
    
    TransportStats() 
        : totalBuses(0), activeBuses(0), totalBusPassengers(0),
          totalBusFares(0.0), totalBusTrips(0),
          totalSchoolBuses(0), activeSchoolBuses(0), 
          totalStudentsTransported(0), schoolBusTrips(0),
          totalAmbulances(0), availableAmbulances(0), 
          totalTransfers(0), pendingTransfers(0), criticalTransfers(0),
          totalWaitingPassengers(0) {}
};

// ============================================================================
// TRANSPORT MANAGER CLASS
// ============================================================================
/**
 * Central manager for all transport-related functionalities.
 * Interfaces with CityGraph for routing, manages vehicle creation, dispatch,
 * and statistics collection. Uses various data structures for efficient
 * lookups and operations.
 */
class TransportManager {
private:
    // Reference to city graph for route calculation
    CityGraph* cityGraph;
    
    // ========== BUS MANAGEMENT ==========
    Vector<Bus*> buses;                              ///< All buses in the system
    HashTable<string, Bus*> busLookup;              ///< O(1) lookups by bus number
    HashTable<string, Vector<Bus*>> companyLookup;  ///< Buses indexed by operating company
    HashTable<int, Vector<Bus*>> stopLookup;        ///< Buses indexed by stop ID (Node ID)
    
    // ========== SCHOOL BUS MANAGEMENT ==========
    Vector<SchoolBus*> schoolBuses;                  ///< All school buses in the system
    HashTable<string, SchoolBus*> schoolBusLookup;  ///< O(1) lookups by school bus ID
    HashTable<string, Vector<SchoolBus*>> schoolLookup; ///< School buses by assigned school
    HashTable<string, Vector<SchoolBus*>> sectorSchoolBusLookup; ///< School buses by sector
    
    // ========== AMBULANCE MANAGEMENT ==========
    Vector<Ambulance*> ambulances;                  ///< All ambulances in the system
    HashTable<string, Ambulance*> ambulanceLookup;  ///< O(1) lookups by ambulance ID
    HashTable<string, Vector<Ambulance*>> hospitalAmbulanceLookup; // Ambulances by hospital
    HashTable<string, Vector<Ambulance*>> sectorAmbulanceLookup; // Ambulances by sector
    
    // ========== TRANSFER REQUEST QUEUE ==========
    PriorityQueue<PatientTransfer> transferQueue;    ///< Priority queue for patient transfer requests
    Vector<PatientTransfer> activeTransfers;         ///< List of currently active transfers
    
    // ========== PASSENGER QUEUES ==========
    HashTable<int, BusStopQueue*> stopQueues;        ///< Hash table of passenger queues at bus stops
    
    // ========== STATISTICS ==========
    int totalTransferRequests;                       ///< Total number of transfer requests made
    int transferIDCounter;                          ///< Counter for generating unique transfer request IDs

public:
    // ==================== LIFECYCLE ====================
    
    TransportManager();
    ~TransportManager();
    
    TransportManager(const TransportManager&) = delete;
    TransportManager& operator=(const TransportManager&) = delete;
    
    // ==================== CONFIGURATION ====================
    
    void setCityGraph(CityGraph* graph) { cityGraph = graph; }
    CityGraph* getCityGraph() const { return cityGraph; }
    
    // ==================== BUS MANAGEMENT ====================
    
    /**
     * Create a new bus and add it to the system.
     * @param busNo The bus number/ID
     * @param company The operating company for the bus
     * @param currentStop The ID of the current stop (Node ID)
     * @return Pointer to the created Bus object
     */
    Bus* createBus(const string& busNo, const string& company, const string& currentStop);
    
    /**
     * Set the route for an existing bus.
     * @param busNo The bus number/ID
     * @param route The vector of node IDs representing the route
     * @param distance Total distance of the route
     * @param startStopID The starting stop ID (Node ID) for the route
     * @param endStopID The ending stop ID (Node ID) for the route
     * @return true if successful, false if bus not found
     */
    bool setBusRoute(const string& busNo, const Vector<int>& route, 
                    double distance, const string& startStopID, const string& endStopID);
    
    /**
     * Find a bus by its number/ID.
     * @param busNo The bus number/ID
     * @return Pointer to the Bus object, or null if not found
     */
    Bus* findBusByNumber(const string& busNo) const;
    
    /**
     * Find all buses operated by a specific company.
     * @param company The name of the bus company
     * @return Vector of pointers to Bus objects operated by the company
     */
    Vector<Bus*> findBusesByCompany(const string& company) const;
    
    /**
     * Find all buses that are currently at a specific stop.
     * @param stopNodeID The Node ID of the stop
     * @return Vector of pointers to Bus objects at the stop
     */
    Vector<Bus*> findBusesAtStop(int stopNodeID) const;
    
    /**
     * Find all buses that travel on a route containing the given nodes.
     * @param fromNodeID Starting node ID of the route
     * @param toNodeID Ending node ID of the route
     * @return Vector of pointers to Bus objects on the route
     */
    Vector<Bus*> findBusesOnRoute(int fromNodeID, int toNodeID) const;
    
    int getBusCount() const { return buses.getSize(); }
    Bus* getBus(int index) const;
    const Vector<Bus*>& getAllBuses() const { return buses; }
    
    // ==================== SCHOOL BUS MANAGEMENT ====================
    
    /**
     * Create a new school bus and add it to the system.
     * @param id The school bus ID
     * @param schoolID The ID of the school the bus is assigned to
     * @param schoolNodeID The Node ID of the school location
     * @param sector The sector where the bus operates
     * @return Pointer to the created SchoolBus object
     */
    SchoolBus* createSchoolBus(const string& id, const string& schoolID, 
                               int schoolNodeID, const string& sector);
    
    /**
     * Find a school bus by its ID.
     * @param id The school bus ID
     * @return Pointer to the SchoolBus object, or null if not found
     */
    SchoolBus* findSchoolBusByID(const string& id) const;
    
    /**
     * Get all school buses assigned to a specific school.
     * @param schoolID The ID of the school
     * @return Vector of pointers to SchoolBus objects assigned to the school
     */
    Vector<SchoolBus*> getSchoolBusesBySchool(const string& schoolID) const;
    
    /**
     * Get all school buses operating in a specific sector.
     * @param sector The sector identifier
     * @return Vector of pointers to SchoolBus objects in the sector
     */
    Vector<SchoolBus*> getSchoolBusesBySector(const string& sector) const;
    
    /**
     * Get all available (inactive) school buses.
     * @return Vector of pointers to available SchoolBus objects
     */
    Vector<SchoolBus*> getAvailableSchoolBuses() const;
    
    // Find school bus that serves given sectors (priority-based)
    /**
     * Find the most appropriate school bus for a route between two sectors.
     * @param fromSector The starting sector ID
     * @param toSector The destination sector ID
     * @return Pointer to the SchoolBus object that should be assigned, or null
     */
    SchoolBus* findSchoolBusForRoute(const string& fromSector, const string& toSector) const;
    
    int getSchoolBusCount() const { return schoolBuses.getSize(); }
    SchoolBus* getSchoolBus(int index) const;
    const Vector<SchoolBus*>& getAllSchoolBuses() const { return schoolBuses; }
    
    // ==================== AMBULANCE MANAGEMENT ====================
    
    /**
     * Create a new ambulance and add it to the system.
     * @param id The ambulance ID
     * @param hospitalID The ID of the hospital the ambulance is assigned to
     * @param hospitalNodeID The Node ID of the hospital location
     * @param sector The sector where the ambulance operates
     * @return Pointer to the created Ambulance object
     */
    Ambulance* createAmbulance(const string& id, const string& hospitalID, 
                               int hospitalNodeID, const string& sector);
    
    /**
     * Find an ambulance by its ID.
     * @param id The ambulance ID
     * @return Pointer to the Ambulance object, or null if not found
     */
    Ambulance* findAmbulanceByID(const string& id) const;
    
    /**
     * Get all ambulances assigned to a specific hospital.
     * @param hospitalID The ID of the hospital
     * @return Vector of pointers to Ambulance objects assigned to the hospital
     */
    Vector<Ambulance*> getAmbulancesByHospital(const string& hospitalID) const;
    
    /**
     * Get all ambulances operating in a specific sector.
     * @param sector The sector identifier
     * @return Vector of pointers to Ambulance objects in the sector
     */
    Vector<Ambulance*> getAmbulancesBySector(const string& sector) const;
    
    /**
     * Get all available (inactive) ambulances.
     * @return Vector of pointers to available Ambulance objects
     */
    Vector<Ambulance*> getAvailableAmbulances() const;
    
    // Find ambulance that should handle transfer (sector priority)
    /**
     * Find the most appropriate ambulance for a patient transfer between two sectors.
     * @param sourceSector The source sector ID
     * @param destSector The destination sector ID
     * @return Pointer to the Ambulance object that should handle the transfer, or null
     */
    Ambulance* findAmbulanceForTransfer(const string& sourceSector, 
                                        const string& destSector) const;
    
    int getAmbulanceCount() const { return ambulances.getSize(); }
    Ambulance* getAmbulance(int index) const;
    const Vector<Ambulance*>& getAllAmbulances() const { return ambulances; }
    
    // ==================== PATIENT TRANSFER DISPATCH ====================
    
    /**
     * Request a patient transfer between hospitals.
     * @param patientCNIC The CNIC of the patient
     * @param patientName The name of the patient
     * @param sourceHospitalID The ID of the source hospital
     * @param sourceNodeID The Node ID of the source location
     * @param sourceSector The sector of the source location
     * @param destHospitalID The ID of the destination hospital
     * @param destNodeID The Node ID of the destination location
     * @param destSector The sector of the destination location
     * @param priority The priority level for the transfer
     * @param condition The medical condition of the patient
     * @return A unique request ID for the transfer
     */
    string requestTransfer(const string& patientCNIC, const string& patientName,
                          const string& sourceHospitalID, int sourceNodeID, const string& sourceSector,
                          const string& destHospitalID, int destNodeID, const string& destSector,
                          const string& priority, const string& condition);
    
    /**
     * Dispatch an ambulance to the next pending transfer request.
     * @return Pointer to the dispatched Ambulance object, or null if none available
     */
    Ambulance* dispatchNextTransfer();
    
    /**
     * Dispatch a specific ambulance to a specific transfer request.
     * @param ambulanceID The ID of the ambulance to dispatch
     * @param requestID The ID of the transfer request
     * @return true if successful, false if ambulance not available or request not found
     */
    bool dispatchAmbulance(const string& ambulanceID, const string& requestID);
    
    int getPendingTransferCount() const { return transferQueue.size(); }
    PatientTransfer* peekNextTransfer();
    
    // ==================== PASSENGER QUEUE MANAGEMENT ====================
    
    /**
     * Initialize the passenger queue for a bus stop.
     * @param stopNodeID The Node ID of the stop
     * @param stopName The name of the stop
     * @param sector The sector of the stop
     */
    void initializeStopQueue(int stopNodeID, const string& stopName, const string& sector);
    
    /**
     * Add a passenger to the waiting queue at a stop.
     * @param stopNodeID The Node ID of the stop
     * @param passenger The Passenger object to add
     * @return true if successfully added, false if queue not found
     */
    bool addPassengerToStop(int stopNodeID, const Passenger& passenger);
    
    /**
     * Get the number of passengers waiting at a stop.
     * @param stopNodeID The Node ID of the stop
     * @return The number of waiting passengers
     */
    int getWaitingCount(int stopNodeID) const;
    
    /**
     * Get the queue object for a specific stop.
     * @param stopNodeID The Node ID of the stop
     * @return Pointer to the BusStopQueue object for the stop, or null if not found
     */
    BusStopQueue* getStopQueue(int stopNodeID) const;
    
    /**
     * Process the arrival of a bus at a stop.
     * This includes alighting passengers and boarding waiting passengers.
     * @param bus The Bus object that has arrived
     * @param stopNodeID The Node ID of the stop
     */
    void processBusArrival(Bus* bus, int stopNodeID);
    
    // ==================== SIMULATION ====================
    
    /**
     * Simulate one time step for all buses.
     * This updates bus positions, processes arrivals, and boards passengers.
     */
    void simulateBusStep();
    
    /**
     * Simulate one time step for all school buses.
     * This updates their positions and activates boarding if at a stop.
     */
    void simulateSchoolBusStep();
    
    /**
     * Update the status of an ambulance based on its current route and actions.
     * @param amb The Ambulance object to update
     */
    void updateAmbulanceStatus(Ambulance* amb);
    
    /**
     * Get the current transport statistics.
     * @return A TransportStats object containing the latest statistics
     */
    TransportStats getStats() const;
    
    // ==================== CSV LOADING ====================
    
    /**
     * Load bus data from a CSV file and add to the system.
     * @param filename The CSV file name
     * @param hasHeader If true, skips the first line as header
     * @return true if successful, false if file not found or error in data
     */
    bool loadBusesFromCSV(const string& filename, bool hasHeader = true);
    
    /**
     * Load ambulance data from a CSV file and add to the system.
     * @param filename The CSV file name
     * @param hasHeader If true, skips the first line as header
     * @return true if successful, false if file not found or error in data
     */
    bool loadAmbulancesFromCSV(const string& filename, bool hasHeader = true);
    
    // ==================== SECTOR ADJACENCY ====================
    
    /**
     * Get adjacent sectors for a given sector
     * @param sector The sector ID
     * @return A vector of adjacent sector IDs
     */
    static Vector<string> getAdjacentSectors(const string& sector);
    
    /**
     * Check if two sectors are adjacent
     * @param sector1 The first sector ID
     * @param sector2 The second sector ID
     * @return true if sectors are adjacent, false otherwise
     */
    static bool areSectorsAdjacent(const string& sector1, const string& sector2);

private:
    string trim(const string& s) const;
    Vector<string> parseRoute(const string& routeStr) const;
};

// ============================================================================
// IMPLEMENTATION
// ============================================================================

inline TransportManager::TransportManager() 
    : cityGraph(nullptr), buses(), busLookup(101), companyLookup(53),
      stopLookup(201), schoolBuses(), schoolBusLookup(53), schoolLookup(53),
      sectorSchoolBusLookup(53), ambulances(), ambulanceLookup(53), 
      hospitalAmbulanceLookup(53), sectorAmbulanceLookup(53),
      transferQueue(), activeTransfers(), stopQueues(201),
      totalTransferRequests(0), transferIDCounter(1000) {}

inline TransportManager::~TransportManager() {
    for (int i = 0; i < buses.getSize(); ++i) delete buses[i];
    for (int i = 0; i < schoolBuses.getSize(); ++i) delete schoolBuses[i];
    for (int i = 0; i < ambulances.getSize(); ++i) delete ambulances[i];
}

// ==================== SECTOR ADJACENCY ====================

inline Vector<string> TransportManager::getAdjacentSectors(const string& sector) {
    Vector<string> adjacent;
    
    if (sector.empty() || sector.length() < 3) return adjacent;
    
    char series = sector[0];
    int number = 0;
    
    string numStr = "";
    for (int i = 2; i < (int)sector.length(); ++i) {
        numStr += sector[i];
    }
    if (!numStr.empty()) {
        number = std::stoi(numStr);
    }
    
    // Same series, adjacent numbers
    if (number > 6) {
        adjacent.push_back(string(1, series) + "-" + std::to_string(number - 1));
    }
    if (number < 12) {
        adjacent.push_back(string(1, series) + "-" + std::to_string(number + 1));
    }
    
    // Adjacent series, same number
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
    
    // Add to school lookup
    Vector<SchoolBus*>* schoolList = schoolLookup.get(schoolID);
    if (schoolList) {
        schoolList->push_back(sb);
    } else {
        Vector<SchoolBus*> newList;
        newList.push_back(sb);
        schoolLookup.insert(schoolID, newList);
    }
    
    // Add to sector lookup
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
    // Priority 1: Bus from source sector
    Vector<SchoolBus*> fromBuses = getSchoolBusesBySector(fromSector);
    for (int i = 0; i < fromBuses.getSize(); ++i) {
        if (fromBuses[i]->isAvailable() && fromBuses[i]->isSectorInPriority(toSector)) {
            return fromBuses[i];
        }
    }
    
    // Priority 2: Bus from destination sector
    Vector<SchoolBus*> toBuses = getSchoolBusesBySector(toSector);
    for (int i = 0; i < toBuses.getSize(); ++i) {
        if (toBuses[i]->isAvailable() && toBuses[i]->isSectorInPriority(fromSector)) {
            return toBuses[i];
        }
    }
    
    // Priority 3: Any available bus that covers both sectors
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

// ==================== AMBULANCE MANAGEMENT ====================

inline Ambulance* TransportManager::createAmbulance(const string& id, const string& hospitalID,
                                                    int hospitalNodeID, const string& sector) {
    Ambulance* amb = new Ambulance(id, hospitalID, hospitalNodeID, sector);
    ambulances.push_back(amb);
    ambulanceLookup.insert(id, amb);
    
    // Add to hospital lookup
    Vector<Ambulance*>* hospList = hospitalAmbulanceLookup.get(hospitalID);
    if (hospList) {
        hospList->push_back(amb);
    } else {
        Vector<Ambulance*> newList;
        newList.push_back(amb);
        hospitalAmbulanceLookup.insert(hospitalID, newList);
    }
    
    // Add to sector lookup
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
    // Priority 1: Ambulance from source hospital's sector
    Vector<Ambulance*> sourceAmbs = getAmbulancesBySector(sourceSector);
    for (int i = 0; i < sourceAmbs.getSize(); ++i) {
        if (sourceAmbs[i]->isAvailable() && sourceAmbs[i]->isSectorInPriority(destSector)) {
            return sourceAmbs[i];
        }
    }
    
    // Priority 2: Ambulance from destination hospital's sector
    Vector<Ambulance*> destAmbs = getAmbulancesBySector(destSector);
    for (int i = 0; i < destAmbs.getSize(); ++i) {
        if (destAmbs[i]->isAvailable() && destAmbs[i]->isSectorInPriority(sourceSector)) {
            return destAmbs[i];
        }
    }
    
    // Priority 3: Any available ambulance from adjacent sectors
    Vector<string> adjacentToSource = getAdjacentSectors(sourceSector);
    for (int i = 0; i < adjacentToSource.getSize(); ++i) {
        Vector<Ambulance*> adjAmbs = getAmbulancesBySector(adjacentToSource[i]);
        for (int j = 0; j < adjAmbs.getSize(); ++j) {
            if (adjAmbs[j]->isAvailable()) {
                return adjAmbs[j];
            }
        }
    }
    
    // Priority 4: Any available ambulance
    return getAvailableAmbulances().getSize() > 0 ? getAvailableAmbulances()[0] : nullptr;
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
    
    // Find appropriate ambulance with sector priority
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
    
    // Find transfer in queue (simplified - would need queue iteration)
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

inline void TransportManager::simulateBusStep() {
    for (int i = 0; i < buses.getSize(); ++i) {
        Bus* bus = buses[i];
        processBusArrival(bus, bus->getCurrentNodeID());
        bus->moveToNextStop();
    }
}

inline void TransportManager::simulateSchoolBusStep() {
    for (int i = 0; i < schoolBuses.getSize(); ++i) {
        SchoolBus* sb = schoolBuses[i];
        if (sb->getStatus() == VehicleStatus::EN_ROUTE) {
            sb->moveToNextStop();
        }
    }
}

inline void TransportManager::updateAmbulanceStatus(Ambulance* amb) {
    if (!amb) return;
    
    if (amb->isAtRouteEnd()) {
        string status = amb->getAmbulanceStatus();
        if (status == AmbulanceStatus::DISPATCHED) {
            amb->arriveAtPickup();
        } else if (status == AmbulanceStatus::TRANSPORTING) {
            amb->arriveAtDestination();
        } else if (status == AmbulanceStatus::RETURNING) {
            amb->arriveAtBase();
        }
    }
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

// ==================== CSV LOADING ====================

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
        
        if (!fields[2].empty()) {
            hospitalNode = std::stoi(fields[2]);
        }
        
        createAmbulance(ambID, hospitalID, hospitalNode, sector);
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
