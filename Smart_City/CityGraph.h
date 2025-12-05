#pragma once
#include "CityUtils.h" 

class CityGraph {
private:
    CityNode* nodes[MAX_NODES];
    int nodeCount;

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


};

/*------ Constructors and Destructors ------*/
inline CityGraph::CityGraph() {
    nodeCount = 0;
    for (int i = 0; i < MAX_NODES; i++) nodes[i] = nullptr;
}

inline CityGraph::~CityGraph() {
    for (int i = 0; i < nodeCount; i++) {
        if (nodes[i] != nullptr) delete nodes[i];
    }
}

/*------ Helper: Initialize Sector Frame ------*/
inline void CityGraph::initializeSectorFrame(string sectorName) {
    int idx = GeometryUtils::getSectorIndex(sectorName);

    //Safety check
    if (idx == -1) return;
    if (SECTOR_GRID[idx].initialized) return;

    SectorBox box = SECTOR_GRID[idx];

    // For corner nodes: databaseID and stopID are the same (corner identifier)
    // stopID is empty since corners are not bus stops
    int c1 = nodeCount; addLocation("C-" + sectorName + "-1", "", sectorName + " Corner 1", "CORNER", box.minLat, box.minLon);
    int c2 = nodeCount; addLocation("C-" + sectorName + "-2", "", sectorName + " Corner 2", "CORNER", box.maxLat, box.minLon);
    int c3 = nodeCount; addLocation("C-" + sectorName + "-3", "", sectorName + " Corner 3", "CORNER", box.maxLat, box.maxLon);
    int c4 = nodeCount; addLocation("C-" + sectorName + "-4", "", sectorName + " Corner 4", "CORNER", box.minLat, box.maxLon);

    addRoad(c1, c2);
    addRoad(c2, c3);
    addRoad(c3, c4);
    addRoad(c4, c1);

    SECTOR_GRID[idx].initialized = true;
}

/*------ Add Node Logic ------*/
inline int CityGraph::addLocation(string databaseID, string stopID, string name, string type, double lat, double lon) {
    if (nodeCount >= MAX_NODES) {
        return -1;
    }

    string sector = GeometryUtils::resolveSector(lat, lon);

    //1. Check if we need to build the frame FIRST
    if (type != "CORNER" && sector != "Unknown" && !SECTOR_GRID[GeometryUtils::getSectorIndex(sector)].initialized) {
        initializeSectorFrame(sector);
    }

    //2. Create Node
    int newID = nodeCount;
    nodes[newID] = new CityNode(newID, databaseID, stopID, name, type, lat, lon);
    nodeCount++;

    //3. Anchor Logic: Connect to nearest frame corner
    if (type != "CORNER" && sector != "Unknown") {
        int nearestCorner = -1;
        double minDst = 99999.0;

        for (int i = 0; i < nodeCount - 1; i++) {
            if (nodes[i]->sector == sector && nodes[i]->type == "CORNER") {
                double d = GeometryUtils::getHaversineDistance(lat, lon, nodes[i]->lat, nodes[i]->lon);
                if (d < minDst) {
                    minDst = d;
                    nearestCorner = i;
                }
            }
        }
        if (nearestCorner != -1) {
            addRoad(newID, nearestCorner);
        }
    }

    return newID;
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

/*------ Generic Building Loader ------*/
//Loads CSVs like schools.csv, hospitals.csv that LACK coordinates.
//It generates coordinates based on the 'Sector' column.
inline void CityGraph::loadBuildingsCSV(const string& filename, string type) {
    ifstream file(filename);
    if (!file.is_open()) {
        return;
    }

    string line;
    getline(file, line); //Skip header

    int successCount = 0;

    while (getline(file, line)) {
        if (line.empty()) continue;

        string databaseID = "";  // Changed from idStr
        string name = "";
        string sector = "";
        int i = 0;

        //Manual Parsing to handle quotes
        //1. Database ID (Column 0)
        bool inQuotes = false;
        while (i < line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            databaseID += c;
        }

        //2. Name (Column 1)
        inQuotes = false;
        while (i < line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            name += c;
        }

        //3. Sector (Column 2)
        inQuotes = false;
        while (i < line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            sector += c;
        }

        //Validate Sector
        if (sector.empty()) continue;

        //Generate Coordinates!
        double lat = 0.0, lon = 0.0;
        GeometryUtils::generateCoords(sector, lat, lon);

        //Add to Graph (databaseID, empty stopID, name, type, lat, lon)
        if (addLocation(databaseID, "", name, type, lat, lon) != -1) {
            successCount++;
        }
    }
    file.close();
}

/*------ Stop Loader (With Coords) ------*/
inline void CityGraph::loadStopsCSV(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        return;
    }

    string line;
    getline(file, line); //Skip header

    while (getline(file, line)) {
        if (line.empty()) continue;

        string databaseID = "", stopID = "", name = "", lat_str = "", lon_str = "";
        int i = 0;

        //DatabaseID (same as StopID for stops)
        while (i < line.size() && line[i] != ',') databaseID += line[i++];
        if (i >= line.size()) continue;  //Malformed line
        i++;
        
        //StopID (for transport system - same as databaseID for stops)
        stopID = databaseID;
        
        //Name
        while (i < line.size() && line[i] != ',') name += line[i++];
        if (i >= line.size()) continue;  //Malformed line
        i++;
        
        //Coordinates (Skip quotes manually)
        while (i < line.size() && (line[i] == '"' || line[i] == ' ')) i++; //Skip start quote/space
        while (i < line.size() && line[i] != ',') lat_str += line[i++];
        if (i >= line.size()) continue;  //Malformed line
        i++;
        
        while (i < line.size() && (line[i] == '"' || line[i] == ' ')) i++;
        while (i < line.size() && line[i] != '"' && line[i] != '\r' && line[i] != '\n') lon_str += line[i++];

        //Validate required fields
        if (databaseID.empty() || name.empty() || lat_str.empty() || lon_str.empty()) {
            continue;  //Skip invalid entries
        }

        try {
            double lat = stod(lat_str);
            double lon = stod(lon_str);
            // For stops: databaseID and stopID are the same
            addLocation(databaseID, stopID, name, "STOP", lat, lon);
        }
        catch (...) {
            //Handle parsing errors silently or log
            continue;
        }
    }
    file.close();
}

/*------ Algorithms ------*/
// New version that returns the path
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

        for (size_t i = 0; i < nodes[u]->roads.size(); i++) {
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
    
    // Reconstruct path
    if (parent[endID] != -1 || startID == endID) {
        int current = endID;
        while (current != -1) {
            path.push_back(current);
            current = parent[current];
        }
        
        // Reverse path to get start->end order
        for (int i = 0; i < path.getSize() / 2; i++) {
            int temp = path[i];
            path[i] = path[path.getSize() - 1 - i];
            path[path.getSize() - 1 - i] = temp;
        }
        
        totalDistance = distance[endID];
    }
    
    return path;
}

// Old version for compatibility
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

        for (size_t i = 0; i < nodes[u]->roads.size(); i++) {
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
        
        // Check if this node is the facility type we're looking for
        if (u != fromNodeID && nodes[u] != nullptr && nodes[u]->type == facilityType) {
            if (distance[u] < minDistance) {
                minDistance = distance[u];
                nearestFacility = u;
            }
            // Found the nearest, can stop early
            break;
        }
        
        if (u < 0 || u >= nodeCount || nodes[u] == nullptr) continue;
        
        for (size_t i = 0; i < nodes[u]->roads.size(); i++) {
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
        
        // Check if this node is the facility type we're looking for
        if (u != fromNodeID && nodes[u] != nullptr && nodes[u]->type == facilityType) {
            results.push_back(u);
        }
        
        if (u < 0 || u >= nodeCount || nodes[u] == nullptr) continue;
        
        for (size_t i = 0; i < nodes[u]->roads.size(); i++) {
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



