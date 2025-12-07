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
 * ISLAMABAD SECTOR GRID - REALISTIC SECTOR LAYOUT
 * ============================================================================
 * 
 * Based on ACTUAL Islamabad residential sectors that exist in real life.
 * Islamabad's residential sectors follow a letter-number pattern where:
 * - Letters (E, F, G, H, I) indicate rows (E is northernmost)
 * - Numbers (6-12) indicate columns
 * 
 * NOT all combinations exist. The actual sectors are:
 * 
 * E-series: E-7, E-8, E-9, E-10, E-11 (5 sectors)
 * F-series: F-6, F-7, F-8, F-9, F-10, F-11 (6 sectors)
 * G-series: G-6, G-7, G-8, G-9, G-10, G-11 (6 sectors)
 * H-series: H-8, H-9, H-10, H-11, H-12, H-13 (6 sectors)
 * I-series: I-8, I-9, I-10, I-11, I-12, I-13, I-14 (7 sectors)
 * 
 * Total: 30 real sectors
 * 
 * Grid Layout (Geographic - North at Top):
 * 
 *     WEST  <--  Longitude (73.00 to 73.16)  -->  EAST
 *           73.00   73.02   73.04   73.06   73.08   73.10   73.12   73.14   73.16
 *              |       |       |       |       |       |       |       |       |
 *  N  33.74 --+-------+-------+-------+-------+-------+-------+-------+-------+
 *  O         |       | E-7   | E-8   | E-9   | E-10  | E-11  |       |       |
 *  R  33.72 --+-------+-------+-------+-------+-------+-------+-------+-------+
 *  T         | F-6   | F-7   | F-8   | F-9   | F-10  | F-11  |       |       |
 *  H  33.70 --+-------+-------+-------+-------+-------+-------+-------+-------+
 *     ^      | G-6   | G-7   | G-8   | G-9   | G-10  | G-11  |       |       |
 *  L  33.68 --+-------+-------+-------+-------+-------+-------+-------+-------+
 *  A         |       |       | H-8   | H-9   | H-10  | H-11  | H-12  | H-13  |
 *  T  33.66 --+-------+-------+-------+-------+-------+-------+-------+-------+
 *  I         |       |       | I-8   | I-9   | I-10  | I-11  | I-12  | I-13  | I-14
 *  T  33.64 --+-------+-------+-------+-------+-------+-------+-------+-------+
 *  U
 *  D
 *  E
 * 
 * Each sector is 0.02° x 0.02° (~2.2km x 1.8km)
 * 
 * Sector characteristics:
 * - E-7 to E-11: Upper-middle class residential, near Margalla Hills
 * - F-6 to F-11: Prime residential, Blue Area nearby (F-6, F-7)
 * - G-6 to G-11: Central hub, commercial centers (Centaurus in G-8)
 * - H-8 to H-13: Mixed residential, expanding eastward
 * - I-8 to I-14: Newer sectors, developing areas
 * 
 * ============================================================================
 */

// Define grid parameters
const double SECTOR_SIZE_LAT = 0.02;  // ~2.2 km north-south
const double SECTOR_SIZE_LON = 0.02;  // ~1.8 km east-west at this latitude

// Base coordinates (Southwest corner of entire grid)
const double BASE_LAT = 33.64;  // Southern boundary
const double BASE_LON = 73.00;  // Western boundary

// Maximum boundaries (Northeast corner)
const double MAX_LAT = 33.74;   // Northern boundary
const double MAX_LON = 73.18;   // Eastern boundary (extended for I-14 at 73.16-73.18)

// Column positions (longitude) - column number to west boundary
// Column 6: 73.00, Column 7: 73.02, Column 8: 73.04, etc.
#define COL_LON(col) (73.00 + ((col) - 6) * 0.02)

// Row positions (latitude) - row to south boundary  
// E: 33.72, F: 33.70, G: 33.68, H: 33.66, I: 33.64
#define ROW_E_LAT 33.72
#define ROW_F_LAT 33.70
#define ROW_G_LAT 33.68
#define ROW_H_LAT 33.66
#define ROW_I_LAT 33.64

static SectorBox SECTOR_GRID[] = { 
    // =========================================================================
    // E-SERIES (Row 1 - Northernmost) - Near Margalla Hills
    // Lat: 33.72 to 33.74
    // Only E-7 through E-11 exist in real Islamabad
    // =========================================================================
    {"E-7",  ROW_E_LAT, ROW_E_LAT + 0.02, COL_LON(7),  COL_LON(7) + 0.02},   // 73.02-73.04
    {"E-8",  ROW_E_LAT, ROW_E_LAT + 0.02, COL_LON(8),  COL_LON(8) + 0.02},   // 73.04-73.06
    {"E-9",  ROW_E_LAT, ROW_E_LAT + 0.02, COL_LON(9),  COL_LON(9) + 0.02},   // 73.06-73.08
    {"E-10", ROW_E_LAT, ROW_E_LAT + 0.02, COL_LON(10), COL_LON(10) + 0.02},  // 73.08-73.10
    {"E-11", ROW_E_LAT, ROW_E_LAT + 0.02, COL_LON(11), COL_LON(11) + 0.02},  // 73.10-73.12

    // =========================================================================
    // F-SERIES (Row 2) - Prime residential, Blue Area
    // Lat: 33.70 to 33.72
    // F-6 through F-11 exist
    // =========================================================================
    {"F-6",  ROW_F_LAT, ROW_F_LAT + 0.02, COL_LON(6),  COL_LON(6) + 0.02},   // 73.00-73.02
    {"F-7",  ROW_F_LAT, ROW_F_LAT + 0.02, COL_LON(7),  COL_LON(7) + 0.02},   // 73.02-73.04
    {"F-8",  ROW_F_LAT, ROW_F_LAT + 0.02, COL_LON(8),  COL_LON(8) + 0.02},   // 73.04-73.06
    {"F-9",  ROW_F_LAT, ROW_F_LAT + 0.02, COL_LON(9),  COL_LON(9) + 0.02},   // 73.06-73.08
    {"F-10", ROW_F_LAT, ROW_F_LAT + 0.02, COL_LON(10), COL_LON(10) + 0.02},  // 73.08-73.10
    {"F-11", ROW_F_LAT, ROW_F_LAT + 0.02, COL_LON(11), COL_LON(11) + 0.02},  // 73.10-73.12

    // =========================================================================
    // G-SERIES (Row 3 - Central) - Commercial hub, Centaurus Mall
    // Lat: 33.68 to 33.70
    // G-6 through G-11 exist
    // =========================================================================
    {"G-6",  ROW_G_LAT, ROW_G_LAT + 0.02, COL_LON(6),  COL_LON(6) + 0.02},   // 73.00-73.02
    {"G-7",  ROW_G_LAT, ROW_G_LAT + 0.02, COL_LON(7),  COL_LON(7) + 0.02},   // 73.02-73.04
    {"G-8",  ROW_G_LAT, ROW_G_LAT + 0.02, COL_LON(8),  COL_LON(8) + 0.02},   // 73.04-73.06 (Centaurus)
    {"G-9",  ROW_G_LAT, ROW_G_LAT + 0.02, COL_LON(9),  COL_LON(9) + 0.02},   // 73.06-73.08
    {"G-10", ROW_G_LAT, ROW_G_LAT + 0.02, COL_LON(10), COL_LON(10) + 0.02},  // 73.08-73.10
    {"G-11", ROW_G_LAT, ROW_G_LAT + 0.02, COL_LON(11), COL_LON(11) + 0.02},  // 73.10-73.12

    // =========================================================================
    // H-SERIES (Row 4) - Mixed residential
    // Lat: 33.66 to 33.68
    // H-8 through H-13 exist (no H-6, H-7)
    // =========================================================================
    {"H-8",  ROW_H_LAT, ROW_H_LAT + 0.02, COL_LON(8),  COL_LON(8) + 0.02},   // 73.04-73.06
    {"H-9",  ROW_H_LAT, ROW_H_LAT + 0.02, COL_LON(9),  COL_LON(9) + 0.02},   // 73.06-73.08
    {"H-10", ROW_H_LAT, ROW_H_LAT + 0.02, COL_LON(10), COL_LON(10) + 0.02},  // 73.08-73.10
    {"H-11", ROW_H_LAT, ROW_H_LAT + 0.02, COL_LON(11), COL_LON(11) + 0.02},  // 73.10-73.12
    {"H-12", ROW_H_LAT, ROW_H_LAT + 0.02, COL_LON(12), COL_LON(12) + 0.02},  // 73.12-73.14
    {"H-13", ROW_H_LAT, ROW_H_LAT + 0.02, COL_LON(13), COL_LON(13) + 0.02},  // 73.14-73.16

    // =========================================================================
    // I-SERIES (Row 5 - Southernmost) - Newer/developing sectors
    // Lat: 33.64 to 33.66
    // I-8 through I-14 exist (no I-6, I-7)
    // =========================================================================
    {"I-8",  ROW_I_LAT, ROW_I_LAT + 0.02, COL_LON(8),  COL_LON(8) + 0.02},   // 73.04-73.06
    {"I-9",  ROW_I_LAT, ROW_I_LAT + 0.02, COL_LON(9),  COL_LON(9) + 0.02},   // 73.06-73.08
    {"I-10", ROW_I_LAT, ROW_I_LAT + 0.02, COL_LON(10), COL_LON(10) + 0.02},  // 73.08-73.10
    {"I-11", ROW_I_LAT, ROW_I_LAT + 0.02, COL_LON(11), COL_LON(11) + 0.02},  // 73.10-73.12
    {"I-12", ROW_I_LAT, ROW_I_LAT + 0.02, COL_LON(12), COL_LON(12) + 0.02},  // 73.12-73.14
    {"I-13", ROW_I_LAT, ROW_I_LAT + 0.02, COL_LON(13), COL_LON(13) + 0.02},  // 73.14-73.16
    {"I-14", ROW_I_LAT, ROW_I_LAT + 0.02, COL_LON(14), COL_LON(14) + 0.02}   // 73.16-73.18
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
        
        // Find nearest sector by center distance using grid distance
        double minDist = 1e9;
        int bestIdx = -1;
        
        for (int i = 0; i < SECTOR_COUNT; i++) {
            double centerLat = SECTOR_GRID[i].getCenterLat();
            double centerLon = SECTOR_GRID[i].getCenterLon();
            double dist = getGridDistance(lat, lon, centerLat, centerLon);
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
    
    /**
     * Grid-based distance calculation using simple Euclidean distance
     * Treats lat/lon as a flat grid (suitable for small areas like a city)
     * Returns distance in approximate kilometers
     * 
     * @param lat1, lon1 First point coordinates
     * @param lat2, lon2 Second point coordinates
     * @return Distance in km (approximate)
     */
    static double getGridDistance(double lat1, double lon1, double lat2, double lon2) {
        // Convert lat/lon differences to approximate km
        // At Islamabad's latitude (~33.6°), 1 degree lat ? 111 km, 1 degree lon ? 92 km
        const double KM_PER_LAT_DEGREE = 111.0;
        const double KM_PER_LON_DEGREE = 92.0;  // cos(33.6°) * 111 ? 92
        
        double dLat = (lat2 - lat1) * KM_PER_LAT_DEGREE;
        double dLon = (lon2 - lon1) * KM_PER_LON_DEGREE;
        
        // Euclidean distance on the flat grid
        return std::sqrt(dLat * dLat + dLon * dLon);
    }
    
    /**
     * Manhattan distance on grid (useful for city block calculations)
     * Returns distance in approximate kilometers
     */
    static double getManhattanDistance(double lat1, double lon1, double lat2, double lon2) {
        const double KM_PER_LAT_DEGREE = 111.0;
        const double KM_PER_LON_DEGREE = 92.0;
        
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
    
    /**
     * Generate random coordinates within a sector's bounds
     * Uses a margin to keep points away from edges for cleaner visualization
     * 
     * @param sector The sector name (e.g., "G-9")
     * @param lat Output latitude
     * @param lon Output longitude
     */
    static void generateCoords(const string& sector, double& lat, double& lon) {
        int idx = getSectorIndex(sector);
        if (idx != -1) {
            const SectorBox& box = SECTOR_GRID[idx];
            
            // Use 15% margin on each side to keep points away from boundaries
            double marginLat = (box.maxLat - box.minLat) * 0.15;
            double marginLon = (box.maxLon - box.minLon) * 0.15;
            
            // Generate random position within the safe area (70% of sector)
            double randLat = (double)rand() / RAND_MAX;
            double randLon = (double)rand() / RAND_MAX;
            
            lat = box.minLat + marginLat + randLat * (box.maxLat - box.minLat - 2 * marginLat);
            lon = box.minLon + marginLon + randLon * (box.maxLon - box.minLon - 2 * marginLon);
            return;
        }
        // Default to G-9 center if sector not found
        lat = 33.65; 
        lon = 73.01;
    }
    
    /**
     * Generate coordinates at sector center (for important landmarks)
     */
    static void generateCenterCoords(const string& sector, double& lat, double& lon) {
        int idx = getSectorIndex(sector);
        if (idx != -1) {
            lat = SECTOR_GRID[idx].getCenterLat();
            lon = SECTOR_GRID[idx].getCenterLon();
            return;
        }
        lat = 33.65;
        lon = 73.01;
    }
    
    /**
     * Generate coordinates at a specific position within sector
     * @param posX 0.0 = west edge, 1.0 = east edge
     * @param posY 0.0 = south edge, 1.0 = north edge
     */
    static void generateCoordsAtPosition(const string& sector, double posX, double posY, 
                                          double& lat, double& lon) {
        int idx = getSectorIndex(sector);
        if (idx != -1) {
            const SectorBox& box = SECTOR_GRID[idx];
            // posY maps to latitude (0=south/minLat, 1=north/maxLat)
            lat = box.minLat + posY * (box.maxLat - box.minLat);
            // posX maps to longitude (0=west/minLon, 1=east/maxLon)
            lon = box.minLon + posX * (box.maxLon - box.minLon);
            return;
        }
        lat = 33.65;
        lon = 73.01;
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
        return lat >= BASE_LAT && lat <= MAX_LAT && lon >= BASE_LON && lon <= MAX_LON;
    }
    
    /**
     * Get the grid bounds for the entire Islamabad map
     * Returns a rectangular bounding box aligned to north
     */
    static void getIslamabadBounds(double& minLat, double& maxLat, double& minLon, double& maxLon) {
        minLat = BASE_LAT;  // 33.60 - South
        maxLat = MAX_LAT;   // 33.74 - North
        minLon = BASE_LON;  // 72.96 - West
        maxLon = MAX_LON;   // 73.10 - East
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

