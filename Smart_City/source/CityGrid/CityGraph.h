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

    // Snaps a node position to a logical place within its SubSubSector cell
    void snapNodePosition(const SubSubSector* cell, int nodeIndex, double& lat, double& lon);

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
    void addRoad(int id1, int id2, int capacity);  // With custom capacity
    void addFacilityRoad(int id1, int id2);        // Adds road with weight penalty
    void removeRoad(int id1, int id2);
    bool hasRoad(int id1, int id2) const;
    Edge* getEdge(int fromNode, int toNode);
    const Edge* getEdge(int fromNode, int toNode) const;

    // ==================== TRAFFIC MANAGEMENT ====================
    // Try to enter a road segment. Returns true if successful, false if road is full.
    bool tryEnterEdge(int fromNode, int toNode);
    
    // Leave a road segment, decreasing its load
    void leaveEdge(int fromNode, int toNode);
    
    // Update all dynamic weights based on current traffic loads
    void updateTrafficWeights();
    
    // Get traffic statistics
    double getEdgeCongestion(int fromNode, int toNode) const;
    int getTotalVehiclesOnRoads() const;

    // ==================== PATHFINDING ====================
    Vector<int> findShortestPath(int startID, int endID, double& totalDistance);
    Vector<int> findShortestPathDynamic(int startID, int endID, double& totalDistance);  // Uses dynamicWeight
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
    // Safety check: if sector doesn't exist or is already initialized, do nothing
    if (idx == -1 || SECTOR_GRID[idx].initialized) return;

    SectorBox& box = SECTOR_GRID[idx];

    // =========================================================
    // 1. GENERATE 5x5 SKELETON GRID (CORNER NODES)
    // =========================================================
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

    // =========================================================
    // 2. CONNECT SKELETON NODES INTERNALLY (ROADS)
    // Boundary roads (r=0,4 or c=0,4) get highway capacity
    // Internal roads get default capacity
    // =========================================================
    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) {
            int current = box.gridCorners[r][c];
            if (current == -1) continue;

            // Connect Horizontal (East)
            if (c < 4) {
                int right = box.gridCorners[r][c + 1];
                if (right != -1) {
                    // Boundary roads (top or bottom edge) are highways
                    bool isBoundary = (r == 0 || r == 4);
                    int capacity = isBoundary ? HIGHWAY_ROAD_CAPACITY : DEFAULT_ROAD_CAPACITY;
                    addRoad(current, right, capacity);
                }
            }
            // Connect Vertical (North)
            if (r < 4) {
                int up = box.gridCorners[r + 1][c];
                if (up != -1) {
                    // Boundary roads (left or right edge) are highways
                    bool isBoundary = (c == 0 || c == 4);
                    int capacity = isBoundary ? HIGHWAY_ROAD_CAPACITY : DEFAULT_ROAD_CAPACITY;
                    addRoad(current, up, capacity);
                }
            }
        }
    }

    // =========================================================
    // 3. INITIALIZE SUB-SECTORS (16 CELLS)
    // =========================================================
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

    // Mark as initialized NOW so we can connect to neighbors
    SECTOR_GRID[idx].initialized = true;

    // =========================================================
    // 4. STITCHING: CONNECT TO NEIGHBOR SECTORS (HIGHWAY CAPACITY)
    // =========================================================
    Vector<string> neighbors = GeometryUtils::getAdjacentSectors(sectorName);

    for (int i = 0; i < neighbors.getSize(); i++) {
        string neighborName = neighbors[i];
        int nIdx = GeometryUtils::getSectorIndex(neighborName);

        // We can only connect if the neighbor exists AND is already initialized.
        // If it's not initialized yet, IT will connect to US when it gets initialized later.
        if (nIdx == -1 || !SECTOR_GRID[nIdx].initialized) continue;

        SectorBox& otherBox = SECTOR_GRID[nIdx];

        // Inter-sector roads are highways
        if (std::abs(box.minLon - otherBox.maxLon) < 0.001) {
            for (int r = 0; r < 5; r++) {
                int myNode = box.gridCorners[r][0];
                int otherNode = otherBox.gridCorners[r][4];
                if (myNode != -1 && otherNode != -1) 
                    addRoad(myNode, otherNode, HIGHWAY_ROAD_CAPACITY);
            }
        }
        else if (std::abs(box.maxLon - otherBox.minLon) < 0.001) {
            for (int r = 0; r < 5; r++) {
                int myNode = box.gridCorners[r][4];
                int otherNode = otherBox.gridCorners[r][0];
                if (myNode != -1 && otherNode != -1) 
                    addRoad(myNode, otherNode, HIGHWAY_ROAD_CAPACITY);
            }
        }
        else if (std::abs(box.minLat - otherBox.maxLat) < 0.001) {
            for (int c = 0; c < 5; c++) {
                int myNode = box.gridCorners[0][c];
                int otherNode = otherBox.gridCorners[4][c];
                if (myNode != -1 && otherNode != -1) 
                    addRoad(myNode, otherNode, HIGHWAY_ROAD_CAPACITY);
            }
        }
        else if (std::abs(box.maxLat - otherBox.minLat) < 0.001) {
            for (int c = 0; c < 5; c++) {
                int myNode = box.gridCorners[4][c];
                int otherNode = otherBox.gridCorners[0][c];
                if (myNode != -1 && otherNode != -1) 
                    addRoad(myNode, otherNode, HIGHWAY_ROAD_CAPACITY);
            }
        }
    }
}

// ==================== ADD LOCATION (CORE LOGIC) ====================
// Smart Insertion Algorithm with:
// - Position snapping to avoid roads and create realistic placement
// - Hierarchical road weights (facility roads have penalty to prefer highways)
// - 1st node: Connects to all 4 corners of the cell
// - 2nd+ nodes: Insert between existing nodes and corners based on position

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

    // 2. For non-corner nodes, snap position to a logical place in the cell
    double finalLat = lat;
    double finalLon = lon;
    
    if (type != FacilityType::CORNER && sector != "Unknown") {
        int sectorIdx = GeometryUtils::getSectorIndex(sector);
        if (sectorIdx != -1) {
            SectorBox& box = SECTOR_GRID[sectorIdx];
            int cellIdx = GeometryUtils::getSubSectorIndex(lat, lon, box);
            
            if (cellIdx != -1) {
                SubSubSector* targetCell = &box.cells[cellIdx];
                
                // Handle spillover if cell is full
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
                    if (bestBackup != nullptr) {
                        targetCell = bestBackup;
                    }
                }
                
                // Snap position to logical quadrant within the cell
                snapNodePosition(targetCell, targetCell->nodeCount, finalLat, finalLon);
            }
        }
    }

    // 3. Create the Node with snapped position
    int newID = createNodeRaw(databaseID, stopID, name, type, finalLat, finalLon);
    if (newID == -1) return -1;

    // 4. Logic for Non-Corner Nodes (Connectivity)
    if (type != FacilityType::CORNER && sector != "Unknown") {
        int sectorIdx = GeometryUtils::getSectorIndex(sector);
        if (sectorIdx != -1) {
            SectorBox& box = SECTOR_GRID[sectorIdx];
            int cellIdx = GeometryUtils::getSubSectorIndex(finalLat, finalLon, box);

            if (cellIdx != -1) {
                SubSubSector* targetCell = &box.cells[cellIdx];

                // Handle spillover again (in case original cell was full)
                if (targetCell->isFull()) {
                    double minDist = INF;
                    SubSubSector* bestBackup = nullptr;
                    for (int i = 0; i < 16; i++) {
                        if (!box.cells[i].isFull()) {
                            double d = GeometryUtils::getGridDistance(finalLat, finalLon,
                                box.cells[i].getCenterLat(), box.cells[i].getCenterLon());
                            if (d < minDist) {
                                minDist = d;
                                bestBackup = &box.cells[i];
                            }
                        }
                    }
                    if (bestBackup != nullptr) {
                        targetCell = bestBackup;
                    }
                }

                // Add to Cell
                int nodeSlot = targetCell->nodeCount;
                if (nodeSlot < 4) {
                    targetCell->nodeIDs[targetCell->nodeCount++] = newID;
                }

                // =========================================================
                // SMART CONNECTIVITY BASED ON NODE COUNT IN CELL
                // =========================================================
                
                int existingCount = 0;
                int existingNodes[4] = {-1, -1, -1, -1};
                for (int i = 0; i < 4; i++) {
                    if (targetCell->nodeIDs[i] != -1 && targetCell->nodeIDs[i] != newID) {
                        existingNodes[existingCount++] = targetCell->nodeIDs[i];
                    }
                }

                if (existingCount == 0) {
                    // =====================================================
                    // CASE 1: First node in cell - Connect to ALL 4 corners
                    // Uses penalty weight for facility-to-corner roads
                    // =====================================================
                    for (int i = 0; i < 4; i++) {
                        int cornerID = targetCell->cornerIDs[i];
                        if (cornerID != -1) {
                            addFacilityRoad(newID, cornerID);
                        }
                    }
                }
                else {
                    // =====================================================
                    // CASE 2+: Insert node into existing network
                    // =====================================================
                    
                    // Find which corner this new node is closest to
                    int closestCornerIdx = -1;
                    double minCornerDist = INF;
                    for (int i = 0; i < 4; i++) {
                        int cID = targetCell->cornerIDs[i];
                        if (cID != -1 && nodes[cID]) {
                            double d = GeometryUtils::getGridDistance(finalLat, finalLon, 
                                nodes[cID]->lat, nodes[cID]->lon);
                            if (d < minCornerDist) {
                                minCornerDist = d;
                                closestCornerIdx = i;
                            }
                        }
                    }

                    int closestCornerID = (closestCornerIdx >= 0) ? 
                        targetCell->cornerIDs[closestCornerIdx] : -1;

                    // Find which existing node currently "owns" this corner
                    int nodeOwningCorner = -1;
                    double ownerDistToCorner = INF;
                    
                    for (int i = 0; i < existingCount; i++) {
                        int existID = existingNodes[i];
                        if (existID == -1 || !nodes[existID]) continue;
                        
                        const LinkedList<Edge> &roads = nodes[existID]->roads;
                        bool hasRoadToCorner = false;
                        for (int r = 0; r < roads.size(); r++) {
                            if (roads[r].destinationID == closestCornerID) {
                                hasRoadToCorner = true;
                                break;
                            }
                        }
                        
                        if (hasRoadToCorner) {
                            double d = GeometryUtils::getGridDistance(
                                nodes[existID]->lat, nodes[existID]->lon,
                                nodes[closestCornerID]->lat, nodes[closestCornerID]->lon);
                            if (d < ownerDistToCorner) {
                                ownerDistToCorner = d;
                                nodeOwningCorner = existID;
                            }
                        }
                    }

                    // Connect new node to its closest corner (with penalty weight)
                    if (closestCornerID != -1) {
                        addFacilityRoad(newID, closestCornerID);
                    }

                    // If an existing node owned this corner and we're closer,
                    // break their connection and connect them to us instead
                    if (nodeOwningCorner != -1 && closestCornerID != -1) {
                        if (minCornerDist < ownerDistToCorner) {
                            removeRoad(nodeOwningCorner, closestCornerID);
                            addFacilityRoad(nodeOwningCorner, newID);
                        } else {
                            addFacilityRoad(newID, nodeOwningCorner);
                        }
                    }

                    // Connect to the 2 closest existing nodes (for mesh connectivity)
                    double dists[4];
                    int sortedNodes[4];
                    int sortCount = 0;
                    
                    for (int i = 0; i < existingCount; i++) {
                        int existID = existingNodes[i];
                        if (existID != -1 && nodes[existID]) {
                            sortedNodes[sortCount] = existID;
                            dists[sortCount] = GeometryUtils::getGridDistance(
                                finalLat, finalLon, nodes[existID]->lat, nodes[existID]->lon);
                            sortCount++;
                        }
                    }

                    // Sort by distance
                    for (int i = 0; i < sortCount - 1; i++) {
                        for (int j = 0; j < sortCount - i - 1; j++) {
                            if (dists[j] > dists[j + 1]) {
                                std::swap(dists[j], dists[j + 1]);
                                std::swap(sortedNodes[j], sortedNodes[j + 1]);
                            }
                        }
                    }

                    // Connect to up to 2 closest (if not already connected)
                    int links = 0;
                    for (int i = 0; i < sortCount && links < 2; i++) {
                        int targetNode = sortedNodes[i];
                        if (!hasRoad(newID, targetNode)) {
                            addFacilityRoad(newID, targetNode);
                            links++;
                        }
                    }
                }
            }
        }
    }

    return newID;
}

// ==================== POSITION SNAPPING ====================
// Snaps a node to a logical position within its SubSubSector cell
// Positions are arranged to avoid the skeleton roads (which run along edges)
// 
// Cell layout with 4 node positions:
//   NW Corner -------- NE Corner
//       |   [1]    [2]   |
//       |                |
//       |   [0]    [3]   |
//   SW Corner -------- SE Corner
//
// Each position is offset from the center towards its corresponding corner

inline void CityGraph::snapNodePosition(const SubSubSector* cell, int nodeIndex, double& lat, double& lon) {
    if (!cell) return;
    
    // Cell dimensions
    double cellHeight = cell->maxLat - cell->minLat;
    double cellWidth = cell->maxLon - cell->minLon;
    double centerLat = cell->getCenterLat();
    double centerLon = cell->getCenterLon();
    
    // Offset from center (30% towards corner, with small random variation)
    double offsetRatio = 0.30;
    double randomVariation = 0.05;
    
    // Add small random variation for realism
    double randLat = ((double)rand() / RAND_MAX - 0.5) * 2.0 * randomVariation * cellHeight;
    double randLon = ((double)rand() / RAND_MAX - 0.5) * 2.0 * randomVariation * cellWidth;
    
    // Position based on node index (0-3), each in a different quadrant
    // This ensures nodes don't overlap with skeleton roads on the cell edges
    switch (nodeIndex % 4) {
        case 0: // SW quadrant
            lat = centerLat - offsetRatio * cellHeight + randLat;
            lon = centerLon - offsetRatio * cellWidth + randLon;
            break;
        case 1: // NW quadrant
            lat = centerLat + offsetRatio * cellHeight + randLat;
            lon = centerLon - offsetRatio * cellWidth + randLon;
            break;
        case 2: // NE quadrant
            lat = centerLat + offsetRatio * cellHeight + randLat;
            lon = centerLon + offsetRatio * cellWidth + randLon;
            break;
        case 3: // SE quadrant
            lat = centerLat - offsetRatio * cellHeight + randLat;
            lon = centerLon + offsetRatio * cellWidth + randLon;
            break;
    }
    
    // Clamp to cell bounds with margin (10% from edges to avoid roads)
    double marginLat = cellHeight * 0.10;
    double marginLon = cellWidth * 0.10;
    
    if (lat < cell->minLat + marginLat) lat = cell->minLat + marginLat;
    if (lat > cell->maxLat - marginLat) lat = cell->maxLat - marginLat;
    if (lon < cell->minLon + marginLon) lon = cell->minLon + marginLon;
    if (lon > cell->maxLon - marginLon) lon = cell->maxLon - marginLon;
}

// ==================== ROAD WEIGHT CONSTANTS ====================
// Penalty multiplier for facility roads - makes Dijkstra prefer skeleton/highway routes
constexpr double FACILITY_ROAD_PENALTY = 1.5;  // 50% penalty on facility roads
constexpr double INTER_FACILITY_PENALTY = 1.3; // 30% penalty between facilities

// ==================== FACILITY ROAD (WITH WEIGHT PENALTY) ====================
// Adds a road with weight penalty to discourage using facility roads for through-traffic
// This makes Dijkstra prefer the main skeleton roads (highways)

inline void CityGraph::addFacilityRoad(int id1, int id2) {
    if (id1 < 0 || id2 < 0 || id1 >= nodeCount || id2 >= nodeCount || id1 == id2) {
        return;
    }
    if (hasRoad(id1, id2)) return;

    double dist = GeometryUtils::getGridDistance(
        nodes[id1]->lat, nodes[id1]->lon,
        nodes[id2]->lat, nodes[id2]->lon
    );

    // Apply penalty based on whether this is facility-to-corner or facility-to-facility
    bool id1IsCorner = (nodes[id1]->type == FacilityType::CORNER);
    bool id2IsCorner = (nodes[id2]->type == FacilityType::CORNER);
    
    double penalty;
    if (id1IsCorner || id2IsCorner) {
        penalty = FACILITY_ROAD_PENALTY;
    } else {
        penalty = INTER_FACILITY_PENALTY;
    }
    
    double weightedDist = dist * penalty;

    // Facility roads have lower capacity
    Edge edge1(id2, weightedDist, FACILITY_ROAD_CAPACITY);
    edge1.dynamicWeight = weightedDist;
    
    Edge edge2(id1, weightedDist, FACILITY_ROAD_CAPACITY);
    edge2.dynamicWeight = weightedDist;

    nodes[id1]->roads.push_back(edge1);
    nodes[id2]->roads.push_back(edge2);
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

        // Generate coords at cell center (will be snapped by addLocation)
        lat = cell.getCenterLat();
        lon = cell.getCenterLon();
    }
    else {
        // Fallback: Generate generic coords in sector
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
    addRoad(id1, id2, DEFAULT_ROAD_CAPACITY);
}

inline void CityGraph::addRoad(int id1, int id2, int capacity) {
    if (id1 < 0 || id2 < 0 || id1 >= nodeCount || id2 >= nodeCount || id1 == id2) {
        return;
    }

    // Check if road already exists to avoid duplicates
    if (hasRoad(id1, id2)) return;

    double dist = GeometryUtils::getGridDistance(
        nodes[id1]->lat, nodes[id1]->lon,
        nodes[id2]->lat, nodes[id2]->lon
    );

    nodes[id1]->roads.push_back(Edge(id2, dist, capacity));
    nodes[id2]->roads.push_back(Edge(id1, dist, capacity));
}

inline void CityGraph::removeRoad(int id1, int id2) {
    if (id1 < 0 || id2 < 0 || id1 >= nodeCount || id2 >= nodeCount) return;
    if (!nodes[id1] || !nodes[id2]) return;

    // Remove id2 from id1's roads
    LinkedList<Edge>& roads1 = nodes[id1]->roads;
    for (int i = 0; i < roads1.size(); i++) {
        if (roads1[i].destinationID == id2) {
            roads1.erase(i);
            break;
        }
    }

    // Remove id1 from id2's roads
    LinkedList<Edge>& roads2 = nodes[id2]->roads;
    for (int i = 0; i < roads2.size(); i++) {
        if (roads2[i].destinationID == id1) {
            roads2.erase(i);
            break;
        }
    }
}

inline bool CityGraph::hasRoad(int id1, int id2) const {
    if (id1 < 0 || id2 < 0 || id1 >= nodeCount || id2 >= nodeCount) return false;
    if (!nodes[id1]) return false;

    const LinkedList<Edge>& roads = nodes[id1]->roads;
    for (int i = 0; i < roads.size(); i++) {
        if (roads[i].destinationID == id2) {
            return true;
        }
    }
    return false;
}

inline Edge* CityGraph::getEdge(int fromNode, int toNode) {
    if (fromNode < 0 || fromNode >= nodeCount || !nodes[fromNode]) return nullptr;
    
    LinkedList<Edge>& roads = nodes[fromNode]->roads;
    for (int i = 0; i < roads.size(); i++) {
        if (roads[i].destinationID == toNode) {
            return &roads[i];
        }
    }
    return nullptr;
}

inline const Edge* CityGraph::getEdge(int fromNode, int toNode) const {
    if (fromNode < 0 || fromNode >= nodeCount || !nodes[fromNode]) return nullptr;
    
    const LinkedList<Edge>& roads = nodes[fromNode]->roads;
    for (int i = 0; i < roads.size(); i++) {
        if (roads[i].destinationID == toNode) {
            return &roads[i];
        }
    }
    return nullptr;
}

// ==================== TRAFFIC MANAGEMENT ====================

inline bool CityGraph::tryEnterEdge(int fromNode, int toNode) {
    Edge* edge = getEdge(fromNode, toNode);
    if (!edge) return false;
    
    if (edge->currentLoad < edge->capacity) {
        edge->currentLoad++;
        
        // Also update the reverse edge (bidirectional roads share load)
        Edge* reverseEdge = getEdge(toNode, fromNode);
        if (reverseEdge) {
            reverseEdge->currentLoad++;
        }
        
        return true;
    }
    
    return false;  // Road is at capacity, vehicle must wait
}

inline void CityGraph::leaveEdge(int fromNode, int toNode) {
    Edge* edge = getEdge(fromNode, toNode);
    if (edge && edge->currentLoad > 0) {
        edge->currentLoad--;
    }
    
    // Also update the reverse edge
    Edge* reverseEdge = getEdge(toNode, fromNode);
    if (reverseEdge && reverseEdge->currentLoad > 0) {
        reverseEdge->currentLoad--;
    }
}

inline void CityGraph::updateTrafficWeights() {
    for (int i = 0; i < nodeCount; i++) {
        if (!nodes[i]) continue;
        
        LinkedList<Edge>& roads = nodes[i]->roads;
        for (int j = 0; j < roads.size(); j++) {
            roads[j].updateDynamicWeight();
        }
    }
}

inline double CityGraph::getEdgeCongestion(int fromNode, int toNode) const {
    const Edge* edge = getEdge(fromNode, toNode);
    if (!edge) return 0.0;
    return edge->getCongestionFactor();
}

inline int CityGraph::getTotalVehiclesOnRoads() const {
    int total = 0;
    for (int i = 0; i < nodeCount; i++) {
        if (!nodes[i]) continue;
        
        const LinkedList<Edge>& roads = nodes[i]->roads;
        for (int j = 0; j < roads.size(); j++) {
            total += roads[j].currentLoad;
        }
    }
    // Divide by 2 because roads are bidirectional and we count each edge twice
    return total / 2;
}

// ==================== PATHFINDING WITH DYNAMIC WEIGHTS ====================

inline Vector<int> CityGraph::findShortestPathDynamic(int startID, int endID, double& totalDistance) {
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
            // Use dynamicWeight instead of weight for traffic-aware routing
            double weight = roads[i].dynamicWeight;

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

// ==================== PATHFINDING ====================

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

// ==================== CSV LOADING ====================

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