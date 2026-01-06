#pragma once
#ifndef CITY_GRAPH_VIEW_H
#define CITY_GRAPH_VIEW_H

#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <iostream>

#include "../../SmartCity.h"
#include "../../termgl/Termgl.h"
#include "../../data_structures/Vector.h"

using std::string;

// ============================================================================
// STRUCTS & ENUMS
// ============================================================================

struct Point2D {
    double x, y;
    Point2D(double _x = 0, double _y = 0) : x(_x), y(_y) {}
};

struct GraphNode2D {
    int id;
    double lat, lon;
    Point2D pos;
    string name;
    string type;
    string sector;
    termgl::Color color;
    bool isCorner;
    bool isOnPath;
    bool isVisited;
    bool isStart;
    bool isEnd;
    int gridRow, gridCol;  // Position in sector's 5x5 grid (for corners)

    GraphNode2D() : id(-1), lat(0), lon(0), pos(), name(""), type(""), sector(""),
        color(termgl::Color::White()), isCorner(false), isOnPath(false),
        isVisited(false), isStart(false), isEnd(false), gridRow(-1), gridCol(-1) {
    }
};

// ============================================================================
// ROAD HIERARCHY CLASSIFICATION
// ============================================================================
// Sector Grid (5x5 corners per sector):
//   - SECTOR_BOUNDARY: Edges on the perimeter (row 0, row 4, col 0, col 4)
//   - SUB_SECTOR: Edges at row 2 or col 2 (dividing into 4 quadrants)
//   - SUB_SUB_SECTOR: All other internal skeleton edges (rows 1,3 or cols 1,3)
//   - FACILITY_ROAD: Connections involving non-corner nodes

enum class RoadType {
    SECTOR_BOUNDARY,    // Perimeter roads of sector (thickest)
    SUB_SECTOR,         // Roads dividing sector into 4 quadrants
    SUB_SUB_SECTOR,     // Roads dividing quadrants into 4 cells each
    FACILITY_ROAD       // Connections to/between facilities (thinnest)
};

struct GraphEdge2D {
    int fromID, toID;
    bool isOnPath;
    RoadType roadType;
    string fromSector;
    string toSector;

    GraphEdge2D(int f = -1, int t = -1) : fromID(f), toID(t), isOnPath(false), 
        roadType(RoadType::FACILITY_ROAD), fromSector(""), toSector("") {}
};

struct SectorRegion {
    string name;
    double minLat, maxLat, minLon, maxLon;
    Point2D topLeft, bottomRight;
    Point2D center;
    bool isHovered;

    SectorRegion() : name(""), minLat(0), maxLat(0), minLon(0), maxLon(0),
        topLeft(), bottomRight(), center(), isHovered(false) {
    }

    bool contains(Point2D p) const {
        return p.x >= topLeft.x && p.x <= bottomRight.x &&
            p.y >= topLeft.y && p.y <= bottomRight.y;
    }
};

struct TrafficVehicle {
    int edgeFromID, edgeToID;
    double progress;
    double speed;
    termgl::Color color;
    bool isBus;
    bool isReal;           // True for actual simulation vehicles
    bool isStuck;          // Shows ! icon if stuck
    string vehicleID;      // ID of the vehicle

    TrafficVehicle() : edgeFromID(-1), edgeToID(-1), progress(0), speed(0.02),
        color(termgl::Color::Yellow()), isBus(false), isReal(false), isStuck(false), vehicleID("") {
    }
};

// ==================== CITIZEN RENDER DATA ====================
struct CitizenRenderData {
    double lat, lon;
    Point2D pos;
    string name;
    string state;
    string thought;
    termgl::Color color;
    bool isSelected;
    
    CitizenRenderData() : lat(0), lon(0), pos(), name(""), state(""), thought(""), 
        color(termgl::Color::White()), isSelected(false) {}
};

class GraphViewport {
private:
    double minLat, maxLat, minLon, maxLon;
    int canvasWidth, canvasHeight;
    double offsetX, offsetY;
    double zoom;

public:
    GraphViewport() :
        minLat(BASE_LAT), maxLat(MAX_LAT),
        minLon(BASE_LON), maxLon(MAX_LON),
        canvasWidth(1280), canvasHeight(720),
        offsetX(0), offsetY(0), zoom(1.0) {
    }

    void setBounds(double minLa, double maxLa, double minLo, double maxLo) {
        minLat = minLa; maxLat = maxLa; minLon = minLo; maxLon = maxLo;
    }

    void setCanvasSize(int w, int h) {
        canvasWidth = w; canvasHeight = h;
    }

    int getCanvasWidth() const { return canvasWidth; }
    int getCanvasHeight() const { return canvasHeight; }

    Point2D geoToCanvas(double lat, double lon) const {
        // Calculate the real-world aspect ratio
        // At Islamabad's latitude (~33.7°), longitude degrees are shorter than latitude degrees
        // 1 degree lat ? 111 km, 1 degree lon ? 92 km at this latitude
        double latRange = maxLat - minLat;
        double lonRange = maxLon - minLon;
        
        // Real-world dimensions in km
        double realHeight = latRange * KM_PER_LAT_DEGREE;  // ~111 km per degree
        double realWidth = lonRange * KM_PER_LON_DEGREE;   // ~92 km per degree at this latitude
        
        // Calculate aspect ratio to make squares look square
        double geoAspect = realWidth / realHeight;
        double canvasAspect = (double)canvasWidth / (double)canvasHeight;
        
        // Normalize coordinates (0-1 range)
        double normX = (lon - minLon) / lonRange;
        double normY = (maxLat - lat) / latRange;  // Flip Y since screen Y increases downward
        
        // Apply zoom and pan
        normX = (normX - 0.5) * zoom + 0.5 + offsetX;
        normY = (normY - 0.5) * zoom + 0.5 + offsetY;
        
        // Padding
        double pad = 0.05;
        double drawWidth = canvasWidth * (1.0 - 2 * pad);
        double drawHeight = canvasHeight * (1.0 - 2 * pad);
        
        // Adjust for aspect ratio - fit the map while maintaining proportions
        double scaleX, scaleY, offsetAdjustX = 0, offsetAdjustY = 0;
        
        if (geoAspect > canvasAspect) {
            // Map is wider than canvas - fit to width
            scaleX = drawWidth;
            scaleY = drawWidth / geoAspect;
            offsetAdjustY = (drawHeight - scaleY) / 2.0;
        } else {
            // Map is taller than canvas - fit to height
            scaleY = drawHeight;
            scaleX = drawHeight * geoAspect;
            offsetAdjustX = (drawWidth - scaleX) / 2.0;
        }
        
        double canvasX = pad * canvasWidth + offsetAdjustX + normX * scaleX;
        double canvasY = pad * canvasHeight + offsetAdjustY + normY * scaleY;

        return Point2D(canvasX, canvasY);
    }

    void zoomIn() { zoom *= 1.1; if (zoom > 10.0) zoom = 10.0; }
    void zoomOut() { zoom /= 1.1; if (zoom < 0.1) zoom = 0.1; }

    void drag(double dx, double dy) {
        double effectiveW = canvasWidth * 0.9;
        double effectiveH = canvasHeight * 0.9;
        if (effectiveW > 0) offsetX += dx / effectiveW;
        if (effectiveH > 0) offsetY += dy / effectiveH;
    }

    void resetView() { zoom = 1.0; offsetX = 0; offsetY = 0; }

    double getZoom() const { return zoom; }
    double getOffsetX() const { return offsetX; }
    double getOffsetY() const { return offsetY; }

    // Get scale factor for rendering (things get bigger as you zoom in)
    double getScaleFactor() const {
        return std::max(1.0, zoom * 0.5);
    }

    bool isVisible(Point2D p, int margin = 50) const {
        return (p.x >= -margin && p.x <= canvasWidth + margin &&
            p.y >= -margin && p.y <= canvasHeight + margin);
    }
};

enum class DijkstraMode {
    SELECT_START,
    SELECT_TARGET_TYPE,
    RUNNING,
    COMPLETE
};

// ============================================================================
// CITY GRAPH VIEW CLASS
// ============================================================================

class CityGraphView {
private:
    SmartCity* city;
    Vector<GraphNode2D> graphNodes;
    Vector<GraphEdge2D> graphEdges;
    Vector<SectorRegion> sectorRegions;
    Vector<int> nodeIdToIndex;
    Vector<TrafficVehicle> trafficVehicles;
    Vector<CitizenRenderData> citizenRenderList;
    GraphViewport viewport;

    // Assets
    termgl::Texture texSchool, texHospital, texPharmacy, texStop, texMall;
    termgl::Texture texMosque, texPark, texPolice, texFire, texLibrary;
    termgl::Texture texRestaurant, texHouse, texDefault;
    termgl::Texture texBus, texCar, texAmbulance, texRickshaw;

    termgl::Sprite sprSchool, sprHospital, sprPharmacy, sprStop, sprMall;
    termgl::Sprite sprMosque, sprPark, sprPolice, sprFire, sprLibrary;
    termgl::Sprite sprRestaurant, sprHouse, sprDefault;
    termgl::Sprite sprBus, sprCar, sprAmbulance, sprRickshaw;

    int mouseX, mouseY;
    int hoveredNodeID;
    string hoveredSector;

    // View toggles
    bool showCorners;
    bool showRoads;
    bool showSectorBounds;
    bool showHouses;
    bool showTraffic;
    bool trafficPaused;
    bool showCongestionHeatmap;    // Phase 5: Heatmap overlay
    bool showRealVehicles;          // Phase 5: Render simulation vehicles
    bool showCitizens;              // Phase 5: Render walking citizens
    bool useAgentSimulation;        // Phase 5: Enable AI simulation

    // God Mode inspection
    int selectedVehicleIndex;
    int selectedCitizenIndex;
    bool godModeEnabled;

    DijkstraMode dijkstraMode;
    bool inDijkstraMode;
    int dijkstraStartNode;
    int dijkstraEndNode;
    string dijkstraTargetType;
    Vector<int> dijkstraPath;
    double dijkstraDistance;
    int dijkstraNodeSelection;
    int dijkstraEndNodeSelection;
    Vector<int> selectableNodes;

    int intersectionCounter;

    termgl::Color getNodeColor(const string& type) {
        if (type == "CORNER") return termgl::Color::Grey();
        if (type == "STOP") return termgl::Color::Green();
        if (type == "SCHOOL") return termgl::Color::Blue();
        if (type == "HOSPITAL") return termgl::Color::Red();
        if (type == "PHARMACY") return termgl::Color(255, 0, 255);
        if (type == "MALL" ) return termgl::Color::Yellow();
        if (type == "MOSQUE") return termgl::Color::Cyan();
        if (type == "PARK" ) return termgl::Color(0, 100, 0);
        if (type == "POLICE_STATION") return termgl::Color(100, 0, 0);
        if (type == "FIRE_STATION") return termgl::Color(255, 100, 0);
        if (type == "LIBRARY") return termgl::Color(0, 0, 100);
        if (type == "ATM" ) return termgl::Color(200, 200, 0);
        if (type == "RESTAURANT") return termgl::Color(255, 165, 0);
        if (type == "HOUSE" ) return termgl::Color(100, 100, 100);
        return termgl::Color::White();
    }

    // Parse grid position from corner node name (e.g., "C-F7-R2-C3" -> row=2, col=3)
    void parseCornerGridPosition(const string& name, int& row, int& col) {
        row = -1; col = -1;
        size_t rPos = name.find("-R");
        size_t cPos = name.find("-C", rPos);
        if (rPos != string::npos && cPos != string::npos) {
            try {
                row = std::stoi(name.substr(rPos + 2, cPos - rPos - 2));
                col = std::stoi(name.substr(cPos + 2));
            } catch (...) {}
        }
    }

    // Classify road based on corner grid positions
    RoadType classifySkeletonRoad(int row1, int col1, int row2, int col2) {
        // Check if edge is on sector boundary (perimeter)
        bool onBoundary1 = (row1 == 0 || row1 == 4 || col1 == 0 || col1 == 4);
        bool onBoundary2 = (row2 == 0 || row2 == 4 || col2 == 0 || col2 == 4);
        
        if (onBoundary1 && onBoundary2) {
            // Both endpoints on boundary = sector boundary road
            return RoadType::SECTOR_BOUNDARY;
        }
        
        // Check if edge involves center lines (row 2 or col 2) - divides into 4 quadrants
        bool onCenter1 = (row1 == 2 || col1 == 2);
        bool onCenter2 = (row2 == 2 || col2 == 2);
        
        if (onCenter1 && onCenter2) {
            return RoadType::SUB_SECTOR;
        }
        
        // Everything else is sub-sub-sector
        return RoadType::SUB_SUB_SECTOR;
    }

public:
    CityGraphView(SmartCity* cityPtr) : city(cityPtr),
        mouseX(0), mouseY(0), hoveredNodeID(-1), hoveredSector(""),
        showCorners(true), showRoads(true), showSectorBounds(false),
        showHouses(true), showTraffic(true), trafficPaused(false),
        showCongestionHeatmap(false), showRealVehicles(false), showCitizens(false),
        useAgentSimulation(false), selectedVehicleIndex(-1), selectedCitizenIndex(-1),
        godModeEnabled(false),
        dijkstraMode(DijkstraMode::SELECT_START), inDijkstraMode(false),
        dijkstraStartNode(-1), dijkstraEndNode(-1), dijkstraDistance(0.0),
        dijkstraNodeSelection(0), dijkstraEndNodeSelection(0),
        intersectionCounter(0) {
    }

    void loadResources() {
        auto loadAsset = [](termgl::Texture& tex, termgl::Sprite& spr, const string& filename) {
            std::vector<string> paths = {
                "assets/" + filename,
                "Simulator/assets/" + filename,
                "source/Simulator/assets/" + filename,
                "Smart_City/assets/" + filename,
                "Smart_City/source/Simulator/assets/" + filename,
                "../assets/" + filename,
                filename
            };

            for (const auto& path : paths) {
                std::ifstream f(path.c_str());
                if (f.good()) {
                    f.close();
                    if (tex.loadFromFile(path)) {
                        spr.setTexture(&tex);
                        std::cout << "Loaded: " << path << std::endl;
                        return;
                    }
                }
            }
        };

        loadAsset(texSchool, sprSchool, "school.png");
        loadAsset(texHospital, sprHospital, "hospital.png");
        loadAsset(texPharmacy, sprPharmacy, "pharmacy.png");
        loadAsset(texStop, sprStop, "stop.png");
        loadAsset(texMall, sprMall, "mall.png");
        loadAsset(texMosque, sprMosque, "mosque.png");
        loadAsset(texPark, sprPark, "park.png");
        loadAsset(texPolice, sprPolice, "police.png");
        loadAsset(texFire, sprFire, "fire.png");
        loadAsset(texLibrary, sprLibrary, "library.png");
        loadAsset(texRestaurant, sprRestaurant, "restaurant.png");
        loadAsset(texHouse, sprHouse, "house.png");
        loadAsset(texBus, sprBus, "bus.png");
        loadAsset(texCar, sprCar, "car.png");
        loadAsset(texDefault, sprDefault, "default.png");
        loadAsset(texAmbulance, sprAmbulance, "ambulance.png");
        loadAsset(texRickshaw, sprRickshaw, "rickshaw.png");
    }

    void buildGraphVisualization() {
        graphNodes.clear();
        graphEdges.clear();
        sectorRegions.clear();
        nodeIdToIndex.clear();
        intersectionCounter = 0;

        if (!city || !city->getCityGraph()) return;
        CityGraph* graph = city->getCityGraph();

        viewport.setBounds(BASE_LAT, MAX_LAT, BASE_LON, MAX_LON);

        int maxId = 0;
        for (int i = 0; i < graph->getNodeCount(); i++) {
            CityNode* node = graph->getNode(i);
            if (node && node->id > maxId) maxId = node->id;
        }
        nodeIdToIndex.resize(maxId + 1, -1);

        // Build nodes
        for (int i = 0; i < graph->getNodeCount(); i++) {
            CityNode* node = graph->getNode(i);
            if (!node) continue;

            GraphNode2D gNode;
            gNode.id = node->id;
            gNode.lat = node->lat;
            gNode.lon = node->lon;
            gNode.pos = Point2D(0, 0);
            gNode.type = node->type;
            gNode.sector = node->sector;
            gNode.color = getNodeColor(node->type);
            gNode.isCorner = (node->type == "CORNER");
            gNode.isOnPath = false;
            gNode.isVisited = false;
            gNode.isStart = false;
            gNode.isEnd = false;

            if (gNode.isCorner) {
                intersectionCounter++;
                gNode.name = node->databaseID;  // Keep original ID for parsing
                parseCornerGridPosition(node->databaseID, gNode.gridRow, gNode.gridCol);
            }
            else {
                gNode.name = node->name;
            }

            nodeIdToIndex[node->id] = (int)graphNodes.getSize();
            graphNodes.push_back(gNode);
        }

        // Build edges with proper classification
        for (int i = 0; i < graph->getNodeCount(); i++) {
            CityNode* node = graph->getNode(i);
            if (!node) continue;

            const LinkedList<Edge>& roads = node->getRoads();
            for (int j = 0; j < roads.size(); j++) {
                Edge edge = roads.at(j);
                if (node->id < edge.destinationID) {
                    CityNode* destNode = graph->getNode(edge.destinationID);
                    if (!destNode) continue;

                    GraphEdge2D gEdge(node->id, edge.destinationID);
                    gEdge.isOnPath = false;
                    gEdge.fromSector = node->sector;
                    gEdge.toSector = destNode->sector;

                    bool fromIsCorner = (node->type == "CORNER");
                    bool toIsCorner = (destNode->type == "CORNER");

                    if (fromIsCorner && toIsCorner) {
                        // Both corners - classify based on grid position
                        int idx1 = nodeIdToIndex[node->id];
                        int idx2 = nodeIdToIndex[edge.destinationID];
                        
                        if (idx1 >= 0 && idx2 >= 0) {
                            const GraphNode2D& n1 = graphNodes[idx1];
                            const GraphNode2D& n2 = graphNodes[idx2];
                            
                            if (n1.gridRow >= 0 && n2.gridRow >= 0) {
                                gEdge.roadType = classifySkeletonRoad(
                                    n1.gridRow, n1.gridCol, n2.gridRow, n2.gridCol);
                            } else {
                                gEdge.roadType = RoadType::SUB_SUB_SECTOR;
                            }
                        }
                    }
                    else {
                        // At least one is a facility
                        gEdge.roadType = RoadType::FACILITY_ROAD;
                    }

                    graphEdges.push_back(gEdge);
                }
            }
        }

        // Build sector regions
        for (int i = 0; i < SECTOR_COUNT; i++) {
            SectorRegion region;
            region.name = SECTOR_GRID[i].name;
            region.minLat = SECTOR_GRID[i].minLat;
            region.maxLat = SECTOR_GRID[i].maxLat;
            region.minLon = SECTOR_GRID[i].minLon;
            region.maxLon = SECTOR_GRID[i].maxLon;
            region.isHovered = false;
            sectorRegions.push_back(region);
        }

        initializeTraffic();
    }

    void initializeTraffic() {
        trafficVehicles.clear();
        if (graphEdges.empty()) return;

        int numVehicles = std::min(80, (int)graphEdges.getSize() / 2);

        for (int i = 0; i < numVehicles; i++) {
            TrafficVehicle vehicle;
            int edgeIdx = rand() % graphEdges.getSize();
            vehicle.edgeFromID = graphEdges[edgeIdx].fromID;
            vehicle.edgeToID = graphEdges[edgeIdx].toID;
            vehicle.progress = (rand() % 100) / 100.0;
            vehicle.speed = 0.008 + (rand() % 15) / 1000.0;
            vehicle.isReal = false;

            int colorChoice = rand() % 5;
            if (colorChoice == 0) vehicle.color = termgl::Color::Yellow();
            else if (colorChoice == 1) vehicle.color = termgl::Color(255, 100, 0);
            else if (colorChoice == 2) vehicle.color = termgl::Color::Cyan();
            else if (colorChoice == 3) vehicle.color = termgl::Color::Green();
            else vehicle.color = termgl::Color(200, 200, 200);

            vehicle.isBus = (rand() % 5 == 0);
            trafficVehicles.push_back(vehicle);
        }
    }

    // ==================== PHASE 5: REAL VEHICLE SYNC ====================
    void syncRealVehicles() {
        if (!city || !showRealVehicles) return;
        
        trafficVehicles.clear();
        TransportManager* tm = city->getTransportManager();
        if (!tm) return;
        
        // Sync buses
        const Vector<Bus*>& buses = tm->getAllBuses();
        for (int i = 0; i < buses.getSize(); i++) {
            Bus* bus = buses[i];
            if (!bus) continue;
            
            TrafficVehicle tv;
            tv.edgeFromID = bus->getCurrentNodeID();
            tv.edgeToID = bus->getNextNodeID();
            tv.progress = bus->getProgressOnEdge();
            tv.isReal = true;
            tv.isBus = true;
            tv.isStuck = bus->getIsStuck();
            tv.vehicleID = bus->getID();
            tv.color = bus->getIsStuck() ? termgl::Color::Red() : termgl::Color::Green();
            
            trafficVehicles.push_back(tv);
        }
        
        // Sync ambulances
        const Vector<Ambulance*>& ambulances = tm->getAllAmbulances();
        for (int i = 0; i < ambulances.getSize(); i++) {
            Ambulance* amb = ambulances[i];
            if (!amb || amb->isAvailable()) continue;
            
            TrafficVehicle tv;
            tv.edgeFromID = amb->getCurrentNodeID();
            tv.edgeToID = amb->getNextNodeID();
            tv.progress = amb->getProgressOnEdge();
            tv.isReal = true;
            tv.isBus = false;
            tv.isStuck = amb->getIsStuck();
            tv.vehicleID = amb->getID();
            tv.color = termgl::Color(255, 0, 0);  // Red for ambulance
            
            trafficVehicles.push_back(tv);
        }
        
        // Sync school buses
        const Vector<SchoolBus*>& schoolBuses = tm->getAllSchoolBuses();
        for (int i = 0; i < schoolBuses.getSize(); i++) {
            SchoolBus* sb = schoolBuses[i];
            if (!sb || sb->isAvailable()) continue;
            
            TrafficVehicle tv;
            tv.edgeFromID = sb->getCurrentNodeID();
            tv.edgeToID = sb->getNextNodeID();
            tv.progress = sb->getProgressOnEdge();
            tv.isReal = true;
            tv.isBus = true;
            tv.isStuck = sb->getIsStuck();
            tv.vehicleID = sb->getID();
            tv.color = termgl::Color::Yellow();
            
            trafficVehicles.push_back(tv);
        }
    }
    
    // ==================== PHASE 5: CITIZEN SYNC ====================
    void syncCitizens() {
        citizenRenderList.clear();
        if (!city || !showCitizens) return;
        
        PopulationManager* pm = city->getPopulationManager();
        if (!pm) return;
        
        const Vector<Citizen*>& citizens = pm->masterList;
        for (int i = 0; i < citizens.getSize(); i++) {
            Citizen* c = citizens[i];
            if (!c) continue;
            
            // Only render citizens who are moving
            if (c->state != CitizenState::WALKING && 
                c->state != CitizenState::WAITING_FOR_BUS &&
                c->state != CitizenState::WAITING_FOR_RIDE) continue;
            
            CitizenRenderData crd;
            crd.lat = c->lat;
            crd.lon = c->lon;
            crd.name = c->name;
            crd.state = c->getStateString();
            crd.thought = c->getThought();
            crd.isSelected = (i == selectedCitizenIndex);
            
            // Color based on state
            if (c->needs.isCriticallyHungry() || c->needs.isCritical()) {
                crd.color = termgl::Color::Red();
            } else if (c->state == CitizenState::WALKING) {
                crd.color = termgl::Color::Blue();
            } else if (c->state == CitizenState::WAITING_FOR_BUS) {
                crd.color = termgl::Color::Cyan();
            } else {
                crd.color = termgl::Color::Green();
            }
            
            citizenRenderList.push_back(crd);
        }
    }

    void clearDijkstraVisualization() {
        for (int i = 0; i < graphNodes.getSize(); i++) {
            graphNodes[i].isOnPath = false;
            graphNodes[i].isVisited = false;
            graphNodes[i].isStart = false;
            graphNodes[i].isEnd = false;
        }
        for (int i = 0; i < graphEdges.getSize(); i++) {
            graphEdges[i].isOnPath = false;
        }
        dijkstraPath.clear();
        dijkstraDistance = 0.0;
        dijkstraStartNode = -1;
        dijkstraEndNode = -1;
    }

    void runDijkstraAlgorithm() {
        if (!city || !city->getCityGraph() || dijkstraStartNode < 0) return;
        CityGraph* graph = city->getCityGraph();

        if (dijkstraTargetType == "SCHOOL" || dijkstraTargetType == "HOSPITAL" ||
            dijkstraTargetType == "PHARMACY" || dijkstraTargetType == "STOP") {
            dijkstraEndNode = graph->findNearestFacility(dijkstraStartNode, dijkstraTargetType);
        }

        if (dijkstraEndNode < 0) return;
        dijkstraPath = graph->findShortestPath(dijkstraStartNode, dijkstraEndNode, dijkstraDistance);
        applyPathToVisualization();
    }

    void runDijkstraPointToPoint() {
        if (!city || !city->getCityGraph() || dijkstraStartNode < 0 || dijkstraEndNode < 0) return;
        CityGraph* graph = city->getCityGraph();
        dijkstraPath = graph->findShortestPath(dijkstraStartNode, dijkstraEndNode, dijkstraDistance);
        applyPathToVisualization();
    }

    void applyPathToVisualization() {
        for (int i = 0; i < dijkstraPath.getSize(); i++) {
            int nodeId = dijkstraPath[i];
            if (nodeId < nodeIdToIndex.getSize()) {
                int idx = nodeIdToIndex[nodeId];
                if (idx >= 0 && idx < graphNodes.getSize()) {
                    graphNodes[idx].isOnPath = true;
                }
            }
        }
        if (dijkstraStartNode < nodeIdToIndex.getSize()) {
            int idx = nodeIdToIndex[dijkstraStartNode];
            if (idx >= 0) graphNodes[idx].isStart = true;
        }
        if (dijkstraEndNode < nodeIdToIndex.getSize()) {
            int idx = nodeIdToIndex[dijkstraEndNode];
            if (idx >= 0) graphNodes[idx].isEnd = true;
        }
        for (int i = 0; i < dijkstraPath.getSize() - 1; i++) {
            int from = dijkstraPath[i];
            int to = dijkstraPath[i + 1];
            for (int j = 0; j < graphEdges.getSize(); j++) {
                GraphEdge2D& edge = graphEdges[j];
                if ((edge.fromID == from && edge.toID == to) || 
                    (edge.fromID == to && edge.toID == from)) {
                    edge.isOnPath = true;
                }
            }
        }
    }

    // ========================================================================
    // DRAWING HELPERS
    // ========================================================================

    void drawThickLine(termgl::Window& window, int x1, int y1, int x2, int y2, int thickness, termgl::Color c) {
        if (thickness <= 1) {
            window.drawLine(x1, y1, x2, y2, c);
            return;
        }

        double dx = x2 - x1;
        double dy = y2 - y1;
        double len = std::sqrt(dx * dx + dy * dy);
        if (len == 0) return;

        double nx = -dy / len;
        double ny = dx / len;

        for (int i = -thickness / 2; i <= thickness / 2; i++) {
            int ox = (int)(nx * i);
            int oy = (int)(ny * i);
            window.drawLine(x1 + ox, y1 + oy, x2 + ox, y2 + oy, c);
        }
    }

    void drawDashedLine(termgl::Window& window, int x1, int y1, int x2, int y2, termgl::Color c, double dashLen = 10.0) {
        double dx = x2 - x1;
        double dy = y2 - y1;
        double dist = std::sqrt(dx * dx + dy * dy);
        if (dist == 0) return;

        double nx = dx / dist;
        double ny = dy / dist;
        double gapLen = dashLen;

        double current = 0;
        while (current < dist) {
            double end = std::min(current + dashLen, dist);
            window.drawLine(
                (int)(x1 + nx * current), (int)(y1 + ny * current),
                (int)(x1 + nx * end), (int)(y1 + ny * end), c);
            current += dashLen + gapLen;
        }
    }

    // ========================================================================
    // ROAD STYLING - Zoom dependent thickness
    // ========================================================================

    void getRoadStyle(RoadType roadType, double zoom, int& thickness, 
                      termgl::Color& roadColor, termgl::Color& stripeColor, bool& visible) {
        double scale = std::max(1.0, zoom * 0.6);
        visible = true;

        switch (roadType) {
            case RoadType::SECTOR_BOUNDARY:
                // Thickest - always visible, yellow stripe
                thickness = (int)(4 * scale);
                roadColor = termgl::Color(70, 70, 80);
                stripeColor = termgl::Color(255, 200, 0);
                break;

            case RoadType::SUB_SECTOR:
                // Medium - always visible, white stripe
                thickness = (int)(3 * scale);
                roadColor = termgl::Color(55, 55, 65);
                stripeColor = termgl::Color(180, 180, 180);
                break;

            case RoadType::SUB_SUB_SECTOR:
                // Thinner - always visible at base, no stripe at low zoom
                thickness = (int)(2 * scale);
                roadColor = termgl::Color(45, 45, 55);
                stripeColor = termgl::Color(120, 120, 120);
                break;

            case RoadType::FACILITY_ROAD:
                // Thinnest - only visible when zoomed in
                if (zoom < 1.5) {
                    visible = false;
                    thickness = 0;
                } else {
                    thickness = (int)(1 * scale);
                    roadColor = termgl::Color(40, 40, 50);
                    stripeColor = termgl::Color(80, 80, 80);
                }
                break;
        }

        // Minimum thickness of 1 if visible
        if (visible && thickness < 1) thickness = 1;
    }

    // ========================================================================
    // RENDERING
    // ========================================================================

    void renderGraph(termgl::Window& window) {
        int cw = window.getWidth();
        int ch = window.getHeight();
        viewport.setCanvasSize(cw, ch);

        double zoom = viewport.getZoom();
        double scale = viewport.getScaleFactor();

        // Pre-calculate all positions
        for (int i = 0; i < graphNodes.getSize(); i++) {
            graphNodes[i].pos = viewport.geoToCanvas(graphNodes[i].lat, graphNodes[i].lon);
        }

        for (int i = 0; i < sectorRegions.getSize(); i++) {
            SectorRegion& region = sectorRegions[i];
            Point2D tl = viewport.geoToCanvas(region.maxLat, region.minLon);
            Point2D br = viewport.geoToCanvas(region.minLat, region.maxLon);
            region.topLeft = Point2D(std::min(tl.x, br.x), std::min(tl.y, br.y));
            region.bottomRight = Point2D(std::max(tl.x, br.x), std::max(tl.y, br.y));
            region.center = Point2D((region.topLeft.x + region.bottomRight.x) / 2,
                (region.topLeft.y + region.bottomRight.y) / 2);
        }

        // LAYER 1: Sector boundaries (optional)
        if (showSectorBounds) {
            renderSectorBoundaries(window, zoom);
        }

        // LAYER 2: Roads (hierarchical, back to front)
        if (showRoads) {
            renderRoadsByType(window, RoadType::FACILITY_ROAD, zoom);
            renderRoadsByType(window, RoadType::SUB_SUB_SECTOR, zoom);
            renderRoadsByType(window, RoadType::SUB_SECTOR, zoom);
            renderRoadsByType(window, RoadType::SECTOR_BOUNDARY, zoom);
            
            // Path highlight on top
            renderPathHighlight(window, zoom);
        }

        // LAYER 2.5: Congestion Heatmap (Phase 5)
        if (showCongestionHeatmap) {
            renderCongestionHeatmap(window, zoom);
        }

        // LAYER 3: Traffic - use real or fake based on mode
        if (showTraffic || showRealVehicles) {
            if (showRealVehicles) {
                renderRealTraffic(window, zoom);
            } else if (!trafficPaused && dijkstraPath.getSize() == 0) {
                renderTraffic(window, zoom);
            }
        }

        // LAYER 3.5: Citizens (Phase 5)
        if (showCitizens) {
            renderCitizens(window, zoom);
        }

        // LAYER 4: Houses
        if (showHouses && dijkstraPath.getSize() == 0) {
            renderHouses(window, zoom);
        }

        // LAYER 5: Corners
        if (showCorners) {
            renderCorners(window, zoom);
        }

        // LAYER 6: Facilities
        renderFacilities(window, zoom);
    }

    void renderSectorBoundaries(termgl::Window& window, double zoom) {
        for (int i = 0; i < sectorRegions.getSize(); i++) {
            const SectorRegion& region = sectorRegions[i];

            if (!viewport.isVisible(region.topLeft, 0) && 
                !viewport.isVisible(region.bottomRight, 0)) continue;

            int x1 = (int)region.topLeft.x;
            int y1 = (int)region.topLeft.y;
            int w = (int)(region.bottomRight.x - region.topLeft.x);
            int h = (int)(region.bottomRight.y - region.topLeft.y);

            termgl::Color boundColor = region.isHovered ? 
                termgl::Color::Yellow() : termgl::Color(60, 60, 70);

            window.drawRect(x1, y1, w, h, boundColor);

            // Draw sector name at center
            if (zoom > 0.8 || region.isHovered) {
                termgl::Color textColor = region.isHovered ? 
                    termgl::Color::Yellow() : termgl::Color(120, 120, 140);
                int textX = (int)region.center.x - (region.name.length() * 4);
                int textY = (int)region.center.y - 8;
                window.drawText(textX, textY, region.name, textColor);
            }
        }
    }

    void renderRoadsByType(termgl::Window& window, RoadType roadType, double zoom) {
        int thickness;
        termgl::Color roadColor, stripeColor;
        bool visible;
        
        getRoadStyle(roadType, zoom, thickness, roadColor, stripeColor, visible);
        if (!visible) return;

        bool drawStripes = (zoom > 2.0) && (roadType != RoadType::FACILITY_ROAD);

        for (int i = 0; i < graphEdges.getSize(); i++) {
            const GraphEdge2D& edge = graphEdges[i];
            
            if (edge.roadType != roadType) continue;
            if (edge.isOnPath) continue;
            if (dijkstraPath.getSize() > 0 && dijkstraMode == DijkstraMode::COMPLETE) continue;

            int idx1 = (edge.fromID < nodeIdToIndex.getSize()) ? nodeIdToIndex[edge.fromID] : -1;
            int idx2 = (edge.toID < nodeIdToIndex.getSize()) ? nodeIdToIndex[edge.toID] : -1;
            if (idx1 < 0 || idx2 < 0) continue;

            const GraphNode2D& n1 = graphNodes[idx1];
            const GraphNode2D& n2 = graphNodes[idx2];

            if (!viewport.isVisible(n1.pos) && !viewport.isVisible(n2.pos)) continue;

            drawThickLine(window, (int)n1.pos.x, (int)n1.pos.y, 
                          (int)n2.pos.x, (int)n2.pos.y, thickness, roadColor);
            
            if (drawStripes && thickness >= 3) {
                drawDashedLine(window, (int)n1.pos.x, (int)n1.pos.y,
                               (int)n2.pos.x, (int)n2.pos.y, stripeColor);
            }
        }
    }

    void renderPathHighlight(termgl::Window& window, double zoom) {
        double scale = viewport.getScaleFactor();
        int pathThickness = (int)(6 * scale);

        for (int i = 0; i < graphEdges.getSize(); i++) {
            const GraphEdge2D& edge = graphEdges[i];
            if (!edge.isOnPath) continue;

            int idx1 = (edge.fromID < nodeIdToIndex.getSize()) ? nodeIdToIndex[edge.fromID] : -1;
            int idx2 = (edge.toID < nodeIdToIndex.getSize()) ? nodeIdToIndex[edge.toID] : -1;
            if (idx1 < 0 || idx2 < 0) continue;

            const GraphNode2D& n1 = graphNodes[idx1];
            const GraphNode2D& n2 = graphNodes[idx2];

            if (!viewport.isVisible(n1.pos) && !viewport.isVisible(n2.pos)) continue;

            // Glow effect
            drawThickLine(window, (int)n1.pos.x, (int)n1.pos.y, (int)n2.pos.x, (int)n2.pos.y, 
                          pathThickness + 4, termgl::Color(0, 40, 0));
            drawThickLine(window, (int)n1.pos.x, (int)n1.pos.y, (int)n2.pos.x, (int)n2.pos.y, 
                          pathThickness, termgl::Color(0, 150, 0));
            drawDashedLine(window, (int)n1.pos.x, (int)n1.pos.y, (int)n2.pos.x, (int)n2.pos.y, 
                           termgl::Color::Green());
        }
    }

    void renderTraffic(termgl::Window& window, double zoom) {
        double scale = viewport.getScaleFactor();
        int vehicleRadius = (int)(3 * scale);

        for (int i = 0; i < trafficVehicles.getSize(); i++) {
            const TrafficVehicle& vehicle = trafficVehicles[i];
            int idx1 = (vehicle.edgeFromID < nodeIdToIndex.getSize()) ? 
                       nodeIdToIndex[vehicle.edgeFromID] : -1;
            int idx2 = (vehicle.edgeToID < nodeIdToIndex.getSize()) ? 
                       nodeIdToIndex[vehicle.edgeToID] : -1;

            if (idx1 >= 0 && idx2 >= 0) {
                const GraphNode2D& n1 = graphNodes[idx1];
                const GraphNode2D& n2 = graphNodes[idx2];
                int vx = (int)(n1.pos.x + (n2.pos.x - n1.pos.x) * vehicle.progress);
                int vy = (int)(n1.pos.y + (n2.pos.y - n1.pos.y) * vehicle.progress);

                if (!viewport.isVisible(Point2D(vx, vy))) continue;

                window.fillCircle(vx, vy, vehicleRadius, vehicle.color);
            }
        }
    }

    void renderHouses(termgl::Window& window, double zoom) {
        if (zoom < 2.0) return;  // Only show houses when zoomed in
        
        double scale = viewport.getScaleFactor();
        int houseRadius = (int)(2 * scale);

        for (int i = 0; i < graphNodes.getSize(); i++) {
            const GraphNode2D& node = graphNodes[i];
            if (node.type != "HOUSE") continue;
            if (!viewport.isVisible(node.pos)) continue;

            window.fillCircle((int)node.pos.x, (int)node.pos.y, houseRadius, 
                              termgl::Color(80, 80, 80));
        }
    }

    void renderCorners(termgl::Window& window, double zoom) {
        double scale = viewport.getScaleFactor();
        int cornerRadius = (int)(2 * scale);

        for (int i = 0; i < graphNodes.getSize(); i++) {
            const GraphNode2D& node = graphNodes[i];
            if (!node.isCorner) continue;
            if (dijkstraPath.getSize() > 0 && !node.isOnPath) continue;
            if (!viewport.isVisible(node.pos)) continue;

            termgl::Color c = node.isOnPath ? termgl::Color::Green() : termgl::Color(90, 90, 100);
            window.fillCircle((int)node.pos.x, (int)node.pos.y, cornerRadius, c);
        }
    }

    void renderFacilities(termgl::Window& window, double zoom) {
        double scale = viewport.getScaleFactor();
        int baseRadius = (int)(5 * scale);
        float spriteSize = 24.0f * (float)scale;

        for (int i = 0; i < graphNodes.getSize(); i++) {
            const GraphNode2D& node = graphNodes[i];

            if (!viewport.isVisible(node.pos)) continue;
            if (node.isCorner || node.type == "HOUSE") continue;
            if (dijkstraPath.getSize() > 0 && !node.isOnPath && !node.isStart && !node.isEnd) continue;

            termgl::Sprite* s = nullptr;
            if (node.type == "SCHOOL") s = &sprSchool;
            else if (node.type == "HOSPITAL") s = &sprHospital;
            else if (node.type == "PHARMACY") s = &sprPharmacy;
            else if (node.type == "STOP") s = &sprStop;
            else if (node.type == "MALL") s = &sprMall;
            else if (node.type == "MOSQUE") s = &sprMosque;
            else if (node.type == "PARK") s = &sprPark;
            else if (node.type == "POLICE_STATION") s = &sprPolice;
            else if (node.type == "FIRE_STATION") s = &sprFire;
            else if (node.type == "LIBRARY") s = &sprLibrary;
            else if (node.type == "RESTAURANT") s = &sprRestaurant;

            bool spriteDrawn = false;
            float targetSize = (node.isStart || node.isEnd) ? spriteSize * 1.5f : spriteSize;

            if (zoom > 1.5 && s != nullptr && s->texture != nullptr && s->texture->width > 0) {
                float sprScale = targetSize / s->texture->width;
                s->setPosition((float)node.pos.x - (targetSize / 2),
                               (float)node.pos.y - (targetSize / 2));
                s->setScale(sprScale);
                window.drawSprite(*s);
                spriteDrawn = true;
            }

            if (!spriteDrawn) {
                termgl::Color nodeColor = node.color;
                int radius = baseRadius;

                if (node.isStart) { 
                    nodeColor = termgl::Color::Cyan(); 
                    radius = (int)(baseRadius * 1.5);
                }
                else if (node.isEnd) { 
                    nodeColor = termgl::Color::Yellow(); 
                    radius = (int)(baseRadius * 1.5);
                }
                else if (node.isOnPath) { 
                    nodeColor = termgl::Color::Green(); 
                    radius = (int)(baseRadius * 1.2);
                }

                window.fillCircle((int)node.pos.x, (int)node.pos.y, radius, nodeColor);
                window.drawCircle((int)node.pos.x, (int)node.pos.y, radius + 1, termgl::Color::White());
            }

            if (node.isStart) {
                int markerR = (int)(baseRadius * 2);
                window.drawCircle((int)node.pos.x, (int)node.pos.y, markerR, termgl::Color::Green());
                window.drawText((int)node.pos.x + markerR + 5, (int)node.pos.y - 8, "START", termgl::Color::Green());
            }
            if (node.isEnd) {
                int markerR = (int)(baseRadius * 2);
                window.drawCircle((int)node.pos.x, (int)node.pos.y, markerR, termgl::Color::Red());
                window.drawText((int)node.pos.x + markerR + 5, (int)node.pos.y - 8, "END", termgl::Color::Red());
            }

            if (node.id == hoveredNodeID) {
                window.drawText((int)node.pos.x + 15, (int)node.pos.y - 10, node.name, termgl::Color::White());
            }
        }
    }

    // ==================== PHASE 5: CONGESTION HEATMAP ====================
    termgl::Color getCongestionColor(double congestion) {
        // Green (0.0) -> Yellow (0.5) -> Red (1.0)
        if (congestion <= 0.0) return termgl::Color(0, 100, 0);   // Dark green
        if (congestion >= 1.0) return termgl::Color(200, 0, 0);   // Dark red
        
        if (congestion < 0.5) {
            // Green to Yellow
            int r = (int)(congestion * 2.0 * 255);
            return termgl::Color(r, 200, 0);
        } else {
            // Yellow to Red
            int g = (int)((1.0 - congestion) * 2.0 * 200);
            return termgl::Color(255, g, 0);
        }
    }
    
    void renderCongestionHeatmap(termgl::Window& window, double zoom) {
        if (!city || !city->getCityGraph()) return;
        CityGraph* graph = city->getCityGraph();
        
        double scale = viewport.getScaleFactor();
        
        for (int i = 0; i < graphEdges.getSize(); i++) {
            const GraphEdge2D& edge = graphEdges[i];
            
            // Only show heatmap on skeleton roads
            if (edge.roadType == RoadType::FACILITY_ROAD) continue;
            
            int idx1 = (edge.fromID < nodeIdToIndex.getSize()) ? nodeIdToIndex[edge.fromID] : -1;
            int idx2 = (edge.toID < nodeIdToIndex.getSize()) ? nodeIdToIndex[edge.toID] : -1;
            if (idx1 < 0 || idx2 < 0) continue;
            
            const GraphNode2D& n1 = graphNodes[idx1];
            const GraphNode2D& n2 = graphNodes[idx2];
            
            if (!viewport.isVisible(n1.pos) && !viewport.isVisible(n2.pos)) continue;
            
            // Get congestion for this edge
            double congestion = graph->getEdgeCongestion(edge.fromID, edge.toID);
            
            if (congestion < 0.01) continue;  // Skip non-congested roads
            
            termgl::Color heatColor = getCongestionColor(congestion);
            int thickness = (int)(4 * scale * (0.5 + congestion * 0.5));
            
            drawThickLine(window, (int)n1.pos.x, (int)n1.pos.y, 
                          (int)n2.pos.x, (int)n2.pos.y, thickness, heatColor);
        }
    }

    void renderRealTraffic(termgl::Window& window, double zoom) {
        double scale = viewport.getScaleFactor();
        int vehicleRadius = (int)(5 * scale);
        
        for (int i = 0; i < trafficVehicles.getSize(); i++) {
            const TrafficVehicle& vehicle = trafficVehicles[i];
            
            int idx1 = (vehicle.edgeFromID >= 0 && vehicle.edgeFromID < nodeIdToIndex.getSize()) ? 
                       nodeIdToIndex[vehicle.edgeFromID] : -1;
            int idx2 = (vehicle.edgeToID >= 0 && vehicle.edgeToID < nodeIdToIndex.getSize()) ? 
                       nodeIdToIndex[vehicle.edgeToID] : -1;
            
            Point2D pos;
            
            if (idx1 >= 0 && idx2 >= 0) {
                const GraphNode2D& n1 = graphNodes[idx1];
                const GraphNode2D& n2 = graphNodes[idx2];
                double t = vehicle.progress;
                if (t > 1.0) t = 1.0;
                if (t < 0.0) t = 0.0;
                pos.x = n1.pos.x + (n2.pos.x - n1.pos.x) * t;
                pos.y = n1.pos.y + (n2.pos.y - n1.pos.y) * t;
            } else if (idx1 >= 0) {
                pos = graphNodes[idx1].pos;
            } else {
                continue;
            }
            
            if (!viewport.isVisible(pos)) continue;
            
            int vx = (int)pos.x;
            int vy = (int)pos.y;
            
            // Draw vehicle body
            int r = vehicle.isBus ? vehicleRadius + 2 : vehicleRadius;
            window.fillCircle(vx, vy, r, vehicle.color);
            
            // Outline
            window.drawCircle(vx, vy, r + 1, termgl::Color::White());
            
            // Stuck indicator
            if (vehicle.isStuck) {
                window.drawText(vx - 3, vy - r - 12, "!", termgl::Color::Red());
            }
            
            // Selection highlight
            if (i == selectedVehicleIndex) {
                window.drawCircle(vx, vy, r + 4, termgl::Color::Cyan());
                window.drawCircle(vx, vy, r + 5, termgl::Color::Cyan());
            }
        }
    }

    void renderCitizens(termgl::Window& window, double zoom) {
        if (zoom < 2.5) return;  // Only render when zoomed in
        
        double scale = viewport.getScaleFactor();
        int citizenRadius = (int)(3 * scale);
        
        for (int i = 0; i < citizenRenderList.getSize(); i++) {
            CitizenRenderData& crd = citizenRenderList[i];
            crd.pos = viewport.geoToCanvas(crd.lat, crd.lon);
            
            if (!viewport.isVisible(crd.pos)) continue;
            
            int cx = (int)crd.pos.x;
            int cy = (int)crd.pos.y;
            
            // Draw citizen dot
            window.fillCircle(cx, cy, citizenRadius, crd.color);
            
            // Selection highlight
            if (crd.isSelected) {
                window.drawCircle(cx, cy, citizenRadius + 3, termgl::Color::Yellow());
                window.drawText(cx + 10, cy - 8, crd.name, termgl::Color::Yellow());
                window.drawText(cx + 10, cy + 4, crd.thought, termgl::Color::Grey());
            }
        }
    }

    void updateHoverState(int mx, int my) {
        mouseX = mx;
        mouseY = my;
        hoveredNodeID = -1;
        hoveredSector = "";

        double minDist = 15.0 * viewport.getScaleFactor();

        for (int i = 0; i < graphNodes.getSize(); i++) {
            const GraphNode2D& node = graphNodes[i];
            if (!viewport.isVisible(node.pos)) continue;
            if (node.isCorner && !showCorners) continue;
            if (node.type == "HOUSE" && !showHouses) continue;

            double dx = node.pos.x - mx;
            double dy = node.pos.y - my;
            if (std::abs(dx) > 30 || std::abs(dy) > 30) continue;

            double dist = std::sqrt(dx * dx + dy * dy);
            if (dist < minDist) {
                minDist = dist;
                hoveredNodeID = node.id;
            }
        }

        Point2D p(mx, my);
        for (int i = 0; i < sectorRegions.getSize(); i++) {
            SectorRegion& region = sectorRegions[i];
            region.isHovered = region.contains(p);
            if (region.isHovered) {
                hoveredSector = region.name;
            }
        }
    }

    string getHoverInfo() {
        std::stringstream ss;
        if (hoveredNodeID >= 0 && hoveredNodeID < nodeIdToIndex.getSize()) {
            int idx = nodeIdToIndex[hoveredNodeID];
            if (idx >= 0 && idx < graphNodes.getSize()) {
                const GraphNode2D& node = graphNodes[idx];
                ss << node.name << "\n";
                ss << "Type: " << node.type << "\n";
                ss << "Sector: " << node.sector;
                return ss.str();
            }
        }
        if (!hoveredSector.empty()) {
            ss << "SECTOR: " << hoveredSector;
            return ss.str();
        }
        ss << "Hover over nodes";
        return ss.str();
    }

    void buildSelectableNodesList() {
        selectableNodes.clear();
        for (int i = 0; i < graphNodes.getSize(); i++) {
            if (!graphNodes[i].isCorner) {
                selectableNodes.push_back(graphNodes[i].id);
            }
        }
    }

    void updateTraffic() {
        if (trafficPaused || trafficVehicles.empty()) return;

        for (int i = 0; i < trafficVehicles.getSize(); i++) {
            TrafficVehicle& vehicle = trafficVehicles[i];
            
            if (vehicle.isReal) continue;  // Skip real vehicles - they're updated by simulation
            
            vehicle.progress += vehicle.speed;

            if (vehicle.progress >= 1.0) {
                vehicle.progress = 0.0;
                int currentEndID = vehicle.edgeToID;
                Vector<int> connectedEdges;

                for (int j = 0; j < graphEdges.getSize(); j++) {
                    if (graphEdges[j].fromID == currentEndID || graphEdges[j].toID == currentEndID) {
                        connectedEdges.push_back(j);
                    }
                }

                if (connectedEdges.getSize() > 0) {
                    int nextEdgeIdx = connectedEdges[rand() % connectedEdges.getSize()];
                    const GraphEdge2D& nextEdge = graphEdges[nextEdgeIdx];
                    if (nextEdge.fromID == currentEndID) {
                        vehicle.edgeFromID = nextEdge.fromID;
                        vehicle.edgeToID = nextEdge.toID;
                    }
                    else {
                        vehicle.edgeFromID = nextEdge.toID;
                        vehicle.edgeToID = nextEdge.fromID;
                    }
                }
            }
        }
    }

    // ========================================================================
    // MAIN RUN LOOP
    // ========================================================================

    void run() {
        loadResources();
        buildGraphVisualization();
        buildSelectableNodesList();
        
        // Initialize with fake traffic if not using simulation
        if (!showRealVehicles) {
            initializeTraffic();
        }

        termgl::Window window(1600, 900, "Islamabad City Simulator - Interactive Map", true);
        window.setFramerateLimit(60);

        int width = window.getWidth();
        int height = window.getHeight();

        int mapPartition = window.addPartition(0, 0, (int)(width * 0.75), height, "City Map");
        int sidePartition = window.addPartition((int)(width * 0.75), 0, (int)(width * 0.25), height, "Control Panel");

        bool running = true;
        inDijkstraMode = false;

        std::vector<string> targetTypes = {
            "Nearest School", "Nearest Hospital", "Nearest Pharmacy", "Nearest Bus Stop", "Custom Location"
        };
        int targetSel = 0;
        int scrollOffset = 0;

        bool isDragging = false;
        int lastMouseX = 0, lastMouseY = 0;
        int dragStartX = 0, dragStartY = 0;

        viewport.setCanvasSize((int)(width * 0.75), height);
        
        int simulationTick = 0;

        while (running && window.processEvents()) {
            if (window.isKeyPressed(VK_ESCAPE)) {
                if (inDijkstraMode) {
                    inDijkstraMode = false;
                    clearDijkstraVisualization();
                    dijkstraMode = DijkstraMode::SELECT_START;
                    dijkstraNodeSelection = 0;
                    dijkstraEndNodeSelection = 0;
                    targetSel = 0;
                    scrollOffset = 0;
                }
                else {
                    running = false;
                }
            }

            // INPUT HANDLING
            window.setActivePartition(mapPartition);
            termgl::Vec2 mousePos = window.getMousePos();

            bool clickDetected = false;

            if (window.isMouseLeftDown()) {
                if (!isDragging) {
                    isDragging = true;
                    lastMouseX = mousePos.x;
                    lastMouseY = mousePos.y;
                    dragStartX = mousePos.x;
                    dragStartY = mousePos.y;
                }
                else {
                    int dx = mousePos.x - lastMouseX;
                    int dy = mousePos.y - lastMouseY;
                    viewport.drag(dx, dy);
                    lastMouseX = mousePos.x;
                    lastMouseY = mousePos.y;
                }
            }
            else {
                if (isDragging) {
                    isDragging = false;
                    if (std::abs(mousePos.x - dragStartX) < 5 && std::abs(mousePos.y - dragStartY) < 5) {
                        clickDetected = true;
                    }
                }
            }

            // Zooming
            int scroll = window.getMouseScrollDelta();
            if (scroll > 0) viewport.zoomIn();
            if (scroll < 0) viewport.zoomOut();

            if (window.isControlDown()) {
                if (window.isKeyPressed(VK_ADD) || window.isKeyPressed('=')) viewport.zoomIn();
                if (window.isKeyPressed(VK_SUBTRACT) || window.isKeyPressed('-')) viewport.zoomOut();
            }

            updateHoverState(mousePos.x, mousePos.y);

            if (clickDetected && hoveredNodeID != -1) {
                if (inDijkstraMode) {
                    if (dijkstraMode == DijkstraMode::SELECT_START) {
                        dijkstraStartNode = hoveredNodeID;
                        int idx = nodeIdToIndex[dijkstraStartNode];
                        if (idx >= 0) graphNodes[idx].isStart = true;
                        dijkstraMode = DijkstraMode::SELECT_TARGET_TYPE;
                    }
                    else if (dijkstraMode == DijkstraMode::SELECT_TARGET_TYPE || 
                             dijkstraMode == DijkstraMode::RUNNING) {
                        dijkstraEndNode = hoveredNodeID;
                        dijkstraTargetType = "CUSTOM";
                        runDijkstraPointToPoint();
                        dijkstraMode = DijkstraMode::COMPLETE;
                    }
                }
            }

            // ==================== SIMULATION UPDATE ====================
            if (!trafficPaused) {
                simulationTick++;
                
                // Run transport simulation every few frames
                if (useAgentSimulation && city && city->getTransportManager()) {
                    if (simulationTick % 3 == 0) {  // Every 3 frames
                        city->getTransportManager()->runSimulationStep();
                        if (city->getCityGraph()) {
                            city->getCityGraph()->updateTrafficWeights();
                        }
                    }
                }
                
                // Sync visualization with simulation
                if (showRealVehicles) {
                    syncRealVehicles();
                    syncCitizens();
                } else {
                    updateTraffic();
                }
            }

            // RENDERING
            window.setActivePartition(-1);
            window.clear(termgl::Color(0, 0, 0));
            window.drawPartitionFrames();

            window.setActivePartition(mapPartition);
            window.clear(termgl::Color(10, 10, 15));
            renderGraph(window);

            // SIDE PANEL
            window.setActivePartition(sidePartition);
            window.clear(termgl::Color(0, 0, 0));

            int cy = 10;
            int panelW = window.getWidth();

            window.drawText(10, cy, "VISUALIZATION", termgl::Color::Cyan()); cy += 30;

            auto drawToggleBtn = [&](const string& label, bool& state, int bx, int by) {
                string text = (state ? "[ON] " : "[OFF] ") + label;
                if (window.drawButton(bx, by, (panelW - 30) / 2, 30, text)) {
                    state = !state;
                    // Re-initialize traffic when toggling real vehicles
                    if (&state == &showRealVehicles) {
                        if (!showRealVehicles) {
                            initializeTraffic();
                        } else {
                            trafficVehicles.clear();
                        }
                    }
                }
            };

            int col1 = 10;
            int col2 = 10 + (panelW - 30) / 2 + 10;

            drawToggleBtn("Roads", showRoads, col1, cy);
            drawToggleBtn("Corners", showCorners, col2, cy); cy += 40;

            drawToggleBtn("Sectors", showSectorBounds, col1, cy);
            drawToggleBtn("Houses", showHouses, col2, cy); cy += 40;

            drawToggleBtn("Traffic", showTraffic, col1, cy);
            drawToggleBtn("Pause", trafficPaused, col2, cy); cy += 40;

            drawToggleBtn("Heatmap", showCongestionHeatmap, col1, cy);
            drawToggleBtn("Sim Vehicles", showRealVehicles, col2, cy); cy += 40;

            drawToggleBtn("Citizens", showCitizens, col1, cy);
            drawToggleBtn("Agent Sim", useAgentSimulation, col2, cy); cy += 40;

            // Zoom info
            std::stringstream zoomSS;
            zoomSS << "Zoom: " << std::fixed << std::setprecision(1) << viewport.getZoom() << "x";
            window.drawText(10, cy, zoomSS.str(), termgl::Color::Grey()); cy += 25;
            
            // Traffic stats
            if (city && city->getCityGraph()) {
                int vehiclesOnRoads = city->getCityGraph()->getTotalVehiclesOnRoads();
                window.drawText(10, cy, "Vehicles: " + std::to_string(vehiclesOnRoads), termgl::Color::Grey());
                cy += 25;
            }

            window.drawText(10, cy, "PATHFINDING", termgl::Color::Green()); cy += 30;

            if (!inDijkstraMode) {
                if (window.drawButton(10, cy, panelW - 20, 35, "Start Navigation")) {
                    inDijkstraMode = true;
                    dijkstraMode = DijkstraMode::SELECT_START;
                    dijkstraNodeSelection = 0;
                    scrollOffset = 0;
                    clearDijkstraVisualization();
                }
            }
            else {
                if (window.drawButton(10, cy, panelW - 20, 35, "Exit Navigation")) {
                    inDijkstraMode = false;
                    dijkstraMode = DijkstraMode::SELECT_START;
                    clearDijkstraVisualization();
                }
                cy += 45;

                if (dijkstraMode == DijkstraMode::SELECT_START) {
                    window.drawText(10, cy, "STEP 1: Select Start", termgl::Color::Yellow()); cy += 20;
                    window.drawText(10, cy, "Click a node on map", termgl::Color::Grey()); cy += 25;
                }
                else if (dijkstraMode == DijkstraMode::SELECT_TARGET_TYPE) {
                    window.drawText(10, cy, "STEP 2: Select Destination", termgl::Color::Yellow()); cy += 20;
                    window.drawText(10, cy, "Click node OR select type:", termgl::Color::Grey()); cy += 25;

                    for (size_t i = 0; i < targetTypes.size(); i++) {
                        termgl::Color c = (i == targetSel) ? termgl::Color::Green() : termgl::Color::White();
                        string prefix = (i == targetSel) ? "> " : "  ";
                        window.drawText(10, cy, prefix + targetTypes[i], c);
                        cy += 25;
                    }
                    if (window.isKeyPressed(VK_UP) && targetSel > 0) targetSel--;
                    if (window.isKeyPressed(VK_DOWN) && targetSel < (int)targetTypes.size() - 1) targetSel++;
                    if (window.isKeyPressed(VK_RETURN)) {
                        if (targetSel == 4) {
                            dijkstraTargetType = "CUSTOM";
                            dijkstraMode = DijkstraMode::RUNNING;
                        }
                        else {
                            if (targetSel == 0) dijkstraTargetType = "SCHOOL";
                            else if (targetSel == 1) dijkstraTargetType = "HOSPITAL";
                            else if (targetSel == 2) dijkstraTargetType = "PHARMACY";
                            else dijkstraTargetType = "STOP";
                            runDijkstraAlgorithm();
                            dijkstraMode = DijkstraMode::COMPLETE;
                        }
                    }
                }
                else if (dijkstraMode == DijkstraMode::COMPLETE) {
                    if (dijkstraPath.getSize() > 0) {
                        window.drawText(10, cy, "ROUTE FOUND", termgl::Color::Green()); cy += 30;
                        std::stringstream distSS;
                        distSS << "Distance: " << std::fixed << std::setprecision(2) << dijkstraDistance << " km";
                        window.drawText(10, cy, distSS.str(), termgl::Color::White()); cy += 25;
                        window.drawText(10, cy, "Stops: " + std::to_string(dijkstraPath.getSize()), termgl::Color::White()); cy += 35;
                    }
                    else {
                        window.drawText(10, cy, "NO PATH FOUND", termgl::Color::Red()); cy += 35;
                    }
                    if (window.drawButton(10, cy, panelW - 20, 30, "Reset Path")) {
                        clearDijkstraVisualization();
                        dijkstraMode = DijkstraMode::SELECT_START;
                    }
                }
            }

            // Info panel
            cy = window.getHeight() - 120;
            window.drawRect(5, cy, panelW - 10, 110, termgl::Color(40, 40, 40));
            window.drawText(12, cy + 10, "INFO:", termgl::Color::Yellow());
            window.drawText(12, cy + 30, getHoverInfo(), termgl::Color::White());

            window.display();
        }
    }
};

#endif // CITY_GRAPH_VIEW_H