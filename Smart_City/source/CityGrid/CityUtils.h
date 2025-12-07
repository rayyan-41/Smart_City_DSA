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

#define MAX_NODES 500
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
        return lat >= minLat && lat < maxLat && lon >= minLon && lon < maxLon;
    }
    
    // ==================== SETTERS ====================
    void setName(const string& n) { name = n; }
    void setBounds(double minLa, double maxLa, double minLo, double maxLo) {
        minLat = minLa; maxLat = maxLa;
        minLon = minLo; maxLon = maxLo;
    }
    void setInitialized(bool init) { initialized = init; }
};

/*
 * ============================================================================
 * ISLAMABAD SECTOR GRID - NON-OVERLAPPING BOUNDARIES
 * ============================================================================
 * 
 * Grid Layout (looking from top/north):
 * 
 *           72.96   72.98   73.00   73.02   73.04   73.06   73.08   73.10
 *              |       |       |       |       |       |       |       |
 *  33.74 -----+-------+-------+-------+-------+-------+-------+-------+
 *             |       |       |       |       | E-7   | F-6   |       |
 *  33.72 -----+-------+-------+-------+-------+-------+-------+-------+
 *             |       |       |       | E-8   | F-7   | G-6   |       |
 *  33.70 -----+-------+-------+-------+-------+-------+-------+-------+
 *             |       |       | E-9   | F-8   | G-7   | H-8   | I-8   |
 *  33.68 -----+-------+-------+-------+-------+-------+-------+-------+
 *             |       | E-10  | F-9   | G-8   | H-9   | I-9   |       |
 *  33.66 -----+-------+-------+-------+-------+-------+-------+-------+
 *             | E-11  | F-10  | G-9   | H-10  | I-10  |       |       |
 *  33.64 -----+-------+-------+-------+-------+-------+-------+-------+
 *             | F-11  | G-10  | H-11  | I-11  |       |       |       |
 *  33.62 -----+-------+-------+-------+-------+-------+-------+-------+
 *             | G-11  | H-12  | I-12  |       |       |       |       |
 *  33.60 -----+-------+-------+-------+-------+-------+-------+-------+
 * 
 * Each sector is exactly 0.02 degrees in both lat and lon (approximately 2km x 2km)
 * ============================================================================
 */

// Define grid parameters
const double SECTOR_SIZE_LAT = 0.02;  // ~2.2 km
const double SECTOR_SIZE_LON = 0.02;  // ~1.8 km at this latitude

// Base coordinates for the grid
const double BASE_LAT = 33.60;  // Southern boundary
const double BASE_LON = 72.96;  // Western boundary

static SectorBox SECTOR_GRID[] = { 
    // --- E-Series (Northern Row 7) ---
    // E-7: Row 7 from top (lat 33.72-33.74), Column 5 (lon 73.04-73.06)
    {"E-7",  33.72, 33.74, 73.04, 73.06},
    // E-8: Row 6 from top (lat 33.70-33.72), Column 4 (lon 73.02-73.04)  
    {"E-8",  33.70, 33.72, 73.02, 73.04},
    // E-9: Row 5 from top (lat 33.68-33.70), Column 3 (lon 73.00-73.02)
    {"E-9",  33.68, 33.70, 73.00, 73.02},
    // E-10: Row 4 from top (lat 33.66-33.68), Column 2 (lon 72.98-73.00)
    {"E-10", 33.66, 33.68, 72.98, 73.00},
    // E-11: Row 3 from top (lat 33.64-33.66), Column 1 (lon 72.96-72.98)
    {"E-11", 33.64, 33.66, 72.96, 72.98},

    // --- F-Series (One row South of E) ---
    // F-6: Row 7 (lat 33.72-33.74), Column 6 (lon 73.06-73.08)
    {"F-6",  33.72, 33.74, 73.06, 73.08},
    // F-7: Row 6 (lat 33.70-33.72), Column 5 (lon 73.04-73.06)
    {"F-7",  33.70, 33.72, 73.04, 73.06},
    // F-8: Row 5 (lat 33.68-33.70), Column 4 (lon 73.02-73.04)
    {"F-8",  33.68, 33.70, 73.02, 73.04},
    // F-9: Row 4 (lat 33.66-33.68), Column 3 (lon 73.00-73.02)
    {"F-9",  33.66, 33.68, 73.00, 73.02},
    // F-10: Row 3 (lat 33.64-33.66), Column 2 (lon 72.98-73.00)
    {"F-10", 33.64, 33.66, 72.98, 73.00},
    // F-11: Row 2 (lat 33.62-33.64), Column 1 (lon 72.96-72.98)
    {"F-11", 33.62, 33.64, 72.96, 72.98},

    // --- G-Series (One row South of F) ---
    // G-6: Row 6 (lat 33.70-33.72), Column 6 (lon 73.06-73.08)
    {"G-6",  33.70, 33.72, 73.06, 73.08},
    // G-7: Row 5 (lat 33.68-33.70), Column 5 (lon 73.04-73.06)
    {"G-7",  33.68, 33.70, 73.04, 73.06},
    // G-8: Row 4 (lat 33.66-33.68), Column 4 (lon 73.02-73.04)
    {"G-8",  33.66, 33.68, 73.02, 73.04},
    // G-9: Row 3 (lat 33.64-33.66), Column 3 (lon 73.00-73.02)
    {"G-9",  33.64, 33.66, 73.00, 73.02},
    // G-10: Row 2 (lat 33.62-33.64), Column 2 (lon 72.98-73.00)
    {"G-10", 33.62, 33.64, 72.98, 73.00},
    // G-11: Row 1 (lat 33.60-33.62), Column 1 (lon 72.96-72.98)
    {"G-11", 33.60, 33.62, 72.96, 72.98},

    // --- H-Series (One row South of G for most) ---
    // H-8: Row 5 (lat 33.68-33.70), Column 6 (lon 73.06-73.08)
    {"H-8",  33.68, 33.70, 73.06, 73.08},
    // H-9: Row 4 (lat 33.66-33.68), Column 5 (lon 73.04-73.06)
    {"H-9",  33.66, 33.68, 73.04, 73.06},
    // H-10: Row 3 (lat 33.64-33.66), Column 4 (lon 73.02-73.04)
    {"H-10", 33.64, 33.66, 73.02, 73.04},
    // H-11: Row 2 (lat 33.62-33.64), Column 3 (lon 73.00-73.02)
    {"H-11", 33.62, 33.64, 73.00, 73.02},
    // H-12: Row 1 (lat 33.60-33.62), Column 2 (lon 72.98-73.00)
    {"H-12", 33.60, 33.62, 72.98, 73.00},

    // --- I-Series (Easternmost) ---
    // I-8: Row 5 (lat 33.68-33.70), Column 7 (lon 73.08-73.10)
    {"I-8",  33.68, 33.70, 73.08, 73.10},
    // I-9: Row 4 (lat 33.66-33.68), Column 6 (lon 73.06-73.08)
    {"I-9",  33.66, 33.68, 73.06, 73.08},
    // I-10: Row 3 (lat 33.64-33.66), Column 5 (lon 73.04-73.06)
    {"I-10", 33.64, 33.66, 73.04, 73.06},
    // I-11: Row 2 (lat 33.62-33.64), Column 4 (lon 73.02-73.04)
    {"I-11", 33.62, 33.64, 73.02, 73.04},
    // I-12: Row 1 (lat 33.60-33.62), Column 3 (lon 73.00-73.02)
    {"I-12", 33.60, 33.62, 73.00, 73.02}
};

// ============================================================================
// GEOMETRY UTILITIES
// ============================================================================
class GeometryUtils {
public:
    // Resolve sector from coordinates - uses strict boundary checking
    static string resolveSector(double lat, double lon) {
        for (int i = 0; i < SECTOR_COUNT; i++) {
            // Use exclusive upper bounds to avoid overlaps
            if (lat >= SECTOR_GRID[i].minLat && lat < SECTOR_GRID[i].maxLat &&
                lon >= SECTOR_GRID[i].minLon && lon < SECTOR_GRID[i].maxLon) {
                return SECTOR_GRID[i].name;
            }
        }
        // Check for points exactly on the upper boundary
        for (int i = 0; i < SECTOR_COUNT; i++) {
            if (lat >= SECTOR_GRID[i].minLat && lat <= SECTOR_GRID[i].maxLat &&
                lon >= SECTOR_GRID[i].minLon && lon <= SECTOR_GRID[i].maxLon) {
                return SECTOR_GRID[i].name;
            }
        }
        return "Unknown Sector";
    }
    
    // Find the best matching sector (nearest center if not within bounds)
    static string resolveSectorFuzzy(double lat, double lon) {
        // First try exact match
        string exact = resolveSector(lat, lon);
        if (exact != "Unknown Sector") return exact;
        
        // Find nearest sector by center distance
        double minDist = 1e9;
        int bestIdx = -1;
        
        for (int i = 0; i < SECTOR_COUNT; i++) {
            double centerLat = SECTOR_GRID[i].getCenterLat();
            double centerLon = SECTOR_GRID[i].getCenterLon();
            double dist = getHaversineDistance(lat, lon, centerLat, centerLon);
            if (dist < minDist) {
                minDist = dist;
                bestIdx = i;
            }
        }
        
        if (bestIdx != -1) {
            return SECTOR_GRID[bestIdx].name;
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
            // Generate coordinates within the sector (avoiding boundaries)
            double margin = 0.001; // Small margin to stay inside
            double f1 = 0.1 + ((double)rand() / RAND_MAX) * 0.8; // 10%-90% range
            double f2 = 0.1 + ((double)rand() / RAND_MAX) * 0.8;
            lat = SECTOR_GRID[idx].minLat + margin + f1 * (SECTOR_GRID[idx].maxLat - SECTOR_GRID[idx].minLat - 2*margin);
            lon = SECTOR_GRID[idx].minLon + margin + f2 * (SECTOR_GRID[idx].maxLon - SECTOR_GRID[idx].minLon - 2*margin);
            return;
        }
        // Default to G-9 center if sector not found
        lat = 33.65; lon = 73.01;
    }
    
    static SectorBox* getSectorBox(const string& name) {
        int idx = getSectorIndex(name);
        if (idx != -1) return &SECTOR_GRID[idx];
        return nullptr;
    }
    
    // Get all adjacent sectors for a given sector
    static Vector<string> getAdjacentSectors(const string& sectorName) {
        Vector<string> adjacent;
        int idx = getSectorIndex(sectorName);
        if (idx == -1) return adjacent;
        
        SectorBox& box = SECTOR_GRID[idx];
        double centerLat = box.getCenterLat();
        double centerLon = box.getCenterLon();
        
        // Check all other sectors for adjacency
        for (int i = 0; i < SECTOR_COUNT; i++) {
            if (i == idx) continue;
            
            SectorBox& other = SECTOR_GRID[i];
            double otherCenterLat = other.getCenterLat();
            double otherCenterLon = other.getCenterLon();
            
            // Adjacent if centers are within ~1.5 sector widths
            double latDiff = std::abs(centerLat - otherCenterLat);
            double lonDiff = std::abs(centerLon - otherCenterLon);
            
            if (latDiff <= SECTOR_SIZE_LAT * 1.1 && lonDiff <= SECTOR_SIZE_LON * 1.1) {
                adjacent.push_back(other.name);
            }
        }
        
        return adjacent;
    }
    
    // Validate that a point is within Islamabad's bounds
    static bool isWithinIslamabad(double lat, double lon) {
        return lat >= 33.60 && lat <= 33.74 && lon >= 72.96 && lon <= 73.10;
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

