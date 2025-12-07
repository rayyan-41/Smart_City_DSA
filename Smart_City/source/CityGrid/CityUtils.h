//@desc: Basic utilities for the city map grid system
#pragma once
#include <string>
#include <cmath>
#include <iostream>
#include <fstream>
#include "../../data_structures/CustomSTL.h"
using namespace std;

// ============================================================================
// CONSTANTS
// ============================================================================
const int INF = 1e9;

#define MAX_NODES 500
#define SECTOR_COUNT 30
#define MAX_ROADS_PER_NODE 5
#define MAX_SCHOOLS_PER_SECTOR 3
#define MAX_HOSPITALS_PER_SECTOR 2
#define MAX_MALLS_PER_SECTOR 2
#define MAX_PUBLIC_FACILITIES_PER_SECTOR 10

// Grid constants - approximate km per degree at Islamabad's latitude
const double KM_PER_LAT_DEGREE = 111.0;
const double KM_PER_LON_DEGREE = 92.0;  // cos(33.6°) * 111

// ============================================================================
// FACILITY TYPES
// ============================================================================
namespace FacilityType {
    const string STOP = "STOP";
    const string CORNER = "CORNER";
    const string SCHOOL = "SCHOOL";
    const string HOSPITAL = "HOSPITAL";
    const string PHARMACY = "PHARMACY";
    const string MALL = "MALL";
    const string SHOP = "SHOP";
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
    
    inline bool isPublicFacility(const string& type) {
        return type == MOSQUE || type == PARK || type == WATER_COOLER ||
               type == PLAYGROUND || type == LIBRARY || type == COMMUNITY_CENTER ||
               type == POLICE_STATION || type == FIRE_STATION || type == POST_OFFICE ||
               type == BANK || type == ATM || type == PETROL_STATION ||
               type == RESTAURANT || type == PUBLIC_TOILET;
    }
    
    inline bool isTransportStop(const string& type) {
        return type == STOP || isPublicFacility(type);
    }
    
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
// SECTOR BOX - Geographic boundaries for a sector
// ============================================================================
struct SectorBox {
    string name;
    double minLat, maxLat;
    double minLon, maxLon;
    bool initialized = false;
    
    SectorBox() : name(""), minLat(0), maxLat(0), minLon(0), maxLon(0), initialized(false) {}
    SectorBox(const string& n, double minLa, double maxLa, double minLo, double maxLo)
        : name(n), minLat(minLa), maxLat(maxLa), minLon(minLo), maxLon(maxLo), initialized(false) {}
    
    double getCenterLat() const { return (minLat + maxLat) / 2.0; }
    double getCenterLon() const { return (minLon + maxLon) / 2.0; }
    double getWidth() const { return maxLon - minLon; }
    double getHeight() const { return maxLat - minLat; }
    
    bool containsPoint(double lat, double lon) const {
        return lat >= minLat && lat < maxLat && lon >= minLon && lon < maxLon;
    }
};

// ============================================================================
// ISLAMABAD SECTOR GRID
// ============================================================================
// Real Islamabad sectors: E(7-11), F(6-11), G(6-11), H(8-13), I(8-14)
// Each sector is 0.02° x 0.02° (~2.2km x 1.8km)

const double SECTOR_SIZE_LAT = 0.02;  
const double SECTOR_SIZE_LON = 0.02;  
const double BASE_LAT = 33.64;  
const double BASE_LON = 73.00;  
const double MAX_LAT = 33.74;   
const double MAX_LON = 73.18;   

#define COL_LON(col) (73.00 + ((col) - 6) * 0.02)
#define ROW_E_LAT 33.72
#define ROW_F_LAT 33.70
#define ROW_G_LAT 33.68
#define ROW_H_LAT 33.66
#define ROW_I_LAT 33.64

static SectorBox SECTOR_GRID[] = { 
    // E-SERIES (Northernmost, near Margalla Hills)
    {"E-7",  ROW_E_LAT, ROW_E_LAT + 0.02, COL_LON(7),  COL_LON(7) + 0.02},
    {"E-8",  ROW_E_LAT, ROW_E_LAT + 0.02, COL_LON(8),  COL_LON(8) + 0.02},
    {"E-9",  ROW_E_LAT, ROW_E_LAT + 0.02, COL_LON(9),  COL_LON(9) + 0.02},
    {"E-10", ROW_E_LAT, ROW_E_LAT + 0.02, COL_LON(10), COL_LON(10) + 0.02},
    {"E-11", ROW_E_LAT, ROW_E_LAT + 0.02, COL_LON(11), COL_LON(11) + 0.02},

    // F-SERIES (Prime residential, Blue Area)
    {"F-6",  ROW_F_LAT, ROW_F_LAT + 0.02, COL_LON(6),  COL_LON(6) + 0.02},
    {"F-7",  ROW_F_LAT, ROW_F_LAT + 0.02, COL_LON(7),  COL_LON(7) + 0.02},
    {"F-8",  ROW_F_LAT, ROW_F_LAT + 0.02, COL_LON(8),  COL_LON(8) + 0.02},
    {"F-9",  ROW_F_LAT, ROW_F_LAT + 0.02, COL_LON(9),  COL_LON(9) + 0.02},
    {"F-10", ROW_F_LAT, ROW_F_LAT + 0.02, COL_LON(10), COL_LON(10) + 0.02},
    {"F-11", ROW_F_LAT, ROW_F_LAT + 0.02, COL_LON(11), COL_LON(11) + 0.02},

    // G-SERIES (Central hub, Centaurus Mall in G-8)
    {"G-6",  ROW_G_LAT, ROW_G_LAT + 0.02, COL_LON(6),  COL_LON(6) + 0.02},
    {"G-7",  ROW_G_LAT, ROW_G_LAT + 0.02, COL_LON(7),  COL_LON(7) + 0.02},
    {"G-8",  ROW_G_LAT, ROW_G_LAT + 0.02, COL_LON(8),  COL_LON(8) + 0.02},
    {"G-9",  ROW_G_LAT, ROW_G_LAT + 0.02, COL_LON(9),  COL_LON(9) + 0.02},
    {"G-10", ROW_G_LAT, ROW_G_LAT + 0.02, COL_LON(10), COL_LON(10) + 0.02},
    {"G-11", ROW_G_LAT, ROW_G_LAT + 0.02, COL_LON(11), COL_LON(11) + 0.02},

    // H-SERIES (Mixed residential)
    {"H-8",  ROW_H_LAT, ROW_H_LAT + 0.02, COL_LON(8),  COL_LON(8) + 0.02},
    {"H-9",  ROW_H_LAT, ROW_H_LAT + 0.02, COL_LON(9),  COL_LON(9) + 0.02},
    {"H-10", ROW_H_LAT, ROW_H_LAT + 0.02, COL_LON(10), COL_LON(10) + 0.02},
    {"H-11", ROW_H_LAT, ROW_H_LAT + 0.02, COL_LON(11), COL_LON(11) + 0.02},
    {"H-12", ROW_H_LAT, ROW_H_LAT + 0.02, COL_LON(12), COL_LON(12) + 0.02},
    {"H-13", ROW_H_LAT, ROW_H_LAT + 0.02, COL_LON(13), COL_LON(13) + 0.02},

    // I-SERIES (Southernmost, newer sectors)
    {"I-8",  ROW_I_LAT, ROW_I_LAT + 0.02, COL_LON(8),  COL_LON(8) + 0.02},
    {"I-9",  ROW_I_LAT, ROW_I_LAT + 0.02, COL_LON(9),  COL_LON(9) + 0.02},
    {"I-10", ROW_I_LAT, ROW_I_LAT + 0.02, COL_LON(10), COL_LON(10) + 0.02},
    {"I-11", ROW_I_LAT, ROW_I_LAT + 0.02, COL_LON(11), COL_LON(11) + 0.02},
    {"I-12", ROW_I_LAT, ROW_I_LAT + 0.02, COL_LON(12), COL_LON(12) + 0.02},
    {"I-13", ROW_I_LAT, ROW_I_LAT + 0.02, COL_LON(13), COL_LON(13) + 0.02},
    {"I-14", ROW_I_LAT, ROW_I_LAT + 0.02, COL_LON(14), COL_LON(14) + 0.02}
};

// ============================================================================
// GEOMETRY UTILITIES
// ============================================================================
class GeometryUtils {
public:
    // Find sector containing given coordinates
    static string resolveSector(double lat, double lon) {
        for (int i = 0; i < SECTOR_COUNT; i++) {
            if (SECTOR_GRID[i].containsPoint(lat, lon)) {
                return SECTOR_GRID[i].name;
            }
        }
        // Check boundary edge case
        for (int i = 0; i < SECTOR_COUNT; i++) {
            if (lat >= SECTOR_GRID[i].minLat && lat <= SECTOR_GRID[i].maxLat &&
                lon >= SECTOR_GRID[i].minLon && lon <= SECTOR_GRID[i].maxLon) {
                return SECTOR_GRID[i].name;
            }
        }
        return "Unknown";
    }
    
    // Euclidean grid distance in km
    static double getGridDistance(double lat1, double lon1, double lat2, double lon2) {
        double dLat = (lat2 - lat1) * KM_PER_LAT_DEGREE;
        double dLon = (lon2 - lon1) * KM_PER_LON_DEGREE;
        return std::sqrt(dLat * dLat + dLon * dLon);
    }
    
    // Manhattan distance in km (city block style)
    static double getManhattanDistance(double lat1, double lon1, double lat2, double lon2) {
        double dLat = std::abs(lat2 - lat1) * KM_PER_LAT_DEGREE;
        double dLon = std::abs(lon2 - lon1) * KM_PER_LON_DEGREE;
        return dLat + dLon;
    }
    
    static int getSectorIndex(const string& name) {
        for (int i = 0; i < SECTOR_COUNT; i++) {
            if (SECTOR_GRID[i].name == name) return i;
        }
        return -1;
    }
    
    // Generate random coordinates within a sector (with margin from edges)
    static void generateCoords(const string& sector, double& lat, double& lon) {
        int idx = getSectorIndex(sector);
        if (idx != -1) {
            const SectorBox& box = SECTOR_GRID[idx];
            double marginLat = box.getHeight() * 0.15;
            double marginLon = box.getWidth() * 0.15;
            
            double randLat = (double)rand() / RAND_MAX;
            double randLon = (double)rand() / RAND_MAX;
            
            lat = box.minLat + marginLat + randLat * (box.getHeight() - 2 * marginLat);
            lon = box.minLon + marginLon + randLon * (box.getWidth() - 2 * marginLon);
            return;
        }
        lat = 33.69; lon = 73.07; // Default to G-9 center
    }
    
    // Generate coordinates at sector center
    static void generateCenterCoords(const string& sector, double& lat, double& lon) {
        int idx = getSectorIndex(sector);
        if (idx != -1) {
            lat = SECTOR_GRID[idx].getCenterLat();
            lon = SECTOR_GRID[idx].getCenterLon();
            return;
        }
        lat = 33.69; lon = 73.07;
    }
    
    // Generate coordinates at specific position within sector (0-1 range for x,y)
    static void generateCoordsAtPosition(const string& sector, double posX, double posY, 
                                          double& lat, double& lon) {
        int idx = getSectorIndex(sector);
        if (idx != -1) {
            const SectorBox& box = SECTOR_GRID[idx];
            lat = box.minLat + posY * box.getHeight();
            lon = box.minLon + posX * box.getWidth();
            return;
        }
        lat = 33.69; lon = 73.07;
    }
    
    static SectorBox* getSectorBox(const string& name) {
        int idx = getSectorIndex(name);
        return (idx != -1) ? &SECTOR_GRID[idx] : nullptr;
    }
    
    // Get adjacent sectors (within ~1.5 sector widths)
    static Vector<string> getAdjacentSectors(const string& sectorName) {
        Vector<string> adjacent;
        int idx = getSectorIndex(sectorName);
        if (idx == -1) return adjacent;
        
        double centerLat = SECTOR_GRID[idx].getCenterLat();
        double centerLon = SECTOR_GRID[idx].getCenterLon();
        
        for (int i = 0; i < SECTOR_COUNT; i++) {
            if (i == idx) continue;
            
            double latDiff = std::abs(centerLat - SECTOR_GRID[i].getCenterLat());
            double lonDiff = std::abs(centerLon - SECTOR_GRID[i].getCenterLon());
            
            if (latDiff <= SECTOR_SIZE_LAT * 1.1 && lonDiff <= SECTOR_SIZE_LON * 1.1) {
                adjacent.push_back(SECTOR_GRID[i].name);
            }
        }
        return adjacent;
    }
    
    static bool isWithinIslamabad(double lat, double lon) {
        return lat >= BASE_LAT && lat <= MAX_LAT && lon >= BASE_LON && lon <= MAX_LON;
    }
    
    static void getIslamabadBounds(double& minLat, double& maxLat, double& minLon, double& maxLon) {
        minLat = BASE_LAT;
        maxLat = MAX_LAT;
        minLon = BASE_LON;
        maxLon = MAX_LON;
    }
};

// ============================================================================
// EDGE - Road connection between nodes
// ============================================================================
struct Edge {
    int destinationID;
    double weight;

    Edge() : destinationID(-1), weight(0.0) {}
    Edge(int destID, double w) : destinationID(destID), weight(w) {}
    
    bool operator==(const Edge& other) const {
        return destinationID == other.destinationID && weight == other.weight;
    }
};

// ============================================================================
// CITY NODE - Location in the city graph
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
    
    int getConnectionCount() const { return roads.size(); }
    const LinkedList<Edge>& getRoads() const { return roads; }
    
    bool canBeTransportStop() const { return FacilityType::isTransportStop(type); }
    bool isPublicFacility() const { return FacilityType::isPublicFacility(type); }
};

// ============================================================================
// DIJKSTRA NODE - For pathfinding priority queue
// ============================================================================
struct DijkstraNode {
    int nodeID;
    double distance;

    DijkstraNode() : nodeID(-1), distance(INF) {}
    DijkstraNode(int id, double dist) : nodeID(id), distance(dist) {}

    bool operator<(const DijkstraNode& other) const {
        return distance > other.distance; // Min-heap
    }
};

// ============================================================================
// TRAVEL RECORD - For citizen travel history (used with Stack)
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
    
    bool operator==(const TravelRecord& other) const {
        return citizenCNIC == other.citizenCNIC && timestamp == other.timestamp;
    }
};

// ============================================================================
// CITY STATISTICS - Aggregate data for reporting
// ============================================================================
struct CityStats {
    int totalNodes = 0;
    int busStops = 0;
    int schoolNodes = 0;
    int hospitalNodes = 0;
    int pharmacyNodes = 0;
    int sectorCorners = 0;
    
    int totalSchools = 0;
    int totalHospitals = 0;
    int totalPharmacies = 0;
    int totalMalls = 0;
    
    int totalBuses = 0;
    int activeBuses = 0;
    int totalSchoolBuses = 0;
    int activeSchoolBuses = 0;
    int totalAmbulances = 0;
    int availableAmbulances = 0;
    int pendingTransfers = 0;
    
    int totalSectors = 0;
    int totalStreets = 0;
    int totalHouses = 0;
    int totalCitizens = 0;
    
    int totalPassengersServed = 0;
    int totalStudentsTransported = 0;
    int totalPatientsTransported = 0;
    int totalTravelRecords = 0;
};

