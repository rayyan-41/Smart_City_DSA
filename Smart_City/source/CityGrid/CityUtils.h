//@desc: This file contains basic utilities for the city map.
#pragma once
#include <string>
#include <cmath>
#include <iostream>
#include <fstream>
#include "../../data_structures/CustomSTL.h"
using namespace std; 

const double pi = 3.14159265358979323846;
const int INF = 1e9;
const double Rad = 6371.0;

#define MAX_NODES 250
#define SECTOR_COUNT 30
#define MAX_ROADS_PER_NODE 5
#define MAX_SCHOOLS_PER_SECTOR 3
#define MAX_HOSPITALS_PER_SECTOR 2
#define MAX_MALLS_PER_SECTOR 2
#define MAX_PUBLIC_FACILITIES_PER_SECTOR 10

// ============================================================================
// FACILITY TYPES - String constants for node types
// ============================================================================
namespace FacilityType {
    // Transport
    const string STOP = "STOP";
    const string CORNER = "CORNER";
    
    // Education
    const string SCHOOL = "SCHOOL";
    
    // Medical
    const string HOSPITAL = "HOSPITAL";
    const string PHARMACY = "PHARMACY";
    
    // Commercial
    const string MALL = "MALL";
    const string SHOP = "SHOP";
    
    // Public Facilities (can be used as transport route stops)
    const string MOSQUE = "MOSQUE";
    const string PARK = "PARK";
    const string WATER_COOLER = "WATER_COOLER";
    const string PLAYGROUND = "PLAYGROUND";
    const string LIBRARY = "LIBRARY";
    const string COMMUNITY_CENTER = "COMMUNITY_CENTER";
    const string POLICE_STATION = "POLICE_STATION";
    const string FIRE_STATION = "FIRE_STATION";
    const string POST_OFFICE = "POST_OFFICE";
    const string BANK = "BANK";
    const string ATM = "ATM";
    const string PETROL_STATION = "PETROL_STATION";
    const string RESTAURANT = "RESTAURANT";
    const string PUBLIC_TOILET = "PUBLIC_TOILET";
    
    // Check if a type is a public facility (can be used as route stop)
    inline bool isPublicFacility(const string& type) {
        return type == MOSQUE || type == PARK || type == WATER_COOLER ||
               type == PLAYGROUND || type == LIBRARY || type == COMMUNITY_CENTER ||
               type == POLICE_STATION || type == FIRE_STATION || type == POST_OFFICE ||
               type == BANK || type == ATM || type == PETROL_STATION ||
               type == RESTAURANT || type == PUBLIC_TOILET;
    }
    
    // Check if a type can be used as a transport stop
    inline bool isTransportStop(const string& type) {
        return type == STOP || isPublicFacility(type);
    }
    
    // Get prefix for generating stop IDs
    inline string getStopIDPrefix(const string& type) {
        if (type == MOSQUE) return "MSQ";
        if (type == PARK) return "PRK";
        if (type == WATER_COOLER) return "WTR";
        if (type == PLAYGROUND) return "PLY";
        if (type == LIBRARY) return "LIB";
        if (type == COMMUNITY_CENTER) return "COM";
        if (type == POLICE_STATION) return "POL";
        if (type == FIRE_STATION) return "FIR";
        if (type == POST_OFFICE) return "PST";
        if (type == BANK) return "BNK";
        if (type == ATM) return "ATM";
        if (type == PETROL_STATION) return "PET";
        if (type == RESTAURANT) return "RST";
        if (type == PUBLIC_TOILET) return "TOI";
        if (type == STOP) return "STP";
        return "FAC";
    }
}

// ============================================================================
// SECTOR BOX - Defines geographic boundaries for a sector
// ============================================================================
struct SectorBox {
    string name;
    double minLat, maxLat;
    double minLon, maxLon;
    bool initialized = false;
    
    SectorBox() : name(""), minLat(0.0), maxLat(0.0), minLon(0.0), maxLon(0.0), initialized(false) {}
    SectorBox(const string& n, double minLa, double maxLa, double minLo, double maxLo)
        : name(n), minLat(minLa), maxLat(maxLa), minLon(minLo), maxLon(maxLo), initialized(false) {
    }
    
    // ==================== GETTERS ====================
    string getName() const { return name; }
    double getMinLat() const { return minLat; }
    double getMaxLat() const { return maxLat; }
    double getMinLon() const { return minLon; }
    double getMaxLon() const { return maxLon; }
    bool isInitialized() const { return initialized; }
    
    double getCenterLat() const { return (minLat + maxLat) / 2.0; }
    double getCenterLon() const { return (minLon + maxLon) / 2.0; }
    double getWidth() const { return maxLon - minLon; }
    double getHeight() const { return maxLat - minLat; }
    
    bool containsPoint(double lat, double lon) const {
        return lat >= minLat && lat <= maxLat && lon >= minLon && lon <= maxLon;
    }
    
    // ==================== SETTERS ====================
    void setName(const string& n) { name = n; }
    void setBounds(double minLa, double maxLa, double minLo, double maxLo) {
        minLat = minLa; maxLat = maxLa;
        minLon = minLo; maxLon = maxLo;
    }
    void setInitialized(bool init) { initialized = init; }
};

static SectorBox SECTOR_GRID[] = { 
    // --- E-Series (Most Northern) ---
    {"E-7", 33.730, 33.750, 73.055, 73.075},
    {"E-8", 33.720, 33.740, 73.040, 73.060},
    {"E-9", 33.710, 33.730, 73.025, 73.045},
    {"E-10", 33.700, 33.720, 73.010, 73.030},
    {"E-11", 33.690, 33.710, 72.990, 73.010},

    // --- F-Series (North) ---
    {"F-6", 33.720, 33.740, 73.060, 73.085},
    {"F-7", 33.710, 33.730, 73.045, 73.065},
    {"F-8", 33.700, 33.720, 73.030, 73.050},
    {"F-9", 33.690, 33.710, 73.015, 73.035},
    {"F-10", 33.680, 33.700, 73.000, 73.020},
    {"F-11", 33.670, 33.690, 72.980, 73.000},

    // --- G-Series (Central) ---
    {"G-6", 33.710, 33.730, 73.070, 73.090},
    {"G-7", 33.700, 33.720, 73.050, 73.070},
    {"G-8", 33.690, 33.710, 73.030, 73.050},
    {"G-9", 33.680, 33.700, 73.010, 73.030},
    {"G-10", 33.670, 33.690, 72.990, 73.010},
    {"G-11", 33.660, 33.680, 72.970, 72.990},

    // --- H-Series (Education/Institutions) ---
    {"H-8", 33.680, 33.700, 73.040, 73.060},
    {"H-9", 33.670, 33.690, 73.020, 73.040},
    {"H-10", 33.660, 33.680, 73.000, 73.020},
    {"H-11", 33.650, 33.670, 72.980, 73.000},
    {"H-12", 33.640, 33.660, 72.960, 72.980}, 

    // I-Series is directly South of H-Series 
    {"I-8", 33.670, 33.690, 73.050, 73.070},
    {"I-9", 33.660, 33.680, 73.030, 73.050},
    {"I-10", 33.650, 33.670, 73.010, 73.030},
    {"I-11", 33.640, 33.660, 72.990, 73.010},
    {"I-12", 33.630, 33.650, 72.970, 72.990}
};

// ============================================================================
// GEOMETRY UTILITIES
// ============================================================================
class GeometryUtils {
public:
    static string resolveSector(double lat, double lon) {
        for (int i = 0; i < SECTOR_COUNT; i++) {
            if (lat >= SECTOR_GRID[i].minLat && lat <= SECTOR_GRID[i].maxLat &&
                lon >= SECTOR_GRID[i].minLon && lon <= SECTOR_GRID[i].maxLon) {
                return SECTOR_GRID[i].name;
            }
        }
        return "Unknown Sector";
    }
    
    static double getHaversineDistance(double lat1, double lon1, double lat2, double lon2) {
        double dLat = (lat2 - lat1) * pi / 180.0;
        double dLon = (lon2 - lon1) * pi / 180.0;
        double a = sin(dLat / 2) * sin(dLat / 2) +
            cos(lat1 * pi / 180.0) * cos(lat2 * pi / 180.0) *
            sin(dLon / 2) * sin(dLon / 2);
        double c = 2 * atan2(sqrt(a), sqrt(1 - a));
        return Rad * c;
    }
    
    static int getSectorIndex(string name) {
        for (int i = 0; i < SECTOR_COUNT; i++) {
            if (SECTOR_GRID[i].name == name) return i;
        }
        return -1;
    }
    
    static void generateCoords(string sector, double& lat, double& lon) {
        int idx = getSectorIndex(sector);
        if (idx != -1) {
            double f1 = (double)rand() / RAND_MAX;
            double f2 = (double)rand() / RAND_MAX;
            lat = SECTOR_GRID[idx].minLat + f1 * (SECTOR_GRID[idx].maxLat - SECTOR_GRID[idx].minLat);
            lon = SECTOR_GRID[idx].minLon + f2 * (SECTOR_GRID[idx].maxLon - SECTOR_GRID[idx].minLon);
            return;
        }
        lat = 33.69; lon = 73.04;
    }
    
    static SectorBox* getSectorBox(const string& name) {
        int idx = getSectorIndex(name);
        if (idx != -1) return &SECTOR_GRID[idx];
        return nullptr;
    }
};

// ============================================================================
// EDGE - Represents a road connection between nodes
// ============================================================================
struct Edge {
    int destinationID;
    double weight;

    Edge() : destinationID(-1), weight(0.0) {}
    Edge(int destID, double w) : destinationID(destID), weight(w) {}
    
    // ==================== GETTERS ====================
    int getDestinationID() const { return destinationID; }
    double getWeight() const { return weight; }
    
    // ==================== SETTERS ====================
    void setDestinationID(int id) { destinationID = id; }
    void setWeight(double w) { weight = w; }
    
    bool operator==(const Edge& other) const {
        return destinationID == other.destinationID && weight == other.weight;
    }
};

// ============================================================================
// CITY NODE - Represents a location in the city graph
// ============================================================================
struct CityNode {
    int id;
    string databaseID;
    string stopID;
    string name;
    string sector;
    string type;
    double lat, lon;
    
    string operatingHours;
    bool isAccessible;
    string additionalInfo;

    LinkedList<Edge> roads;

    CityNode(int i, string dbID, string sID, string n, string t, double lt, double ln)
        : id(i), databaseID(dbID), stopID(sID), name(n), type(t), lat(lt), lon(ln),
          operatingHours(""), isAccessible(true), additionalInfo("") {
        sector = GeometryUtils::resolveSector(lt, ln);
    }
    
    // ==================== GETTERS ====================
    int getId() const { return id; }
    string getDatabaseID() const { return databaseID; }
    string getStopID() const { return stopID; }
    string getName() const { return name; }
    string getSector() const { return sector; }
    string getType() const { return type; }
    double getLatitude() const { return lat; }
    double getLongitude() const { return lon; }
    string getOperatingHours() const { return operatingHours; }
    bool getIsAccessible() const { return isAccessible; }
    string getAdditionalInfo() const { return additionalInfo; }
    int getConnectionCount() const { return roads.size(); }
    const LinkedList<Edge>& getRoads() const { return roads; }
    
    bool canBeTransportStop() const {
        return FacilityType::isTransportStop(type);
    }
    
    bool isPublicFacility() const {
        return FacilityType::isPublicFacility(type);
    }
    
    // ==================== SETTERS ====================
    void setId(int i) { id = i; }
    void setDatabaseID(const string& dbID) { databaseID = dbID; }
    void setStopID(const string& sID) { stopID = sID; }
    void setName(const string& n) { name = n; }
    void setSector(const string& s) { sector = s; }
    void setType(const string& t) { type = t; }
    void setLatitude(double lt) { lat = lt; }
    void setLongitude(double ln) { lon = ln; }
    void setCoordinates(double lt, double ln) { lat = lt; lon = ln; sector = GeometryUtils::resolveSector(lt, ln); }
    void setOperatingHours(const string& hours) { operatingHours = hours; }
    void setIsAccessible(bool accessible) { isAccessible = accessible; }
    void setAdditionalInfo(const string& info) { additionalInfo = info; }
};

// ============================================================================
// DIJKSTRA NODE - Helper for pathfinding priority queue
// ============================================================================
struct DijkstraNode {
    int nodeID;
    double distance;

    DijkstraNode() : nodeID(-1), distance(INF) {}
    DijkstraNode(int id, double dist) : nodeID(id), distance(dist) {}

    // ==================== GETTERS ====================
    int getNodeID() const { return nodeID; }
    double getDistance() const { return distance; }
    
    // ==================== SETTERS ====================
    void setNodeID(int id) { nodeID = id; }
    void setDistance(double dist) { distance = dist; }

    bool operator<(const DijkstraNode& other) const {
        return distance > other.distance;
    }
};
// ============================================================================
// TRAVEL HISTORY RECORD - Used with Stack for citizen travel tracking
// ============================================================================
struct TravelRecord {
    string citizenCNIC;
    int fromNodeID;
    int toNodeID;
    string timestamp;
    double distance;
    string vehicleID;
    string vehicleType;
    
    TravelRecord() 
        : citizenCNIC(""), fromNodeID(-1), toNodeID(-1), timestamp(""), 
          distance(0.0), vehicleID(""), vehicleType("WALK") {}
    
    TravelRecord(const string& cnic, int from, int to, const string& time, 
                double dist, const string& vehID = "", const string& vehType = "WALK")
        : citizenCNIC(cnic), fromNodeID(from), toNodeID(to), timestamp(time), 
          distance(dist), vehicleID(vehID), vehicleType(vehType) {}
    
    // ==================== GETTERS ====================
    string getCitizenCNIC() const { return citizenCNIC; }
    int getFromNodeID() const { return fromNodeID; }
    int getToNodeID() const { return toNodeID; }
    string getTimestamp() const { return timestamp; }
    double getDistance() const { return distance; }
    string getVehicleID() const { return vehicleID; }
    string getVehicleType() const { return vehicleType; }
    
    // ==================== SETTERS ====================
    void setCitizenCNIC(const string& cnic) { citizenCNIC = cnic; }
    void setFromNodeID(int id) { fromNodeID = id; }
    void setToNodeID(int id) { toNodeID = id; }
    void setTimestamp(const string& ts) { timestamp = ts; }
    void setDistance(double dist) { distance = dist; }
    void setVehicleID(const string& id) { vehicleID = id; }
    void setVehicleType(const string& type) { vehicleType = type; }
    
    bool operator==(const TravelRecord& other) const {
        return citizenCNIC == other.citizenCNIC && timestamp == other.timestamp;
    }
};

// ============================================================================
// CITY STATISTICS - Aggregate data structure for reporting
// ============================================================================
struct CityStats {
    // Infrastructure counts
    int totalNodes;
    int busStops;
    int schoolNodes;
    int hospitalNodes;
    int pharmacyNodes;
    int sectorCorners;
    
    // Module counts
    int totalSchools;
    int totalHospitals;
    int totalPharmacies;
    int totalMalls;
    
    // Transport counts
    int totalBuses;
    int activeBuses;
    int totalSchoolBuses;
    int activeSchoolBuses;
    int totalAmbulances;
    int availableAmbulances;
    int pendingTransfers;
    
    // Population hierarchy
    int totalSectors;
    int totalStreets;
    int totalHouses;
    int totalCitizens;
    
    // Simulation metrics
    int totalPassengersServed;
    int totalStudentsTransported;
    int totalPatientsTransported;
    int totalTravelRecords;
    
    CityStats() 
        : totalNodes(0), busStops(0), schoolNodes(0), hospitalNodes(0),
          pharmacyNodes(0), sectorCorners(0), totalSchools(0), totalHospitals(0),
          totalPharmacies(0), totalMalls(0), totalBuses(0), activeBuses(0),
          totalSchoolBuses(0), activeSchoolBuses(0),
          totalAmbulances(0), availableAmbulances(0), pendingTransfers(0),
          totalSectors(0), totalStreets(0), totalHouses(0), totalCitizens(0),
          totalPassengersServed(0), totalStudentsTransported(0), 
          totalPatientsTransported(0), totalTravelRecords(0) {}
    
    // ==================== GETTERS ====================
    int getTotalNodes() const { return totalNodes; }
    int getBusStops() const { return busStops; }
    int getSchoolNodes() const { return schoolNodes; }
    int getHospitalNodes() const { return hospitalNodes; }
    int getPharmacyNodes() const { return pharmacyNodes; }
    int getSectorCorners() const { return sectorCorners; }
    int getTotalSchools() const { return totalSchools; }
    int getTotalHospitals() const { return totalHospitals; }
    int getTotalPharmacies() const { return totalPharmacies; }
    int getTotalMalls() const { return totalMalls; }
    int getTotalBuses() const { return totalBuses; }
    int getActiveBuses() const { return activeBuses; }
    int getTotalSchoolBuses() const { return totalSchoolBuses; }
    int getActiveSchoolBuses() const { return activeSchoolBuses; }
    int getTotalAmbulances() const { return totalAmbulances; }
    int getAvailableAmbulances() const { return availableAmbulances; }
    int getPendingTransfers() const { return pendingTransfers; }
    int getTotalSectors() const { return totalSectors; }
    int getTotalStreets() const { return totalStreets; }
    int getTotalHouses() const { return totalHouses; }
    int getTotalCitizens() const { return totalCitizens; }
    int getTotalPassengersServed() const { return totalPassengersServed; }
    int getTotalStudentsTransported() const { return totalStudentsTransported; }
    int getTotalPatientsTransported() const { return totalPatientsTransported; }
    int getTotalTravelRecords() const { return totalTravelRecords; }
};

