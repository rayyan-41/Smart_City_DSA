#pragma once
#include "CityUtils.h" 

class CityGraph {
private:
    CityNode* nodes[MAX_NODES];
    int nodeCount;
    
    // Counters for generating unique IDs for public facilities
    int mosqueCount;
    int parkCount;
    int waterCoolerCount;
    int playgroundCount;
    int libraryCount;
    int communityCenterCount;
    int policeStationCount;
    int fireStationCount;
    int postOfficeCount;
    int bankCount;
    int atmCount;
    int petrolStationCount;
    int restaurantCount;
    int publicToiletCount;

    void initializeSectorFrame(string sectorName);
	friend class CityVisualizer;
	friend class SmartCity;

public:
    CityGraph();
    ~CityGraph(); 

    CityNode* getNode(int index) const {
        if (index < 0 || index >= nodeCount) return nullptr;
        return nodes[index];
    }
	int getNodeCount() { 
        return nodeCount; 
    }

    int addLocation(string databaseID, string stopID, string name, string type, double lat, double lon);
    void addRoad(int id1, int id2);

    // Pathfinding - now returns path and distance
    Vector<int> findShortestPath(int startID, int endID, double& totalDistance);
    void findShortestPathOld(int startID, int endID); // Keep old version for compatibility
	void getBounds(double& minLat, double& maxLat, double& minLon, double& maxLon);
    void resetVisuals();
    
    // Transport Module Functions
    int findNearestFacility(int fromNodeID, const string& facilityType);
    Vector<int> findAllNearestFacilities(int fromNodeID, const string& facilityType, int maxCount = 5);
    Vector<int> calculateBusRoute(int startNodeID, int endNodeID, double& distance);
    
    int getIDByName(string name);
    int getIDByDatabaseID(string dbID);
    int getIDByStopID(string sID);

    void loadStopsCSV(const string& filename);
    void loadBuildingsCSV(const string& filename, string type);
    
    // ==================== PUBLIC FACILITY MANAGEMENT ====================
    
    // Add a public facility with auto-generated stop ID
    int addPublicFacility(const string& name, const string& type, const string& sector,
                         const string& operatingHours = "", bool isAccessible = true,
                         const string& additionalInfo = "");
    
    // Add a public facility with specific coordinates
    int addPublicFacilityWithCoords(const string& name, const string& type, 
                                    double lat, double lon,
                                    const string& operatingHours = "", 
                                    bool isAccessible = true,
                                    const string& additionalInfo = "");
    
    // Convenience methods for common facility types
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
    
    // Find all public facilities of a type in a sector
    Vector<int> getPublicFacilitiesInSector(const string& sector, const string& type = "");
    
    // Find all transport stops (bus stops + public facilities) in a sector
    Vector<int> getAllTransportStopsInSector(const string& sector);
    
    // Load public facilities from CSV
    void loadPublicFacilitiesCSV(const string& filename);
    
    // Generate unique stop ID for a facility type
    string generateStopID(const string& type);
};

/*------ Constructors and Destructors ------*/
inline CityGraph::CityGraph() {
    nodeCount = 0;
    for (int i = 0; i < MAX_NODES; i++) nodes[i] = nullptr;
    
    // Initialize facility counters
    mosqueCount = 0;
    parkCount = 0;
    waterCoolerCount = 0;
    playgroundCount = 0;
    libraryCount = 0;
    communityCenterCount = 0;
    policeStationCount = 0;
    fireStationCount = 0;
    postOfficeCount = 0;
    bankCount = 0;
    atmCount = 0;
    petrolStationCount = 0;
    restaurantCount = 0;
    publicToiletCount = 0;
}

inline CityGraph::~CityGraph() {
    for (int i = 0; i < nodeCount; i++) {
        if (nodes[i] != nullptr) delete nodes[i];
    }
}

/*------ Generate Unique Stop ID ------*/
inline string CityGraph::generateStopID(const string& type) {
    string prefix = FacilityType::getStopIDPrefix(type);
    int count = 0;
    
    if (type == FacilityType::MOSQUE) count = ++mosqueCount;
    else if (type == FacilityType::PARK) count = ++parkCount;
    else if (type == FacilityType::WATER_COOLER) count = ++waterCoolerCount;
    else if (type == FacilityType::PLAYGROUND) count = ++playgroundCount;
    else if (type == FacilityType::LIBRARY) count = ++libraryCount;
    else if (type == FacilityType::COMMUNITY_CENTER) count = ++communityCenterCount;
    else if (type == FacilityType::POLICE_STATION) count = ++policeStationCount;
    else if (type == FacilityType::FIRE_STATION) count = ++fireStationCount;
    else if (type == FacilityType::POST_OFFICE) count = ++postOfficeCount;
    else if (type == FacilityType::BANK) count = ++bankCount;
    else if (type == FacilityType::ATM) count = ++atmCount;
    else if (type == FacilityType::PETROL_STATION) count = ++petrolStationCount;
    else if (type == FacilityType::RESTAURANT) count = ++restaurantCount;
    else if (type == FacilityType::PUBLIC_TOILET) count = ++publicToiletCount;
    else count = nodeCount + 1;
    
    // Format: PREFIX-XXX (e.g., MSQ-001, PRK-002)
    string countStr = std::to_string(count);
    while (countStr.length() < 3) countStr = "0" + countStr;
    
    return prefix + "-" + countStr;
}

/*------ Helper: Initialize Sector Frame ------*/
inline void CityGraph::initializeSectorFrame(string sectorName) {
    int idx = GeometryUtils::getSectorIndex(sectorName);

    // Safety check
    if (idx == -1) return;
    if (SECTOR_GRID[idx].initialized) return;

    SectorBox box = SECTOR_GRID[idx];

    // Create 4 corners of the sector rectangle
    // Geographic coordinates: lat increases going North, lon increases going East
    // Corner naming: SW (Southwest), NW (Northwest), NE (Northeast), SE (Southeast)
    
    // SW: minLat, minLon (bottom-left in standard map view)
    int cSW = nodeCount; 
    addLocation("C-" + sectorName + "-SW", "", sectorName + " SW", FacilityType::CORNER, box.minLat, box.minLon);
    
    // NW: maxLat, minLon (top-left)
    int cNW = nodeCount; 
    addLocation("C-" + sectorName + "-NW", "", sectorName + " NW", FacilityType::CORNER, box.maxLat, box.minLon);
    
    // NE: maxLat, maxLon (top-right)
    int cNE = nodeCount; 
    addLocation("C-" + sectorName + "-NE", "", sectorName + " NE", FacilityType::CORNER, box.maxLat, box.maxLon);
    
    // SE: minLat, maxLon (bottom-right)
    int cSE = nodeCount; 
    addLocation("C-" + sectorName + "-SE", "", sectorName + " SE", FacilityType::CORNER, box.minLat, box.maxLon);

    // Connect corners to form a rectangle (perimeter roads)
    // West edge: SW <-> NW
    addRoad(cSW, cNW);
    // North edge: NW <-> NE
    addRoad(cNW, cNE);
    // East edge: NE <-> SE
    addRoad(cNE, cSE);
    // South edge: SE <-> SW
    addRoad(cSE, cSW);

    SECTOR_GRID[idx].initialized = true;
}

/*------ Add Node Logic ------*/
inline int CityGraph::addLocation(string databaseID, string stopID, string name, string type, double lat, double lon) {
    if (nodeCount >= MAX_NODES) {
        return -1;
    }

    string sector = GeometryUtils::resolveSector(lat, lon);

    //1. Check if we need to build the frame FIRST
    if (type != FacilityType::CORNER && sector != "Unknown" && sector != "Unknown Sector" && 
        GeometryUtils::getSectorIndex(sector) != -1 && !SECTOR_GRID[GeometryUtils::getSectorIndex(sector)].initialized) {
        initializeSectorFrame(sector);
    }

    //2. Create Node
    int newID = nodeCount;
    nodes[newID] = new CityNode(newID, databaseID, stopID, name, type, lat, lon);
    nodeCount++;

    //3. Connect to ALL FOUR corners of the sector (not just nearest)
    if (type != FacilityType::CORNER && sector != "Unknown" && sector != "Unknown Sector") {
        // Find all 4 corners of this sector and connect to each
        for (int i = 0; i < nodeCount - 1; i++) {
            if (nodes[i]->sector == sector && nodes[i]->type == FacilityType::CORNER) {
                addRoad(newID, i);
            }
        }
    }

    return newID;
}

/*------ Public Facility Management ------*/

inline int CityGraph::addPublicFacility(const string& name, const string& type, const string& sector,
                                        const string& operatingHours, bool isAccessible,
                                        const string& additionalInfo) {
    // Generate coordinates in the sector
    double lat = 0.0, lon = 0.0;
    GeometryUtils::generateCoords(sector, lat, lon);
    
    return addPublicFacilityWithCoords(name, type, lat, lon, operatingHours, isAccessible, additionalInfo);
}

inline int CityGraph::addPublicFacilityWithCoords(const string& name, const string& type,
                                                   double lat, double lon,
                                                   const string& operatingHours,
                                                   bool isAccessible,
                                                   const string& additionalInfo) {
    // Generate unique IDs
    string stopID = generateStopID(type);
    string databaseID = stopID;  // Use same ID for database
    
    // Add to graph
    int nodeID = addLocation(databaseID, stopID, name, type, lat, lon);
    
    if (nodeID != -1) {
        // Set additional metadata
        nodes[nodeID]->operatingHours = operatingHours;
        nodes[nodeID]->isAccessible = isAccessible;
        nodes[nodeID]->additionalInfo = additionalInfo;
    }
    
    return nodeID;
}

// Convenience methods for common facility types
inline int CityGraph::addMosque(const string& name, const string& sector, const string& prayerTimes) {
    return addPublicFacility(name, FacilityType::MOSQUE, sector, prayerTimes, true, "");
}

inline int CityGraph::addPark(const string& name, const string& sector, const string& hours) {
    return addPublicFacility(name, FacilityType::PARK, sector, hours, true, "");
}

inline int CityGraph::addWaterCooler(const string& name, const string& sector) {
    return addPublicFacility(name, FacilityType::WATER_COOLER, sector, "24/7", true, "Free drinking water");
}

inline int CityGraph::addPlayground(const string& name, const string& sector) {
    return addPublicFacility(name, FacilityType::PLAYGROUND, sector, "06:00-20:00", true, "Children's playground");
}

inline int CityGraph::addLibrary(const string& name, const string& sector, const string& hours) {
    return addPublicFacility(name, FacilityType::LIBRARY, sector, hours, true, "");
}

inline int CityGraph::addPoliceStation(const string& name, const string& sector) {
    return addPublicFacility(name, FacilityType::POLICE_STATION, sector, "24/7", true, "Emergency: 15");
}

inline int CityGraph::addFireStation(const string& name, const string& sector) {
    return addPublicFacility(name, FacilityType::FIRE_STATION, sector, "24/7", true, "Emergency: 16");
}

inline int CityGraph::addPetrolStation(const string& name, const string& sector, bool is24Hours) {
    string hours = is24Hours ? "24/7" : "06:00-22:00";
    return addPublicFacility(name, FacilityType::PETROL_STATION, sector, hours, true, "");
}

inline int CityGraph::addATM(const string& name, const string& sector, const string& bankName) {
    return addPublicFacility(name, FacilityType::ATM, sector, "24/7", true, bankName);
}

inline int CityGraph::addRestaurant(const string& name, const string& sector, const string& cuisine) {
    return addPublicFacility(name, FacilityType::RESTAURANT, sector, "11:00-23:00", true, cuisine);
}

inline int CityGraph::addPublicToilet(const string& name, const string& sector) {
    return addPublicFacility(name, FacilityType::PUBLIC_TOILET, sector, "24/7", true, "");
}

inline Vector<int> CityGraph::getPublicFacilitiesInSector(const string& sector, const string& type) {
    Vector<int> results;
    
    for (int i = 0; i < nodeCount; i++) {
        if (nodes[i] && nodes[i]->sector == sector) {
            if (type.empty()) {
                // Return all public facilities
                if (nodes[i]->isPublicFacility()) {
                    results.push_back(i);
                }
            } else {
                // Return specific type
                if (nodes[i]->type == type) {
                    results.push_back(i);
                }
            }
        }
    }
    
    return results;
}

inline Vector<int> CityGraph::getAllTransportStopsInSector(const string& sector) {
    Vector<int> results;
    
    for (int i = 0; i < nodeCount; i++) {
        if (nodes[i] && nodes[i]->sector == sector && nodes[i]->canBeTransportStop()) {
            results.push_back(i);
        }
    }
    
    return results;
}

inline void CityGraph::loadPublicFacilitiesCSV(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) return;
    
    string line;
    getline(file, line); // Skip header
    
    // Expected format: Name,Type,Sector,OperatingHours,IsAccessible,AdditionalInfo
    while (getline(file, line)) {
        if (line.empty()) continue;
        
        string name = "", type = "", sector = "", hours = "", accessible = "", info = "";
        int i = 0;
        bool inQuotes = false;
        
        // Parse Name
        while (i < (int)line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            name += c;
        }
        
        // Parse Type
        inQuotes = false;
        while (i < (int)line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            type += c;
        }
        
        // Parse Sector
        inQuotes = false;
        while (i < (int)line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            sector += c;
        }
        
        // Parse OperatingHours
        inQuotes = false;
        while (i < (int)line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            hours += c;
        }
        
        // Parse IsAccessible
        inQuotes = false;
        while (i < (int)line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            accessible += c;
        }
        
        // Parse AdditionalInfo (rest of line)
        inQuotes = false;
        while (i < (int)line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == '\r' || c == '\n') break;
            info += c;
        }
        
        // Validate and add
        if (!name.empty() && !type.empty() && !sector.empty()) {
            bool isAccessible = (accessible != "false" && accessible != "0" && accessible != "no");
            addPublicFacility(name, type, sector, hours, isAccessible, info);
        }
    }
    
    file.close();
}

/*------ Core Helpers ------*/
inline int CityGraph::getIDByName(string name) {
    for (int i = 0; i < nodeCount; i++) {
        if (nodes[i]->name == name) return i;
    }
    return -1;
}

inline int CityGraph::getIDByDatabaseID(string dbID) {
    for (int i = 0; i < nodeCount; i++) {
        if (nodes[i]->databaseID == dbID) return i;
    }
    return -1;
}

inline int CityGraph::getIDByStopID(string sID) {
    for (int i = 0; i < nodeCount; i++) {
        if (nodes[i]->stopID == sID) return i;
    }
    return -1;
}

inline void CityGraph::addRoad(int id1, int id2) {
    if (id1 < 0 || id2 < 0 || id1 >= nodeCount || id2 >= nodeCount) return;

    double dist = GeometryUtils::getHaversineDistance(nodes[id1]->lat, nodes[id1]->lon, nodes[id2]->lat, nodes[id2]->lon);

    Edge e1(id2, dist);
    nodes[id1]->roads.push_back(e1);

    Edge e2(id1, dist);
    nodes[id2]->roads.push_back(e2);
}

/*------ Stop Loader (Sector-Based Coordinates) ------*/
inline void CityGraph::loadStopsCSV(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        return;
    }

    string line;
    getline(file, line); // Skip header: StopID,Name,Sector

    while (getline(file, line)) {
        if (line.empty()) continue;

        string databaseID = "", name = "", sector = "";
        int i = 0;

        // Parse StopID
        while (i < (int)line.size() && line[i] != ',') databaseID += line[i++];
        if (i >= (int)line.size()) continue;
        i++; // skip comma
        
        // Parse Name
        while (i < (int)line.size() && line[i] != ',') name += line[i++];
        if (i >= (int)line.size()) continue;
        i++; // skip comma
        
        // Parse Sector (rest of line, trim whitespace)
        while (i < (int)line.size() && (line[i] == ' ' || line[i] == '\t')) i++;
        while (i < (int)line.size() && line[i] != '\r' && line[i] != '\n') {
            sector += line[i++];
        }
        // Trim trailing whitespace
        while (!sector.empty() && (sector.back() == ' ' || sector.back() == '\t' || sector.back() == '\r')) {
            sector.pop_back();
        }

        if (databaseID.empty() || name.empty() || sector.empty()) {
            continue;
        }

        // Generate coordinates within the sector bounds
        double lat = 0.0, lon = 0.0;
        GeometryUtils::generateCoords(sector, lat, lon);
        
        // Use databaseID as stopID as well
        addLocation(databaseID, databaseID, name, FacilityType::STOP, lat, lon);
    }
    file.close();
}

/*------ Generic Building Loader (Sector-Based Coordinates) ------*/
inline void CityGraph::loadBuildingsCSV(const string& filename, string type) {
    ifstream file(filename);
    if (!file.is_open()) {
        return;
    }

    string line;
    getline(file, line); // Skip header

    int successCount = 0;

    while (getline(file, line)) {
        if (line.empty()) continue;

        string databaseID = "";
        string name = "";
        string sector = "";
        int i = 0;

        // Parse databaseID
        bool inQuotes = false;
        while (i < (int)line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            databaseID += c;
        }

        // Parse name
        inQuotes = false;
        while (i < (int)line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            name += c;
        }

        // Parse sector
        inQuotes = false;
        while (i < (int)line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            if (c != '\r' && c != '\n') sector += c;
        }
        
        // Trim whitespace from sector
        while (!sector.empty() && (sector.front() == ' ' || sector.front() == '\t')) {
            sector = sector.substr(1);
        }
        while (!sector.empty() && (sector.back() == ' ' || sector.back() == '\t' || sector.back() == '\r')) {
            sector.pop_back();
        }

        if (sector.empty()) continue;

        // Generate coordinates within the sector bounds
        double lat = 0.0, lon = 0.0;
        GeometryUtils::generateCoords(sector, lat, lon);

        if (addLocation(databaseID, "", name, type, lat, lon) != -1) {
            successCount++;
        }
    }
    file.close();
}

/*------ Algorithms ------*/
inline Vector<int> CityGraph::findShortestPath(int startID, int endID, double& totalDistance) {
    Vector<int> path;
    totalDistance = 0.0;
    
    if (startID < 0 || startID >= nodeCount || endID < 0 || endID >= nodeCount) {
        return path;
    }

    double distance[MAX_NODES];
    int parent[MAX_NODES];
    bool visited[MAX_NODES];

    for (int i = 0; i < MAX_NODES; i++) {
        distance[i] = INF;
        parent[i] = -1;
        visited[i] = false;
    }

    PriorityQueue<DijkstraNode> pq;

    distance[startID] = 0.0;
    pq.push(DijkstraNode(startID, 0.0));

    while (!pq.empty()) {
        DijkstraNode current = pq.top();
        pq.pop();

        int u = current.nodeID;
        if (visited[u]) continue;
        visited[u] = true;

        if (u == endID) break;

        if (u < 0 || u >= nodeCount || nodes[u] == nullptr) continue;

        for (int i = 0; i < nodes[u]->roads.size(); i++) {
            Edge edge = nodes[u]->roads[i];
            int v = edge.destinationID;
            double weight = edge.weight;

            if (!visited[v] && distance[u] + weight < distance[v]) {
                distance[v] = distance[u] + weight;
                parent[v] = u;
                pq.push(DijkstraNode(v, distance[v]));
            }
        }
    }
    
    if (parent[endID] != -1 || startID == endID) {
        int current = endID;
        while (current != -1) {
            path.push_back(current);
            current = parent[current];
        }
        
        for (int i = 0; i < path.getSize() / 2; i++) {
            int temp = path[i];
            path[i] = path[path.getSize() - 1 - i];
            path[path.getSize() - 1 - i] = temp;
        }
        
        totalDistance = distance[endID];
    }
    
    return path;
}

inline void CityGraph::findShortestPathOld(int startID, int endID) {
    if (startID < 0 || startID >= nodeCount || endID < 0 || endID >= nodeCount) {
        return;
    }

    double distance[MAX_NODES];
    int parent[MAX_NODES];
    bool visited[MAX_NODES];

    for (int i = 0; i < MAX_NODES; i++) {
        distance[i] = INF;
        parent[i] = -1;
        visited[i] = false;
    }

    PriorityQueue<DijkstraNode> pq;

    distance[startID] = 0.0;
    pq.push(DijkstraNode(startID, 0.0));

    while (!pq.empty()) {
        DijkstraNode current = pq.top();
        pq.pop();

        int u = current.nodeID;
        if (visited[u]) continue;
        visited[u] = true;

        if (u == endID) break;

        if (u < 0 || u >= nodeCount || nodes[u] == nullptr) continue;

        for (int i = 0; i < nodes[u]->roads.size(); i++) {
            Edge edge = nodes[u]->roads[i];
            int v = edge.destinationID;
            double weight = edge.weight;

            if (!visited[v] && distance[u] + weight < distance[v]) {
                distance[v] = distance[u] + weight;
                parent[v] = u;
                pq.push(DijkstraNode(v, distance[v]));
            }
        }
    }
}

/*------ Transport Module Functions ------*/
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
    
    int nearestFacility = -1;
    double minDistance = INF;
    
    while (!pq.empty()) {
        DijkstraNode current = pq.top();
        pq.pop();
        
        int u = current.nodeID;
        if (visited[u]) continue;
        visited[u] = true;
        
        if (u != fromNodeID && nodes[u] != nullptr && nodes[u]->type == facilityType) {
            if (distance[u] < minDistance) {
                minDistance = distance[u];
                nearestFacility = u;
            }
            break;
        }
        
        if (u < 0 || u >= nodeCount || nodes[u] == nullptr) continue;
        
        for (int i = 0; i < nodes[u]->roads.size(); i++) {
            Edge edge = nodes[u]->roads[i];
            int v = edge.destinationID;
            double weight = edge.weight;
            
            if (!visited[v] && distance[u] + weight < distance[v]) {
                distance[v] = distance[u] + weight;
                pq.push(DijkstraNode(v, distance[v]));
            }
        }
    }
    
    return nearestFacility;
}

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
        
        if (u != fromNodeID && nodes[u] != nullptr && nodes[u]->type == facilityType) {
            results.push_back(u);
        }
        
        if (u < 0 || u >= nodeCount || nodes[u] == nullptr) continue;
        
        for (int i = 0; i < nodes[u]->roads.size(); i++) {
            Edge edge = nodes[u]->roads[i];
            int v = edge.destinationID;
            double weight = edge.weight;
            
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

inline void CityGraph::getBounds(double& minLat, double& maxLat, double& minLon, double& maxLon) {
    minLat = 90.0;
    maxLat = -90.0;
    minLon = 180.0;
    maxLon = -180.0;
    
    for (int i = 0; i < nodeCount; i++) {
        if (nodes[i] != nullptr) {
            if (nodes[i]->lat < minLat) minLat = nodes[i]->lat;
            if (nodes[i]->lat > maxLat) maxLat = nodes[i]->lat;
            if (nodes[i]->lon < minLon) minLon = nodes[i]->lon;
            if (nodes[i]->lon > maxLon) maxLon = nodes[i]->lon;
        }
    }
    
    // Add a small padding
    double latPadding = (maxLat - minLat) * 0.05;
    double lonPadding = (maxLon - minLon) * 0.05;
    minLat -= latPadding;
    maxLat += latPadding;
    minLon -= lonPadding;
    maxLon += lonPadding;
}

















