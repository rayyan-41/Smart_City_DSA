#include "CityUtils.h" 

class CityMapGraph {
private:
    CityNode* nodes[MAX_NODES];
    int nodeCount;

    // Helper to build sector boundaries
    void initializeSectorFrame(string sectorName);

public:
    CityMapGraph();
    ~CityMapGraph();

    // --- Core Logic ---
    int addLocation(string stopID, string name, string type, double lat, double lon);
    void addRoad(int id1, int id2);

    // --- Helpers ---
    int getIDByName(string name);
    int getIDByString(string sID);

    // --- CSV Loaders ---
    void loadStopsCSV(const string& filename);
    void loadBuildingsCSV(const string& filename, string type);

    // --- Algorithms ---
    void findShortestPath(int startID, int endID);
    void printPath(int parent[], int j);
};


/*------ Constructors and Destructors ------*/
CityMapGraph::CityMapGraph() {
    cout << ANSI_BRIGHT_CYAN "[SYS] " ANSI_RESET "Initializing CityMapGraph..." << endl;
    nodeCount = 0;
    for (int i = 0; i < MAX_NODES; i++) nodes[i] = nullptr;
}

CityMapGraph::~CityMapGraph() {
    cout << ANSI_BRIGHT_CYAN "[SYS] " ANSI_RESET "Destroying CityMapGraph..." << endl;
    for (int i = 0; i < nodeCount; i++) {
        if (nodes[i] != nullptr) delete nodes[i];
    }
}

/*------ Helper: Initialize Sector Frame ------*/
void CityMapGraph::initializeSectorFrame(string sectorName) {
    int idx = GeometryUtils::getSectorIndex(sectorName);

    // Safety check
    if (idx == -1) return;
    if (SECTOR_GRID[idx].initialized) return;

    cout << ANSI_BRIGHT_CYAN "[SYS] " ANSI_RESET "Initializing Frame: " << sectorName << endl;

    SectorBox box = SECTOR_GRID[idx];

    int c1 = nodeCount; addLocation("C-" + sectorName + "-1", sectorName + " Corner 1", "CORNER", box.minLat, box.minLon);
    int c2 = nodeCount; addLocation("C-" + sectorName + "-2", sectorName + " Corner 2", "CORNER", box.maxLat, box.minLon);
    int c3 = nodeCount; addLocation("C-" + sectorName + "-3", sectorName + " Corner 3", "CORNER", box.maxLat, box.maxLon);
    int c4 = nodeCount; addLocation("C-" + sectorName + "-4", sectorName + " Corner 4", "CORNER", box.minLat, box.maxLon);

    addRoad(c1, c2);
    addRoad(c2, c3);
    addRoad(c3, c4);
    addRoad(c4, c1);

    SECTOR_GRID[idx].initialized = true;
}

/*------ Add Node Logic ------*/
int CityMapGraph::addLocation(string stopID, string name, string type, double lat, double lon) {
    if (nodeCount >= MAX_NODES) {
        cout << ANSI_BRIGHT_RED "[ERROR] " ANSI_RESET "Graph at Max Capacity!" << endl;
        return -1;
    }

    string sector = GeometryUtils::resolveSector(lat, lon);

    //1. Check if we need to build the frame FIRST
    if (type != "CORNER" && sector != "Unknown" && !SECTOR_GRID[GeometryUtils::getSectorIndex(sector)].initialized) {
        initializeSectorFrame(sector);
    }

    //2. Create Node
    int newID = nodeCount;
    nodes[newID] = new CityNode(newID, stopID, name, type, lat, lon);
    nodeCount++;

    if (type != "CORNER") {
        cout << ANSI_BRIGHT_GREEN "[+] " ANSI_RESET << name << " (" << nodes[newID]->sector << ")";
    }

    //3. Anchor Logic: Connect to nearest frame corner
    if (type != "CORNER" && sector != "Unknown Sector") {
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
            cout << " -> Linked";
        }
        cout << endl;
    } else if (type != "CORNER") {
     cout << endl;  //Add newline for Unknown sector nodes
    }

    return newID;
}

/*------ Core Helpers ------*/
int CityMapGraph::getIDByName(string name) {
    for (int i = 0; i < nodeCount; i++) {
        if (nodes[i]->name == name) return i;
    }
    return -1;
}

int CityMapGraph::getIDByString(string sID) {
    for (int i = 0; i < nodeCount; i++) {
        if (nodes[i]->stopID == sID) return i;
    }
    return -1;
}

void CityMapGraph::addRoad(int id1, int id2) {
    if (id1 < 0 || id2 < 0 || id1 >= nodeCount || id2 >= nodeCount) return;

    double dist = GeometryUtils::getHaversineDistance(nodes[id1]->lat, nodes[id1]->lon, nodes[id2]->lat, nodes[id2]->lon);

    Edge e1(id1,dist);
    nodes[id1]->roads.push_back(e1);

    Edge e2(id2, dist);
    nodes[id2]->roads.push_back(e2);
}

/*------ [NEW] Generic Building Loader ------*/
// Loads CSVs like schools.csv, hospitals.csv that LACK coordinates.
// It generates coordinates based on the 'Sector' column.
void CityMapGraph::loadBuildingsCSV(const string& filename, string type) {
    cout << ANSI_BRIGHT_CYAN "[SYS] " ANSI_RESET "Loading Buildings: " << filename << endl;
    ifstream file(filename);
    if (!file.is_open()) {
        cout << ANSI_BRIGHT_RED "[ERROR] " ANSI_RESET "Cannot open file: " << filename << endl;
        return;
    }

    string line;
    getline(file, line); // Skip header

    int successCount = 0;

    while (getline(file, line)) {
        if (line.empty()) continue;

        string idStr = "";
        string name = "";
        string sector = "";
        int i = 0;

        // Manual Parsing to handle quotes
        // 1. ID (Column 0)
        bool inQuotes = false;
        while (i < line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            idStr += c;
        }

        // 2. Name (Column 1)
        inQuotes = false;
        while (i < line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            name += c;
        }

        // 3. Sector (Column 2)
        inQuotes = false;
        while (i < line.size()) {
            char c = line[i++];
            if (c == '"') { inQuotes = !inQuotes; continue; }
            if (c == ',' && !inQuotes) break;
            sector += c;
        }

        // Validate Sector
        if (sector.empty()) continue;

        // Generate Coordinates!
        double lat = 0.0, lon = 0.0;
        GeometryUtils::generateCoords(sector, lat, lon);

        // Add to Graph
        if (addLocation(idStr, name, type, lat, lon) != -1) {
            successCount++;
        }
    }
    file.close();
    cout << ANSI_BRIGHT_GREEN "[SYS] " ANSI_RESET "Loaded " << successCount << " " << type << "(s)." << endl;
}

/*------ Stop Loader (With Coords) ------*/
void CityMapGraph::loadStopsCSV(const string& filename) {
    cout << ANSI_BRIGHT_CYAN "[SYS] " ANSI_RESET "Loading Stops: " << filename << endl;
    ifstream file(filename);
    if (!file.is_open()) {
        cout << ANSI_BRIGHT_RED "[ERROR] " ANSI_RESET "Cannot open file" << endl;
        return;
    }

    string line;
    getline(file, line); //Skip header

    while (getline(file, line)) {
        if (line.empty()) continue;

        string stopID = "", name = "", lat_str = "", lon_str = "";
        int i = 0;

        //StopID
        while (i < line.size() && line[i] != ',') stopID += line[i++];
        if (i >= line.size()) continue;  //Malformed line
        i++;
        
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
        if (stopID.empty() || name.empty() || lat_str.empty() || lon_str.empty()) {
            continue;  //Skip invalid entries
        }

        try {
            double lat = stod(lat_str);
            double lon = stod(lon_str);
            addLocation(stopID, name, "STOP", lat, lon);
        }
        catch (...) {
            //Handle parsing errors silently or log
            continue;
        }
    }
    file.close();
}

/*------ Algorithms ------*/
void CityMapGraph::findShortestPath(int startID, int endID) {
    if (startID < 0 || startID >= nodeCount || endID < 0 || endID >= nodeCount) {
        cout << ANSI_BRIGHT_RED "[ERROR] " ANSI_RESET "Invalid Node IDs." << endl;
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

    // Assuming PriorityQueue is a MinHeap<DijkstraNode> defined in DataStructures.h
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

    if (distance[endID] == INF) {
        cout << ANSI_BRIGHT_RED "\n[RESULT] " ANSI_RESET "No path found between these locations." << endl;
    }
    else {
        cout << ANSI_BRIGHT_GREEN "\n[RESULT] " ANSI_RESET "Shortest Distance: " << distance[endID] << " km" << endl;
        cout << "Path: ";
        printPath(parent, endID);
        cout << endl;
    }
}

void CityMapGraph::printPath(int parent[], int j) {
    if (parent[j] == -1) {
        cout << ANSI_BOLD << nodes[j]->name << ANSI_RESET;
        return;
    }
    printPath(parent, parent[j]);
    cout << " -> " << nodes[j]->name;
}

