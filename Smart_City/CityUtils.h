//@desc: This file contains basic utilities for the city map. 
#include <string>
#include <cmath>
#include "CustomSTL.h"
using namespace std;

#define pi 3.14159265358979323846
#define INF 1e9
#define MAX_NODES 250
#define SECTOR_COUNT 12
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

};
static SectorBox SECTOR_GRID[SECTOR_COUNT] = {
    //F-Series (North)
    {"F-6", 33.720, 33.740, 73.060, 73.085},
    {"F-7", 33.710, 33.730, 73.045, 73.065},
    {"F-8", 33.700, 33.720, 73.030, 73.050},
    {"F-9", 33.690, 33.710, 73.015, 73.035}, 
    {"F-10", 33.680, 33.700, 73.000, 73.020},
    {"F-11", 33.670, 33.690, 72.980, 73.000},

    //G-Series (South)
    {"G-6", 33.710, 33.730, 73.070, 73.090},
    {"G-7", 33.700, 33.720, 73.050, 73.070},
    {"G-8", 33.690, 33.710, 73.030, 73.050},
    {"G-9", 33.680, 33.700, 73.010, 73.030},
    {"G-10", 33.670, 33.690, 72.990, 73.010},
    {"G-11", 33.660, 33.680, 72.970, 72.990}
};
//////////////////////////////////////////////////////////////////////////////////////

//@purpose: This namespace contains various geometry related utilities for the city map. 
//Basically just a collection of tools.
namespace GeometryUtils {
    
    struct Edge {
        int destinationID;
        double weight; // Distance in KM
    };

    struct CityNode {
        int id;
        string stopID;
        string name;
        string sector;

        double lat, lon;

        LinkedList<Edge> roads; //Adjacency List

        CityNode(int i, string sid, string n, double lt, double ln) {
            id = i; stopID = sid; name = n; lat = lt; lon = ln;
            //MAGIC: Auto-detect sector upon creation
            sector = GeometryUtils::resolveSector(lt, ln);
        }
    };

    //Haversine Formula: Returns distance in Kilometers
    double getHaversineDistance(double lat1, double lon1, double lat2, double lon2) {
        const double R = 6371.0; //Radius of Earth in KM
        double dLat = (lat2 - lat1) * pi / 180.0;
        double dLon = (lon2 - lon1) * pi / 180.0;
        double a = sin(dLat / 2) * sin(dLat / 2) +
            cos(lat1 * pi / 180.0) * cos(lat2 * pi / 180.0) *
            sin(dLon / 2) * sin(dLon / 2);
        double c = 2 * atan2(sqrt(a), sqrt(1 - a));
        return R * c;
    }

    //Identifies the sector based on coordinates
    string resolveSector(double lat, double lon) {
        for (int i = 0; i < SECTOR_COUNT; i++) {
            if (lat >= SECTOR_GRID[i].minLat && lat <= SECTOR_GRID[i].maxLat &&
                lon >= SECTOR_GRID[i].minLon && lon <= SECTOR_GRID[i].maxLon) {
                return SECTOR_GRID[i].name;
            }
        }
        return "Unknown Sector";
    }

    int getSectorIndex(string name) {
        for (int i = 0; i < SECTOR_COUNT; i++) {
            if (SECTOR_GRID[i].name == name) return i;
        }
        return -1;
    }

    //Generate random coordinate in a sector (maintaining integrity)
    void generateCoords(string sector, double& lat, double& lon) {
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
}

