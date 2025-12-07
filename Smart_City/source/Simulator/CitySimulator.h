#ifndef CITY_SIMULATOR_ENHANCED_H
#define CITY_SIMULATOR_ENHANCED_H

#include <string>
#include <thread>
#include <chrono>
#include <iostream>
#include <functional>
#include <fstream>
#include <atomic>
#include <vector>
#include <sstream>
#include <cmath>
#include <algorithm> 
#include <map>
#include <set>
#include <iomanip>

#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/component/event.hpp"

#include "../../SmartCity.h"
#include "../../data_structures/Vector.h"
#include "CityManagement.h"

using namespace ftxui;
using std::string;

// ============================================================================
// GRAPH VISUALIZATION STRUCTURES
// ============================================================================

struct Point2D {
    double x, y;
    Point2D(double _x = 0, double _y = 0) : x(_x), y(_y) {}
};

struct GraphNode2D {
    int id;
    Point2D pos;
    string name;
    string type;
    string sector;
    Color color;
    bool isCorner;

    // Visualization States
    bool isOnPath;      // Part of the result path
    bool isVisited;     // Explored by algorithm
    bool isStart;       // Starting node
    bool isEnd;         // Destination node
    bool isFiltered;    // Matches current menu filter (for highlighting)

    GraphNode2D() : id(-1), pos(), name(""), type(""), sector(""),
        color(Color::White), isCorner(false), isOnPath(false),
        isVisited(false), isStart(false), isEnd(false), isFiltered(false) {
    }
};

struct GraphEdge2D {
    int fromID, toID;
    bool isOnPath;  // For Dijkstra visualization

    GraphEdge2D(int f, int t) : fromID(f), toID(t), isOnPath(false) {}
};

struct SectorRegion {
    string name;
    Point2D topLeft, bottomRight;
    Point2D center;
    bool isHovered;

    SectorRegion() : name(""), topLeft(), bottomRight(), center(), isHovered(false) {}

    bool contains(Point2D p) const {
        return p.x >= topLeft.x && p.x <= bottomRight.x &&
            p.y >= topLeft.y && p.y <= bottomRight.y;
    }
};

// ============================================================================
// GRAPH VIEWPORT - Robust Aspect Ratio & Scaling
// ============================================================================
class GraphViewport {
private:
    double minLat, maxLat, minLon, maxLon;
    double dataRatio; // Aspect ratio of the actual geographic data

    int canvasWidth, canvasHeight; // In Braille dots (not characters)
    double offsetX, offsetY;
    double zoom;

public:
    GraphViewport() :
        minLat(BASE_LAT), maxLat(MAX_LAT),
        minLon(BASE_LON), maxLon(MAX_LON),
        canvasWidth(200), canvasHeight(100),
        offsetX(0), offsetY(0), zoom(1.0) {
        updateDataRatio();
    }

    void setBounds(double minLa, double maxLa, double minLo, double maxLo) {
        minLat = minLa; maxLat = maxLa; minLon = minLo; maxLon = maxLo;
        updateDataRatio();
    }

    void updateDataRatio() {
        // Calculate physical aspect ratio for Islamabad (approx 33.7N)
        // 1 deg Lat ~= 111km, 1 deg Lon ~= 92km
        double latDist = (maxLat - minLat) * 111.0;
        double lonDist = (maxLon - minLon) * 92.0;
        if (latDist < 0.0001) latDist = 1.0;
        dataRatio = lonDist / latDist;
    }

    void setCanvasSize(int w, int h) {
        canvasWidth = w; canvasHeight = h;
    }

    Point2D geoToCanvas(double lat, double lon) const {
        double nX = (lon - minLon) / (maxLon - minLon);
        double nY = (maxLat - lat) / (maxLat - minLat);

        double screenRatio = (double)canvasWidth / (double)canvasHeight;
        double scaleX = 1.0;
        double scaleY = 1.0;

        // WIDE GRAPH: Stretch width by 2.2x to make it look spacious and nice on terminal
        double widthStretch = 2.2;

        if (screenRatio > dataRatio * widthStretch) {
            scaleX = (dataRatio * widthStretch) / screenRatio;
        }
        else {
            scaleY = screenRatio / (dataRatio * widthStretch);
        }

        double cX = (nX - 0.5) * scaleX * widthStretch;
        double cY = (nY - 0.5) * scaleY;

        cX = (cX - offsetX) * zoom;
        cY = (cY - offsetY) * zoom;

        double screenX = (cX + 0.5) * canvasWidth;
        double screenY = (cY + 0.5) * canvasHeight;

        return Point2D(screenX, screenY);
    }

    void zoomIn() { zoom *= 1.1; if (zoom > 10.0) zoom = 10.0; }
    void zoomOut() { zoom /= 1.1; if (zoom < 0.2) zoom = 0.2; }
    void panLeft() { offsetX -= 0.05 / zoom; }
    void panRight() { offsetX += 0.05 / zoom; }
    void panUp() { offsetY -= 0.05 / zoom; }
    void panDown() { offsetY += 0.05 / zoom; }
    void resetView() { zoom = 1.0; offsetX = 0; offsetY = 0; }
    double getZoom() const { return zoom; }
};

// ============================================================================
// ENUMS
// ============================================================================

enum class SimulatorState {
    INTRO_PHASE_1, INTRO_PHASE_2, INTRO_PHASE_3,
    WELCOME_ANIMATION, MAIN_MENU, CSV_SELECTION, LOADING,
    GRAPH_VIEW, DATABASE_VIEW, MANAGEMENT_MENU,
    DIJKSTRA_VIEW,
    SEARCH_VIEW,
    SIMULATION_DASHBOARD,
    ROUTE_WIZARD, // New state for configuring routes
    EXIT
};

enum class CSVLoadMode { DEMO_MODE, FULL_MODE };

enum class DijkstraMode {
    START_SECTOR, START_CATEGORY, START_NODE,
    DESTINATION_MODE, END_SECTOR, END_CATEGORY, END_NODE,
    RUNNING, COMPLETE
};

namespace ASCIIArt {
    const string ISLAMABAD_TITLE[] = {
        R"( ██╗███████╗██╗      █████╗ ███╗   ███╗ █████╗ ██████╗  █████╗ ██████╗ )",
        R"( ██║██╔════╝██║     ██╔══██╗████╗ ████║██╔══██╗██╔══██╗██╔══██║██╔══██╗)",
        R"( ██║███████╗██║     ███████║██╔████╔██║███████║██████╔╝███████║██║  ██║)",
        R"( ██║╚════██║██║     ██╔══██║██║╚██╔╝██║██╔══██║██╔══██╗██╔══██║██║  ██║)",
        R"( ██║███████║███████╗██║  ██║██║ ╚═╝ ██║██║  ██║██████╔╝██║  ██║██████╔╝)",
        R"( ╚═╝╚══════╝╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝╚═════╝ ╚═╝  ╚═╝╚═════╝ )",
    };
    const int ISLAMABAD_TITLE_HEIGHT = 6;

    const string REDEFINED_TITLE[] = {
        R"( ██████╗ ███████╗██████╗ ███████╗███████╗██╗███╗   ██╗███████╗██████╗ )",
        R"( ██╔══██╗██╔════╝██╔══██╗██╔════╝██╔════╝██║████╗  ██║██╔════╝██╔══██╗)",
        R"( ██████╔╝█████╗  ██║  ██║█████╗  █████╗  ██║██╔██╗ ██║█████╗  ██║  ██║)",
        R"( ██╔══██╗██╔══╝  ██║  ██║██╔══╝  ██╔══╝  ██║██║╚██╗██║██╔══╝  ██║  ██║)",
        R"( ██║  ██║███████╗██████╔╝███████╗██║     ██║██║ ╚████║███████╗██████╔╝)",
        R"( ╚═╝  ╚═╝╚══════╝╚═════╝ ╚══════╝╚═╝     ╚═╝╚═╝  ╚═══╝╚══════╝╚═════╝ )",
    };
    const int REDEFINED_TITLE_HEIGHT = 6;
}

// ============================================================================
// CITY SIMULATOR CLASS
// ============================================================================

class CitySimulator {
private:
    SmartCity* islamabad;
    CityManagement* cityMgmt;
    SimulatorState currentState;
    CSVLoadMode loadMode;
    bool cityInitialized;

    std::vector<GraphNode2D> graphNodes;
    std::vector<GraphEdge2D> graphEdges;
    std::vector<SectorRegion> sectorRegions;
    std::vector<int> nodeIdToIndex;
    GraphViewport viewport;

    int mouseX, mouseY;
    int hoveredNodeID;
    string hoveredSector;

    bool showCorners;
    bool showRoads;
    bool showSectorBounds;

    // --- Dijkstra Logic & UI State ---
    DijkstraMode dijkstraMode;
    int dijkstraStartNode;
    int dijkstraEndNode;
    string dijkstraTargetType;
    Vector<int> dijkstraPath;
    double dijkstraDistance;

    // UI Selection State
    int menuIndex;
    std::vector<string> menuOptions;
    std::vector<int> menuNodeIDs;

    // Selection Memory
    string selStartSector, selStartCat;
    string selEndSector, selEndCat;

    string stopsCSV, schoolsCSV, hospitalsCSV, pharmaciesCSV;
    string busesCSV, populationCSV, mallsCSV, shopsCSV, ambulancesCSV;

    int intersectionCounter;

    Color getNodeColor(const string& type) {
        if (type == "CORNER") return Color::GrayDark;
        if (type == "STOP") return Color::GreenLight;
        if (type == "SCHOOL") return Color::Blue;
        if (type == "HOSPITAL") return Color::Red;
        if (type == "PHARMACY") return Color::Magenta;
        if (type == "MALL") return Color::Magenta;
        if (type == "HOUSE") return Color::BlueLight;
        if (type == "MOSQUE") return Color::Cyan;
        if (type == "PARK") return Color::Green;
        if (type == "POLICE_STATION") return Color::RedLight;
        if (type == "FIRE_STATION") return Color::Orange1;
        if (type == "LIBRARY") return Color::Blue1;
        if (type == "ATM") return Color::Gold1;
        if (type == "RESTAURANT") return Color::Orange3;
        return Color::White;
    }

public:
    CitySimulator();
    ~CitySimulator();

    void run();
    void runDebugMode();
    void buildGraphVisualization();

    void runIntroPhase1();
    void runIntroPhase2();
    void runIntroPhase3();

    void runWelcomeAnimation();
    void runMainMenu();
    void runCSVSelection();
    void runLoadingScreen();
    void runGraphView();
    void runDatabaseView();
    void runSearchView();
    void runManagementMenu();

    // NEW: Dashboard & Wizard
    void runSimulationDashboard();
    void runRouteWizard(); // NEW: Route Configuration

    // Dijkstra Functions
    void runDijkstraView();
    void updateDijkstraMenu();
    void handleDijkstraSelection();
    void updateFilteredNodes();

    void runAddFacilityForm(const string& sector);
    void runAddOfferingForm(CityNode* node);

    Canvas renderGraphToCanvas(int width, int height, bool onlyShowPath = false);
    void updateHoverState(int mx, int my);
    string getHoverInfo();

    void clearDijkstraVisualization();
    void runDijkstraAlgorithm();

    void sleepMs(int ms);
    bool fileExists(const string& path);
};

// ============================================================================
// IMPLEMENTATION
// ============================================================================

inline CitySimulator::CitySimulator()
    : islamabad(nullptr), cityMgmt(nullptr),
    currentState(SimulatorState::INTRO_PHASE_1),
    loadMode(CSVLoadMode::DEMO_MODE),
    cityInitialized(false),
    mouseX(0), mouseY(0),
    hoveredNodeID(-1), hoveredSector(""),
    showCorners(false), showRoads(true), showSectorBounds(false),
    dijkstraMode(DijkstraMode::START_SECTOR),
    dijkstraStartNode(-1), dijkstraEndNode(-1),
    dijkstraTargetType(""), dijkstraDistance(0.0),
    menuIndex(0), intersectionCounter(0) {

    stopsCSV = "dataset/stops.csv";
    schoolsCSV = "dataset/schools.csv";
    hospitalsCSV = "dataset/hospitals.csv";
    pharmaciesCSV = "dataset/pharmacies.csv";
    busesCSV = "dataset/buses.csv";
    populationCSV = "dataset/population.csv";
    mallsCSV = "dataset/malls.csv";
    shopsCSV = "dataset/shops.csv";
    ambulancesCSV = "dataset/ambulances.csv";
}

inline CitySimulator::~CitySimulator() {
    delete cityMgmt;
    delete islamabad;
}

inline void CitySimulator::sleepMs(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline bool CitySimulator::fileExists(const string& path) {
    std::ifstream f(path);
    return f.good();
}

// ============================================================================
// GRAPH VISUALIZATION - Building Logic
// ============================================================================

inline void CitySimulator::buildGraphVisualization() {
    graphNodes.clear();
    graphEdges.clear();
    sectorRegions.clear();
    nodeIdToIndex.clear();
    intersectionCounter = 0;

    if (!islamabad || !islamabad->getCityGraph()) return;
    CityGraph* graph = islamabad->getCityGraph();

    viewport.setBounds(BASE_LAT, MAX_LAT, BASE_LON, MAX_LON);

    int maxId = 0;
    for (int i = 0; i < graph->getNodeCount(); i++) {
        CityNode* node = graph->getNode(i);
        if (node && node->id > maxId) maxId = node->id;
    }
    nodeIdToIndex.resize(maxId + 1, -1);

    for (int i = 0; i < graph->getNodeCount(); i++) {
        CityNode* node = graph->getNode(i);
        if (!node) continue;

        Point2D p = viewport.geoToCanvas(node->lat, node->lon);

        GraphNode2D gNode;
        gNode.id = node->id;
        gNode.pos = p;
        gNode.type = node->type;
        gNode.sector = node->sector;
        gNode.color = getNodeColor(node->type);
        gNode.isCorner = (node->type == "CORNER");
        gNode.isOnPath = false;
        gNode.isVisited = false;
        gNode.isStart = false;
        gNode.isEnd = false;
        gNode.isFiltered = false;

        if (gNode.isCorner) {
            intersectionCounter++;
            gNode.name = "Intersection " + std::to_string(intersectionCounter);
        }
        else {
            gNode.name = node->name;
        }

        nodeIdToIndex[node->id] = (int)graphNodes.size();
        graphNodes.push_back(gNode);
    }

    for (int i = 0; i < graph->getNodeCount(); i++) {
        CityNode* node = graph->getNode(i);
        if (!node) continue;

        const LinkedList<Edge>& roads = node->getRoads();
        for (int j = 0; j < roads.size(); j++) {
            Edge edge = roads[j];
            if (node->id < edge.destinationID) {
                GraphEdge2D gEdge(node->id, edge.destinationID);
                gEdge.isOnPath = false;
                graphEdges.push_back(gEdge);
            }
        }
    }

    for (int i = 0; i < SECTOR_COUNT; i++) {
        SectorRegion region;
        region.name = SECTOR_GRID[i].name;

        Point2D tl = viewport.geoToCanvas(SECTOR_GRID[i].maxLat, SECTOR_GRID[i].minLon);
        Point2D br = viewport.geoToCanvas(SECTOR_GRID[i].minLat, SECTOR_GRID[i].maxLon);

        region.topLeft = Point2D(std::min(tl.x, br.x), std::min(tl.y, br.y));
        region.bottomRight = Point2D(std::max(tl.x, br.x), std::max(tl.y, br.y));

        region.center = Point2D((region.topLeft.x + region.bottomRight.x) / 2,
            (region.topLeft.y + region.bottomRight.y) / 2);
        region.isHovered = false;
        sectorRegions.push_back(region);
    }
}

// ============================================================================
// GRAPH RENDERER
// ============================================================================

inline Canvas CitySimulator::renderGraphToCanvas(int width, int height, bool onlyShowPath) {
    int cWidth = width * 2;
    int cHeight = height * 4;
    Canvas canvas(cWidth, cHeight);
    viewport.setCanvasSize(cWidth, cHeight);

    // Recalculate positions
    for (auto& node : graphNodes) {
        CityNode* n = islamabad->getCityGraph()->getNode(node.id);
        if (n) node.pos = viewport.geoToCanvas(n->lat, n->lon);
    }
    for (int i = 0; i < SECTOR_COUNT; i++) {
        Point2D tl = viewport.geoToCanvas(SECTOR_GRID[i].maxLat, SECTOR_GRID[i].minLon);
        Point2D br = viewport.geoToCanvas(SECTOR_GRID[i].minLat, SECTOR_GRID[i].maxLon);
        sectorRegions[i].topLeft = Point2D(std::min(tl.x, br.x), std::min(tl.y, br.y));
        sectorRegions[i].bottomRight = Point2D(std::max(tl.x, br.x), std::max(tl.y, br.y));
    }

    auto stylize = [](Color c) -> Canvas::Stylizer {
        return [c](Pixel& p) { p.foreground_color = c; };
        };

    // Layer 1: Sector Bounds
    if (showSectorBounds && !onlyShowPath) {
        for (const auto& region : sectorRegions) {
            int x1 = (int)region.topLeft.x;
            int y1 = (int)region.topLeft.y;
            int x2 = (int)region.bottomRight.x;
            int y2 = (int)region.bottomRight.y;
            Color c = region.isHovered ? Color::Yellow : Color::GrayDark;
            canvas.DrawPointLine(x1, y1, x2, y1, stylize(c));
            canvas.DrawPointLine(x2, y1, x2, y2, stylize(c));
            canvas.DrawPointLine(x2, y2, x1, y2, stylize(c));
            canvas.DrawPointLine(x1, y2, x1, y1, stylize(c));
        }
    }

    // Layer 2: Roads
    if (showRoads) {
        // LOD Check: If zoomed out, hide minor roads
        bool showMinor = (viewport.getZoom() > 0.7) || onlyShowPath;

        for (const auto& edge : graphEdges) {
            if (onlyShowPath && !edge.isOnPath) continue;

            int idx1 = (edge.fromID < (int)nodeIdToIndex.size()) ? nodeIdToIndex[edge.fromID] : -1;
            int idx2 = (edge.toID < (int)nodeIdToIndex.size()) ? nodeIdToIndex[edge.toID] : -1;

            if (idx1 >= 0 && idx2 >= 0) {
                const GraphNode2D& n1 = graphNodes[idx1];
                const GraphNode2D& n2 = graphNodes[idx2];
                if ((n1.pos.x < 0 && n2.pos.x < 0) || (n1.pos.x > cWidth && n2.pos.x > cWidth)) continue;

                Color roadColor;
                bool isMajor = (n1.isCorner && n2.isCorner);

                if (edge.isOnPath) roadColor = Color::GreenLight;
                else if (isMajor) roadColor = Color::GrayDark;
                else roadColor = Color::GrayLight;

                if (!showMinor && !isMajor && !edge.isOnPath) continue;

                if (onlyShowPath) {
                    canvas.DrawBlockLine((int)n1.pos.x, (int)n1.pos.y, (int)n2.pos.x, (int)n2.pos.y, stylize(roadColor));
                }
                else {
                    canvas.DrawPointLine((int)n1.pos.x, (int)n1.pos.y, (int)n2.pos.x, (int)n2.pos.y, stylize(roadColor));
                }
            }
        }
    }

    // Layer 3: Nodes
    for (const auto& node : graphNodes) {
        if (onlyShowPath) {
            if (!node.isOnPath && !node.isStart && !node.isEnd) continue;
        }
        else {
            if (node.isCorner && !showCorners) continue;
        }

        int x = (int)node.pos.x;
        int y = (int)node.pos.y;
        if (x < 0 || x >= cWidth || y < 0 || y >= cHeight) continue;

        Color c = node.color;
        if (node.id == hoveredNodeID) c = Color::White;
        else if (node.isStart) c = Color::Cyan;
        else if (node.isEnd) c = Color::Yellow;
        else if (node.isOnPath) c = Color::GreenLight;
        else if (node.isFiltered) c = Color::White;

        auto pen = stylize(c);

        if (onlyShowPath && node.isCorner) {
            canvas.DrawBlock(x, y, true, stylize(Color::GrayLight));
            continue;
        }

        if (node.type == "HOSPITAL") {
            canvas.DrawPointLine(x, y - 3, x, y + 3, pen);
            canvas.DrawPointLine(x - 3, y, x + 3, y, pen);
        }
        else if (node.type == "SCHOOL") {
            for (int i = -2; i <= 2; ++i) canvas.DrawPointLine(x - 2, y + i, x + 2, y + i, pen);
        }
        else if (node.type == "MALL") {
            canvas.DrawPointLine(x, y - 3, x + 3, y, pen);
            canvas.DrawPointLine(x + 3, y, x, y + 3, pen);
            canvas.DrawPointLine(x, y + 3, x - 3, y, pen);
            canvas.DrawPointLine(x - 3, y, x, y - 3, pen);
            canvas.DrawPoint(x, y, true, pen);
        }
        else if (node.type == "STOP") {
            canvas.DrawPointLine(x - 3, y - 3, x + 3, y - 3, pen);
            canvas.DrawPointLine(x - 2, y - 3, x - 2, y + 2, pen);
            canvas.DrawPointLine(x + 2, y - 3, x + 2, y + 2, pen);
            canvas.DrawPointLine(x - 3, y + 2, x + 3, y + 2, pen);
        }
        else if (node.type == "HOUSE") {
            canvas.DrawPointLine(x - 2, y - 1, x, y - 3, pen);
            canvas.DrawPointLine(x, y - 3, x + 2, y - 1, pen);
            canvas.DrawPointLine(x - 2, y - 1, x - 2, y + 2, pen);
            canvas.DrawPointLine(x + 2, y - 1, x + 2, y + 2, pen);
        }
        else if (node.type == "PHARMACY") {
            canvas.DrawPointLine(x - 2, y - 2, x + 2, y + 2, pen);
            canvas.DrawPointLine(x + 2, y - 2, x - 2, y + 2, pen);
        }
        else if (node.isCorner) {
            canvas.DrawPoint(x, y, true, stylize(Color::GrayDark));
        }
        else {
            canvas.DrawPoint(x, y, true, pen);
            canvas.DrawPoint(x + 1, y, true, pen);
            canvas.DrawPoint(x, y + 1, true, pen);
            canvas.DrawPoint(x + 1, y + 1, true, pen);
        }
    }

    // Layer 4: Vehicles
    if (!onlyShowPath && cityInitialized && islamabad && islamabad->getTransportManager()) {
        TransportManager* tm = islamabad->getTransportManager();
        auto drawVehicle = [&](int vx, int vy, Color vc) {
            for (int dx = -1; dx <= 1; dx++) for (int dy = -1; dy <= 1; dy++) canvas.DrawPoint(vx + dx, vy + dy, true, stylize(vc));
            };

        const Vector<Bus*>& buses = tm->getAllBuses();
        for (int i = 0; i < buses.getSize(); i++) {
            if (buses[i]->getStatus() == VehicleStatus::EN_ROUTE || buses[i]->getStatus() == VehicleStatus::AT_STOP) {
                int nID = buses[i]->getCurrentNodeID();
                if (nID >= 0 && nID < nodeIdToIndex.size()) {
                    int idx = nodeIdToIndex[nID];
                    if (idx != -1) drawVehicle((int)graphNodes[idx].pos.x, (int)graphNodes[idx].pos.y, Color::Cyan);
                }
            }
        }

        bool flash = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() / 200) % 2;
        Color ambColor = flash ? Color::Red : Color::RedLight;
        const Vector<Ambulance*>& ambulances = tm->getAllAmbulances();
        for (int i = 0; i < ambulances.getSize(); i++) {
            if (ambulances[i]->getStatus() != VehicleStatus::IDLE && ambulances[i]->getStatus() != VehicleStatus::MAINTENANCE) {
                int nID = ambulances[i]->getCurrentNodeID();
                if (nID >= 0 && nID < nodeIdToIndex.size()) {
                    int idx = nodeIdToIndex[nID];
                    if (idx != -1) drawVehicle((int)graphNodes[idx].pos.x, (int)graphNodes[idx].pos.y, ambColor);
                }
            }
        }
    }

    return canvas;
}

inline void CitySimulator::updateHoverState(int mx, int my) {
    mouseX = mx;
    mouseY = my;
    hoveredNodeID = -1;
    hoveredSector = "";

    int canvasX = mx * 2;
    int canvasY = my * 4;
    double minDist = 30.0;

    for (const auto& node : graphNodes) {
        if (node.isCorner && !showCorners) continue;
        double dx = node.pos.x - canvasX;
        double dy = node.pos.y - canvasY;
        double dist = std::sqrt(dx * dx + dy * dy);
        if (dist < minDist) {
            minDist = dist;
            hoveredNodeID = node.id;
        }
    }

    for (auto& region : sectorRegions) {
        if (canvasX >= region.topLeft.x && canvasX <= region.bottomRight.x &&
            canvasY >= region.topLeft.y && canvasY <= region.bottomRight.y) {
            region.isHovered = true;
            hoveredSector = region.name;
        }
        else {
            region.isHovered = false;
        }
    }
}

inline string CitySimulator::getHoverInfo() {
    std::stringstream ss;
    if (hoveredNodeID >= 0 && hoveredNodeID < (int)nodeIdToIndex.size()) {
        int idx = nodeIdToIndex[hoveredNodeID];
        if (idx >= 0 && idx < (int)graphNodes.size()) {
            const GraphNode2D& node = graphNodes[idx];
            ss << "NODE: " << node.name << "\nType: " << node.type << "\nSector: " << node.sector << "\nID: " << node.id;
            return ss.str();
        }
    }
    if (!hoveredSector.empty()) { ss << "SECTOR: " << hoveredSector; return ss.str(); }
    ss << "Hover over\nnodes for info";
    return ss.str();
}

// ============================================================================
// ROUTE CONFIGURATION WIZARD
// ============================================================================

inline void CitySimulator::runRouteWizard() {
    auto screen = ScreenInteractive::Fullscreen();

    // State Tracking
    std::vector<string> steps = { "Public Buses", "School Buses", "Ambulances", "Complete" };
    int currentStep = 0;
    string logMessage = "Initializing Route Wizard...";

    // State for inputs
    int selectedItem = 0;
    std::vector<string> itemList;
    std::vector<void*> itemPointers; // To store Bus*, School*, etc.

    // School Bus Specifics
    string targetSector = "G-9"; // Default

    // Public Bus Specifics
    string startStop = "", endStop = "";

    auto updateList = [&]() {
        itemList.clear();
        itemPointers.clear();
        selectedItem = 0;

        TransportManager* tm = islamabad->getTransportManager();
        if (!tm) return;

        if (currentStep == 0) { // Public Buses
            const Vector<Bus*>& buses = tm->getAllBuses();
            for (int i = 0; i < buses.getSize(); i++) {
                if (buses[i]->getRouteLength() < 2) {
                    itemList.push_back(buses[i]->getBusNo() + " (" + buses[i]->getCompany() + ") - NO ROUTE");
                    itemPointers.push_back(buses[i]);
                }
            }
            if (itemList.empty()) logMessage = "All public buses have routes!";
        }
        else if (currentStep == 1) { // School Buses
            SchoolManager* sm = islamabad->getSchoolManager();
            if (sm) {
                for (int i = 0; i < sm->schools.getSize(); i++) {
                    School* s = sm->schools[i];
                    Vector<SchoolBus*> sb = tm->getSchoolBusesBySchool(s->id);
                    if (sb.getSize() < 2) {
                        itemList.push_back(s->name + " (Has " + std::to_string(sb.getSize()) + " buses) - NEEDS 2");
                        itemPointers.push_back(s);
                    }
                }
            }
            if (itemList.empty()) logMessage = "All schools have required buses.";
        }
        else if (currentStep == 2) { // Ambulances
            MedicalManager* mm = islamabad->getMedicalManager();
            if (mm) {
                for (int i = 0; i < mm->hospitals.getSize(); i++) {
                    Hospital* h = mm->hospitals[i];
                    Vector<Ambulance*> ambs = tm->getAmbulancesByHospital(h->id);
                    if (ambs.getSize() == 0) {
                        itemList.push_back(h->name + " - NEEDS AMBULANCE");
                        itemPointers.push_back(h);
                    }
                }
            }
            if (itemList.empty()) logMessage = "All hospitals have ambulances.";
        }
        };

    updateList();

    // Components
    Component input_sector = Input(&targetSector, "Enter Sector (e.g. G-9)");
    Component input_start = Input(&startStop, "Start Stop ID (e.g. Stop1)");
    Component input_end = Input(&endStop, "End Stop ID (e.g. Stop10)");

    auto renderer = Renderer(Container::Vertical({ input_sector, input_start, input_end }), [&] {
        Elements listElements;
        for (int i = 0; i < (int)itemList.size(); i++) {
            auto item = text((i == selectedItem ? "> " : "  ") + itemList[i]);
            if (i == selectedItem) item = item | bold | color(Color::Green);
            listElements.push_back(item);
        }

        auto listBox = vbox(listElements) | border | flex;

        Element controlPanel;
        if (currentStep == 0) {
            controlPanel = vbox({
                text("CONFIGURE PUBLIC BUS") | bold | color(Color::Blue),
                separator(),
                hbox({text("Start: "), input_start->Render() | border}),
                hbox({text("End:   "), input_end->Render() | border}),
                text("Press Enter to Assign Route") | dim
                });
        }
        else if (currentStep == 1) {
            controlPanel = vbox({
                text("CONFIGURE SCHOOL BUSES") | bold | color(Color::Yellow),
                separator(),
                hbox({text("Pickups From: "), input_sector->Render() | border}),
                text("Press Enter to Auto-Generate 2 Buses") | dim
                });
        }
        else if (currentStep == 2) {
            controlPanel = vbox({
               text("CONFIGURE AMBULANCES") | bold | color(Color::Red),
               separator(),
               text("Press Enter to Assign Ambulance to Hospital") | dim
                });
        }
        else {
            controlPanel = text("Configuration Complete!") | bold | color(Color::Green);
        }

        return vbox({
            text("ROUTE CONFIGURATION WIZARD") | bold | center | bgcolor(Color::Blue) | color(Color::White),
            text("Step " + std::to_string(currentStep + 1) + "/4: " + steps[currentStep]) | center,
            separator(),
            hbox({listBox, controlPanel}) | flex,
            separator(),
            text(logMessage) | color(Color::Yellow),
            text("Press [N] for Next Step, [Esc] to Exit") | dim | center
            }) | border;
        });

    auto comp = CatchEvent(renderer, [&](Event e) {
        if (e == Event::ArrowUp && selectedItem > 0) { selectedItem--; return true; }
        if (e == Event::ArrowDown && selectedItem < (int)itemList.size() - 1) { selectedItem++; return true; }

        if (e == Event::Character('n') || e == Event::Character('N')) {
            if (currentStep < 3) {
                currentStep++;
                updateList();
            }
            else {
                currentState = SimulatorState::MAIN_MENU;
                screen.Exit();
            }
            return true;
        }

        if (e == Event::Return && !itemList.empty()) {
            if (currentStep == 0) { // Configure Bus
                Bus* b = (Bus*)itemPointers[selectedItem];
                if (startStop.empty() || endStop.empty()) {
                    logMessage = "Error: Enter Start and End Stop IDs";
                }
                else {
                    CityGraph* g = islamabad->getCityGraph();
                    int sID = g->getIDByDatabaseID(startStop);
                    int eID = g->getIDByDatabaseID(endStop);
                    if (sID != -1 && eID != -1) {
                        double dist = 0;
                        Vector<int> route = g->findShortestPath(sID, eID, dist);
                        if (route.getSize() > 0) {
                            islamabad->getTransportManager()->setBusRoute(b->getBusNo(), route, dist, startStop, endStop);
                            logMessage = "Success! Route assigned to " + b->getBusNo();
                            updateList();
                        }
                        else {
                            logMessage = "Error: No path found between stops";
                        }
                    }
                    else {
                        logMessage = "Error: Invalid Stop IDs";
                    }
                }
            }
            else if (currentStep == 1) { // Configure School
                School* s = (School*)itemPointers[selectedItem];
                // Add missing buses
                int currentCount = islamabad->getTransportManager()->getSchoolBusesBySchool(s->id).getSize();
                for (int k = currentCount; k < 2; k++) {
                    cityMgmt->registerSchoolBus(s->id, targetSector);
                }

                // Route logic: Pickups in sector -> School
                Vector<SchoolBus*> buses = islamabad->getTransportManager()->getSchoolBusesBySchool(s->id);
                CityGraph* g = islamabad->getCityGraph();
                Vector<int> pickups = islamabad->getTransportManager()->getPickupPointsInSector(targetSector);

                // If no pickup points, create some at stops
                if (pickups.getSize() == 0) {
                    islamabad->generatePickupPointsForSector(targetSector);
                    pickups = islamabad->getTransportManager()->getPickupPointsInSector(targetSector);
                }

                for (int k = 0; k < buses.getSize(); k++) {
                    int schoolNode = std::stoi(s->graphNodeID);
                    islamabad->setupSchoolBusRoute(buses[k]->getBusID(), pickups, schoolNode, s->id);
                }

                logMessage = "Assigned 2 buses to " + s->name + " serving " + targetSector;
                updateList();
            }
            else if (currentStep == 2) { // Configure Ambulance
                Hospital* h = (Hospital*)itemPointers[selectedItem];
                cityMgmt->registerAmbulance(h->id, h->getSector());
                logMessage = "Ambulance assigned to " + h->name;
                updateList();
            }
            return true;
        }

        if (e == Event::Escape) {
            currentState = SimulatorState::MAIN_MENU;
            screen.Exit();
            return true;
        }
        return false;
        });

    screen.Loop(comp);
}

// ============================================================================
// SIMULATION DASHBOARD
// ============================================================================

inline void CitySimulator::runSimulationDashboard() {
    auto screen = ScreenInteractive::Fullscreen();
    std::atomic<bool> autoRun{ false };
    auto container = Container::Vertical({});

    auto renderer = Renderer(container, [&] {
        TransportManager* tm = islamabad->getTransportManager();

        // --- 1. PUBLIC TRANSPORT (BUSES) ---
        Elements busList;
        busList.push_back(text("PUBLIC TRANSPORT (BLUE)") | bold | color(Color::Blue));
        busList.push_back(separator());
        if (tm) {
            const Vector<Bus*>& buses = tm->getAllBuses();
            int activeCount = 0;
            for (int i = 0; i < buses.getSize(); i++) {
                Bus* b = buses[i];
                if (b->getStatus() == VehicleStatus::EN_ROUTE || b->getStatus() == VehicleStatus::AT_STOP) {
                    activeCount++;
                    if (activeCount <= 12) {
                        string info = b->getID() + ": " + b->getCurrentStop();
                        busList.push_back(text(info));
                        busList.push_back(text("   Pass: " + std::to_string(b->getOnboardCount()) +
                            " | Earned: " + std::to_string((int)b->getTotalFareCollected())) | dim);
                    }
                }
            }
            if (activeCount == 0) busList.push_back(text("No active buses") | dim);
            else if (activeCount > 12) busList.push_back(text("... and " + std::to_string(activeCount - 12) + " more") | dim);
        }
        auto busWidget = vbox(busList) | border | color(Color::Blue) | flex;

        // --- 2. SCHOOL TRANSPORT (YELLOW) ---
        Elements schoolList;
        schoolList.push_back(text("SCHOOL BUSES (YELLOW)") | bold | color(Color::Yellow));
        schoolList.push_back(separator());
        if (tm) {
            const Vector<SchoolBus*>& sbuses = tm->getAllSchoolBuses();
            int activeCount = 0;
            for (int i = 0; i < sbuses.getSize(); i++) {
                SchoolBus* sb = sbuses[i];
                if (sb->getStatus() != VehicleStatus::IDLE) {
                    activeCount++;
                    if (activeCount <= 12) {
                        string info = sb->getID() + ": " + sb->getCurrentSector();
                        schoolList.push_back(text(info));
                        schoolList.push_back(text("   Students: " + std::to_string(sb->getOnboardStudentCount()) +
                            " | " + sb->getSchoolBusStatus()) | dim);
                    }
                }
            }
            if (activeCount == 0) schoolList.push_back(text("No active school buses") | dim);
            else if (activeCount > 12) schoolList.push_back(text("... and " + std::to_string(activeCount - 12) + " more") | dim);
        }
        auto schoolWidget = vbox(schoolList) | border | color(Color::Yellow) | flex;

        // --- 3. EMERGENCY SERVICES (RED) ---
        Elements ambList;
        ambList.push_back(text("AMBULANCES (RED)") | bold | color(Color::Red));
        ambList.push_back(separator());
        if (tm) {
            const Vector<Ambulance*>& ambs = tm->getAllAmbulances();
            int activeCount = 0;
            for (int i = 0; i < ambs.getSize(); i++) {
                Ambulance* a = ambs[i];
                if (a->getStatus() != VehicleStatus::IDLE && a->getStatus() != VehicleStatus::MAINTENANCE) {
                    activeCount++;
                    if (activeCount <= 12) {
                        string info = a->getID() + " -> " + a->getCurrentSector();
                        ambList.push_back(text(info));
                        ambList.push_back(text("   Status: " + a->getAmbulanceStatus()) | dim);
                    }
                }
            }
            if (activeCount == 0) ambList.push_back(text("No active emergencies") | dim);
            else if (activeCount > 12) ambList.push_back(text("... and " + std::to_string(activeCount - 12) + " more") | dim);
        }
        auto ambWidget = vbox(ambList) | border | color(Color::Red) | flex;

        return vbox({
            text("SIMULATION DASHBOARD") | bold | center | bgcolor(Color::Blue) | color(Color::White),
            separator(),
            hbox({ busWidget, schoolWidget, ambWidget }) | flex,
            separator(),
            hbox({
                text(" Controls: ") | bold,
                text("[SPACE] Toggle Auto-Run ") | color(autoRun ? Color::Green : Color::RedLight),
                text("[S] Step Once "),
                text("[Esc] Back"),
                filler(),
                text("Tick: " + std::to_string(islamabad->getTransportManager()->getSimulationTick())) | bold
            })
            });
        });

    auto comp = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Escape) { autoRun = false; currentState = SimulatorState::MAIN_MENU; screen.Exit(); return true; }
        if (e == Event::Character(' ')) { autoRun = !autoRun; return true; }
        if (e == Event::Character('s') || e == Event::Character('S')) { islamabad->simulateStep(); return true; }
        return false;
        });

    std::thread simThread([&] {
        while (currentState == SimulatorState::SIMULATION_DASHBOARD) {
            if (autoRun) { islamabad->simulateStep(); screen.PostEvent(Event::Custom); std::this_thread::sleep_for(std::chrono::milliseconds(200)); }
            else { std::this_thread::sleep_for(std::chrono::milliseconds(100)); }
        }
        });

    screen.Loop(comp);
    if (simThread.joinable()) simThread.join();
}

// ============================================================================
// DIJKSTRA UI RENDERER
// ============================================================================

inline void CitySimulator::runDijkstraView() {
    auto screen = ScreenInteractive::Fullscreen();
    buildGraphVisualization();
    clearDijkstraVisualization();

    // Initial State
    dijkstraMode = DijkstraMode::START_SECTOR;
    menuIndex = 0;
    updateDijkstraMenu();

    auto renderer = Renderer([&] {
        int termW = Terminal::Size().dimx;
        int termH = Terminal::Size().dimy;

        bool showPathOnly = (dijkstraMode == DijkstraMode::COMPLETE);
        Canvas c = renderGraphToCanvas(termW - 55, termH - 4, showPathOnly);

        // --- Wizard Panel ---
        Elements items;
        items.push_back(text("PATHFINDING WIZARD") | bold | color(Color::Cyan) | center);
        items.push_back(separator());

        string header;
        if (dijkstraMode < DijkstraMode::DESTINATION_MODE) header = "STEP 1: START POINT";
        else if (dijkstraMode == DijkstraMode::DESTINATION_MODE) header = "STEP 2: DESTINATION TYPE";
        else if (dijkstraMode < DijkstraMode::COMPLETE) header = "STEP 3: END POINT";
        else header = "RESULT";
        items.push_back(text(header) | bold | color(Color::Yellow));
        items.push_back(separator());

        if (dijkstraStartNode != -1) {
            int idx = nodeIdToIndex[dijkstraStartNode];
            items.push_back(hbox({ text("FROM: ") | dim, text(graphNodes[idx].name.substr(0,35)) | color(Color::Cyan) }));
        }
        if (dijkstraMode >= DijkstraMode::START_CATEGORY) items.push_back(text(" Sec: " + selStartSector) | dim);
        if (dijkstraMode >= DijkstraMode::START_NODE) items.push_back(text(" Cat: " + selStartCat) | dim);
        items.push_back(separator());

        if (dijkstraMode != DijkstraMode::COMPLETE) {
            int start = std::max(0, menuIndex - 6);
            int end = std::min((int)menuOptions.size(), start + 14);
            for (int i = start; i < end; i++) {
                auto item = text((i == menuIndex ? " > " : "   ") + menuOptions[i]);
                if (i == menuIndex) item = item | bold | color(Color::Green);
                items.push_back(item);
            }
            items.push_back(separator());
            items.push_back(text("Tip: Click map to select node!") | blink | color(Color::RedLight));
        }
        else {
            if (dijkstraEndNode == -1) items.push_back(text("No Path Found!") | color(Color::Red));
            else {
                int eIdx = nodeIdToIndex[dijkstraEndNode];
                items.push_back(hbox({ text("TO:   "), text(graphNodes[eIdx].name.substr(0,35)) | color(Color::Yellow) }));
                items.push_back(text("Distance: " + std::to_string((int)dijkstraDistance) + " km"));
                items.push_back(text("Stops:    " + std::to_string(dijkstraPath.getSize())));
                items.push_back(separator());
                items.push_back(text("PATH SEQUENCE") | bold);
                for (int i = 0; i < std::min(dijkstraPath.getSize(), 8); i++) {
                    int nodeId = dijkstraPath[i];
                    int idx = nodeIdToIndex[nodeId];
                    if (idx >= 0) {
                        string name = graphNodes[idx].name;
                        if (name.length() > 40) name = name.substr(0, 37) + "...";
                        items.push_back(text(" " + std::to_string(i + 1) + ". " + name) | dim);
                    }
                }
                if (dijkstraPath.getSize() > 8) items.push_back(text(" ... and " + std::to_string(dijkstraPath.getSize() - 8) + " more") | dim);
                items.push_back(separator());
                items.push_back(text("Press 'R' to Reset") | bold);
            }
        }

        auto panel = vbox(items) | border | size(WIDTH, EQUAL, 50);
        return vbox({
            text("CITY NAVIGATION") | bold | center | color(Color::Green),
            hbox({ canvas(c) | border | flex, text(" "), panel }) | flex
            });
        });

    auto comp = CatchEvent(renderer, [&](Event e) {
        if (e == Event::ArrowUp && menuIndex > 0) { menuIndex--; updateFilteredNodes(); return true; }
        if (e == Event::ArrowDown && menuIndex < (int)menuOptions.size() - 1) { menuIndex++; updateFilteredNodes(); return true; }
        if (e == Event::Return && dijkstraMode != DijkstraMode::COMPLETE) { handleDijkstraSelection(); return true; }

        if (e.is_mouse() && e.mouse().button == Mouse::Left && e.mouse().motion == Mouse::Pressed) {
            updateHoverState(e.mouse().x, e.mouse().y);
            if (hoveredNodeID != -1) {
                if (dijkstraMode < DijkstraMode::DESTINATION_MODE) {
                    dijkstraStartNode = hoveredNodeID;
                    int idx = nodeIdToIndex[dijkstraStartNode]; if (idx >= 0) graphNodes[idx].isStart = true;
                    dijkstraMode = DijkstraMode::DESTINATION_MODE;
                    menuIndex = 0; updateDijkstraMenu();
                }
                else if (dijkstraMode > DijkstraMode::DESTINATION_MODE && dijkstraMode < DijkstraMode::COMPLETE) {
                    dijkstraEndNode = hoveredNodeID;
                    dijkstraTargetType = "";
                    runDijkstraAlgorithm();
                    dijkstraMode = DijkstraMode::COMPLETE;
                }
                return true;
            }
        }
        if (e.is_mouse()) { updateHoverState(e.mouse().x, e.mouse().y); return true; }

        if (e == Event::Character('r') || e == Event::Character('R')) {
            clearDijkstraVisualization();
            dijkstraMode = DijkstraMode::START_SECTOR;
            menuIndex = 0; updateDijkstraMenu();
            return true;
        }
        if (e == Event::Character('+')) { viewport.zoomIn(); buildGraphVisualization(); return true; }
        if (e == Event::Character('-')) { viewport.zoomOut(); buildGraphVisualization(); return true; }
        if (e == Event::ArrowLeft) { viewport.panLeft(); buildGraphVisualization(); return true; }
        if (e == Event::ArrowRight) { viewport.panRight(); buildGraphVisualization(); return true; }
        if (e == Event::Escape) { currentState = SimulatorState::GRAPH_VIEW; screen.Exit(); return true; }
        return false;
        });
    screen.Loop(comp);
}

// ============================================================================
// DIJKSTRA LOGIC & MENU UPDATES
// ============================================================================

inline void CitySimulator::clearDijkstraVisualization() {
    for (auto& node : graphNodes) {
        node.isOnPath = false; node.isVisited = false; node.isStart = false; node.isEnd = false; node.isFiltered = false;
    }
    for (auto& edge : graphEdges) edge.isOnPath = false;
    dijkstraPath.clear();
    dijkstraDistance = 0.0;
    dijkstraStartNode = -1;
    dijkstraEndNode = -1;
}

inline void CitySimulator::updateDijkstraMenu() {
    menuOptions.clear();
    menuNodeIDs.clear();

    if (dijkstraMode == DijkstraMode::START_SECTOR || dijkstraMode == DijkstraMode::END_SECTOR) {
        for (int i = 0; i < SECTOR_COUNT; i++) menuOptions.push_back(SECTOR_GRID[i].name);
    }
    else if (dijkstraMode == DijkstraMode::START_CATEGORY || dijkstraMode == DijkstraMode::END_CATEGORY) {
        menuOptions = { "Transport (Stops)", "Education (Schools)", "Health (Hospitals/Pharmacies)",
                        "Commercial (Malls)", "Residential (Houses)", "Public (Parks/Mosques)", "All" };
    }
    else if (dijkstraMode == DijkstraMode::START_NODE || dijkstraMode == DijkstraMode::END_NODE) {
        string sector = (dijkstraMode == DijkstraMode::START_NODE) ? selStartSector : selEndSector;
        string cat = (dijkstraMode == DijkstraMode::START_NODE) ? selStartCat : selEndCat;

        for (const auto& node : graphNodes) {
            if (node.isCorner) continue;
            if (node.sector != sector) continue;

            bool match = (cat == "All");
            if (cat.find("Transport") != string::npos && node.type == "STOP") match = true;
            if (cat.find("Education") != string::npos && node.type == "SCHOOL") match = true;
            if (cat.find("Health") != string::npos && (node.type == "HOSPITAL" || node.type == "PHARMACY")) match = true;
            if (cat.find("Commercial") != string::npos && (node.type == "MALL" || node.type == "SHOP")) match = true;
            if (cat.find("Residential") != string::npos && node.type == "HOUSE") match = true;
            if (cat.find("Public") != string::npos && (node.type == "PARK" || node.type == "MOSQUE")) match = true;

            if (match) {
                menuOptions.push_back(node.name);
                menuNodeIDs.push_back(node.id);
            }
        }
        if (menuOptions.empty()) menuOptions.push_back("(No facilities found)");
    }
    else if (dijkstraMode == DijkstraMode::DESTINATION_MODE) {
        menuOptions = { "Nearest School", "Nearest Hospital", "Nearest Pharmacy", "Nearest Stop", "SPECIFIC LOCATION" };
    }
}

inline void CitySimulator::updateFilteredNodes() {
    for (auto& node : graphNodes) node.isFiltered = false;

    if (dijkstraMode == DijkstraMode::START_NODE || dijkstraMode == DijkstraMode::END_NODE) {
        for (int id : menuNodeIDs) {
            if (id >= 0 && id < nodeIdToIndex.size()) {
                int idx = nodeIdToIndex[id];
                if (idx >= 0) graphNodes[idx].isFiltered = true;
            }
        }
    }
}

inline void CitySimulator::handleDijkstraSelection() {
    if (dijkstraMode == DijkstraMode::START_SECTOR) {
        selStartSector = menuOptions[menuIndex];
        dijkstraMode = DijkstraMode::START_CATEGORY;
        menuIndex = 0;
    }
    else if (dijkstraMode == DijkstraMode::START_CATEGORY) {
        selStartCat = menuOptions[menuIndex];
        dijkstraMode = DijkstraMode::START_NODE;
        menuIndex = 0;
    }
    else if (dijkstraMode == DijkstraMode::START_NODE) {
        if (!menuNodeIDs.empty()) {
            dijkstraStartNode = menuNodeIDs[menuIndex];
            int idx = nodeIdToIndex[dijkstraStartNode];
            if (idx >= 0) graphNodes[idx].isStart = true;
            dijkstraMode = DijkstraMode::DESTINATION_MODE;
            menuIndex = 0;
        }
    }
    else if (dijkstraMode == DijkstraMode::DESTINATION_MODE) {
        if (menuOptions[menuIndex] == "SPECIFIC LOCATION") {
            dijkstraMode = DijkstraMode::END_SECTOR;
            menuIndex = 0;
        }
        else {
            if (menuIndex == 0) dijkstraTargetType = "SCHOOL";
            else if (menuIndex == 1) dijkstraTargetType = "HOSPITAL";
            else if (menuIndex == 2) dijkstraTargetType = "PHARMACY";
            else dijkstraTargetType = "STOP";

            runDijkstraAlgorithm();
            dijkstraMode = DijkstraMode::COMPLETE;
        }
    }
    else if (dijkstraMode == DijkstraMode::END_SECTOR) {
        selEndSector = menuOptions[menuIndex];
        dijkstraMode = DijkstraMode::END_CATEGORY;
        menuIndex = 0;
    }
    else if (dijkstraMode == DijkstraMode::END_CATEGORY) {
        selEndCat = menuOptions[menuIndex];
        dijkstraMode = DijkstraMode::END_NODE;
        menuIndex = 0;
    }
    else if (dijkstraMode == DijkstraMode::END_NODE) {
        if (!menuNodeIDs.empty()) {
            dijkstraEndNode = menuNodeIDs[menuIndex];
            dijkstraTargetType = ""; // Specific
            runDijkstraAlgorithm();
            dijkstraMode = DijkstraMode::COMPLETE;
        }
    }
    updateDijkstraMenu();
    updateFilteredNodes();
}

inline void CitySimulator::runDijkstraAlgorithm() {
    if (!islamabad || !islamabad->getCityGraph() || dijkstraStartNode < 0) return;
    CityGraph* graph = islamabad->getCityGraph();

    if (!dijkstraTargetType.empty()) {
        dijkstraEndNode = graph->findNearestFacility(dijkstraStartNode, dijkstraTargetType);
    }

    if (dijkstraEndNode >= 0) {
        dijkstraPath = graph->findShortestPath(dijkstraStartNode, dijkstraEndNode, dijkstraDistance);

        for (int i = 0; i < dijkstraPath.getSize(); i++) {
            int idx = nodeIdToIndex[dijkstraPath[i]];
            if (idx >= 0) graphNodes[idx].isOnPath = true;
        }
        for (int i = 0; i < dijkstraPath.getSize() - 1; i++) {
            int from = dijkstraPath[i], to = dijkstraPath[i + 1];
            for (auto& edge : graphEdges) {
                if ((edge.fromID == from && edge.toID == to) || (edge.fromID == to && edge.toID == from))
                    edge.isOnPath = true;
            }
        }
        int sIdx = nodeIdToIndex[dijkstraStartNode]; if (sIdx >= 0) graphNodes[sIdx].isStart = true;
        int eIdx = nodeIdToIndex[dijkstraEndNode]; if (eIdx >= 0) graphNodes[eIdx].isEnd = true;
    }
}

// ============================================================================
// MAIN MENU & STARTUP
// ============================================================================

inline void CitySimulator::runMainMenu() {
    auto screen = ScreenInteractive::Fullscreen();
    std::vector<string> options;

    if (cityInitialized) {
        options = { "Graph View [1]", "Dijkstra Pathfinding [D]", "Simulation Dashboard [V]", "Route Wizard [R]", "Database [2]", "Search Engine [S]", "Management [3]", "Exit" };
    }
    else {
        options = { "Initialize City", "Exit" };
    }
    int sel = 0;

    auto renderer = Renderer([&] {
        Elements items;
        for (int i = 0; i < (int)options.size(); i++) {
            auto item = text((i == sel ? " > " : "   ") + options[i]);
            if (i == sel) item = item | bold | color(Color::Green);
            items.push_back(item);
        }

        string statusText = cityInitialized ? "CITY LOADED" : "NOT INITIALIZED";
        Color statusColor = cityInitialized ? Color::Green : Color::Yellow;

        Elements titleArt;
        for (int i = 0; i < 6; i++) {
            titleArt.push_back(text(ASCIIArt::ISLAMABAD_TITLE[i]) | color(Color::Green));
        }

        auto menuBox = vbox({
            text("MAIN MENU") | bold | center | color(Color::Cyan),
            separator(),
            text(statusText) | center | color(statusColor),
            separator(),
            vbox(items),
            }) | border | size(WIDTH, EQUAL, 35);

        return vbox({
            filler(),
            vbox(titleArt) | center,
            text("R E D E F I N E D") | bold | center | color(Color::GrayLight),
            text(""),
            hbox({ filler(), menuBox, filler() }),
            filler()
            });
        });

    auto comp = CatchEvent(renderer, [&](Event e) {
        if (e == Event::ArrowUp) { sel = (sel - 1 + options.size()) % options.size(); return true; }
        if (e == Event::ArrowDown) { sel = (sel + 1) % options.size(); return true; }
        if (e == Event::Return) {
            if (!cityInitialized) {
                currentState = (sel == 0) ? SimulatorState::CSV_SELECTION : SimulatorState::EXIT;
            }
            else {
                if (sel == 0) currentState = SimulatorState::GRAPH_VIEW;
                else if (sel == 1) currentState = SimulatorState::DIJKSTRA_VIEW;
                else if (sel == 2) currentState = SimulatorState::SIMULATION_DASHBOARD;
                else if (sel == 3) currentState = SimulatorState::ROUTE_WIZARD;
                else if (sel == 4) currentState = SimulatorState::DATABASE_VIEW;
                else if (sel == 5) currentState = SimulatorState::SEARCH_VIEW;
                else if (sel == 6) currentState = SimulatorState::MANAGEMENT_MENU;
                else currentState = SimulatorState::EXIT;
            }
            screen.Exit();
            return true;
        }
        if (cityInitialized) {
            if (e == Event::Character('1')) { currentState = SimulatorState::GRAPH_VIEW; screen.Exit(); return true; }
            if (e == Event::Character('2')) { currentState = SimulatorState::DATABASE_VIEW; screen.Exit(); return true; }
            if (e == Event::Character('3')) { currentState = SimulatorState::MANAGEMENT_MENU; screen.Exit(); return true; }
            if (e == Event::Character('d') || e == Event::Character('D')) {
                currentState = SimulatorState::DIJKSTRA_VIEW; screen.Exit(); return true;
            }
            if (e == Event::Character('v') || e == Event::Character('V')) {
                currentState = SimulatorState::SIMULATION_DASHBOARD; screen.Exit(); return true;
            }
            if (e == Event::Character('r') || e == Event::Character('R')) {
                currentState = SimulatorState::ROUTE_WIZARD; screen.Exit(); return true;
            }
            if (e == Event::Character('s') || e == Event::Character('S')) {
                currentState = SimulatorState::SEARCH_VIEW; screen.Exit(); return true;
            }
        }
        if (e == Event::Escape) { currentState = SimulatorState::EXIT; screen.Exit(); return true; }
        return false;
        });
    screen.Loop(comp);
}

inline void CitySimulator::run() {
    while (currentState != SimulatorState::EXIT) {
        switch (currentState) {
        case SimulatorState::INTRO_PHASE_1: runIntroPhase1(); break;
        case SimulatorState::INTRO_PHASE_2: runIntroPhase2(); break;
        case SimulatorState::INTRO_PHASE_3: runIntroPhase3(); break;
        case SimulatorState::WELCOME_ANIMATION: runWelcomeAnimation(); break;
        case SimulatorState::MAIN_MENU: runMainMenu(); break;
        case SimulatorState::CSV_SELECTION: runCSVSelection(); break;
        case SimulatorState::LOADING: runLoadingScreen(); break;
        case SimulatorState::GRAPH_VIEW: runGraphView(); break;
        case SimulatorState::DATABASE_VIEW: runDatabaseView(); break;
        case SimulatorState::MANAGEMENT_MENU: runManagementMenu(); break;
        case SimulatorState::DIJKSTRA_VIEW: runDijkstraView(); break;
        case SimulatorState::SEARCH_VIEW: runSearchView(); break;
        case SimulatorState::SIMULATION_DASHBOARD: runSimulationDashboard(); break;
        case SimulatorState::ROUTE_WIZARD: runRouteWizard(); break;
        case SimulatorState::EXIT: break;
        }
    }
    std::cout << "\nThank you for using Islamabad Redefined!\n" << std::endl;
}

// ============================================================================
// INTRO AND DEBUG
// ============================================================================

inline void CitySimulator::runDebugMode() {
    std::cout << "=== DEBUG MODE ===" << std::endl;
    islamabad = new SmartCity();
    islamabad->setDatasetPaths(stopsCSV, schoolsCSV, hospitalsCSV, pharmaciesCSV,
        busesCSV, populationCSV, mallsCSV, shopsCSV, ambulancesCSV);
    islamabad->initialize();
    cityInitialized = true;
    cityMgmt = new CityManagement(islamabad);
    runGraphView();
}

inline void CitySimulator::runIntroPhase1() {
    auto screen = ScreenInteractive::Fullscreen();
    const string fullText = "A product of Rayyan's Emporium";
    std::atomic<int> charIndex{ 0 };
    std::atomic<bool> done{ false };

    auto renderer = Renderer([&] {
        string txt = fullText.substr(0, std::min(charIndex.load(), (int)fullText.length()));
        if (charIndex.load() < (int)fullText.length()) txt += "_";
        return vbox({ filler(), text(txt) | bold | center, filler() });
        });

    std::thread t([&]() {
        for (int i = 0; i <= (int)fullText.length(); i++) {
            charIndex.store(i); screen.PostEvent(Event::Custom); sleepMs(50);
        }
        sleepMs(800); done.store(true); screen.PostEvent(Event::Custom);
        });

    auto comp = CatchEvent(renderer, [&](Event e) {
        if (done.load()) { currentState = SimulatorState::INTRO_PHASE_2; screen.Exit(); return true; }
        return false;
        });
    screen.Loop(comp);
    if (t.joinable()) t.join();
}

inline void CitySimulator::runIntroPhase2() {
    auto screen = ScreenInteractive::Fullscreen();
    const string fullText = "Created by Rayyan, Omar and Aryan";
    std::atomic<int> charIndex{ 0 };
    std::atomic<bool> done{ false };

    auto renderer = Renderer([&] {
        string txt = fullText.substr(0, std::min(charIndex.load(), (int)fullText.length()));
        if (charIndex.load() < (int)fullText.length()) txt += "_";
        return vbox({ filler(), text(txt) | bold | center, filler() });
        });

    std::thread t([&]() {
        for (int i = 0; i <= (int)fullText.length(); i++) {
            charIndex.store(i); screen.PostEvent(Event::Custom); sleepMs(45);
        }
        sleepMs(800); done.store(true); screen.PostEvent(Event::Custom);
        });

    auto comp = CatchEvent(renderer, [&](Event e) {
        if (done.load()) { currentState = SimulatorState::INTRO_PHASE_3; screen.Exit(); return true; }
        return false;
        });
    screen.Loop(comp);
    if (t.joinable()) t.join();
}

inline void CitySimulator::runIntroPhase3() {
    auto screen = ScreenInteractive::Fullscreen();
    std::atomic<bool> done{ false };

    auto renderer = Renderer([&] {
        Elements lines;
        for (int i = 0; i < ASCIIArt::ISLAMABAD_TITLE_HEIGHT; i++)
            lines.push_back(text(ASCIIArt::ISLAMABAD_TITLE[i]) | bold);
        lines.push_back(text(""));
        for (int i = 0; i < ASCIIArt::REDEFINED_TITLE_HEIGHT; i++)
            lines.push_back(text(ASCIIArt::REDEFINED_TITLE[i]) | color(Color::GrayLight));
        return vbox({ filler(), vbox(lines) | center, filler(),
                     text(done.load() ? "Press Enter" : "") | center | dim, filler() });
        });

    std::thread t([&]() { sleepMs(1500); done.store(true); screen.PostEvent(Event::Custom); });

    auto comp = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Return && done.load()) { currentState = SimulatorState::MAIN_MENU; screen.Exit(); return true; }
        return false;
        });
    screen.Loop(comp);
    if (t.joinable()) t.join();
}

inline void CitySimulator::runWelcomeAnimation() { currentState = SimulatorState::MAIN_MENU; }

#endif // CITY_SIMULATOR_ENHANCED_H