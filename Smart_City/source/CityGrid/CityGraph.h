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

    // Modified to build Visual Grid Frame
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

    // IMPORTANT: inputLat/inputLon are REAL WORLD coords from CSV/Generator
    // They will be mapped to VISUAL GRID coords inside this function
    int addLocation(string databaseID, string stopID, string name, string type, double inputLat, double inputLon);

    void addRoad(int id1, int id2);

    // Pathfinding
    Vector<int> findShortestPath(int startID, int endID, double& totalDistance);
    void findShortestPathOld(int startID, int endID);
    void getBounds(double& minLat, double& maxLat, double& minLon, double& maxLon);
    void resetVisuals(); // Placeholder if needed

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

/*------ Helper: Initialize Sector Frame (VISUAL GRID) ------*/
inline void CityGraph::initializeSectorFrame(string sectorName) {
    int idx = GeometryUtils::getSectorIndex(sectorName);

    // Safety check
    if (idx == -1) return;
    if (SECTOR_GRID[idx].initialized) return;

    // Get VISUAL GRID coordinates for the Top-Left of the sector box
    double gridX, gridY;
    if (!GridUtils::getSectorGridPos(sectorName, gridX, gridY)) return;

    double w = GridUtils::CELL_WIDTH;
    double h = GridUtils::CELL_HEIGHT;

    // Create 4 corners of the sector rectangle in VISUAL SPACE
    // Naming: NW (Top-Left), NE (Top-Right), SE (Bottom-Right), SW (Bottom-Left)

    // Top-Left (NW) - (gridX, gridY)
    int cNW = addLocation("C-" + sectorName + "-NW", "", sectorName + " NW", FacilityType::CORNER, gridX, gridY);

    // Top-Right (NE) - (gridX + w, gridY)
    int cNE = addLocation("C-" + sectorName + "-NE", "", sectorName + " NE", FacilityType::CORNER, gridX + w, gridY);

    // Bottom-Right (SE) - (gridX + w, gridY + h)
    int cSE = addLocation("C-" + sectorName + "-SE", "", sectorName + " SE", FacilityType::CORNER, gridX + w, gridY + h);

    // Bottom-Left (SW) - (gridX, gridY + h)
    int cSW = addLocation("C-" + sectorName + "-SW", "", sectorName + " SW", FacilityType::CORNER, gridX, gridY + h);

    // Connect corners to form a rectangle (perimeter roads)
    addRoad(cNW, cNE); // Top
    addRoad(cNE, cSE); // Right
    addRoad(cSE, cSW); // Bottom
    addRoad(cSW, cNW); // Left

    SECTOR_GRID[idx].initialized = true;
}

/*------ Add Location Logic (Input -> Visual Grid Mapping) ------*/
inline int CityGraph::addLocation(string databaseID, string stopID, string name, string type, double inputLat, double inputLon) {
    if (nodeCount >= MAX_NODES) {
        return -1;
    }

    // 1. Resolve Sector from REAL WORLD coords (inputLat, inputLon)
    //    We rely on GeometryUtils logic which checks the real geographic bounds.
    string sector = GeometryUtils::resolveSector(inputLat, inputLon);

    // Fallback: Try to parse sector from name if coords fail (e.g. "PIMS G-8")
    if (sector == "Unknown Sector") {
        if (name.find("G-8") != string::npos) sector = "G-8";
        else if (name.find("E-11") != string::npos) sector = "E-11";
        // Basic heuristics can be added here
    }

    // 2. Initialize the visual frame if this is a new sector
    if (type != FacilityType::CORNER && sector != "Unknown" && sector != "Unknown Sector") {
        int sIdx = GeometryUtils::getSectorIndex(sector);
        if (sIdx != -1 && !SECTOR_GRID[sIdx].initialized) {
            initializeSectorFrame(sector);
        }
    }

    // 3. Determine Final Visual Coordinates (finalX, finalY)
    double finalX, finalY;

    if (type == FacilityType::CORNER) {
        // For CORNER nodes, the inputs ARE ALREADY the Grid X/Y passed by initializeSectorFrame.
        // We use them directly.
        finalX = inputLat;
        finalY = inputLon;
    }
    else {
        // For all other nodes, we DISCARD the inputLat/inputLon (Real GPS).
        // We calculate a position inside the grid cell.
        double gridX, gridY;
        if (GridUtils::getSectorGridPos(sector, gridX, gridY)) {
            // Randomize position inside the sector box to prevent overlap
            int margin = 15;
            int randX = rand() % (int)(GridUtils::CELL_WIDTH - 2 * margin);
            int randY = rand() % (int)(GridUtils::CELL_HEIGHT - 2 * margin);
            finalX = gridX + margin + randX;
            finalY = gridY + margin + randY;
        }
        else {
            // Fallback for unknown sectors (off-grid)
            finalX = 0;
            finalY = 0;
        }
    }

    // 4. Create Node
    // Storing VISUAL X in 'lat' and VISUAL Y in 'lon' members of CityNode
    int newID = nodeCount;
    nodes[newID] = new CityNode(newID, databaseID, stopID, name, type, finalX, finalY);
    nodes[newID]->sector = sector; // Force correct sector assignment
    nodeCount++;

    // 5. Connect to sector corners (Visualization logic)
    // Connect standard nodes to the nearest corner or just to the frame to visually anchor them
    if (type != FacilityType::CORNER && sector != "Unknown" && sector != "Unknown Sector") {
        for (int i = 0; i < nodeCount - 1; i++) {
            if (nodes[i]->sector == sector && nodes[i]->type == FacilityType::CORNER) {
                // Connect to corners. We just add road to the first one found or all.
                // Adding to all corners makes a "star" pattern. 
                // Adding to just one links it to the grid. 
                // Let's check distance to find the "visually" nearest corner in the grid cell
                // Or just randomly connect to one for simplicity.
                addRoad(newID, i);
                break; // Just connect to one corner to keep graph clean
            }
        }
    }

    return newID;
}

/*------ Add Road (Euclidean Distance for Visual Grid) ------*/
inline void CityGraph::addRoad(int id1, int id2) {
    if (id1 < 0 || id2 < 0 || id1 >= nodeCount || id2 >= nodeCount) return;

    // Use Euclidean distance because lat/lon are now Pixel X/Y
    double dx = nodes[id1]->lat - nodes[id2]->lat;
    double dy = nodes[id1]->lon - nodes[id2]->lon;
    double dist = std::sqrt(dx * dx + dy * dy);

    Edge e1(id2, dist);
    nodes[id1]->roads.push_back(e1);

    Edge e2(id1, dist);
    nodes[id2]->roads.push_back(e2);
}

/*------ Algorithms (Dijkstra) ------*/
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
    // Deprecated wrapper
    double d;
    findShortestPath(startID, endID, d);
}

inline void CityGraph::getBounds(double& minLat, double& maxLat, double& minLon, double& maxLon) {
    // Return bounds of the visual coordinates
    minLat = 99999.0; maxLat = -99999.0;
    minLon = 99999.0; maxLon = -99999.0;

    for (int i = 0; i < nodeCount; i++) {
        if (nodes[i] != nullptr) {
            if (nodes[i]->lat < minLat) minLat = nodes[i]->lat;
            if (nodes[i]->lat > maxLat) maxLat = nodes[i]->lat;
            if (nodes[i]->lon < minLon) minLon = nodes[i]->lon;
            if (nodes[i]->lon > maxLon) maxLon = nodes[i]->lon;
        }
    }
}

inline void CityGraph::resetVisuals() {
    // Optional: Reset logic if needed
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

/*------ Stop Loader (Sector-Based Visual Mapping) ------*/
inline void CityGraph::loadStopsCSV(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) return;

    string line;
    getline(file, line); // Skip header

    while (getline(file, line)) {
        if (line.empty()) continue;

        string databaseID = "", name = "", sector = "";
        int i = 0;

        // Parse CSV manually to avoid dependency issues
        while (i < (int)line.size() && line[i] != ',') databaseID += line[i++];
        if (i >= (int)line.size()) continue;
        i++;

        while (i < (int)line.size() && line[i] != ',') name += line[i++];
        if (i >= (int)line.size()) continue;
        i++;

        while (i < (int)line.size() && (line[i] == ' ' || line[i] == '\t')) i++;
        while (i < (int)line.size() && line[i] != '\r' && line[i] != '\n') sector += line[i++];
        while (!sector.empty() && (sector.back() == ' ' || sector.back() == '\t')) sector.pop_back();

        if (databaseID.empty() || name.empty() || sector.empty()) continue;

        // Generate REAL WORLD coordinates first to identify sector boundaries
        // These will be passed to addLocation, which will then Map them to the VISUAL GRID
        double lat = 0.0, lon = 0.0;
        GeometryUtils::generateCoords(sector, lat, lon);

        addLocation(databaseID, databaseID, name, FacilityType::STOP, lat, lon);
    }
    file.close();
}

/*------ Building Loader (Sector-Based Visual Mapping) ------*/
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
            if (c != '\r' && c != '\n') sector += c;
        }

        while (!sector.empty() && (sector.front() == ' ' || sector.front() == '\t')) sector = sector.substr(1);
        while (!sector.empty() && (sector.back() == ' ' || sector.back() == '\t' || sector.back() == '\r')) sector.pop_back();

        if (sector.empty()) continue;

        // Generate REAL WORLD coordinates first
        double lat = 0.0, lon = 0.0;
        GeometryUtils::generateCoords(sector, lat, lon);

        // Map to VISUAL GRID in addLocation
        addLocation(databaseID, "", name, type, lat, lon);
    }
    file.close();
}

/*------ Public Facility Management ------*/
inline int CityGraph::addPublicFacility(const string& name, const string& type, const string& sector,
    const string& operatingHours, bool isAccessible,
    const string& additionalInfo) {
    double lat = 0.0, lon = 0.0;
    GeometryUtils::generateCoords(sector, lat, lon);
    return addPublicFacilityWithCoords(name, type, lat, lon, operatingHours, isAccessible, additionalInfo);
}

inline int CityGraph::addPublicFacilityWithCoords(const string& name, const string& type,
    double lat, double lon,
    const string& operatingHours,
    bool isAccessible,
    const string& additionalInfo) {
    string stopID = generateStopID(type);
    string databaseID = stopID;

    // addLocation maps the lat/lon (real) to Grid X/Y (visual)
    int nodeID = addLocation(databaseID, stopID, name, type, lat, lon);

    if (nodeID != -1) {
        nodes[nodeID]->operatingHours = operatingHours;
        nodes[nodeID]->isAccessible = isAccessible;
        nodes[nodeID]->additionalInfo = additionalInfo;
    }
    return nodeID;
}

// Convenience methods
inline int CityGraph::addMosque(const string& name, const string& sector, const string& prayerTimes) {
    return addPublicFacility(name, FacilityType::MOSQUE, sector, prayerTimes, true, "");
}
inline int CityGraph::addPark(const string& name, const string& sector, const string& hours) {
    return addPublicFacility(name, FacilityType::PARK, sector, hours, true, "");
}
inline int CityGraph::addWaterCooler(const string& name, const string& sector) {
    return addPublicFacility(name, FacilityType::WATER_COOLER, sector, "24/7", true, "");
}
inline int CityGraph::addPlayground(const string& name, const string& sector) {
    return addPublicFacility(name, FacilityType::PLAYGROUND, sector, "06:00-20:00", true, "");
}
inline int CityGraph::addLibrary(const string& name, const string& sector, const string& hours) {
    return addPublicFacility(name, FacilityType::LIBRARY, sector, hours, true, "");
}
inline int CityGraph::addPoliceStation(const string& name, const string& sector) {
    return addPublicFacility(name, FacilityType::POLICE_STATION, sector, "24/7", true, "");
}
inline int CityGraph::addFireStation(const string& name, const string& sector) {
    return addPublicFacility(name, FacilityType::FIRE_STATION, sector, "24/7", true, "");
}
inline int CityGraph::addPetrolStation(const string& name, const string& sector, bool is24Hours) {
    return addPublicFacility(name, FacilityType::PETROL_STATION, sector, is24Hours ? "24/7" : "06:00-22:00", true, "");
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
                if (nodes[i]->isPublicFacility()) results.push_back(i);
            }
            else {
                if (nodes[i]->type == type) results.push_back(i);
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
    // Placeholder implementation as logic mirrors other load functions
    // Uses addPublicFacility which handles the grid mapping
}