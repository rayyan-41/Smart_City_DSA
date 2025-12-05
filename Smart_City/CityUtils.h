//@desc: This file contains basic utilities for the city map.
#pragma once
#include <string>
#include <cmath>
#include <iostream>
#include <fstream>
#include "CustomSTL.h"
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


//@purpose: Organizes sector boundaries. Pure utility struct that doesn't do much else.
struct SectorBox {
    string name;
    double minLat, maxLat;
    double minLon, maxLon;
	bool initialized = false;
    SectorBox() : name(""), minLat(0.0), maxLat(0.0), minLon(0.0), maxLon(0.0), initialized(false) {}
    SectorBox(const string& n, double minLa, double maxLa, double minLo, double maxLo)
        : name(n), minLat(minLa), maxLat(maxLa), minLon(minLo), maxLon(maxLo), initialized(false) {
    }
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

//@purpose: This namespace contains various geometry related utilities for the city map. 
class GeometryUtils {
public:
    static string resolveSector(double lat, double lon) {
        //Identifies the sector based on coordinates

        for (int i = 0; i < SECTOR_COUNT; i++) {
            if (lat >= SECTOR_GRID[i].minLat && lat <= SECTOR_GRID[i].maxLat &&
                lon >= SECTOR_GRID[i].minLon && lon <= SECTOR_GRID[i].maxLon) {
                return SECTOR_GRID[i].name;
            }
        }
        return "Unknown Sector";
    }
    static double getHaversineDistance(double lat1, double lon1, double lat2, double lon2) {
        //Haversine Formula: Returns distance in Kilometers

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
        //Generate random coordinate in a sector (maintaining integrity)

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
};


struct Edge {
    int destinationID;
    double weight; // Distance in KM

	Edge() : destinationID(-1), weight(0.0) {}
	Edge(int destID, double w) : destinationID(destID), weight(w) {}
    bool operator==(const Edge& other) const {
        return destinationID == other.destinationID && weight == other.weight;
    }
};

struct CityNode {
    int id;              // Internal graph node ID (position in array)
    string databaseID;   // Original ID from CSV file (SCH001, HOSP042, etc.)
    string stopID;       // Transport system ID (only for bus stops, empty otherwise)
    string name;
    string sector;
    string type;
    double lat, lon;

    LinkedList<Edge> roads; //Adjacency List

    CityNode(int i, string dbID, string sID, string n, string t, double lt, double ln)
        : id(i), databaseID(dbID), stopID(sID), name(n), type(t), lat(lt), lon(ln) {
        sector = GeometryUtils::resolveSector(lt, ln);   //MAGIC: Auto-detect sector upon creation
       
    }
};

struct DijkstraNode {
    
    //Helper struct for Dijkstra's Priority Queue (Min-Heap based on distance)
    int nodeID;
    double distance;

    DijkstraNode() : nodeID(-1), distance(INF) {}
    DijkstraNode(int id, double dist) : nodeID(id), distance(dist) {}

    // Operator< for min-heap (smaller distance = higher priority)
    // Note: PriorityQueue implements max-heap, so we reverse the comparison
    bool operator<(const DijkstraNode& other) const {
        return distance > other.distance;  // Reverse for min-heap behavior
    }
};

