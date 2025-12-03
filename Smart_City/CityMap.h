#ifndef CITY_GRAPH_H
#define CITY_GRAPH_H

#include <string>
#include <iostream>
#include <fstream>

#include "CityUtils.h"

using namespace std; using namespace GeometryUtils;


class CityMapGraph {
private:
    CityNode* nodes[MAX_NODES];
    int nodeCount;

    void initializeSectorFrame(string sectorName);
public:
    CityMapGraph();
    ~CityMapGraph();

    //Core Logic
    int addLocation(string stopID, string name, string type, double lat, double lon);
    void addRoad(int id1, int id2);

    // Helpers
    int getIDByName(string name);
    int getIDByString(string sID);

    //CSV Loaders
    void loadBuildingsFromCSV(string filename, string type);
    void loadStopsFromCSV(string filename);

    // Algorithms
    void findShortestPath(int startID, int endID);
    void printPath(int parent[], int j);
};


/*------Constructors and Destructors------*/
CityMapGraph::CityMapGraph() {
    nodeCount = 0;
    for (int i = 0; i < MAX_NODES; i++) nodes[i] = nullptr;
}

CityMapGraph::~CityMapGraph() {
    for (int i = 0; i < nodeCount; i++) {
        if (nodes[i] != nullptr) delete nodes[i];
    }
}

//Helper: Initialize Sector Frame (Corners)
void CityMapGraph::initializeSectorFrame(string sectorName) {
    int idx = GeometryUtils::getSectorIndex(sectorName);
    if (idx == -1) return;
    if (SECTOR_GRID[idx].initialized) return;

    cout << "[SYS] Initializing Grid Frame for " << sectorName << endl;

    SectorBox box = SECTOR_GRID[idx];

    int c1 = nodeCount; addLocation("C-" + sectorName + "-1", sectorName + " Corner 1", "Intersection", box.minLat, box.minLon);
    int c2 = nodeCount; addLocation("C-" + sectorName + "-2", sectorName + " Corner 2", "Intersection", box.maxLat, box.minLon);
    int c3 = nodeCount; addLocation("C-" + sectorName + "-3", sectorName + " Corner 3", "Intersection", box.maxLat, box.maxLon);
    int c4 = nodeCount; addLocation("C-" + sectorName + "-4", sectorName + " Corner 4", "Intersection", box.minLat, box.maxLon);

    addRoad(c1, c2);
    addRoad(c2, c3);
    addRoad(c3, c4);
    addRoad(c4, c1);

    SECTOR_GRID[idx].initialized = true;
}

//Add Node Logic
int CityMapGraph::addLocation(string stopID, string name, string type, double lat, double lon) {
    if (nodeCount >= MAX_NODES) return -1;

    string sector = GeometryUtils::resolveSector(lat, lon);

    //Check if we need to build the frame FIRST
    if (type != "Intersection" && sector != "Unknown" && !SECTOR_GRID[GeometryUtils::getSectorIndex(sector)].initialized) {
        initializeSectorFrame(sector);
    }

    int newID = nodeCount;
    nodes[newID] = new CityNode(newID, stopID, name, type, lat, lon);
    nodeCount++;

    if (type != "CORNER") {
        cout << "[Graph] Added: " << name << " (" << nodes[newID]->sector << ")";
    }

    // Anchor Logic: Connect to nearest frame corner
    if (type != "CORNER") {
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
            cout << " -> Linked to Frame";
        }
        cout << endl;
    }

    return newID;
}

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
    nodes[id1]->roads.insert({ id2, dist });
    nodes[id2]->roads.insert({ id1, dist });
}

// Generic CSV Loader
void CityMapGraph::loadBuildingsFromCSV(string filename, string type) {
    ifstream file(filename.c_str());
    if (!file.is_open()) return;

    string line;
    char c;
    while (file.get(c) && c != '\n'); // Skip header

    while (getline(file, line)) {
        if (line.empty()) continue;

        size_t pos = 0;
        size_t nextComma;

        // 1. ID
        nextComma = line.find(',', pos);
        if (nextComma == string::npos) continue;
        string idStr = line.substr(pos, nextComma - pos);
        pos = nextComma + 1;

        // 2. Name
        nextComma = line.find(',', pos);
        if (nextComma == string::npos) continue;
        string name = line.substr(pos, nextComma - pos);
        pos = nextComma + 1;

        // 3. Sector
        nextComma = line.find(',', pos);
        string sector;
        if (nextComma == string::npos) sector = line.substr(pos);
        else sector = line.substr(pos, nextComma - pos);

        if (getIDByName(name) != -1) continue;

        double lat, lon;
        GeometryUtils::generateCoords(sector, lat, lon);

        addLocation(idStr, name, type, lat, lon);
    }
    file.close();
}

// Stops CSV Parser
void CityMapGraph::loadStopsFromCSV(string filename) {
    ifstream file(filename.c_str());
    if (!file.is_open()) { cout << "Error opening " << filename << endl; return; }
    string line;

    char c;
    while (file.get(c) && c != '\n'); // Skip header

    while (getline(file, line)) {
        if (line.empty()) continue;

        size_t pos = 0;
        size_t nextComma;

        // 1. StopID
        nextComma = line.find(',', pos);
        if (nextComma == string::npos) continue;
        string sID = line.substr(pos, nextComma - pos);
        pos = nextComma + 1;

        // 2. Name
        nextComma = line.find(',', pos);
        if (nextComma == string::npos) continue;
        string name = line.substr(pos, nextComma - pos);
        pos = nextComma + 1;

        // 3. Coordinates
        if (pos < line.length() && line[pos] == '"') {
            pos++;
            size_t latComma = line.find(',', pos);

            if (latComma != string::npos) {
                string latStr = line.substr(pos, latComma - pos);
                pos = latComma + 1;
                while (pos < line.length() && line[pos] == ' ') pos++;

                size_t closeQuote = line.find('"', pos);
                if (closeQuote != string::npos) {
                    string lonStr = line.substr(pos, closeQuote - pos);
                    addLocation(sID, name, "STOP", atof(latStr.c_str()), atof(lonStr.c_str()));
                }
            }
        }
    }
    file.close();
}

// Dijkstra Implementation
void CityMapGraph::findShortestPath(int startID, int endID) {
    if (startID < 0 || endID < 0) { cout << "Invalid IDs" << endl; return; }

    double distances[MAX_NODES];
    int parent[MAX_NODES];
    for (int i = 0; i < MAX_NODES; i++) {
        distances[i] = INF;
        parent[i] = -1;
    }

    MinHeap<PQNode> pq;
    distances[startID] = 0;
    pq.push({ startID, 0 });

    while (!pq.isEmpty()) {
        PQNode current = pq.pop();
        int u = current.id;
        if (u == endID) break;
        ListNode<Edge>* temp = nodes[u]->roads.head;
        while (temp != NULL) {
            int v = temp->data.destinationID;
            double weight = temp->data.weight;
            if (distances[u] + weight < distances[v]) {
                distances[v] = distances[u] + weight;
                parent[v] = u;
                pq.push({ v, distances[v] });
            }
            temp = temp->next;
        }
    }
    if (distances[endID] == INF) { cout << "No path." << endl; return; }

    cout << "--- Route Found ---" << endl;
    printPath(parent, endID);
    cout << "\nTotal Distance: " << distances[endID] << " km" << endl;
}

void CityMapGraph::printPath(int parent[], int j) {
    if (parent[j] == -1) {
        cout << nodes[j]->name;
        return;
    }
    printPath(parent, parent[j]);
    cout << " -> " << nodes[j]->name;
}

#endif