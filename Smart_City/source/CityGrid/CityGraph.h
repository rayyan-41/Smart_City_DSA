#pragma once
#include "CityUtils.h" 

class CityGraph {
private:
    CityNode* nodes[MAX_NODES];
    int nodeCount;
    int facilityCounter;

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

    void connectNodeToSectorCorners(int nodeID, const string& sector);
    
    friend class CityVisualizer;
    friend class SmartCity;

public:
    CityGraph();
    ~CityGraph(); 

    void initializeSectorFrame(string sectorName);

    CityNode* getNode(int index) const {
        if (index < 0 || index >= nodeCount) return nullptr;
        return nodes[index];
    }
    int getNodeCount() { 
        return nodeCount; 
    }

    int addLocation(string databaseID, string stopID, string name, string type, double lat, double lon);
    void addRoad(int id1, int id2);

    // Pathfinding
    Vector<int> findShortestPath(int startID, int endID, double& totalDistance);
    void findShortestPathOld(int startID, int endID);
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
    
    // Public Facility Management
    int addPublicFacility(const string& name, const string& type, const string& sector);
    Vector<int> getFacilitiesInSector(const string& sector, const string& type = "");
    Vector<int> getAllStopsInSector(const string& sector);
    
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
    
    void loadPublicFacilitiesCSV(const string& filename);
    string generateStopID(const string& type);
};

/*------ Constructors and Destructors ------*/
inline CityGraph::CityGraph() {
    nodeCount = 0;
    facilityCounter = 0;
    for (int i = 0; i < MAX_NODES; i++) nodes[i] = nullptr;
    
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
    facilityCounter++;
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
    else count = facilityCounter;
    
    string countStr = std::to_string(count);
    while (countStr.length() < 3) countStr = "0" + countStr;
    
    return prefix + "-" + countStr;
}

/*------ Helper: Initialize Sector Frame ------*/
inline void CityGraph::initializeSectorFrame(string sectorName) {
    int idx = GeometryUtils::getSectorIndex(sectorName);
    if (idx == -1) return;
    if (SECTOR_GRID[idx].initialized) return;

    SectorBox box = SECTOR_GRID[idx];

    int cSW = nodeCount; 
    addLocation("C-" + sectorName + "-SW", "", sectorName + " SW", FacilityType::CORNER, box.minLat, box.minLon);
    
    int cNW = nodeCount; 
    addLocation("C-" + sectorName + "-NW", "", sectorName + " NW", FacilityType::CORNER, box.maxLat, box.minLon);
    
    int cNE = nodeCount; 
    addLocation("C-" + sectorName + "-NE", "", sectorName + " NE", FacilityType::CORNER, box.maxLat, box.maxLon);
    
    int cSE = nodeCount; 
    addLocation("C-" + sectorName + "-SE", "", sectorName + " SE", FacilityType::CORNER, box.minLat, box.maxLon);

    addRoad(cSW, cNW);
    addRoad(cNW, cNE);
    addRoad(cNE, cSE);
    addRoad(cSE, cSW);

    SECTOR_GRID[idx].initialized = true;
}

/*------ Helper: Connect node to ALL corners of its sector ------*/
inline void CityGraph::connectNodeToSectorCorners(int nodeID, const string& sector) {
    if (nodeID < 0 || nodeID >= nodeCount) return;
    if (sector.empty() || sector == "Unknown" || sector == "Unknown Sector") return;
    
    for (int i = 0; i < nodeCount; i++) {
        if (nodes[i] && nodes[i]->sector == sector && nodes[i]->type == FacilityType::CORNER) {
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
}

/*------ Add Node Logic ------*/
inline int CityGraph::addLocation(string databaseID, string stopID, string name, string type, double lat, double lon) {
    if (nodeCount >= MAX_NODES) {
        return -1;
    }

    string sector = GeometryUtils::resolveSector(lat, lon);

    if (type != FacilityType::CORNER && sector != "Unknown" && sector != "Unknown Sector") {
        int sectorIdx = GeometryUtils::getSectorIndex(sector);
        if (sectorIdx != -1 && !SECTOR_GRID[sectorIdx].initialized) {
            initializeSectorFrame(sector);
        }
    }

    int newID = nodeCount;
    nodes[newID] = new CityNode(newID, databaseID, stopID, name, type, lat, lon);
    nodeCount++;

    if (type != FacilityType::CORNER) {
        connectNodeToSectorCorners(newID, sector);
    }

    return newID;
}

/*------ Simple Public Facility ------*/
inline int CityGraph::addPublicFacility(const string& name, const string& type, const string& sector) {
    double lat = 0.0, lon = 0.0;
    GeometryUtils::generateCoords(sector, lat, lon);
    string stopID = generateStopID(type);
    return addLocation(stopID, stopID, name, type, lat, lon);
}

inline int CityGraph::addMosque(const string& name, const string& sector, const string& prayerTimes) {
    int nodeID = addPublicFacility(name, FacilityType::MOSQUE, sector);
    if (nodeID != -1 && !prayerTimes.empty()) {
        nodes[nodeID]->operatingHours = prayerTimes;
    }
    return nodeID;
}

inline int CityGraph::addPark(const string& name, const string& sector, const string& hours) {
    int nodeID = addPublicFacility(name, FacilityType::PARK, sector);
    if (nodeID != -1) {
        nodes[nodeID]->operatingHours = hours;
    }
    return nodeID;
}

inline int CityGraph::addWaterCooler(const string& name, const string& sector) {
    int nodeID = addPublicFacility(name, FacilityType::WATER_COOLER, sector);
    if (nodeID != -1) {
        nodes[nodeID]->operatingHours = "24/7";
        nodes[nodeID]->additionalInfo = "Free drinking water";
    }
    return nodeID;
}

inline int CityGraph::addPlayground(const string& name, const string& sector) {
    int nodeID = addPublicFacility(name, FacilityType::PLAYGROUND, sector);
    if (nodeID != -1) {
        nodes[nodeID]->operatingHours = "06:00-20:00";
        nodes[nodeID]->additionalInfo = "Children's playground";
    }
    return nodeID;
}

inline int CityGraph::addLibrary(const string& name, const string& sector, const string& hours) {
    int nodeID = addPublicFacility(name, FacilityType::LIBRARY, sector);
    if (nodeID != -1) {
        nodes[nodeID]->operatingHours = hours;
    }
    return nodeID;
}

inline int CityGraph::addPoliceStation(const string& name, const string& sector) {
    int nodeID = addPublicFacility(name, FacilityType::POLICE_STATION, sector);
    if (nodeID != -1) {
        nodes[nodeID]->operatingHours = "24/7";
        nodes[nodeID]->additionalInfo = "Emergency: 15";
    }
    return nodeID;
}

inline int CityGraph::addFireStation(const string& name, const string& sector) {
    int nodeID = addPublicFacility(name, FacilityType::FIRE_STATION, sector);
    if (nodeID != -1) {
        nodes[nodeID]->operatingHours = "24/7";
        nodes[nodeID]->additionalInfo = "Emergency: 16";
    }
    return nodeID;
}

inline int CityGraph::addPetrolStation(const string& name, const string& sector, bool is24Hours) {
    int nodeID = addPublicFacility(name, FacilityType::PETROL_STATION, sector);
    if (nodeID != -1) {
        nodes[nodeID]->operatingHours = is24Hours ? "24/7" : "06:00-22:00";
    }
    return nodeID;
}

inline int CityGraph::addATM(const string& name, const string& sector, const string& bankName) {
    int nodeID = addPublicFacility(name, FacilityType::ATM, sector);
    if (nodeID != -1) {
        nodes[nodeID]->operatingHours = "24/7";
        nodes[nodeID]->additionalInfo = bankName;
    }
    return nodeID;
}

inline int CityGraph::addRestaurant(const string& name, const string& sector, const string& cuisine) {
    int nodeID = addPublicFacility(name, FacilityType::RESTAURANT, sector);
    if (nodeID != -1) {
        nodes[nodeID]->operatingHours = "11:00-23:00";
        nodes[nodeID]->additionalInfo = cuisine;
    }
    return nodeID;
}

inline int CityGraph::addPublicToilet(const string& name, const string& sector) {
    int nodeID = addPublicFacility(name, FacilityType::PUBLIC_TOILET, sector);
    if (nodeID != -1) {
        nodes[nodeID]->operatingHours = "24/7";
    }
    return nodeID;
}

inline Vector<int> CityGraph::getFacilitiesInSector(const string& sector, const string& type) {
    Vector<int> results;
    for (int i = 0; i < nodeCount; i++) {
        if (nodes[i] && nodes[i]->sector == sector && nodes[i]->type != FacilityType::CORNER) {
            if (type.empty() || nodes[i]->type == type) {
                results.push_back(i);
            }
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

inline void CityGraph::loadPublicFacilitiesCSV(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) return;
    
    string line;
    getline(file, line);
    
    while (getline(file, line)) {
        if (line.empty()) continue;
        
        string name = "", type = "", sector = "", hours = "", accessible = "", info = "";
        int i = 0;
        bool inQuotes = false;
        
        while (i < (int)line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            name += c;
        }
        
        inQuotes = false;
        while (i < (int)line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            type += c;
        }
        
        inQuotes = false;
        while (i < (int)line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            sector += c;
        }
        
        inQuotes = false;
        while (i < (int)line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            hours += c;
        }
        
        inQuotes = false;
        while (i < (int)line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            accessible += c;
        }
        
        inQuotes = false;
        while (i < (int)line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == '\r' || c == '\n') break;
            info += c;
        }
        
        if (!name.empty() && !type.empty() && !sector.empty()) {
            int nodeID = addPublicFacility(name, type, sector);
            if (nodeID != -1) {
                if (!hours.empty()) nodes[nodeID]->operatingHours = hours;
                nodes[nodeID]->isAccessible = (accessible != "false" && accessible != "0" && accessible != "no");
                if (!info.empty()) nodes[nodeID]->additionalInfo = info;
            }
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
    if (id1 == id2) return;

    double dist = GeometryUtils::getGridDistance(nodes[id1]->lat, nodes[id1]->lon, nodes[id2]->lat, nodes[id2]->lon);

    Edge e1(id2, dist);
    nodes[id1]->roads.push_back(e1);

    Edge e2(id1, dist);
    nodes[id2]->roads.push_back(e2);
}

/*------ Stop Loader ------*/
inline void CityGraph::loadStopsCSV(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) return;

    string line;
    getline(file, line);

    while (getline(file, line)) {
        if (line.empty()) continue;

        string databaseID = "", name = "", sector = "";
        int i = 0;

        while (i < (int)line.size() && line[i] != ',') databaseID += line[i++];
        if (i >= (int)line.size()) continue;
        i++;
        
        while (i < (int)line.size() && line[i] != ',') name += line[i++];
        if (i >= (int)line.size()) continue;
        i++;
        
        while (i < (int)line.size()) {
            if (line[i] != ' ' && line[i] != '\t' && line[i] != '\r' && line[i] != '\n') {
                sector += line[i];
            }
            i++;
        }

        if (databaseID.empty() || name.empty() || sector.empty()) continue;

        double lat = 0.0, lon = 0.0;
        GeometryUtils::generateCoords(sector, lat, lon);
        addLocation(databaseID, databaseID, name, FacilityType::STOP, lat, lon);
    }
    file.close();
}

/*------ Generic Building Loader ------*/
inline void CityGraph::loadBuildingsCSV(const string& filename, string type) {
    ifstream file(filename);
    if (!file.is_open()) return;

    string line;
    getline(file, line);

    while (getline(file, line)) {
        if (line.empty()) continue;

        string databaseID = "", name = "", sector = "";
        int i = 0;
        bool inQuotes = false;

        while (i < (int)line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            databaseID += c;
        }

        inQuotes = false;
        while (i < (int)line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            name += c;
        }

        inQuotes = false;
        while (i < (int)line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            if (c != '\r' && c != '\n' && c != ' ' && c != '\t') sector += c;
        }

        if (sector.empty()) continue;

        double lat = 0.0, lon = 0.0;
        GeometryUtils::generateCoords(sector, lat, lon);
        addLocation(databaseID, "", name, type, lat, lon);
    }
    file.close();
}

/*------ Dijkstra Pathfinding ------*/
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
    double dist;
    findShortestPath(startID, endID, dist);
}

/*------ Find Nearest Facility ------*/
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
        
        // Found a facility of the target type
        if (u != fromNodeID && nodes[u] != nullptr && nodes[u]->type == facilityType) {
            return u;
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
    
    return -1;
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
    minLat = BASE_LAT;
    maxLat = MAX_LAT;
    minLon = BASE_LON;
    maxLon = MAX_LON;
}

































































