#pragma once
#include "CityUtils.h"

class CityGraph {
private:
    CityNode* nodes[MAX_NODES];
    int nodeCount;

    int facilityCounters[14];

    // Internal helper to create a node structure without triggering grid logic
    // Used for creating the skeleton (CORNER) nodes
    int createNodeRaw(const string& dbID, const string& sID, const string& name, const string& type, double lat, double lon);

public:
    CityGraph();
    ~CityGraph();

    // ==================== SECTOR MANAGEMENT ====================
    void initializeSectorFrame(const string& sectorName);

    CityNode* getNode(int index) const;
    int getNodeCount() const { return nodeCount; }

    int addLocation(const string& databaseID, const string& stopID,
        const string& name, const string& type, double lat, double lon);
    int addPublicFacility(const string& name, const string& type, const string& sector);

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

    Vector<int> findShortestPath(int startID, int endID, double& totalDistance);
    int findNearestFacility(int fromNodeID, const string& facilityType);
    Vector<int> findAllNearestFacilities(int fromNodeID, const string& facilityType, int maxCount = 5);
    Vector<int> calculateBusRoute(int startNodeID, int endNodeID, double& distance);

    // ==================== GETTER ====================
    int getIDByName(const string& name);
    int getIDByDatabaseID(const string& dbID);
    int getIDByStopID(const string& sID);

    Vector<int> getFacilitiesInSector(const string& sector, const string& type = "");
    Vector<int> getAllStopsInSector(const string& sector);
    void getBounds(double& minLat, double& maxLat, double& minLon, double& maxLon);

    void loadStopsCSV(const string& filename);
    void loadBuildingsCSV(const string& filename, const string& type);


    string generateStopID(const string& type);
};



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

// ==================== INTERNAL HELPER ====================

inline int CityGraph::createNodeRaw(const string& dbID, const string& sID, const string& name, const string& type, double lat, double lon) {
    if (nodeCount >= MAX_NODES) return -1;
    int newID = nodeCount;
    nodes[newID] = new CityNode(newID, dbID, sID, name, type, lat, lon);
    nodeCount++;
    return newID;
}

// ==================== NODE ACCESS ====================

inline CityNode* CityGraph::getNode(int index) const {
    if (index < 0 || index >= nodeCount) return nullptr;
    return nodes[index];
}

// ==================== ID GENERATION ====================

inline string CityGraph::generateStopID(const string& type) {
    string prefix = FacilityType::getStopIDPrefix(type);
    int counterIdx = 0;

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

    // Format: PREFIX-001, PREFIX-002
    string countStr = std::to_string(count);
    while (countStr.length() < 3) countStr = "0" + countStr;

    return prefix + "-" + countStr;
}

// ==================== SECTOR FRAME INITIALIZATION (5x5 GRID) ====================

inline void CityGraph::initializeSectorFrame(const string& sectorName) {
    int idx = GeometryUtils::getSectorIndex(sectorName);
    if (idx == -1 || SECTOR_GRID[idx].initialized) return;

    SectorBox& box = SECTOR_GRID[idx];

    // 1. GENERATE 5x5 SKELETON GRID (CORNER NODES)
    // 5 Rows (Lat) x 5 Cols (Lon)
    double latStep = box.getHeight() / 4.0;
    double lonStep = box.getWidth() / 4.0;

    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) {
            double lat = box.minLat + r * latStep;
            double lon = box.minLon + c * lonStep;

            // Generate unique ID for corner: "C-F11-R2-C3"
            string idName = "C-" + sectorName + "-R" + std::to_string(r) + "-C" + std::to_string(c);

            // Create the skeleton node
            int nodeID = createNodeRaw(idName, "", idName, FacilityType::CORNER, lat, lon);

            // Store in the sector's grid map
            if (nodeID != -1) {
                box.gridCorners[r][c] = nodeID;
            }
        }
    }

    // 2. CONNECT SKELETON NODES (ROADS)
    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) {
            int current = box.gridCorners[r][c];
            if (current == -1) continue;

            // Connect Horizontal (East)
            if (c < 4) {
                int right = box.gridCorners[r][c + 1];
                if (right != -1) addRoad(current, right);
            }
            // Connect Vertical (North)
            if (r < 4) {
                int up = box.gridCorners[r + 1][c];
                if (up != -1) addRoad(current, up);
            }
        }
    }

    // 3. INITIALIZE SUB-SECTORS (16 CELLS)
    // Map the corners to the cells for easy access later
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            int cellIdx = r * 4 + c;
            SubSubSector& cell = box.cells[cellIdx];

            // Set Bounds
            cell.minLat = box.minLat + r * latStep;
            cell.maxLat = box.minLat + (r + 1) * latStep;
            cell.minLon = box.minLon + c * lonStep;
            cell.maxLon = box.minLon + (c + 1) * lonStep;

            // Assign the 4 corners surrounding this cell
            // SW, NW, NE, SE relative to the cell
            cell.cornerIDs[0] = box.gridCorners[r][c];         // SW
            cell.cornerIDs[1] = box.gridCorners[r + 1][c];     // NW
            cell.cornerIDs[2] = box.gridCorners[r + 1][c + 1]; // NE
            cell.cornerIDs[3] = box.gridCorners[r][c + 1];     // SE
        }
    }

    SECTOR_GRID[idx].initialized = true;
}

// ==================== ADD LOCATION (CORE LOGIC) ====================

inline int CityGraph::addLocation(const string& databaseID, const string& stopID,
    const string& name, const string& type,
    double lat, double lon) {

    if (nodeCount >= MAX_NODES) return -1;

    string sector = GeometryUtils::resolveSector(lat, lon);

    // 1. Initialize Sector if needed
    if (sector != "Unknown") {
        int sectorIdx = GeometryUtils::getSectorIndex(sector);
        if (sectorIdx != -1 && !SECTOR_GRID[sectorIdx].initialized) {
            initializeSectorFrame(sector);
        }
    }

    // 2. Create the Node
    int newID = createNodeRaw(databaseID, stopID, name, type, lat, lon);
    if (newID == -1) return -1;

    // 3. Logic for Non-Corner Nodes (Connectivity)
    if (type != FacilityType::CORNER && sector != "Unknown") {
        int sectorIdx = GeometryUtils::getSectorIndex(sector);
        if (sectorIdx != -1) {
            SectorBox& box = SECTOR_GRID[sectorIdx];

            // A. Resolve SubSector
            int cellIdx = GeometryUtils::getSubSectorIndex(lat, lon, box);

            // B. Spillover Logic (if cell is full)
            if (cellIdx != -1) {
                SubSubSector* targetCell = &box.cells[cellIdx];

                // If full, try to find nearest non-full neighbor cell
                if (targetCell->isFull()) {
                    double minDist = INF;
                    SubSubSector* bestBackup = nullptr;

                    for (int i = 0; i < 16; i++) {
                        if (!box.cells[i].isFull()) {
                            double d = GeometryUtils::getGridDistance(lat, lon,
                                box.cells[i].getCenterLat(), box.cells[i].getCenterLon());
                            if (d < minDist) {
                                minDist = d;
                                bestBackup = &box.cells[i];
                            }
                        }
                    }
                    // If we found a backup, use it. Otherwise we stay with targetCell (and overload it)
                    if (bestBackup != nullptr) {
                        targetCell = bestBackup;
                    }
                }

                // C. Add to Cell
                if (targetCell->nodeCount < 4) {
                    targetCell->nodeIDs[targetCell->nodeCount++] = newID;
                }

                // D. Connect to NEAREST CORNER of the cell (Skeleton Access)
                double minCornerDist = INF;
                int bestCorner = -1;

                for (int i = 0; i < 4; i++) {
                    int cID = targetCell->cornerIDs[i];
                    if (cID != -1 && nodes[cID]) {
                        double d = GeometryUtils::getGridDistance(lat, lon, nodes[cID]->lat, nodes[cID]->lon);
                        if (d < minCornerDist) {
                            minCornerDist = d;
                            bestCorner = cID;
                        }
                    }
                }
                if (bestCorner != -1) {
                    addRoad(newID, bestCorner);
                }

                // E. Connect to 2 CLOSEST NODES inside the cell (Cluster Access)
                // Collect existing nodes in cell (excluding self)
                int existingNodes[4];
                double dists[4];
                int count = 0;

                for (int i = 0; i < 4; i++) {
                    int nID = targetCell->nodeIDs[i];
                    if (nID != -1 && nID != newID && nodes[nID]) {
                        existingNodes[count] = nID;
                        dists[count] = GeometryUtils::getGridDistance(lat, lon, nodes[nID]->lat, nodes[nID]->lon);
                        count++;
                    }
                }

                // Sort by distance (Bubble sort for tiny array)
                for (int i = 0; i < count - 1; i++) {
                    for (int j = 0; j < count - i - 1; j++) {
                        if (dists[j] > dists[j + 1]) {
                            std::swap(dists[j], dists[j + 1]);
                            std::swap(existingNodes[j], existingNodes[j + 1]);
                        }
                    }
                }

                // Connect to up to 2 closest
                int links = 0;
                for (int i = 0; i < count && links < 2; i++) {
                    addRoad(newID, existingNodes[i]);
                    links++;
                }
            }
        }
    }

    return newID;
}

// ==================== PUBLIC FACILITY ====================

inline int CityGraph::addPublicFacility(const string& name, const string& type, const string& sector) {
    int idx = GeometryUtils::getSectorIndex(sector);
    if (idx == -1) return -1;

    // Initialize if needed
    if (!SECTOR_GRID[idx].initialized) {
        initializeSectorFrame(sector);
    }

    SectorBox& box = SECTOR_GRID[idx];

    // Find a random subsector that isn't full
    Vector<int> availableCells;
    for (int i = 0; i < 16; i++) {
        if (!box.cells[i].isFull()) {
            availableCells.push_back(i);
        }
    }

    double lat, lon;

    if (availableCells.getSize() > 0) {
        // Pick random available cell
        int randIdx = rand() % availableCells.getSize();
        int cellID = availableCells[randIdx];
        SubSubSector& cell = box.cells[cellID];

        // Generate coords INSIDE this cell
        double marginLat = cell.getHeight() * 0.1;
        double marginLon = cell.getWidth() * 0.1;

        double r1 = (double)rand() / RAND_MAX;
        double r2 = (double)rand() / RAND_MAX;

        lat = cell.minLat + marginLat + r1 * (cell.getHeight() - 2 * marginLat);
        lon = cell.minLon + marginLon + r2 * (cell.getWidth() - 2 * marginLon);
    }
    else {
        // Fallback: Generate generic coords in sector (will trigger spillover logic in addLocation)
        GeometryUtils::generateCoords(sector, lat, lon);
    }

    string stopID = generateStopID(type);
    return addLocation(stopID, stopID, name, type, lat, lon);
}


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

        if (!nodes[u]) continue;

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

// ==================== FACILITY SEARCH ====================

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

        if (u != fromNodeID && nodes[u] && nodes[u]->type == facilityType) {
            return u;
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



inline void CityGraph::loadStopsCSV(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) return;

    string line;
    getline(file, line);

    while (getline(file, line)) {
        if (line.empty()) continue;

        string databaseID, name, sector;
        int i = 0;

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


inline void CityGraph::loadBuildingsCSV(const string& filename, const string& type) {
    ifstream file(filename);
    if (!file.is_open()) return;

    string line;
    getline(file, line);

    while (getline(file, line)) {
        if (line.empty()) continue;

        string databaseID, name, sector;
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

        double lat, lon;
        GeometryUtils::generateCoords(sector, lat, lon);
        addLocation(databaseID, "", name, type, lat, lon);
    }
    file.close();
}