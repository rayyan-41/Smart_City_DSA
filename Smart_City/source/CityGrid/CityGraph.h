#pragma once
#include "CityUtils.h"

class CityGraph {
private:
    CityNode* nodes[MAX_NODES];
    int nodeCount;
    
    // Counters for generating unique IDs per facility type
    int facilityCounters[14];  // One for each public facility type
    
    // Connect a node to all corner nodes in its sector
    void connectNodeToSectorCorners(int nodeID, const string& sector);
   
public:
    CityGraph();
    ~CityGraph();
    
    // ==================== SECTOR MANAGEMENT ====================
    void initializeSectorFrame(const string& sectorName);
    
    // ==================== NODE ACCESS ====================
    CityNode* getNode(int index) const;
    int getNodeCount() const { return nodeCount; }
    
    // ==================== NODE CREATION ====================
    int addLocation(const string& databaseID, const string& stopID, 
                    const string& name, const string& type, double lat, double lon);
    int addPublicFacility(const string& name, const string& type, const string& sector);
    
    // Convenience methods for specific facility types
    int addMosque(const string& name, const string& sector, const string& prayerTimes = "");
    int addPark(const string& name, const string& sector, const string& hours = "06:00-22:00");
    int addWaterCooler(const string& name, const string& sector);
    int addPlayground(const string& name, const string& sector);
    int addLibrary(const string& name, const string& sector, const string& hours = "09:00-17:00");
    int addPoliceStation(const string& name, const string& sector);
    int addFireStation(const string& name, const string& sector);
    int addPetrolStation(const string& name, const string& sector, bool is24Hours = true);
    int addATM(const string& name, const string& sector, const string& bankName = "");
    int addRestaurant(const string& name, const string& sector, const string& cuisine = "");
    int addPublicToilet(const string& name, const string& sector);
    
    // ==================== ROAD MANAGEMENT ====================
    void addRoad(int id1, int id2);
    
    // ==================== PATHFINDING ====================
    Vector<int> findShortestPath(int startID, int endID, double& totalDistance);
    int findNearestFacility(int fromNodeID, const string& facilityType);
    Vector<int> findAllNearestFacilities(int fromNodeID, const string& facilityType, int maxCount = 5);
    Vector<int> calculateBusRoute(int startNodeID, int endNodeID, double& distance);
    
    // ==================== LOOKUP FUNCTIONS ====================
    int getIDByName(const string& name);
    int getIDByDatabaseID(const string& dbID);
    int getIDByStopID(const string& sID);
    
    // ==================== QUERY FUNCTIONS ====================
    Vector<int> getFacilitiesInSector(const string& sector, const string& type = "");
    Vector<int> getAllStopsInSector(const string& sector);
    void getBounds(double& minLat, double& maxLat, double& minLon, double& maxLon);
    
    // ==================== CSV LOADING ====================
    void loadStopsCSV(const string& filename);
    void loadBuildingsCSV(const string& filename, const string& type);
    void loadPublicFacilitiesCSV(const string& filename);
    
    // ==================== ID GENERATION ====================
    string generateStopID(const string& type);
};

// ============================================================================
// IMPLEMENTATION
// ============================================================================

// ==================== CONSTRUCTOR / DESTRUCTOR ====================

inline CityGraph::CityGraph() : nodeCount(0) {
    for (int i = 0; i < MAX_NODES; i++) {
        nodes[i] = nullptr;
    }
    for (int i = 0; i < 14; i++) {
        facilityCounters[i] = 0;
    }
}

inline CityGraph::~CityGraph() {
    for (int i = 0; i < nodeCount; i++) {
        delete nodes[i];
    }
}

// ==================== NODE ACCESS ====================

inline CityNode* CityGraph::getNode(int index) const {
    if (index < 0 || index >= nodeCount) return nullptr;
    return nodes[index];
}

// ==================== ID GENERATION ====================
// Maps facility type to counter index and generates unique ID

inline string CityGraph::generateStopID(const string& type) {
    string prefix = FacilityType::getStopIDPrefix(type);
    int counterIdx = 0;
    
    // Map type to counter index
    if (type == FacilityType::MOSQUE) counterIdx = 0;
    else if (type == FacilityType::PARK) counterIdx = 1;
    else if (type == FacilityType::WATER_COOLER) counterIdx = 2;
    else if (type == FacilityType::PLAYGROUND) counterIdx = 3;
    else if (type == FacilityType::LIBRARY) counterIdx = 4;
    else if (type == FacilityType::COMMUNITY_CENTER) counterIdx = 5;
    else if (type == FacilityType::POLICE_STATION) counterIdx = 6;
    else if (type == FacilityType::FIRE_STATION) counterIdx = 7;
    else if (type == FacilityType::POST_OFFICE) counterIdx = 8;
    else if (type == FacilityType::BANK) counterIdx = 9;
    else if (type == FacilityType::ATM) counterIdx = 10;
    else if (type == FacilityType::PETROL_STATION) counterIdx = 11;
    else if (type == FacilityType::RESTAURANT) counterIdx = 12;
    else if (type == FacilityType::PUBLIC_TOILET) counterIdx = 13;
    
    int count = ++facilityCounters[counterIdx];
    
    // Format: PREFIX-001, PREFIX-002, etc.
    string countStr = std::to_string(count);
    while (countStr.length() < 3) countStr = "0" + countStr;
    
    return prefix + "-" + countStr;
}

// ==================== SECTOR FRAME INITIALIZATION ====================
/*
 * Creates 4 corner nodes for a sector and connects them in a ring.
 * This provides the base connectivity framework for the sector.
 * 
 *     NW -------- NE
 *      |          |
 *      |  SECTOR  |
 *      |          |
 *     SW -------- SE
 */

inline void CityGraph::initializeSectorFrame(const string& sectorName) {
    int idx = GeometryUtils::getSectorIndex(sectorName);
    if (idx == -1 || SECTOR_GRID[idx].initialized) return;

    SectorBox& box = SECTOR_GRID[idx];

    // Create corner nodes
    int cSW = nodeCount;
    addLocation("C-" + sectorName + "-SW", "", sectorName + " SW", FacilityType::CORNER, box.minLat, box.minLon);
    
    int cNW = nodeCount;
    addLocation("C-" + sectorName + "-NW", "", sectorName + " NW", FacilityType::CORNER, box.maxLat, box.minLon);
    
    int cNE = nodeCount;
    addLocation("C-" + sectorName + "-NE", "", sectorName + " NE", FacilityType::CORNER, box.maxLat, box.maxLon);
    
    int cSE = nodeCount;
    addLocation("C-" + sectorName + "-SE", "", sectorName + " SE", FacilityType::CORNER, box.minLat, box.maxLon);

    // Connect corners in a ring: SW-NW-NE-SE-SW
    addRoad(cSW, cNW);
    addRoad(cNW, cNE);
    addRoad(cNE, cSE);
    addRoad(cSE, cSW);

    SECTOR_GRID[idx].initialized = true;
}

// ==================== NODE-TO-CORNER CONNECTION ====================
/*
 * Connects a node to all corner nodes in its sector.
 * This ensures the node is reachable from the sector's road network.
 */

inline void CityGraph::connectNodeToSectorCorners(int nodeID, const string& sector) {
    if (nodeID < 0 || nodeID >= nodeCount) return;
    if (sector.empty() || sector == "Unknown") return;
    
    for (int i = 0; i < nodeCount; i++) {
        if (!nodes[i] || nodes[i]->sector != sector || nodes[i]->type != FacilityType::CORNER) {
            continue;
        }
        
        // Check if road already exists
        bool exists = false;
        const LinkedList<Edge>& roads = nodes[nodeID]->getRoads();
        for (int j = 0; j < roads.size(); j++) {
            if (roads[j].destinationID == i) {
                exists = true;
                break;
            }
        }
        
        if (!exists) {
            addRoad(nodeID, i);
        }
    }
}



inline int CityGraph::addLocation(const string& databaseID, const string& stopID, 
                                   const string& name, const string& type, 
                                   double lat, double lon) {
    if (nodeCount >= MAX_NODES) return -1;

    string sector = GeometryUtils::resolveSector(lat, lon);

    // Initialize sector frame for non-corner nodes
    if (type != FacilityType::CORNER && sector != "Unknown") {
        int sectorIdx = GeometryUtils::getSectorIndex(sector);
        if (sectorIdx != -1 && !SECTOR_GRID[sectorIdx].initialized) {
            initializeSectorFrame(sector);
        }
    }

    // Create and store node
    int newID = nodeCount;
    nodes[newID] = new CityNode(newID, databaseID, stopID, name, type, lat, lon);
    nodeCount++;

    // Connect to sector corners
    if (type != FacilityType::CORNER) {
        connectNodeToSectorCorners(newID, sector);
    }

    return newID;
}

// ==================== PUBLIC FACILITY CREATION ====================

inline int CityGraph::addPublicFacility(const string& name, const string& type, const string& sector) {
    double lat, lon;
    GeometryUtils::generateCoords(sector, lat, lon);
    string stopID = generateStopID(type);
    return addLocation(stopID, stopID, name, type, lat, lon);
}

// Convenience methods with default metadata

inline int CityGraph::addMosque(const string& name, const string& sector, const string& prayerTimes) {
    int id = addPublicFacility(name, FacilityType::MOSQUE, sector);
    if (id != -1 && !prayerTimes.empty()) nodes[id]->operatingHours = prayerTimes;
    return id;
}

inline int CityGraph::addPark(const string& name, const string& sector, const string& hours) {
    int id = addPublicFacility(name, FacilityType::PARK, sector);
    if (id != -1) nodes[id]->operatingHours = hours;
    return id;
}

inline int CityGraph::addWaterCooler(const string& name, const string& sector) {
    int id = addPublicFacility(name, FacilityType::WATER_COOLER, sector);
    if (id != -1) {
        nodes[id]->operatingHours = "24/7";
        nodes[id]->additionalInfo = "Free drinking water";
    }
    return id;
}

inline int CityGraph::addPlayground(const string& name, const string& sector) {
    int id = addPublicFacility(name, FacilityType::PLAYGROUND, sector);
    if (id != -1) nodes[id]->operatingHours = "06:00-20:00";
    return id;
}

inline int CityGraph::addLibrary(const string& name, const string& sector, const string& hours) {
    int id = addPublicFacility(name, FacilityType::LIBRARY, sector);
    if (id != -1) nodes[id]->operatingHours = hours;
    return id;
}

inline int CityGraph::addPoliceStation(const string& name, const string& sector) {
    int id = addPublicFacility(name, FacilityType::POLICE_STATION, sector);
    if (id != -1) {
        nodes[id]->operatingHours = "24/7";
        nodes[id]->additionalInfo = "Emergency: 15";
    }
    return id;
}

inline int CityGraph::addFireStation(const string& name, const string& sector) {
    int id = addPublicFacility(name, FacilityType::FIRE_STATION, sector);
    if (id != -1) {
        nodes[id]->operatingHours = "24/7";
        nodes[id]->additionalInfo = "Emergency: 16";
    }
    return id;
}

inline int CityGraph::addPetrolStation(const string& name, const string& sector, bool is24Hours) {
    int id = addPublicFacility(name, FacilityType::PETROL_STATION, sector);
    if (id != -1) nodes[id]->operatingHours = is24Hours ? "24/7" : "06:00-22:00";
    return id;
}

inline int CityGraph::addATM(const string& name, const string& sector, const string& bankName) {
    int id = addPublicFacility(name, FacilityType::ATM, sector);
    if (id != -1) {
        nodes[id]->operatingHours = "24/7";
        nodes[id]->additionalInfo = bankName;
    }
    return id;
}

inline int CityGraph::addRestaurant(const string& name, const string& sector, const string& cuisine) {
    int id = addPublicFacility(name, FacilityType::RESTAURANT, sector);
    if (id != -1) {
        nodes[id]->operatingHours = "11:00-23:00";
        nodes[id]->additionalInfo = cuisine;
    }
    return id;
}

inline int CityGraph::addPublicToilet(const string& name, const string& sector) {
    int id = addPublicFacility(name, FacilityType::PUBLIC_TOILET, sector);
    if (id != -1) nodes[id]->operatingHours = "24/7";
    return id;
}

// ==================== ROAD MANAGEMENT ====================
/*
 * Adds a bidirectional road between two nodes.
 * Weight is calculated using grid distance.
 */

inline void CityGraph::addRoad(int id1, int id2) {
    if (id1 < 0 || id2 < 0 || id1 >= nodeCount || id2 >= nodeCount || id1 == id2) {
        return;
    }

    double dist = GeometryUtils::getGridDistance(
        nodes[id1]->lat, nodes[id1]->lon,
        nodes[id2]->lat, nodes[id2]->lon
    );

    nodes[id1]->roads.push_back(Edge(id2, dist));
    nodes[id2]->roads.push_back(Edge(id1, dist));
}

// ==================== LOOKUP FUNCTIONS ====================

inline int CityGraph::getIDByName(const string& name) {
    for (int i = 0; i < nodeCount; i++) {
        if (nodes[i] && nodes[i]->name == name) return i;
    }
    return -1;
}

inline int CityGraph::getIDByDatabaseID(const string& dbID) {
    for (int i = 0; i < nodeCount; i++) {
        if (nodes[i] && nodes[i]->databaseID == dbID) return i;
    }
    return -1;
}

inline int CityGraph::getIDByStopID(const string& sID) {
    for (int i = 0; i < nodeCount; i++) {
        if (nodes[i] && nodes[i]->stopID == sID) return i;
    }
    return -1;
}

// ==================== QUERY FUNCTIONS ====================

inline Vector<int> CityGraph::getFacilitiesInSector(const string& sector, const string& type) {
    Vector<int> results;
    for (int i = 0; i < nodeCount; i++) {
        if (!nodes[i] || nodes[i]->sector != sector || nodes[i]->type == FacilityType::CORNER) {
            continue;
        }
        if (type.empty() || nodes[i]->type == type) {
            results.push_back(i);
        }
    }
    return results;
}

inline Vector<int> CityGraph::getAllStopsInSector(const string& sector) {
    Vector<int> results;
    for (int i = 0; i < nodeCount; i++) {
        if (nodes[i] && nodes[i]->sector == sector && nodes[i]->canBeTransportStop()) {
            results.push_back(i);
        }
    }
    return results;
}

inline void CityGraph::getBounds(double& minLat, double& maxLat, double& minLon, double& maxLon) {
    GeometryUtils::getIslamabadBounds(minLat, maxLat, minLon, maxLon);
}

inline Vector<int> CityGraph::findShortestPath(int startID, int endID, double& totalDistance) {
    Vector<int> path;
    totalDistance = 0.0;
    
    if (startID < 0 || startID >= nodeCount || endID < 0 || endID >= nodeCount) {
        return path;
    }

    // Dijkstra state arrays
    double distance[MAX_NODES];
    int parent[MAX_NODES];
    bool visited[MAX_NODES];

    for (int i = 0; i < MAX_NODES; i++) {
        distance[i] = INF;
        parent[i] = -1;
        visited[i] = false;
    }

    // Initialize with start node
    PriorityQueue<DijkstraNode> pq;
    distance[startID] = 0.0;
    pq.push(DijkstraNode(startID, 0.0));

    // Process nodes in order of increasing distance
    while (!pq.empty()) {
        DijkstraNode current = pq.top();
        pq.pop();

        int u = current.nodeID;
        if (visited[u]) continue;
        visited[u] = true;

        // Early exit if destination reached
        if (u == endID) break;

        if (!nodes[u]) continue;

        // Relax edges
        const LinkedList<Edge>& roads = nodes[u]->roads;
        for (int i = 0; i < roads.size(); i++) {
            int v = roads[i].destinationID;
            double weight = roads[i].weight;

            if (!visited[v] && distance[u] + weight < distance[v]) {
                distance[v] = distance[u] + weight;
                parent[v] = u;
                pq.push(DijkstraNode(v, distance[v]));
            }
        }
    }
    
    // Reconstruct path by backtracking from end to start
    if (parent[endID] != -1 || startID == endID) {
        int current = endID;
        while (current != -1) {
            path.push_back(current);
            current = parent[current];
        }
        
        // Reverse to get start-to-end order
        for (int i = 0; i < path.getSize() / 2; i++) {
            int temp = path[i];
            path[i] = path[path.getSize() - 1 - i];
            path[path.getSize() - 1 - i] = temp;
        }
        
        totalDistance = distance[endID];
    }
    
    return path;
}

// ==================== FACILITY SEARCH ====================
/*
 * Finds the nearest facility of a given type using modified Dijkstra.
 * Stops as soon as a matching facility is found (guaranteed shortest).
 */

inline int CityGraph::findNearestFacility(int fromNodeID, const string& facilityType) {
    if (fromNodeID < 0 || fromNodeID >= nodeCount) return -1;
    
    double distance[MAX_NODES];
    bool visited[MAX_NODES];
    
    for (int i = 0; i < MAX_NODES; i++) {
        distance[i] = INF;
        visited[i] = false;
    }
    
    PriorityQueue<DijkstraNode> pq;
    distance[fromNodeID] = 0.0;
    pq.push(DijkstraNode(fromNodeID, 0.0));
    
    while (!pq.empty()) {
        DijkstraNode current = pq.top();
        pq.pop();
        
        int u = current.nodeID;
        if (visited[u]) continue;
        visited[u] = true;
        
        // Check if this is the target facility type
        if (u != fromNodeID && nodes[u] && nodes[u]->type == facilityType) {
            return u;
        }
        
        if (!nodes[u]) continue;
        
        // Expand neighbors
        const LinkedList<Edge>& roads = nodes[u]->roads;
        for (int i = 0; i < roads.size(); i++) {
            int v = roads[i].destinationID;
            double weight = roads[i].weight;
            
            if (!visited[v] && distance[u] + weight < distance[v]) {
                distance[v] = distance[u] + weight;
                pq.push(DijkstraNode(v, distance[v]));
            }
        }
    }
    
    return -1;
}

/*
 * Finds multiple nearest facilities of a given type.
 * Continues until maxCount facilities are found.
 */

inline Vector<int> CityGraph::findAllNearestFacilities(int fromNodeID, const string& facilityType, int maxCount) {
    Vector<int> results;
    if (fromNodeID < 0 || fromNodeID >= nodeCount) return results;
    
    double distance[MAX_NODES];
    bool visited[MAX_NODES];
    
    for (int i = 0; i < MAX_NODES; i++) {
        distance[i] = INF;
        visited[i] = false;
    }
    
    PriorityQueue<DijkstraNode> pq;
    distance[fromNodeID] = 0.0;
    pq.push(DijkstraNode(fromNodeID, 0.0));
    
    while (!pq.empty() && results.getSize() < maxCount) {
        DijkstraNode current = pq.top();
        pq.pop();
        
        int u = current.nodeID;
        if (visited[u]) continue;
        visited[u] = true;
        
        if (u != fromNodeID && nodes[u] && nodes[u]->type == facilityType) {
            results.push_back(u);
        }
        
        if (!nodes[u]) continue;
        
        const LinkedList<Edge>& roads = nodes[u]->roads;
        for (int i = 0; i < roads.size(); i++) {
            int v = roads[i].destinationID;
            double weight = roads[i].weight;
            
            if (!visited[v] && distance[u] + weight < distance[v]) {
                distance[v] = distance[u] + weight;
                pq.push(DijkstraNode(v, distance[v]));
            }
        }
    }
    
    return results;
}

inline Vector<int> CityGraph::calculateBusRoute(int startNodeID, int endNodeID, double& distance) {
    return findShortestPath(startNodeID, endNodeID, distance);
}

// ==================== CSV LOADING ====================
/*
 * CSV Format for stops: DatabaseID,Name,Sector
 * Coordinates are auto-generated within the sector bounds.
 */

inline void CityGraph::loadStopsCSV(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) return;

    string line;
    getline(file, line);  // Skip header

    while (getline(file, line)) {
        if (line.empty()) continue;

        string databaseID, name, sector;
        int i = 0;

        // Parse: DatabaseID,Name,Sector
        while (i < (int)line.size() && line[i] != ',') databaseID += line[i++];
        i++;
        while (i < (int)line.size() && line[i] != ',') name += line[i++];
        i++;
        while (i < (int)line.size()) {
            char c = line[i++];
            if (c != ' ' && c != '\t' && c != '\r' && c != '\n') sector += c;
        }

        if (databaseID.empty() || name.empty() || sector.empty()) continue;

        double lat, lon;
        GeometryUtils::generateCoords(sector, lat, lon);
        addLocation(databaseID, databaseID, name, FacilityType::STOP, lat, lon);
    }
    file.close();
}

/*
 * CSV Format for buildings: DatabaseID,Name,Sector,...
 * Generic loader for schools, hospitals, etc.
 */

inline void CityGraph::loadBuildingsCSV(const string& filename, const string& type) {
    ifstream file(filename);
    if (!file.is_open()) return;

    string line;
    getline(file, line);  // Skip header

    while (getline(file, line)) {
        if (line.empty()) continue;

        string databaseID, name, sector;
        int i = 0;
        bool inQuotes = false;

        // Parse DatabaseID
        while (i < (int)line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            databaseID += c;
        }

        // Parse Name
        inQuotes = false;
        while (i < (int)line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            name += c;
        }

        // Parse Sector
        inQuotes = false;
        while (i < (int)line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            if (c != '\r' && c != '\n' && c != ' ' && c != '\t') sector += c;
        }

        if (sector.empty()) continue;

        double lat, lon;
        GeometryUtils::generateCoords(sector, lat, lon);
        addLocation(databaseID, "", name, type, lat, lon);
    }
    file.close();
}

/*
 * CSV Format: Name,Type,Sector,Hours,Accessible,Info
 */

inline void CityGraph::loadPublicFacilitiesCSV(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) return;
    
    string line;
    getline(file, line);  // Skip header
    
    while (getline(file, line)) {
        if (line.empty()) continue;
        
        string name, type, sector, hours, accessible, info;
        int i = 0;
        bool inQuotes = false;
        
        // Helper lambda to parse next field
        auto parseField = [&](string& field) {
            inQuotes = false;
            while (i < (int)line.size()) {
                char c = line[i++];
                if (c == '"') { inQuotes = !inQuotes; continue; }
                if ((c == ',' || c == '\r' || c == '\n') && !inQuotes) break;
                field += c;
            }
        };
        
        parseField(name);
        parseField(type);
        parseField(sector);
        parseField(hours);
        parseField(accessible);
        parseField(info);
        
        if (name.empty() || type.empty() || sector.empty()) continue;
        
        int nodeID = addPublicFacility(name, type, sector);
        if (nodeID != -1) {
            if (!hours.empty()) nodes[nodeID]->operatingHours = hours;
            nodes[nodeID]->isAccessible = (accessible != "false" && accessible != "0" && accessible != "no");
            if (!info.empty()) nodes[nodeID]->additionalInfo = info;
        }
    }
    file.close();
}
