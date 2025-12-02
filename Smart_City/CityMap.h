//@desc: This file contains the main implementation of the City Map Graph.
#include <iostream>
#include <fstream>
#include <string>

#include "CustomSTL.h"
#include "CityUtils.h"
using namespace std; using namespace re;




struct CityNode {
    int id;
    string stopID;   // From CSV (e.g., "Stop1")
    string name;     // "G-10 Markaz"
    string sector;   // Auto-resolved (e.g., "G-10")
    double lat, lon;

    // Adjacency List: Stores a list of 'Edge' objects
    // Each 'Edge' contains the neighbor's ID and the distance to them.
    <Edge> roads;

    CityNode() : id(-1) {}
    CityNode(int i, string sid, string n, double lt, double ln) {
        id = i; stopID = sid; name = n; lat = lt; lon = ln;
        // MAGIC: Auto-detect sector upon creation
        sector = GeometryUtils::resolveSector(lt, ln);
    }
};

// ==========================================
// MAIN GRAPH CLASS
// ==========================================

class CityMapGraph {
    CityNode* nodes[500]; // Array for O(1) access by ID
    int nodeCount;

public:
    CityMapGraph() {
        nodeCount = 0;
        for (int i = 0; i < 500; i++) nodes[i] = NULL;
    }

    // --- ADDING NODES ---
    // Takes raw coords, assigns Sector automatically
    void addLocation(string stopID, string name, double lat, double lon) {
        if (nodeCount >= 500) return;

        nodes[nodeCount] = new CityNode(nodeCount, stopID, name, lat, lon);

        cout << "[Graph] Added: " << name
            << " | Detected Sector: " << nodes[nodeCount]->sector
            << " (ID: " << nodeCount << ")" << endl;

        nodeCount++;
    }

    int getIDByString(string sID) {
        for (int i = 0; i < nodeCount; i++) {
            if (nodes[i]->stopID == sID) return i;
        }
        return -1;
    }

    // --- ADDING ROADS ---
    // Auto-calculates weight using Haversine Formula
    void addRoad(int id1, int id2) {
        if (id1 < 0 || id2 < 0 || id1 >= nodeCount || id2 >= nodeCount) return;

        double dist = GeometryUtils::getHaversineDistance(
            nodes[id1]->lat, nodes[id1]->lon,
            nodes[id2]->lat, nodes[id2]->lon
        );

        // Add 2-way edges
        nodes[id1]->roads.insert({ id2, dist });
        nodes[id2]->roads.insert({ id1, dist });

        cout << fixed << setprecision(2);
        cout << "[Road] Connected: " << nodes[id1]->name << " <--> " << nodes[id2]->name
            << " Distance: " << dist << " km" << endl;
    }

    // --- DIJKSTRA'S ALGORITHM ---
    void findShortestPath(int startID, int endID) {
        if (startID < 0 || endID < 0) { cout << "Invalid IDs" << endl; return; }

        double distances[500];
        int parent[500];
        for (int i = 0; i < 500; i++) {
            distances[i] = numeric_limits<double>::max();
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

        if (distances[endID] == numeric_limits<double>::max()) {
            cout << "No path found." << endl;
            return;
        }

        cout << "\n--- Route Calculation ---" << endl;
        cout << "Total Distance: " << distances[endID] << " km" << endl;
        printPath(parent, endID);
        cout << endl;
    }

    void printPath(int parent[], int j) {
        if (parent[j] == -1) {
            cout << nodes[j]->name;
            return;
        }
        printPath(parent, parent[j]);
        cout << " -> " << nodes[j]->name;
    }

    // --- CSV PARSER ---
    // Format: StopID,Name,"Lat, Lon"
    void initFromCSV(string filename) {
        ifstream file(filename.c_str());
        if (!file.is_open()) {
            cout << "Error: Could not open " << filename << endl;
            return;
        }

        string line;
        getline(file, line); // Skip Header

        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string sID, name, latStr, lonStr, segment;

            // Parsing Stop1,Name,"33.x, 73.y"
            getline(ss, sID, ',');
            getline(ss, name, ',');

            // Handle Quotes around coordinates
            getline(ss, segment, '"');
            if (ss.peek() != EOF) {
                getline(ss, latStr, ',');
                getline(ss, lonStr, '"');

                double lat = atof(latStr.c_str());
                double lon = atof(lonStr.c_str());

                addLocation(sID, name, lat, lon);
            }
        }
        file.close();
    }
};

