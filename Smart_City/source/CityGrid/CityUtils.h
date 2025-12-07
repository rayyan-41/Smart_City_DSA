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
        // ... (truncated for brevity, logic remains same) ...
        if (type == STOP) return "STP";
        return "FAC";
    }
}

// ============================================================================
// SECTOR BOX - Defines geographic boundaries (Keep for Input Resolution)
// ============================================================================
struct SectorBox {
    string name;
    double minLat, maxLat;
    double minLon, maxLon;
    bool initialized = false;
    
    SectorBox(const string& n, double minLa, double maxLa, double minLo, double maxLo)
        : name(n), minLat(minLa), maxLat(maxLa), minLon(minLo), maxLon(maxLo), initialized(false) {}
        
    double getCenterLat() const { return (minLat + maxLat) / 2.0; }
    double getCenterLon() const { return (minLon + maxLon) / 2.0; }
};

// ============================================================================
// GRID UTILS - NEW VISUALIZATION LOGIC
// ============================================================================
class GridUtils {
public:
    // Visual Config
    static constexpr double CELL_WIDTH = 180.0;
    static constexpr double CELL_HEIGHT = 120.0;
    static constexpr double PADDING = 30.0;
    static constexpr int MAX_SECTOR_NUM = 12; // E-12 is usually the limit

    // Maps a sector (e.g., "E-7") to a Top-Left X,Y coordinate for the grid cell
    static bool getSectorGridPos(const string& sector, double& outX, double& outY) {
        if (sector.length() < 3) return false;

        char series = sector[0]; // 'E', 'F', etc.
        int number = 0;
        
        // Parse number (handle "E-7" or "E7")
        string numStr = (sector[1] == '-') ? sector.substr(2) : sector.substr(1);
        try {
            number = stoi(numStr);
        } catch(...) { return false; }

        // 1. Calculate Row (Y-Axis): E=0 (Top), F=1, G=2...
        int row = 0;
        if (series >= 'E' && series <= 'I') {
            row = series - 'E';
        } else {
            return false;
        }

        // 2. Calculate Column (X-Axis): 
        // Requirement: E-5 is Top Right. E-6 is to its Left.
        // This means X decreases as Number Increases? 
        // No, standard grid: E-12 (Left) ... E-6 ... E-5 (Right).
        // Let's map 12 to 0, 11 to 1... 5 to 7.
        int col = MAX_SECTOR_NUM - number; 

        // 3. Scale
        outX = col * (CELL_WIDTH + PADDING) + 50.0; // Margin
        outY = row * (CELL_HEIGHT + PADDING) + 50.0;

        return true;
    }
};

// ============================================================================
// SECTOR GRID DATA (Used for input resolution)
// ============================================================================
// ... [Retain SECTOR_GRID array and constants from original file] ...
// I am including the critical definitions needed for compiling:

const double SECTOR_SIZE_LAT = 0.02;
const double SECTOR_SIZE_LON = 0.02;
const double BASE_LAT = 33.60;
const double BASE_LON = 72.96;
const double MAX_LAT = 33.74;
const double MAX_LON = 73.10;

static SectorBox SECTOR_GRID[] = { 
    {"E-7",  33.72, 33.74, 73.04, 73.06}, {"E-8",  33.70, 33.72, 73.02, 73.04},
    {"E-9",  33.68, 33.70, 73.00, 73.02}, {"E-10", 33.66, 33.68, 72.98, 73.00},
    {"E-11", 33.64, 33.66, 72.96, 72.98},
    {"F-6",  33.72, 33.74, 73.06, 73.08}, {"F-7",  33.70, 33.72, 73.04, 73.06},
    {"F-8",  33.68, 33.70, 73.02, 73.04}, {"F-9",  33.66, 33.68, 73.00, 73.02},
    {"F-10", 33.64, 33.66, 72.98, 73.00}, {"F-11", 33.62, 33.64, 72.96, 72.98},
    {"G-6",  33.70, 33.72, 73.06, 73.08}, {"G-7",  33.68, 33.70, 73.04, 73.06},
    {"G-8",  33.66, 33.68, 73.02, 73.04}, {"G-9",  33.64, 33.66, 73.00, 73.02},
    {"G-10", 33.62, 33.64, 72.98, 73.00}, {"G-11", 33.60, 33.62, 72.96, 72.98},
    {"H-8",  33.68, 33.70, 73.06, 73.08}, {"H-9",  33.66, 33.68, 73.04, 73.06},
    {"H-10", 33.64, 33.66, 73.02, 73.04}, {"H-11", 33.62, 33.64, 73.00, 73.02},
    {"H-12", 33.60, 33.62, 72.98, 73.00},
    {"I-8",  33.68, 33.70, 73.08, 73.10}, {"I-9",  33.66, 33.68, 73.06, 73.08},
    {"I-10", 33.64, 33.66, 73.04, 73.06}, {"I-11", 33.62, 33.64, 73.02, 73.04},
    {"I-12", 33.60, 33.62, 73.00, 73.02},
    // Adding 5s just in case they appear in data (E-5, F-5, etc)
    {"E-5", 33.72, 33.74, 73.08, 73.10}, {"F-5", 33.72, 33.74, 73.08, 73.10},
    {"G-5", 33.72, 33.74, 73.08, 73.10}, {"H-5", 33.72, 33.74, 73.08, 73.10},
    {"I-5", 33.72, 33.74, 73.08, 73.10}
};

class GeometryUtils {
public:
    static string resolveSector(double lat, double lon) {
        // Simple bounding box check
        for (int i = 0; i < SECTOR_COUNT; i++) {
            if (lat >= SECTOR_GRID[i].minLat && lat <= SECTOR_GRID[i].maxLat &&
                lon >= SECTOR_GRID[i].minLon && lon <= SECTOR_GRID[i].maxLon) {
                return SECTOR_GRID[i].name;
            }
        }
        return "Unknown Sector";
    }
    
    static double getHaversineDistance(double lat1, double lon1, double lat2, double lon2) {
        // Just used for edge weights, can remain real-world based if desired,
        // or we can switch to Euclidean distance on the grid.
        // Keeping Haversine allows "realistic" travel times even on a grid map.
        double dLat = (lat2 - lat1) * pi / 180.0;
        double dLon = (lon2 - lon1) * pi / 180.0;
        double a = sin(dLat / 2) * sin(dLat / 2) +
            cos(lat1 * pi / 180.0) * cos(lat2 * pi / 180.0) *
            sin(dLon / 2) * sin(dLon / 2);
        double c = 2 * atan2(sqrt(a), sqrt(1 - a));
        return Rad * c;
    }

    static int getSectorIndex(const string& name) {
        for (int i = 0; i < SECTOR_COUNT; i++) {
            if (SECTOR_GRID[i].name == name) return i;
        }
        return -1;
    }

    static void generateCoords(const string& sector, double& lat, double& lon) {
        // Used by other modules to generate random "real" coords.
        // We keep this to satisfy input requirements of other functions,
        // even though CityGraph will immediately map them to grid coords.
        int idx = getSectorIndex(sector);
        if (idx != -1) {
            lat = SECTOR_GRID[idx].getCenterLat();
            lon = SECTOR_GRID[idx].getCenterLon();
        } else {
            lat = 33.65; lon = 73.01;
        }
    }
};

// ============================================================================
// EDGE & NODE
// ============================================================================
struct Edge {
    int destinationID;
    double weight;
    Edge() : destinationID(-1), weight(0.0) {}
    Edge(int destID, double w) : destinationID(destID), weight(w) {}
};

struct CityNode {
    int id;
    string databaseID;
    string stopID;
    string name;
    string sector;
    string type;
    
    // THESE NOW STORE VISUAL GRID COORDINATES (X, Y)
    double lat; // visual X
    double lon; // visual Y
    
    string operatingHours;
    bool isAccessible;
    string additionalInfo;
    LinkedList<Edge> roads;

    CityNode(int i, string dbID, string sID, string n, string t, double x, double y)
        : id(i), databaseID(dbID), stopID(sID), name(n), type(t), lat(x), lon(y),
          operatingHours(""), isAccessible(true), additionalInfo("") {
        // Resolve sector from Name if possible, otherwise input should handle it
    }
    
    const LinkedList<Edge>& getRoads() const { return roads; }
    bool canBeTransportStop() const { return FacilityType::isTransportStop(type); }
    bool isPublicFacility() const { return FacilityType::isPublicFacility(type); }
};

struct DijkstraNode {
    int nodeID;
    double distance;
	DijkstraNode() : nodeID(-1), distance(INF) {}
    DijkstraNode(int id, double dist) : nodeID(id), distance(dist) {}
    bool operator<(const DijkstraNode& other) const { return distance > other.distance; }
};

struct TravelRecord {
    string citizenCNIC;
    int fromNodeID, toNodeID;
    string timestamp;
    double distance;
    string vehicleID, vehicleType;
    TravelRecord() : fromNodeID(-1) {}
    TravelRecord(const string& c, int f, int t, const string& time, double d, const string& v, const string& vt)
        : citizenCNIC(c), fromNodeID(f), toNodeID(t), timestamp(time), distance(d), vehicleID(v), vehicleType(vt) {}
};

struct CityStats {
    int totalNodes, busStops, schoolNodes, hospitalNodes, pharmacyNodes, sectorCorners;
    int totalSchools, totalHospitals, totalPharmacies, totalMalls;
    int totalBuses, activeBuses, totalSchoolBuses, activeSchoolBuses;
    int totalAmbulances, availableAmbulances, pendingTransfers;
    int totalSectors, totalStreets, totalHouses, totalCitizens;
    int totalPassengersServed, totalStudentsTransported, totalPatientsTransported, totalTravelRecords;
    
    CityStats() : totalNodes(0) {} // Init to 0
};