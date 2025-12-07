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
    bool isOnPath;      // For Dijkstra visualization
    bool isVisited;     // For Dijkstra visualization
    bool isStart;       // Starting node
    bool isEnd;         // Destination node

    GraphNode2D() : id(-1), pos(), name(""), type(""), sector(""),
        color(Color::White), isCorner(false), isOnPath(false),
        isVisited(false), isStart(false), isEnd(false) {
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
// GRAPH VIEWPORT - Simple coordinate transformation (no normalization)
// ============================================================================
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
        canvasWidth(160), canvasHeight(80),
        offsetX(0), offsetY(0), zoom(1.0) {
    }

    void setBounds(double minLa, double maxLa, double minLo, double maxLo) {
        minLat = minLa; maxLat = maxLa; minLon = minLo; maxLon = maxLo;
    }

    void setCanvasSize(int w, int h) {
        canvasWidth = w; canvasHeight = h;
    }

    // Simple linear mapping - keeps natural tilt of geographic data
    Point2D geoToCanvas(double lat, double lon) const {
        // Map longitude to X (west to east = left to right)
        double normX = (lon - minLon) / (maxLon - minLon);
        // Map latitude to Y (north to south = top to bottom, so invert)
        double normY = (maxLat - lat) / (maxLat - minLat);

        // Apply zoom centered
        normX = (normX - 0.5) * zoom + 0.5 + offsetX;
        normY = (normY - 0.5) * zoom + 0.5 + offsetY;

        // Small padding
        double pad = 0.02;
        double canvasX = pad * canvasWidth + normX * canvasWidth * (1.0 - 2 * pad);
        double canvasY = pad * canvasHeight + normY * canvasHeight * (1.0 - 2 * pad);

        return Point2D(canvasX, canvasY);
    }

    void zoomIn() { zoom *= 1.2; if (zoom > 5.0) zoom = 5.0; }
    void zoomOut() { zoom /= 1.2; if (zoom < 0.5) zoom = 0.5; }
    void panLeft() { offsetX -= 0.1 / zoom; }
    void panRight() { offsetX += 0.1 / zoom; }
    void panUp() { offsetY -= 0.1 / zoom; }
    void panDown() { offsetY += 0.1 / zoom; }
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
    EXIT
};

enum class CSVLoadMode { DEMO_MODE, FULL_MODE };

enum class DijkstraMode {
    SELECT_START,
    SELECT_TARGET_TYPE,
    RUNNING,
    COMPLETE
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

    // Dijkstra visualization state
    DijkstraMode dijkstraMode;
    int dijkstraStartNode;
    int dijkstraEndNode;
    string dijkstraTargetType;  // "SCHOOL", "HOSPITAL", "STOP", "SECTOR", etc.
    Vector<int> dijkstraPath;
    double dijkstraDistance;
    int dijkstraNodeSelection;  // For selecting start node
    std::vector<int> selectableNodes;  // Nodes user can select from

    string stopsCSV, schoolsCSV, hospitalsCSV, pharmaciesCSV;
    string busesCSV, populationCSV, mallsCSV, shopsCSV, ambulancesCSV;

    // Counter for naming intersections
    int intersectionCounter;

    Color getNodeColor(const string& type) {
        if (type == "CORNER") return Color::GrayDark;
        if (type == "STOP") return Color::GreenLight;
        if (type == "SCHOOL") return Color::Blue;
        if (type == "HOSPITAL") return Color::Red;
        if (type == "PHARMACY") return Color::Magenta;
        if (type == "MALL") return Color::Yellow;
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
    void runDijkstraView();
    void runEditObjectView(const string& objectID, const string& objectType); // New Edit View

    void runAddFacilityForm(const string& sector);
    void runAddOfferingForm(CityNode* node);

    // ------- EDITING FUNCTIONS MANAGEMENT -------
    void runInputForm(const string& title, const std::vector<string>& labels, std::function<void(std::vector<string>)> onConfirm);
    Citizen* runPopulationSelector(const string& title);
    void runEditSchoolView(School* school);
	void runEditHospitalView(Hospital* hospital);
	void runEditShopView(Shop* shop, Mall* mall);


    Canvas renderGraphToCanvas(int width, int height);
    void updateHoverState(int mx, int my);
    string getHoverInfo();

    // Dijkstra helpers
    void clearDijkstraVisualization();
    void runDijkstraAlgorithm();
    void buildSelectableNodesList();

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
    dijkstraMode(DijkstraMode::SELECT_START),
    dijkstraStartNode(-1), dijkstraEndNode(-1),
    dijkstraTargetType(""), dijkstraDistance(0.0),
    dijkstraNodeSelection(0), intersectionCounter(0) {

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
// GRAPH VISUALIZATION - Cleaned up, single block per node
// ============================================================================

inline void CitySimulator::buildGraphVisualization() {
    graphNodes.clear();
    graphEdges.clear();
    sectorRegions.clear();
    nodeIdToIndex.clear();
    intersectionCounter = 0;

    if (!islamabad || !islamabad->getCityGraph()) return;
    CityGraph* graph = islamabad->getCityGraph();

    // Use actual data bounds
    viewport.setBounds(BASE_LAT, MAX_LAT, BASE_LON, MAX_LON);

    int maxId = 0;
    for (int i = 0; i < graph->getNodeCount(); i++) {
        CityNode* node = graph->getNode(i);
        if (node && node->id > maxId) maxId = node->id;
    }
    nodeIdToIndex.resize(maxId + 1, -1);

    // Build graph nodes
    for (int i = 0; i < graph->getNodeCount(); i++) {
        CityNode* node = graph->getNode(i);
        if (!node) continue;

        GraphNode2D gNode;
        gNode.id = node->id;
        gNode.pos = viewport.geoToCanvas(node->lat, node->lon);
        gNode.type = node->type;
        gNode.sector = node->sector;
        gNode.color = getNodeColor(node->type);
        gNode.isCorner = (node->type == "CORNER");
        gNode.isOnPath = false;
        gNode.isVisited = false;
        gNode.isStart = false;
        gNode.isEnd = false;

        // Rename corners to "Intersection X"
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

    // Build edges from actual road connections in the graph
    for (int i = 0; i < graph->getNodeCount(); i++) {
        CityNode* node = graph->getNode(i);
        if (!node) continue;

        const LinkedList<Edge>& roads = node->getRoads();
        for (int j = 0; j < roads.size(); j++) {
            Edge edge = roads[j];
            // Only add edge once (when fromID < toID)
            if (node->id < edge.destinationID) {
                GraphEdge2D gEdge(node->id, edge.destinationID);
                gEdge.isOnPath = false;
                graphEdges.push_back(gEdge);
            }
        }
    }

    // Build sector regions from hardcoded SECTOR_GRID
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

inline Canvas CitySimulator::renderGraphToCanvas(int width, int height) {
    Canvas canvas(width * 2, height * 4);
    viewport.setCanvasSize(width * 2, height * 4);

    int cw = width * 2;
    int ch = height * 4;

    auto stylize = [](Color c) -> Canvas::Stylizer {
        return [c](Pixel& p) { p.foreground_color = c; };
        };

    // Draw sector boundaries if enabled
    if (showSectorBounds) {
        for (const auto& region : sectorRegions) {
            int x1 = (int)region.topLeft.x;
            int y1 = (int)region.topLeft.y;
            int x2 = (int)region.bottomRight.x;
            int y2 = (int)region.bottomRight.y;

            Color boundColor = region.isHovered ? Color::Yellow : Color::GrayDark;
            canvas.DrawBlockLine(x1, y1, x2, y1, stylize(boundColor));
            canvas.DrawBlockLine(x2, y1, x2, y2, stylize(boundColor));
            canvas.DrawBlockLine(x2, y2, x1, y2, stylize(boundColor));
            canvas.DrawBlockLine(x1, y2, x1, y1, stylize(boundColor));
        }
    }

    // Draw roads (edges)
    if (showRoads) {
        for (const auto& edge : graphEdges) {
            int idx1 = (edge.fromID < (int)nodeIdToIndex.size()) ? nodeIdToIndex[edge.fromID] : -1;
            int idx2 = (edge.toID < (int)nodeIdToIndex.size()) ? nodeIdToIndex[edge.toID] : -1;
            if (idx1 >= 0 && idx2 >= 0 && idx1 < (int)graphNodes.size() && idx2 < (int)graphNodes.size()) {
                const GraphNode2D& n1 = graphNodes[idx1];
                const GraphNode2D& n2 = graphNodes[idx2];
                int x1 = (int)n1.pos.x, y1 = (int)n1.pos.y;
                int x2 = (int)n2.pos.x, y2 = (int)n2.pos.y;

                // Color based on Dijkstra path
                Color roadColor = Color::GrayDark;
                if (edge.isOnPath) {
                    roadColor = Color::Green;
                }
                canvas.DrawBlockLine(x1, y1, x2, y2, stylize(roadColor));
            }
        }
    }

    // Draw corners (intersections) - single block each
    if (showCorners) {
        for (const auto& node : graphNodes) {
            if (!node.isCorner) continue;
            int x = (int)node.pos.x;
            int y = (int)node.pos.y;
            if (x >= 0 && x < cw && y >= 0 && y < ch) {
                Color c = node.isOnPath ? Color::GreenLight : Color::GrayLight;
                canvas.DrawBlock(x, y, true, stylize(c));
            }
        }
    }

    // Draw facilities - single block each with color coding
    for (const auto& node : graphNodes) {
        if (node.isCorner) continue;
        int x = (int)node.pos.x;
        int y = (int)node.pos.y;
        if (x < 0 || x >= cw || y < 0 || y >= ch) continue;

        Color nodeColor = node.color;

        // Override colors for Dijkstra visualization
        if (node.isStart) {
            nodeColor = Color::Cyan;
        }
        else if (node.isEnd) {
            nodeColor = Color::Yellow;
        }
        else if (node.isOnPath) {
            nodeColor = Color::GreenLight;
        }
        else if (node.isVisited) {
            nodeColor = Color::Orange1;
        }

        // Hovered node highlight
        if (node.id == hoveredNodeID) {
            nodeColor = Color::White;
        }

        // Draw single block
        canvas.DrawBlock(x, y, true, stylize(nodeColor));
    }

    return canvas;
}

inline void CitySimulator::updateHoverState(int mx, int my) {
    mouseX = mx;
    mouseY = my;
    hoveredNodeID = -1;
    hoveredSector = "";

    int canvasX = (mx - 1) * 2;
    int canvasY = (my - 2) * 4;

    double minDist = 20.0;
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
        region.isHovered = region.contains(Point2D(canvasX, canvasY));
        if (region.isHovered) {
            hoveredSector = region.name;
        }
    }
}

inline string CitySimulator::getHoverInfo() {
    std::stringstream ss;

    if (hoveredNodeID >= 0 && hoveredNodeID < (int)nodeIdToIndex.size()) {
        int idx = nodeIdToIndex[hoveredNodeID];
        if (idx >= 0 && idx < (int)graphNodes.size()) {
            const GraphNode2D& node = graphNodes[idx];
            ss << "NODE: " << node.name << "\n";
            ss << "Type: " << node.type << "\n";
            ss << "Sector: " << node.sector << "\n";
            ss << "ID: " << node.id;
            return ss.str();
        }
    }

    if (!hoveredSector.empty()) {
        ss << "SECTOR: " << hoveredSector;
        return ss.str();
    }

    ss << "Hover over\nnodes for info";
    return ss.str();
}

// ============================================================================
// DIJKSTRA VISUALIZATION HELPERS
// ============================================================================

inline void CitySimulator::clearDijkstraVisualization() {
    for (auto& node : graphNodes) {
        node.isOnPath = false;
        node.isVisited = false;
        node.isStart = false;
        node.isEnd = false;
    }
    for (auto& edge : graphEdges) {
        edge.isOnPath = false;
    }
    dijkstraPath.clear();
    dijkstraDistance = 0.0;
    dijkstraStartNode = -1;
    dijkstraEndNode = -1;
}

inline void CitySimulator::buildSelectableNodesList() {
    selectableNodes.clear();
    for (const auto& node : graphNodes) {
        if (!node.isCorner) {
            selectableNodes.push_back(node.id);
        }
    }
}

inline void CitySimulator::runDijkstraAlgorithm() {
    if (!islamabad || !islamabad->getCityGraph()) return;
    if (dijkstraStartNode < 0) return;

    CityGraph* graph = islamabad->getCityGraph();

    // Find target based on type
    if (dijkstraTargetType == "SCHOOL" || dijkstraTargetType == "HOSPITAL" ||
        dijkstraTargetType == "PHARMACY" || dijkstraTargetType == "STOP") {
        dijkstraEndNode = graph->findNearestFacility(dijkstraStartNode, dijkstraTargetType);
    }

    if (dijkstraEndNode < 0) return;

    // Run Dijkstra
    dijkstraPath = graph->findShortestPath(dijkstraStartNode, dijkstraEndNode, dijkstraDistance);

    // Mark nodes on path
    for (int i = 0; i < dijkstraPath.getSize(); i++) {
        int nodeId = dijkstraPath[i];
        if (nodeId < (int)nodeIdToIndex.size()) {
            int idx = nodeIdToIndex[nodeId];
            if (idx >= 0 && idx < (int)graphNodes.size()) {
                graphNodes[idx].isOnPath = true;
            }
        }
    }

    // Mark start and end
    if (dijkstraStartNode < (int)nodeIdToIndex.size()) {
        int idx = nodeIdToIndex[dijkstraStartNode];
        if (idx >= 0) graphNodes[idx].isStart = true;
    }
    if (dijkstraEndNode < (int)nodeIdToIndex.size()) {
        int idx = nodeIdToIndex[dijkstraEndNode];
        if (idx >= 0) graphNodes[idx].isEnd = true;
    }

    // Mark edges on path
    for (int i = 0; i < dijkstraPath.getSize() - 1; i++) {
        int from = dijkstraPath[i];
        int to = dijkstraPath[i + 1];
        for (auto& edge : graphEdges) {
            if ((edge.fromID == from && edge.toID == to) ||
                (edge.fromID == to && edge.toID == from)) {
                edge.isOnPath = true;
            }
        }
    }
}

// ============================================================================
// MAIN LOOP
// ============================================================================

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
        case SimulatorState::SEARCH_VIEW: runSearchView(); break; // New State
        case SimulatorState::EXIT: break;
        }
    }
    std::cout << "\nThank you for using Islamabad Redefined!\n" << std::endl;
}

// ============================================================================
// INTRO PHASES
// ============================================================================

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

// ============================================================================
// DEBUG MODE
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

// ============================================================================
// MAIN MENU
// ============================================================================

inline void CitySimulator::runMainMenu() {
    auto screen = ScreenInteractive::Fullscreen();
    std::vector<string> options;

    if (cityInitialized) {
        options = { "Graph View [1]", "Dijkstra Pathfinding [D]", "Database [2]", "Search Engine [S]", "Management [3]", "Exit" };
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
                else if (sel == 2) currentState = SimulatorState::DATABASE_VIEW;
                else if (sel == 3) currentState = SimulatorState::SEARCH_VIEW;
                else if (sel == 4) currentState = SimulatorState::MANAGEMENT_MENU;
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
            if (e == Event::Character('s') || e == Event::Character('S')) {
                currentState = SimulatorState::SEARCH_VIEW; screen.Exit(); return true;
            }
        }
        if (e == Event::Escape) { currentState = SimulatorState::EXIT; screen.Exit(); return true; }
        return false;
        });
    screen.Loop(comp);
}

// ============================================================================
// CSV SELECTION & LOADING
// ============================================================================

inline void CitySimulator::runCSVSelection() {
    auto screen = ScreenInteractive::Fullscreen();
    int sel = 0;

    auto renderer = Renderer([&] {
        Elements csvList;
        csvList.push_back(text("DATASET FILES") | bold | color(Color::Cyan));
        csvList.push_back(separator());

        auto fileRow = [&](const string& label, const string& path) {
            bool exists = fileExists(path);
            Color statusColor = exists ? Color::Green : Color::Red;
            string statusIcon = exists ? "[OK]" : "[MISSING]";
            return hbox({
                text(label + ": ") | bold | size(WIDTH, EQUAL, 14),
                text(path) | dim | size(WIDTH, EQUAL, 30),
                text(" " + statusIcon) | color(statusColor)
                });
            };

        csvList.push_back(fileRow("Stops", stopsCSV));
        csvList.push_back(fileRow("Schools", schoolsCSV));
        csvList.push_back(fileRow("Hospitals", hospitalsCSV));
        csvList.push_back(fileRow("Pharmacies", pharmaciesCSV));
        csvList.push_back(fileRow("Buses", busesCSV));
        csvList.push_back(fileRow("Population", populationCSV));
        csvList.push_back(fileRow("Malls", mallsCSV));
        csvList.push_back(fileRow("Shops", shopsCSV));
        csvList.push_back(fileRow("Ambulances", ambulancesCSV));
        csvList.push_back(separator());

        std::vector<string> options = { "Begin Initialization", "Back to Menu" };
        for (int i = 0; i < (int)options.size(); i++) {
            auto item = text((i == sel ? " > " : "   ") + options[i]);
            if (i == sel) item = item | bold | color(Color::Green);
            csvList.push_back(item);
        }

        auto box = vbox(csvList) | border | size(WIDTH, EQUAL, 58);
        return vbox({ filler(), text("CITY INITIALIZATION") | bold | center | color(Color::Green),
            text(""), hbox({ filler(), box, filler() }), text(""),
            text("Press Enter to select, Esc to go back") | center | dim, filler() });
        });

    auto comp = CatchEvent(renderer, [&](Event e) {
        if (e == Event::ArrowUp) { sel = (sel - 1 + 2) % 2; return true; }
        if (e == Event::ArrowDown) { sel = (sel + 1) % 2; return true; }
        if (e == Event::Return) {
            if (sel == 0) currentState = SimulatorState::LOADING;
            else currentState = SimulatorState::MAIN_MENU;
            screen.Exit(); return true;
        }
        if (e == Event::Escape) { currentState = SimulatorState::MAIN_MENU; screen.Exit(); return true; }
        return false;
        });
    screen.Loop(comp);
}


inline void CitySimulator::runLoadingScreen() {
    auto screen = ScreenInteractive::Fullscreen();
    std::atomic<int> step{ 0 };
    std::atomic<bool> done{ false };
    std::atomic<int> totalSteps{ 14 };  // Total loading steps

    // Loading stage descriptions
    std::vector<string> loadingStages = {
        "Initializing city graph...",
        "Loading sector frames...",
        "Loading bus stops...",
        "Loading schools...",
        "Loading hospitals...",
        "Loading pharmacies...",
        "Loading buses...",
        "Loading ambulances...",
        "Loading school buses...",
        "Loading population data...",
        "Loading malls...",
        "Loading shops...",
        "Setting up transport queues...",
        "Finalizing initialization..."
    };

    auto renderer = Renderer([&] {
        int s = step.load();
        int barWidth = 50;
        int filledWidth = (s * barWidth) / totalSteps.load();
        string progressBar = "";
        for (int i = 0; i < barWidth; i++) {
            progressBar += (i < filledWidth) ? "█" : "░";
        }

        // Get current stage description
        string stageDesc = (s < (int)loadingStages.size()) ? loadingStages[s] : "Completing...";

        return vbox({
            filler(),
            text("LOADING ISLAMABAD") | bold | center | color(Color::Green),
            text(""),
            hbox({text("["), text(progressBar) | color(Color::Green), text("]")}) | center,
            text(std::to_string((s * 100) / totalSteps.load()) + "%") | center,
            text(""),
            text(stageDesc) | center | color(Color::Cyan),
            text(""),
            text(done.load() ? "Press Enter to continue" : "Please wait...") | center | dim,
            filler()
            });
        });

    std::thread t([&]() {
        // Step 0: Create SmartCity
        islamabad = new SmartCity();
        islamabad->setDatasetPaths(stopsCSV, schoolsCSV, hospitalsCSV, pharmaciesCSV,
            busesCSV, populationCSV, mallsCSV, shopsCSV, ambulancesCSV);
        step.store(1); screen.PostEvent(Event::Custom); sleepMs(80);

        // Steps 1-13: Initialize (SmartCity::initialize handles the rest internally)
        // We simulate progress here as initialize() is a single call
        for (int i = 2; i <= 6; i++) {
            step.store(i); screen.PostEvent(Event::Custom); sleepMs(60);
        }

        islamabad->initialize();
        cityInitialized = true;

        for (int i = 7; i <= 12; i++) {
            step.store(i); screen.PostEvent(Event::Custom); sleepMs(50);
        }

        cityMgmt = new CityManagement(islamabad);

        step.store(13); screen.PostEvent(Event::Custom); sleepMs(50);
        step.store(14); screen.PostEvent(Event::Custom); sleepMs(100);

        done.store(true); screen.PostEvent(Event::Custom);
        });

    auto comp = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Return && done.load()) { currentState = SimulatorState::MAIN_MENU; screen.Exit(); return true; }
        return false;
        });
    screen.Loop(comp);
    if (t.joinable()) t.join();
}

// ============================================================================
// GRAPH VIEW
// ============================================================================

inline void CitySimulator::runGraphView() {
    auto screen = ScreenInteractive::Fullscreen();
    buildGraphVisualization();

    auto renderer = Renderer([&] {
        int termW = Terminal::Size().dimx;
        int termH = Terminal::Size().dimy;
        int canvasW = termW - 35;
        int canvasH = termH - 4;

        Canvas c = renderGraphToCanvas(canvasW, canvasH);

        string hoverText = getHoverInfo();
        Elements hoverLines;
        std::istringstream iss(hoverText);
        string line;
        while (std::getline(iss, line)) hoverLines.push_back(text(line));

        auto infoPanel = vbox({
            text("LEGEND") | bold | color(Color::Cyan),
            separator(),
            hbox({text("■") | color(Color::GreenLight), text(" Stop")}),
            hbox({text("■") | color(Color::Blue), text(" School")}),
            hbox({text("■") | color(Color::Red), text(" Hospital")}),
            hbox({text("■") | color(Color::Magenta), text(" Pharmacy")}),
            hbox({text("■") | color(Color::Yellow), text(" Mall")}),
            separator(),
            text("INFO") | bold | color(Color::Yellow),
            vbox(hoverLines),
            separator(),
            text("KEYS") | bold | color(Color::Cyan),
            text("+/-: Zoom"),
            text("Arrows: Pan"),
            text("R: Roads"),
            text("C: Corners"),
            text("S: Sectors"),
            text("D: Dijkstra"),
            separator(),
            text("Esc: Menu"),
            }) | border | size(WIDTH, EQUAL, 20);

        return vbox({
            text("ISLAMABAD MAP") | bold | center | color(Color::Green),
            hbox({ canvas(c) | border | flex, text(" "), infoPanel }) | flex,
            });
        });

    auto comp = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Character('+') || e == Event::Character('=')) { viewport.zoomIn(); buildGraphVisualization(); return true; }
        if (e == Event::Character('-')) { viewport.zoomOut(); buildGraphVisualization(); return true; }
        if (e == Event::ArrowLeft) { viewport.panLeft(); buildGraphVisualization(); return true; }
        if (e == Event::ArrowRight) { viewport.panRight(); buildGraphVisualization(); return true; }
        if (e == Event::ArrowUp) { viewport.panUp(); buildGraphVisualization(); return true; }
        if (e == Event::ArrowDown) { viewport.panDown(); buildGraphVisualization(); return true; }
        if (e == Event::Character('r') || e == Event::Character('R')) { showRoads = !showRoads; return true; }
        if (e == Event::Character('c') || e == Event::Character('C')) { showCorners = !showCorners; return true; }
        if (e == Event::Character('s') || e == Event::Character('S')) { showSectorBounds = !showSectorBounds; return true; }
        if (e == Event::Character('d') || e == Event::Character('D')) { currentState = SimulatorState::DIJKSTRA_VIEW; screen.Exit(); return true; }
        if (e.is_mouse()) { updateHoverState(e.mouse().x, e.mouse().y); return true; }
        if (e == Event::Escape) { currentState = SimulatorState::MAIN_MENU; screen.Exit(); return true; }
        return false;
        });
    screen.Loop(comp);
}

// ============================================================================
// DIJKSTRA VIEW - Interactive Pathfinding Visualization
// ============================================================================

inline void CitySimulator::runDijkstraView() {
    auto screen = ScreenInteractive::Fullscreen();
    buildGraphVisualization();
    clearDijkstraVisualization();
    buildSelectableNodesList();

    dijkstraMode = DijkstraMode::SELECT_START;
    dijkstraNodeSelection = 0;

    std::vector<string> targetTypes = { "Nearest School", "Nearest Hospital", "Nearest Pharmacy", "Nearest Bus Stop" };
    int targetSel = 0;

    auto renderer = Renderer([&] {
        int termW = Terminal::Size().dimx;
        int termH = Terminal::Size().dimy;
        int canvasW = termW - 35;
        int canvasH = termH - 4;

        Canvas c = renderGraphToCanvas(canvasW, canvasH);

        // Build control panel based on mode
        Elements controlItems;
        controlItems.push_back(text("DIJKSTRA PATHFINDING") | bold | color(Color::Cyan));
        controlItems.push_back(separator());

        if (dijkstraMode == DijkstraMode::SELECT_START) {
            controlItems.push_back(text("SELECT START") | bold | color(Color::Yellow));
            controlItems.push_back(separator());

            // Show scrollable list of nodes
            int startIdx = std::max(0, dijkstraNodeSelection - 5);
            int endIdx = std::min((int)selectableNodes.size(), startIdx + 12);

            for (int i = startIdx; i < endIdx; i++) {
                int nodeId = selectableNodes[i];
                int idx = nodeIdToIndex[nodeId];
                if (idx >= 0 && idx < (int)graphNodes.size()) {
                    string nodeName = graphNodes[idx].name;
                    if (nodeName.length() > 18) nodeName = nodeName.substr(0, 15) + "...";
                    auto item = text((i == dijkstraNodeSelection ? "> " : "  ") + nodeName);
                    if (i == dijkstraNodeSelection) item = item | bold | color(Color::Green);
                    controlItems.push_back(item);
                }
            }
            controlItems.push_back(separator());
            controlItems.push_back(text("Up/Down: Select") | dim);
            controlItems.push_back(text("Enter: Confirm") | dim);

        }
        else if (dijkstraMode == DijkstraMode::SELECT_TARGET_TYPE) {
            controlItems.push_back(text("SELECT TARGET") | bold | color(Color::Yellow));
            controlItems.push_back(separator());

            // Show start node
            if (dijkstraStartNode >= 0 && dijkstraStartNode < (int)nodeIdToIndex.size()) {
                int idx = nodeIdToIndex[dijkstraStartNode];
                if (idx >= 0) {
                    controlItems.push_back(text("From: " + graphNodes[idx].name.substr(0, 15)) | color(Color::Cyan));
                }
            }
            controlItems.push_back(separator());

            for (int i = 0; i < (int)targetTypes.size(); i++) {
                auto item = text((i == targetSel ? "> " : "  ") + targetTypes[i]);
                if (i == targetSel) item = item | bold | color(Color::Green);
                controlItems.push_back(item);
            }
            controlItems.push_back(separator());
            controlItems.push_back(text("Up/Down: Select") | dim);
            controlItems.push_back(text("Enter: Find Path") | dim);

        }
        else if (dijkstraMode == DijkstraMode::COMPLETE) {
            controlItems.push_back(text("PATH FOUND!") | bold | color(Color::Green));
            controlItems.push_back(separator());

            // Show path info
            if (dijkstraStartNode >= 0 && dijkstraStartNode < (int)nodeIdToIndex.size()) {
                int idx = nodeIdToIndex[dijkstraStartNode];
                if (idx >= 0) {
                    controlItems.push_back(text("From:") | bold);
                    controlItems.push_back(text(" " + graphNodes[idx].name.substr(0, 15)));
                }
            }
            if (dijkstraEndNode >= 0 && dijkstraEndNode < (int)nodeIdToIndex.size()) {
                int idx = nodeIdToIndex[dijkstraEndNode];
                if (idx >= 0) {
                    controlItems.push_back(text("To:") | bold);
                    controlItems.push_back(text(" " + graphNodes[idx].name.substr(0, 15)));
                }
            }
            controlItems.push_back(separator());
            controlItems.push_back(text("Distance:") | bold);
            controlItems.push_back(text(" " + std::to_string(dijkstraDistance).substr(0, 5) + " km") | color(Color::Yellow));
            controlItems.push_back(text("Stops:") | bold);
            controlItems.push_back(text(" " + std::to_string(dijkstraPath.getSize())) | color(Color::Yellow));
            controlItems.push_back(separator());
            controlItems.push_back(text("R: Reset") | dim);
            controlItems.push_back(text("Esc: Back") | dim);
        }

        // Legend
        controlItems.push_back(separator());
        controlItems.push_back(text("LEGEND") | bold);
        controlItems.push_back(hbox({ text("■") | color(Color::Cyan), text(" Start") }));
        controlItems.push_back(hbox({ text("■") | color(Color::Yellow), text(" End") }));
        controlItems.push_back(hbox({ text("■") | color(Color::GreenLight), text(" Path") }));

        auto controlPanel = vbox(controlItems) | border | size(WIDTH, EQUAL, 28);

        return vbox({
            text("DIJKSTRA VISUALIZATION") | bold | center | color(Color::Green),
            hbox({ canvas(c) | border | flex, text(" "), controlPanel }) | flex,
            });
        });

    auto comp = CatchEvent(renderer, [&](Event e) {
        if (dijkstraMode == DijkstraMode::SELECT_START) {
            if (e == Event::ArrowUp && dijkstraNodeSelection > 0) {
                dijkstraNodeSelection--;
                return true;
            }
            if (e == Event::ArrowDown && dijkstraNodeSelection < (int)selectableNodes.size() - 1) {
                dijkstraNodeSelection++;
                return true;
            }
            if (e == Event::Return && !selectableNodes.empty()) {
                dijkstraStartNode = selectableNodes[dijkstraNodeSelection];
                // Mark start node
                int idx = nodeIdToIndex[dijkstraStartNode];
                if (idx >= 0) graphNodes[idx].isStart = true;
                dijkstraMode = DijkstraMode::SELECT_TARGET_TYPE;
                return true;
            }
        }
        else if (dijkstraMode == DijkstraMode::SELECT_TARGET_TYPE) {
            if (e == Event::ArrowUp && targetSel > 0) {
                targetSel--;
                return true;
            }
            if (e == Event::ArrowDown && targetSel < (int)targetTypes.size() - 1) {
                targetSel++;
                return true;
            }
            if (e == Event::Return) {
                // Set target type
                if (targetSel == 0) dijkstraTargetType = "SCHOOL";
                else if (targetSel == 1) dijkstraTargetType = "HOSPITAL";
                else if (targetSel == 2) dijkstraTargetType = "PHARMACY";
                else dijkstraTargetType = "STOP";

                // Run algorithm
                runDijkstraAlgorithm();
                dijkstraMode = DijkstraMode::COMPLETE;
                return true;
            }
        }
        else if (dijkstraMode == DijkstraMode::COMPLETE) {
            if (e == Event::Character('r') || e == Event::Character('R')) {
                clearDijkstraVisualization();
                dijkstraMode = DijkstraMode::SELECT_START;
                dijkstraNodeSelection = 0;
                return true;
            }
        }

        // Common controls
        if (e == Event::Character('+') || e == Event::Character('=')) { viewport.zoomIn(); buildGraphVisualization(); return true; }
        if (e == Event::Character('-')) { viewport.zoomOut(); buildGraphVisualization(); return true; }
        if (e.is_mouse()) { updateHoverState(e.mouse().x, e.mouse().y); return true; }
        if (e == Event::Escape) {
            clearDijkstraVisualization();
            currentState = SimulatorState::GRAPH_VIEW;
            screen.Exit();
            return true;
        }
        return false;
        });
    screen.Loop(comp);
}
// ============================================================================
// ADD FACILITY FORM
// ============================================================================
inline void CitySimulator::runAddFacilityForm(const string& sector) {
    auto screen = ScreenInteractive::Fullscreen();

    // Form State
    string name_val;
    int type_selected = 0;
    vector<string> types = { "Pharmacy", "School", "Hospital", "Bus Stop" };
    string message = "";

    // Components
    Component input_name = Input(&name_val, "Enter Name");
    Component toggle_type = Toggle(&types, &type_selected);

    Component btn_add = Button("Create Facility", [&] {
        if (name_val.empty()) {
            message = "Error: Name cannot be empty!";
            return;
        }

        string newID = "";
        string type = types[type_selected];

        if (type == "Pharmacy") newID = cityMgmt->addPharmacy(name_val, sector);
        else if (type == "School") newID = cityMgmt->addSchool(name_val, sector, 3.0, {}, {}); // Default rating 3.0
        else if (type == "Hospital") newID = cityMgmt->addHospital(name_val, sector, 50, {}); // Default 50 beds
        else if (type == "Bus Stop") {
            // Auto-generate coords in sector
            int id = cityMgmt->addBusStopInSector(name_val, sector);
            if (id != -1) newID = "STOP-" + std::to_string(id);
        }

        if (!newID.empty() || type == "Bus Stop") {
            screen.Exit(); // Close form on success
        }
        else {
            message = "Error: Creation failed (ID generation error).";
        }
        });

    Component btn_cancel = Button("Cancel", screen.ExitLoopClosure());

    // Layout
    auto component = Container::Vertical({
        input_name,
        toggle_type,
        btn_add,
        btn_cancel
        });

    auto renderer = Renderer(component, [&] {
        return vbox({
            text("ADD NEW FACILITY") | bold | center | color(Color::Green),
            separator(),
            text("Sector: " + sector) | center,
            text(""),
            hbox({text("Name: "), input_name->Render() | border}),
            hbox({text("Type: "), toggle_type->Render() | border}),
            text(""),
            hbox({btn_add->Render(), text("  "), btn_cancel->Render()}) | center,
            text(""),
            text(message) | color(Color::Red) | center
            }) | border | size(WIDTH, EQUAL, 60) | center;
        });

    screen.Loop(renderer);
}

// ============================================================================
// ADD OFFERING FORM (Context-Sensitive)
// ============================================================================
inline void CitySimulator::runAddOfferingForm(CityNode* node) {
    if (!node) return;
    auto screen = ScreenInteractive::Fullscreen();
    string message = "";

    // --- PHARMACY FORM (Add Medicine) ---
    if (node->type == "PHARMACY") {
        string med_name, med_formula, med_price_str;

        Component input_name = Input(&med_name, "Medicine Name");
        Component input_formula = Input(&med_formula, "Formula");
        Component input_price = Input(&med_price_str, "Price");

        Component btn_add = Button("Add Medicine", [&] {
            try {
                float price = std::stof(med_price_str);
                if (cityMgmt->addMedicineToPharmacy(node->databaseID, med_name, med_formula, price)) {
                    screen.Exit();
                }
                else {
                    message = "Error: Could not add medicine.";
                }
            }
            catch (...) {
                message = "Error: Invalid Price.";
            }
            });

        auto component = Container::Vertical({ input_name, input_formula, input_price, btn_add, Button("Cancel", screen.ExitLoopClosure()) });

        auto renderer = Renderer(component, [&] {
            return vbox({
                text("ADD MEDICINE TO " + node->name) | bold | center | color(Color::Magenta),
                separator(),
                hbox({text("Name:    "), input_name->Render() | border}),
                hbox({text("Formula: "), input_formula->Render() | border}),
                hbox({text("Price:   "), input_price->Render() | border}),
                text(""),
                btn_add->Render() | center,
                text(message) | color(Color::Red) | center
                }) | border | size(WIDTH, EQUAL, 50) | center;
            });
        screen.Loop(renderer);
    }

    // --- HOSPITAL FORM (Add Specialization) ---
    else if (node->type == "HOSPITAL") {
        string spec_name;
        Component input_spec = Input(&spec_name, "Specialization (e.g. Cardiology)");
        Component btn_add = Button("Add Specialization", [&] {
            if (cityMgmt->addSpecializationToHospital(node->databaseID, spec_name)) screen.Exit();
            else message = "Error adding specialization.";
            });

        auto component = Container::Vertical({ input_spec, btn_add, Button("Cancel", screen.ExitLoopClosure()) });

        auto renderer = Renderer(component, [&] {
            return vbox({
                text("UPDATE HOSPITAL: " + node->name) | bold | center | color(Color::Red),
                separator(),
                hbox({text("New Dept: "), input_spec->Render() | border}),
                text(""),
                btn_add->Render() | center,
                text(message) | color(Color::Red) | center
                }) | border | size(WIDTH, EQUAL, 50) | center;
            });
        screen.Loop(renderer);
    }
    // ... Add other types (School, Mall) as needed ...
    else {
        // Generic fallback for types without specific "Offerings"
        auto renderer = Renderer([&] {
            return vbox({
                text("No addable offerings for " + node->type) | center,
                text("Press Enter to go back") | dim | center
                }) | border | center;
            });
        auto comp = CatchEvent(renderer, [&](Event e) { if (e == Event::Return) screen.Exit(); return true; });
        screen.Loop(comp);
    }
}

// ============================================================================
// DATABASE VIEW - Robust Sector Browser with Details Panel
// ============================================================================

inline void CitySimulator::runDatabaseView() {
    auto screen = ScreenInteractive::Fullscreen();

    // State variables
    int selectedSectorIdx = 0;
    int selectedCategoryIdx = 0;
    int selectedItemIdx = 0;
    int focusPanel = 0; // 0=Sector, 1=Category(Tab), 2=Items

    std::vector<string> categories = { "All", "Stops", "Schools", "Hospitals", "Pharmacies", "Malls" };
    std::vector<string> sectorList;
    for (int i = 0; i < SECTOR_COUNT; i++) sectorList.push_back(SECTOR_GRID[i].name);

    auto renderer = Renderer([&] {
        string currentSector = sectorList[selectedSectorIdx];
        string currentCategory = categories[selectedCategoryIdx];

        // ===== LEFT PANEL (Sectors) =====
        Elements sectorItems;
        sectorItems.push_back(text("SECTORS") | bold | color(Color::Cyan));
        sectorItems.push_back(separator());
        int sectorStart = std::max(0, selectedSectorIdx - 8);
        int sectorEnd = std::min((int)sectorList.size(), sectorStart + 18);

        for (int i = sectorStart; i < sectorEnd; i++) {
            string prefix = (i == selectedSectorIdx) ? "▶ " : "  ";
            auto item = text(prefix + sectorList[i]);
            if (i == selectedSectorIdx) {
                item = item | bold;
                if (focusPanel == 0) item = item | bgcolor(Color::Blue) | color(Color::White);
                else item = item | color(Color::Green);
            }
            sectorItems.push_back(item);
        }
        auto sectorPanel = vbox(sectorItems) | border | size(WIDTH, EQUAL, 14);
        if (focusPanel == 0) sectorPanel = sectorPanel | color(Color::Cyan);

        // ===== TOP TABS (Categories) =====
        Elements categoryTabs;
        for (int i = 0; i < (int)categories.size(); i++) {
            auto tab = text(" " + categories[i] + " ");
            if (i == selectedCategoryIdx) tab = tab | bold | bgcolor(Color::Green) | color(Color::Black);
            else tab = tab | color(Color::GrayLight);
            categoryTabs.push_back(tab);
        }

        // ===== MIDDLE PANEL (Items + Add Button) =====
        Elements itemList;
        std::vector<CityNode*> filteredNodes;

        // 1. Fetch Nodes
        if (islamabad && islamabad->getCityGraph()) {
            CityGraph* graph = islamabad->getCityGraph();
            for (int i = 0; i < graph->getNodeCount(); i++) {
                CityNode* node = graph->getNode(i);
                if (node && node->sector == currentSector && node->type != "CORNER") {
                    bool include = (currentCategory == "All") ||
                        (currentCategory == "Stops" && node->type == "STOP") ||
                        (currentCategory == "Schools" && node->type == "SCHOOL") ||
                        (currentCategory == "Hospitals" && node->type == "HOSPITAL") ||
                        (currentCategory == "Pharmacies" && node->type == "PHARMACY") ||
                        (currentCategory == "Malls" && node->type == "MALL");
                    if (include) filteredNodes.push_back(node);
                }
            }
        }

        // 2. Render List
        // +1 for the "Add Facility" button at the end
        int totalItems = filteredNodes.size() + 1;
        if (selectedItemIdx >= totalItems) selectedItemIdx = totalItems - 1;

        itemList.push_back(text("FACILITIES (" + std::to_string(filteredNodes.size()) + ")") | bold | color(Color::Yellow));
        itemList.push_back(separator());

        int itemStart = std::max(0, selectedItemIdx - 6);
        int itemEnd = std::min(totalItems, itemStart + 14);

        for (int i = itemStart; i < itemEnd; i++) {
            bool isSelected = (i == selectedItemIdx);
            Element item;

            if (i < filteredNodes.size()) {
                // Regular Node
                CityNode* node = filteredNodes[i];
                string icon = "●";
                if (node->type == "STOP") icon = "◎";
                else if (node->type == "SCHOOL") icon = "◆";
                else if (node->type == "HOSPITAL") icon = "✚";
                else if (node->type == "PHARMACY") icon = "⚕";
                else if (node->type == "MALL") icon = "◈";

                string name = node->name.substr(0, 22);
                string prefix = isSelected ? "▶ " : "  ";
                item = text(prefix + icon + " " + name);
            }
            else {
                // "Add Facility" Button (Always last)
                string prefix = isSelected ? "▶ " : "  ";
                item = text(prefix + "[+] Add Facility") | bold;
                if (isSelected) item = item | color(Color::Yellow);
            }

            if (isSelected) {
                item = item | bold;
                if (focusPanel == 2) item = item | bgcolor(Color::Blue) | color(Color::White);
                else item = item | color(Color::Green);
            }
            itemList.push_back(item);
        }

        auto itemPanel = vbox(itemList) | border | size(WIDTH, EQUAL, 32);
        if (focusPanel == 2) itemPanel = itemPanel | color(Color::Cyan);

        // ===== RIGHT PANEL (Details) =====
        Elements detailItems;
        detailItems.push_back(text("DETAILS") | bold | color(Color::Magenta));
        detailItems.push_back(separator());

        if (selectedItemIdx < filteredNodes.size()) {
            CityNode* selectedNode = filteredNodes[selectedItemIdx];

            // --- BASIC INFO ---
            detailItems.push_back(hbox({ text("Name: ") | bold, text(selectedNode->name) | color(Color::White) }));
            detailItems.push_back(hbox({ text("Type: ") | bold, text(selectedNode->type) | color(Color::Cyan) }));
            detailItems.push_back(hbox({ text("Sector: ") | bold, text(selectedNode->sector) | color(Color::Green) }));
            detailItems.push_back(hbox({ text("ID:   ") | bold, text(selectedNode->databaseID) | color(Color::Yellow) }));
            detailItems.push_back(separator());

            // Position info
            std::stringstream posStream;
            posStream << std::fixed << std::setprecision(2) << selectedNode->lat << ", " << selectedNode->lon;
            detailItems.push_back(hbox({ text("Position: ") | bold, text(posStream.str()) | dim }));
            detailItems.push_back(hbox({ text("Connections: ") | bold, text(std::to_string(selectedNode->roads.size())) | color(Color::Orange1) }));

            // --- TYPE SPECIFIC DETAILS (Restored) ---
            detailItems.push_back(separator());

            if (selectedNode->type == "SCHOOL") {
                if (islamabad && islamabad->getSchoolManager()) {
                    School* school = nullptr;
                    for (int s = 0; s < islamabad->getSchoolManager()->schools.getSize(); s++) {
                        if (islamabad->getSchoolManager()->schools[s]->name == selectedNode->name) {
                            school = islamabad->getSchoolManager()->schools[s];
                            break;
                        }
                    }
                    if (school) {
                        detailItems.push_back(text("SCHOOL INFO") | bold | color(Color::Blue));
                        detailItems.push_back(hbox({ text("Rating: ") | bold, text(std::to_string(school->rating).substr(0,3) + "/5.0") | color(Color::Yellow) }));

                        string subjectsStr = "";
                        for (int si = 0; si < school->subjects.getSize(); si++) {
                            if (si > 0) subjectsStr += ", ";
                            subjectsStr += school->subjects[si];
                            if (subjectsStr.length() > 25) { subjectsStr = subjectsStr.substr(0, 22) + "..."; break; }
                        }
                        if (subjectsStr.empty()) subjectsStr = "N/A";
                        detailItems.push_back(hbox({ text("Subjects: ") | bold, text(subjectsStr) | color(Color::Cyan) }));

                        int deptCount = 0, studentCount = 0;
                        for (int d = 0; d < school->departments.getSize(); d++) {
                            deptCount++;
                            for (int c = 0; c < school->departments[d]->classes.getSize(); c++) {
                                studentCount += school->departments[d]->classes[c]->students.getSize();
                            }
                        }
                        detailItems.push_back(hbox({ text("Departments: ") | bold, text(std::to_string(deptCount)) | color(Color::Green) }));
                        detailItems.push_back(hbox({ text("Students: ") | bold, text(std::to_string(studentCount)) | color(Color::Green) }));
                    }
                }
            }
            else if (selectedNode->type == "HOSPITAL") {
                if (islamabad && islamabad->getMedicalManager()) {
                    Hospital* hospital = islamabad->getMedicalManager()->findHospitalByID(selectedNode->databaseID);
                    if (hospital) {
                        detailItems.push_back(text("HOSPITAL INFO") | bold | color(Color::Red));
                        detailItems.push_back(hbox({ text("Total Beds: ") | bold, text(std::to_string(hospital->totalBeds)) | color(Color::Yellow) }));
                        detailItems.push_back(hbox({ text("Available: ") | bold, text(std::to_string(hospital->getAvailableBeds())) | color(Color::Green) }));
                        detailItems.push_back(hbox({ text("Specializations: ") | bold }));

                        string specStr = "";
                        for (int sp = 0; sp < hospital->specializations.getSize(); sp++) {
                            if (sp > 0) specStr += ", ";
                            specStr += hospital->specializations[sp];
                            if (specStr.length() > 25) { specStr = specStr.substr(0, 22) + "..."; break; }
                        }
                        if (specStr.empty()) specStr = "General";
                        detailItems.push_back(text("  " + specStr) | color(Color::Cyan));
                        detailItems.push_back(hbox({ text("ER Patients: ") | bold, text(std::to_string(hospital->getERQueueSize())) | color(Color::Orange1) }));
                        detailItems.push_back(text("Press 'A' to Add Specialization") | blink | color(Color::Yellow));
                    }
                }
            }
            else if (selectedNode->type == "PHARMACY") {
                if (islamabad && islamabad->getMedicalManager()) {
                    Pharmacy* pharmacy = nullptr;
                    for (int p = 0; p < islamabad->getMedicalManager()->pharmacies.getSize(); p++) {
                        if (islamabad->getMedicalManager()->pharmacies[p]->id == selectedNode->databaseID) {
                            pharmacy = islamabad->getMedicalManager()->pharmacies[p];
                            break;
                        }
                    }
                    if (pharmacy) {
                        detailItems.push_back(text("PHARMACY INFO") | bold | color(Color::Magenta));
                        detailItems.push_back(hbox({ text("Medicines: ") | bold, text(std::to_string(pharmacy->getMedicineCount())) | color(Color::Cyan) }));
                        if (pharmacy->getMedicineCount() > 0) {
                            const Medicine* firstMed = pharmacy->getMedicine(0);
                            if (firstMed) {
                                detailItems.push_back(hbox({ text("Sample: ") | bold, text(firstMed->name.substr(0, 15)) | color(Color::Yellow) }));
                                detailItems.push_back(hbox({ text("Formula: ") | bold, text(firstMed->formula) | color(Color::Yellow) }));
                                detailItems.push_back(hbox({ text("Price: ") | bold, text("Rs. " + std::to_string((int)firstMed->price)) | color(Color::Green) }));
                            }
                        }
                        detailItems.push_back(text("Press 'A' to Add Medicine") | blink | color(Color::Yellow));
                    }
                }
            }
            else if (selectedNode->type == "STOP") {
                detailItems.push_back(text("BUS STOP INFO") | bold | color(Color::GreenLight));
                if (!selectedNode->operatingHours.empty()) {
                    detailItems.push_back(hbox({ text("Hours: ") | bold, text(selectedNode->operatingHours) | color(Color::Cyan) }));
                }
                if (islamabad && islamabad->getTransportManager()) {
                    int waiting = islamabad->getTransportManager()->getWaitingCount(selectedNode->id);
                    detailItems.push_back(hbox({ text("Waiting: ") | bold, text(std::to_string(waiting) + " passengers") | color(Color::Yellow) }));
                }
            }

            // --- SECTOR POPULATION (Restored) ---
            detailItems.push_back(separator());
            detailItems.push_back(text("SECTOR POPULATION") | bold | color(Color::Orange1));

            if (islamabad && islamabad->getPopulationManager()) {
                Vector<Citizen*> residents = islamabad->getPopulationManager()->getCitizensInSector(currentSector);
                detailItems.push_back(hbox({ text("Residents: ") | bold, text(std::to_string(residents.getSize())) | color(Color::Green) }));

                int showCount = std::min(5, residents.getSize());
                if (showCount > 0) {
                    detailItems.push_back(text("Sample Residents:") | dim);
                    for (int r = 0; r < showCount; r++) {
                        string resInfo = "  " + residents[r]->name + " (" + residents[r]->currentStatus + ")";
                        if (resInfo.length() > 32) resInfo = resInfo.substr(0, 29) + "...";
                        detailItems.push_back(text(resInfo) | dim);
                    }
                }
            }

        }
        else {
            // Selected "Add Facility"
            detailItems.push_back(text("Create New Facility") | bold | color(Color::Green));
            detailItems.push_back(text(""));
            detailItems.push_back(text("Press ENTER to add a"));
            detailItems.push_back(text("new facility to"));
            detailItems.push_back(text("Sector " + currentSector) | bold);
        }

        auto detailPanel = vbox(detailItems) | border | flex;

        // ===== HELP BAR =====
        auto helpBar = hbox({
            text("↑↓/←→: Navigate "),
            text("Tab: Category "),
            text("A: Add Offering ") | bold | color(Color::Yellow),
            text("Enter: Select "),
            text("[ SEARCH ENGINE (S) ]") | border | bold | color(Color::Yellow), // New Button
            text("Esc: Back")
            }) | center;

        return vbox({
            hbox({text(" DATABASE VIEW ") | bold | bgcolor(Color::Green) | color(Color::Black), filler()}),
            hbox(categoryTabs) | center,
            separator(),
            hbox({sectorPanel, itemPanel, detailPanel}) | flex,
            separator(),
            helpBar
            });
        });

    auto comp = CatchEvent(renderer, [&](Event e) {
        // Navigation Logic
        if (e == Event::ArrowUp) {
            if (focusPanel == 0 && selectedSectorIdx > 0) selectedSectorIdx--;
            else if (focusPanel == 2 && selectedItemIdx > 0) selectedItemIdx--;
            return true;
        }
        if (e == Event::ArrowDown) {
            if (focusPanel == 0 && selectedSectorIdx < (int)sectorList.size() - 1) selectedSectorIdx++;
            else if (focusPanel == 2) {
                selectedItemIdx++;
            }
            return true;
        }
        if (e == Event::ArrowLeft) { focusPanel = std::max(0, focusPanel - 2); return true; }
        if (e == Event::ArrowRight) { focusPanel = std::min(2, focusPanel + 2); return true; }
        if (e == Event::Tab) { selectedCategoryIdx = (selectedCategoryIdx + 1) % categories.size(); selectedItemIdx = 0; return true; }
        if (e == Event::Escape) { currentState = SimulatorState::MAIN_MENU; screen.Exit(); return true; }

        // --- NEW FUNCTIONALITY TRIGGERS ---

        // 1. ADD FACILITY / SELECT ITEM
        if (e == Event::Return) {
            if (focusPanel == 0) focusPanel = 2; // Move focus to items
            else if (focusPanel == 2) {
                // Re-calculate filtered list size to check if we are on the "Add" button
                int count = 0;
                CityGraph* g = islamabad->getCityGraph();
                string curSec = sectorList[selectedSectorIdx];
                string curCat = categories[selectedCategoryIdx];
                for (int i = 0; i < g->getNodeCount(); i++) {
                    CityNode* n = g->getNode(i);
                    if (n && n->sector == curSec && n->type != "CORNER") {
                        bool inc = (curCat == "All" || (curCat + "S" == n->type + "S") ||
                            (curCat == "Stops" && n->type == "STOP") ||
                            (curCat == "Pharmacies" && n->type == "PHARMACY") ||
                            (curCat == "Schools" && n->type == "SCHOOL") ||
                            (curCat == "Hospitals" && n->type == "HOSPITAL") ||
                            (curCat == "Malls" && n->type == "MALL"));
                        if (inc) count++;
                    }
                }

                // If index is past the items, it's the "Add" button
                if (selectedItemIdx >= count) {
                    runAddFacilityForm(curSec);
                }
            }
            return true;
        }

        // 2. ADD OFFERING (Press 'A')
        if (focusPanel == 2 && (e == Event::Character('a') || e == Event::Character('A'))) {
            CityNode* selectedNode = nullptr;
            int currentIdx = 0;
            CityGraph* g = islamabad->getCityGraph();
            string curSec = sectorList[selectedSectorIdx];
            string curCat = categories[selectedCategoryIdx];

            for (int i = 0; i < g->getNodeCount(); i++) {
                CityNode* n = g->getNode(i);
                if (n && n->sector == curSec && n->type != "CORNER") {
                    bool inc = (curCat == "All" ||
                        (curCat == "Stops" && n->type == "STOP") ||
                        (curCat == "Schools" && n->type == "SCHOOL") ||
                        (curCat == "Hospitals" && n->type == "HOSPITAL") ||
                        (curCat == "Pharmacies" && n->type == "PHARMACY") ||
                        (curCat == "Malls" && n->type == "MALL"));
                    if (inc) {
                        if (currentIdx == selectedItemIdx) {
                            selectedNode = n;
                            break;
                        }
                        currentIdx++;
                    }
                }
            }

            if (selectedNode) {
                runAddOfferingForm(selectedNode);
            }
            return true;
        }

        // 3. SEARCH ENGINE (Press 'S')
        if (e == Event::Character('s') || e == Event::Character('S')) {
            currentState = SimulatorState::SEARCH_VIEW;
            screen.Exit();
            return true;
        }

        return false;
        });

    screen.Loop(comp);
}

// ============================================================================
// UNIVERSAL SEARCH VIEW
// ============================================================================

inline void CitySimulator::runSearchView() {
    auto screen = ScreenInteractive::Fullscreen();

    // State
    string query = "";
    int selected = 0;
    std::vector<string> menu_entries;
    string message = ""; // To show the ID on selection

    // Helper: string lowercase
    auto toLower = [](const string& s) -> string {
        string lower = s;
        for (char& c : lower) {
            if (c >= 'A' && c <= 'Z') c += ('a' - 'A');
        }
        return lower;
        };

    // Helper to perform the search
    auto performSearch = [&]() {
        menu_entries.clear();
        selected = 0;
        message = "";

        if (query.length() < 2) return;

        string q = toLower(query);

        // 1. Search Facilities (Graph Nodes)
        if (islamabad && islamabad->getCityGraph()) {
            CityGraph* g = islamabad->getCityGraph();
            for (int i = 0; i < g->getNodeCount(); i++) {
                CityNode* n = g->getNode(i);
                if (!n || n->type == "CORNER") continue;

                string lowerName = toLower(n->name);

                if (lowerName.find(q) != string::npos) {
                    menu_entries.push_back("[ID: " + n->databaseID + "] " + n->name + " (" + n->type + ") in " + n->sector);
                }
            }
        }

        // 2. Search Commercial (Malls, Shops, Products)
        if (islamabad && islamabad->getCommercialManager()) {
            CommercialManager* cm = islamabad->getCommercialManager();
            for (int i = 0; i < cm->malls.getSize(); i++) {
                Mall* m = cm->malls[i];

                // Search Mall Name
                string mLower = toLower(m->name);
                if (mLower.find(q) != string::npos) {
                    menu_entries.push_back("[ID: " + m->id + "] " + m->name + " (" + m->getSector() + ")");
                }

                // Search Shops
                for (int j = 0; j < m->shops.getSize(); j++) {
                    Shop* s = m->shops[j];
                    string sLower = toLower(s->name);

                    if (sLower.find(q) != string::npos) {
                        menu_entries.push_back("[ID: " + s->id + "] " + s->name + " @ " + m->name);
                    }

                    // Search Products within Shop
                    for (int k = 0; k < s->inventory.getSize(); k++) {
                        const Product* p = s->getProduct(k);
                        if (!p) continue;

                        string pLower = toLower(p->name);

                        if (pLower.find(q) != string::npos) {
                            menu_entries.push_back("[ID: " + s->id + "] Item: " + p->name + " (Rs." + std::to_string(p->price) + ") @ " + s->name);
                        }
                    }
                }
            }
        }

        // 3. Search Medical (Medicines)
        if (islamabad && islamabad->getMedicalManager()) {
            MedicalManager* mm = islamabad->getMedicalManager();

            for (int i = 0; i < mm->pharmacies.getSize(); i++) {
                Pharmacy* p = mm->pharmacies[i];
                for (int j = 0; j < p->inventory.getSize(); j++) {
                    const Medicine* m = p->getMedicine(j);
                    if (!m) continue;

                    string mLower = toLower(m->name);
                    string fLower = toLower(m->formula);

                    if (mLower.find(q) != string::npos || fLower.find(q) != string::npos) {
                        menu_entries.push_back("[ID: " + p->id + "] Med: " + m->name + " (" + m->formula + ") @ " + p->name);
                    }
                }
            }
        }

        // Limit results for performance UI
        if (menu_entries.size() > 100) {
            menu_entries.resize(100);
            menu_entries.push_back("... (matches truncated) ...");
        }
        };

    // --- Components ---

    InputOption input_opt;
    input_opt.on_change = performSearch;
    input_opt.placeholder = "Search for items, medicines, shops, or places...";
    auto input_component = Input(&query, input_opt);

    MenuOption menu_opt;
    menu_opt.on_enter = [&] {
        if (selected >= 0 && selected < (int)menu_entries.size()) {
            string selection = menu_entries[selected];

            // Extract ID logic
            size_t start = selection.find("[ID: ");
            if (start != string::npos) {
                start += 5; // Skip "[ID: "
                size_t end = selection.find("]", start);
                if (end != string::npos) {
                    string extractedID = selection.substr(start, end - start);
                    message = "Selected Item ID: " + extractedID + " (Press Esc to return)";

                    // PLACEHOLDER FOR MANAGEMENT VIEW INTEGRATION
                    // When pressing Enter, we will eventually pass 'extractedID' 
                    // to a Management View that looks up the object via HashTable.
                    // For now, we just display the ID.
                }
                else {
                    message = "ID format not recognized.";
                }
            }
            else {
                message = "No ID associated with this entry.";
            }
        }
        };
    auto menu_component = Menu(&menu_entries, &selected, menu_opt);

    auto container = Container::Vertical({
        input_component,
        menu_component | vscroll_indicator | frame | flex
        });

    // --- Renderer ---
    auto renderer = Renderer(container, [&] {
        return vbox({
            text(" UNIVERSAL SEARCH ENGINE ") | bold | center | bgcolor(Color::Blue) | color(Color::White),
            separator(),
            hbox({ text(" FIND: "), input_component->Render() | flex }),
            separator(),
            (menu_entries.empty())
                ? (query.length() < 2
                    ? text("Type at least 2 characters to begin...") | dim | center
                    : text("No results found.") | color(Color::Red) | center)
                : menu_component->Render() | flex,
            separator(),
            // Message Area for ID display
            (message.empty() ? text("Select an item and press Enter") | dim | center
                             : text(message) | bold | color(Color::Green) | center),
            text("Esc: Back to Database") | dim | center
            }) | border | size(WIDTH, EQUAL, 80) | size(HEIGHT, EQUAL, 40) | center;
        });

    // --- Event Loop ---
    auto component = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Escape) {
            currentState = SimulatorState::DATABASE_VIEW; // Return to DB view
            screen.Exit();
            return true;
        }
        return false;
        });

    screen.Loop(component);
}

// ============================================================================
// HELPER: GENERIC INPUT FORM
// ============================================================================
inline void CitySimulator::runInputForm(const string& title, const std::vector<string>& labels, std::function<void(std::vector<string>)> onConfirm) {
    auto screen = ScreenInteractive::Fullscreen();
    std::vector<string> inputs(labels.size());

    Component container = Container::Vertical({});
    for (size_t i = 0; i < labels.size(); ++i) {
        container->Add(Input(&inputs[i], "Type " + labels[i] + "..."));
    }

    Component btn_confirm = Button("Confirm", [&] {
        onConfirm(inputs);
        screen.Exit();
        });

    Component btn_cancel = Button("Cancel", screen.ExitLoopClosure());

    container->Add(Container::Horizontal({ btn_confirm, btn_cancel }));

    auto renderer = Renderer(container, [&] {
        Elements fields;
        for (size_t i = 0; i < labels.size(); ++i) {
            fields.push_back(hbox({
                text(labels[i] + ": ") | bold | size(WIDTH, EQUAL, 20),
                container->ChildAt(i)->Render() | flex
                }) | border);
        }

        return vbox({
            text(title) | bold | center | bgcolor(Color::Blue) | color(Color::White),
            separator(),
            vbox(fields) | flex,
            separator(),
            hbox({ btn_confirm->Render(), text("  "), btn_cancel->Render() }) | center
            }) | border | size(WIDTH, EQUAL, 60) | center;
        });

    screen.Loop(renderer);
}

// ============================================================================
// HELPER: POPULATION SELECTOR PORTAL
// ============================================================================
inline Citizen* CitySimulator::runPopulationSelector(const string& title) {
    if (!islamabad || !islamabad->getPopulationManager()) return nullptr;

    auto screen = ScreenInteractive::Fullscreen();
    Citizen* selectedCitizen = nullptr;
    string query = "";
    int selected = 0;
    std::vector<string> entries;
    std::vector<Citizen*> displayedCitizens;

    auto refreshList = [&]() {
        entries.clear();
        displayedCitizens.clear();
        PopulationManager* pm = islamabad->getPopulationManager();

        // Linear scan of master list (Population can be large, limiting display to 50 matches)
        int count = 0;
        for (int i = 0; i < pm->masterList.getSize(); ++i) {
            Citizen* c = pm->masterList[i];
            if (!c) continue;

            // Basic search filter
            bool match = query.empty();
            if (!match) {
                string q = query;
                string n = c->name;
                string id = c->cnic;
                // Simple case-insensitive check could be added here, strict for now
                if (n.find(q) != string::npos || id.find(q) != string::npos) match = true;
            }

            if (match) {
                displayedCitizens.push_back(c);
                entries.push_back(c->name + " (" + c->cnic + ") - Age: " + std::to_string(c->age) + " - " + c->currentStatus);
                count++;
                if (count >= 50) break;
            }
        }
        if (entries.empty()) entries.push_back("No citizens found matching query.");
        };

    refreshList(); // Initial population

    InputOption input_opt;
    input_opt.on_change = refreshList;
    Component input = Input(&query, "Search Name or CNIC...", input_opt);

    MenuOption menu_opt;
    menu_opt.on_enter = [&] {
        if (selected >= 0 && selected < (int)displayedCitizens.size()) {
            selectedCitizen = displayedCitizens[selected];
            screen.Exit();
        }
        };
    Component menu = Menu(&entries, &selected, menu_opt);

    auto layout = Container::Vertical({
        input,
        menu | vscroll_indicator | frame | flex
        });

    auto renderer = Renderer(layout, [&] {
        return vbox({
            text(" POPULATION REGISTRY - " + title) | bold | center | bgcolor(Color::Green) | color(Color::Black),
            separator(),
            hbox({ text(" Filter: "), input->Render() }),
            separator(),
            menu->Render() | flex,
            separator(),
            text("Enter: Select | Esc: Cancel") | dim | center
            }) | border | size(WIDTH, EQUAL, 80) | size(HEIGHT, EQUAL, 40) | center;
        });

    auto component = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Escape) { screen.Exit(); return true; }
        return false;
        });

    screen.Loop(component);
    return selectedCitizen;
}

// ============================================================================
// MODULAR EDIT VIEWS (Detailed Control for Each Type)
// ============================================================================

// --- 1. Edit SCHOOL ---
inline void CitySimulator::runEditSchoolView(School* school) {
    if (!school) return;
    auto screen = ScreenInteractive::Fullscreen();
    string message = "";

    // Toggle states for panels
    int tab_index = 0;
    std::vector<string> tabs = { "Info", "Departments", "Faculty", "Students" };

    // --- Component Logic ---

    // Tab 1: Info Actions
    Component btn_rename = Button("Rename School", [&] {
        runInputForm("Rename School", { "New Name" }, [&](std::vector<string> res) {
            if (!res[0].empty()) { school->name = res[0]; message = "Renamed to " + res[0]; }
            });
        });

    // Tab 2: Dept Actions
    Component btn_add_dept = Button("Add Department", [&] {
        runInputForm("New Department", { "Department Name" }, [&](std::vector<string> res) {
            if (!res[0].empty()) {
                if (cityMgmt->addDepartmentToSchool(school->id, res[0])) message = "Added Dept: " + res[0];
                else message = "Error: Dept likely exists.";
            }
            });
        });

    Component btn_rem_dept = Button("Remove Department", [&] {
        runInputForm("Remove Department", { "Department Name" }, [&](std::vector<string> res) {
            if (cityMgmt->removeDepartmentFromSchool(school->id, res[0])) message = "Removed Dept: " + res[0];
            else message = "Error: Dept not found.";
            });
        });

    // Tab 3: Faculty Actions
    Component btn_hire_fac = Button("Hire Faculty", [&] {
        Citizen* c = runPopulationSelector("Select Citizen to Hire");
        if (c) {
            // Check if already employed logic in manager, but we do basic flow here
            runInputForm("Employment Contract", { "Department", "Qualification", "Salary" }, [&](std::vector<string> res) {
                string dept = res[0];
                string qual = res[1];
                double sal = 0;
                try { sal = std::stod(res[2]); }
                catch (...) {}

                string resID = cityMgmt->hireCitizenAsFaculty(c->cnic, school->id, dept, qual, sal);
                if (!resID.empty()) message = "Hired " + c->name + " as " + resID;
                else message = "Hiring Failed (Dept invalid or already employed?)";
                });
        }
        });

    // Tab 4: Student Actions
    Component btn_enroll = Button("Enroll Student", [&] {
        Citizen* c = runPopulationSelector("Select Student to Enroll");
        if (c) {
            runInputForm("Enrollment Form", { "Department", "Class (1-10)" }, [&](std::vector<string> res) {
                string dept = res[0];
                int cls = 1;
                try { cls = std::stoi(res[1]); }
                catch (...) {}

                if (cityMgmt->enrollStudent(c->cnic, school->id, dept, cls)) message = "Enrolled " + c->name;
                else message = "Enrollment Failed (Dept/Class invalid or duplicate)";
                });
        }
        });

    Component btn_back = Button("Back to Manager", screen.ExitLoopClosure());

    // Layout Containers
    auto c_info = Container::Vertical({ btn_rename });
    auto c_dept = Container::Vertical({ btn_add_dept, btn_rem_dept });
    auto c_fac = Container::Vertical({ btn_hire_fac });
    auto c_stu = Container::Vertical({ btn_enroll });
    auto c_tabs = Toggle(&tabs, &tab_index);

    auto main_container = Container::Vertical({
        c_tabs,
        Container::Tab({c_info, c_dept, c_fac, c_stu}, &tab_index),
        btn_back
        });

    auto renderer = Renderer(main_container, [&] {
        // --- VIEW GENERATION ---

        // 1. Stats Panel
        auto stats = vbox({
            hbox({text("ID: ") | bold, text(school->id)}),
            hbox({text("Name: ") | bold, text(school->name)}),
            hbox({text("Sector: ") | bold, text(school->location.sector)}),
            hbox({text("Students: ") | bold, text(std::to_string(school->getTotalEnrolledStudents()))}),
            hbox({text("Faculty: ") | bold, text(std::to_string(school->getTotalFaculty()))}),
            }) | border | size(WIDTH, EQUAL, 40);

        // 2. Dynamic Content Panel
        Element content;
        if (tab_index == 0) { // Info
            content = vbox({ text("School Management Dashboard") | center, separator(), btn_rename->Render() | center });
        }
        else if (tab_index == 1) { // Depts
            Elements list;
            for (int i = 0; i < school->departments.getSize(); ++i) {
                Department* d = school->departments[i];
                list.push_back(text("- " + d->name + " (Classes: " + std::to_string(d->getClassCount()) + ")"));
            }
            content = vbox({
                vbox(list) | flex | border,
                hbox({ btn_add_dept->Render(), text(" "), btn_rem_dept->Render() }) | center
                });
        }
        else if (tab_index == 2) { // Faculty
            Elements list;
            int limit = 0;
            for (int i = 0; i < school->departments.getSize(); ++i) {
                Department* d = school->departments[i];
                for (int j = 0; j < d->faculty.getSize(); ++j) {
                    if (limit++ > 15) { list.push_back(text("...")); break; }
                    Faculty* f = d->faculty[j];
                    list.push_back(text(f->getName() + " [" + d->name + "] - " + f->qualification));
                }
            }
            content = vbox({
                vbox(list) | flex | border,
                btn_hire_fac->Render() | center
                });
        }
        else { // Students
            Elements list;
            int limit = 0;
            // Just listing first few for brevity
            if (school->departments.getSize() > 0) {
                Department* d = school->departments[0]; // Just show first dept sample
                if (d->getClassCount() > 0) {
                    Class* c = d->classes[0];
                    for (int k = 0; k < c->students.getSize(); ++k) {
                        if (limit++ > 15) break;
                        list.push_back(text(c->students[k]->getName() + " (Class " + std::to_string(c->classNumber) + ")"));
                    }
                }
            }
            content = vbox({
                text("Sample Student List (First Dept/Class)") | dim,
                vbox(list) | flex | border,
                btn_enroll->Render() | center
                });
        }

        return vbox({
            text(" SCHOOL ADMINISTRATION PORTAL ") | bold | center | bgcolor(Color::Blue) | color(Color::White),
            hbox({ stats, separator(), content | flex }),
            separator(),
            c_tabs->Render() | center,
            separator(),
            btn_back->Render() | center,
            text(message) | bold | color(Color::Red) | center
            }) | border | center;
        });

    screen.Loop(renderer);
}

// --- 2. Edit HOSPITAL ---
inline void CitySimulator::runEditHospitalView(Hospital* hospital) {
    if (!hospital) return;
    auto screen = ScreenInteractive::Fullscreen();
    string message = "";
    int tab_index = 0;
    std::vector<string> tabs = { "Info", "Doctors", "Patients" };

    // Components
    Component btn_add_beds = Button("Add 10 Beds", [&] { hospital->totalBeds += 10; message = "Beds increased."; });
    Component btn_add_spec = Button("Add Specialization", [&] {
        runInputForm("New Specialization", { "Name (e.g., Cardiology)" }, [&](std::vector<string> res) {
            if (!res[0].empty()) {
                cityMgmt->addSpecializationToHospital(hospital->id, res[0]);
                message = "Added " + res[0];
            }
            });
        });

    Component btn_hire_doc = Button("Hire Doctor", [&] {
        Citizen* c = runPopulationSelector("Select Doctor to Hire");
        if (c) {
            runInputForm("Doctor Contract", { "Specialization" }, [&](std::vector<string> res) {
                if (!res[0].empty()) {
                    Doctor d(c, res[0]); // Creates ID internally
                    hospital->addDoctor(d);
                    message = "Hired Dr. " + c->name;
                }
                });
        }
        });

    Component btn_admit = Button("Admit Patient", [&] {
        Citizen* c = runPopulationSelector("Select Patient");
        if (c) {
            runInputForm("Admission", { "Condition", "Severity (1-10)" }, [&](std::vector<string> res) {
                int sev = 5; try { sev = std::stoi(res[1]); }
                catch (...) {}
               /* if (cityMgmt->admitPatient(c->cnic, hospital->id, sev, res[0])) message = "Admitted " + c->name;*/
                if (false) {}
                else message = "Admission Failed (No beds?)";
                });
        }
        });

    Component btn_back = Button("Back", screen.ExitLoopClosure());

    auto c_info = Container::Vertical({ btn_add_beds, btn_add_spec });
    auto c_docs = Container::Vertical({ btn_hire_doc });
    auto c_pats = Container::Vertical({ btn_admit });
    auto c_tabs = Toggle(&tabs, &tab_index);

    auto layout = Container::Vertical({ c_tabs, Container::Tab({c_info, c_docs, c_pats}, &tab_index), btn_back });

    auto renderer = Renderer(layout, [&] {
        Element content;
        if (tab_index == 0) {
            Elements specs;
            for (int i = 0; i < hospital->specializations.getSize(); ++i) specs.push_back(text("- " + hospital->specializations[i]));
            content = vbox({ text("Specializations:") | bold, vbox(specs) | border, btn_add_spec->Render(), btn_add_beds->Render() });
        }
        else if (tab_index == 1) {
            Elements docs;
            for (int i = 0; i < hospital->doctors.getSize(); ++i)
                docs.push_back(text("Dr. " + hospital->doctors[i].getCitizen()->name + " (" + hospital->doctors[i].specialization + ")"));
            content = vbox({ vbox(docs) | flex | border, btn_hire_doc->Render() | center });
        }
        else {
            Elements pats;
            for (int i = 0; i < hospital->admittedPatients.getSize(); ++i)
                pats.push_back(text(hospital->admittedPatients[i].getName() + " - " + hospital->admittedPatients[i].getDisease()));
            content = vbox({ vbox(pats) | flex | border, btn_admit->Render() | center });
        }

        return vbox({
            text(" HOSPITAL ADMIN: " + hospital->name) | bold | center | bgcolor(Color::Red) | color(Color::White),
            hbox({
                vbox({
                    hbox({text("Beds: "), text(std::to_string(hospital->getOccupiedBeds()) + "/" + std::to_string(hospital->totalBeds))})
                }) | border | size(WIDTH, EQUAL, 30),
                content | flex
            }) | flex,
            separator(),
            c_tabs->Render() | center,
            btn_back->Render() | center,
            text(message) | color(Color::Yellow) | center
            }) | border;
        });

    screen.Loop(renderer);
}

// --- 3. Edit SHOP/COMMERCIAL ---
inline void CitySimulator::runEditShopView(Shop* shop, Mall* mall) {
    if (!shop) return;
    auto screen = ScreenInteractive::Fullscreen();
    string message = "";

    Component btn_add_prod = Button("Add Product", [&] {
        runInputForm("New Product", { "Product Name", "Price" }, [&](std::vector<string> res) {
            if (!res[0].empty() && !res[1].empty()) {
                int p = 0; try { p = std::stoi(res[1]); }
                catch (...) {}
                Product prod(res[0], shop->category, p);
                shop->addProduct(prod);
                message = "Added " + res[0];
            }
            });
        });

    Component btn_rem_prod = Button("Remove Product", [&] {
        runInputForm("Remove Product", { "Product Name" }, [&](std::vector<string> res) {
            if (shop->removeProduct(res[0])) message = "Removed " + res[0];
            else message = "Product not found.";
            });
        });

    Component btn_back = Button("Back", screen.ExitLoopClosure());

    auto container = Container::Vertical({ btn_add_prod, btn_rem_prod, btn_back });

    auto renderer = Renderer(container, [&] {
        Elements inv;
        for (int i = 0; i < shop->inventory.getSize(); ++i) {
            const Product* p = shop->getProduct(i);
            inv.push_back(hbox({ text(p->name), filler(), text("Rs " + std::to_string(p->price)) | color(Color::Green) }));
        }

        return vbox({
            text(" SHOP INVENTORY: " + shop->name) | bold | center | bgcolor(Color::Yellow) | color(Color::Black),
            separator(),
            hbox({
                vbox(inv) | flex | border,
                vbox({
                    text("Actions") | bold | center,
                    separator(),
                    btn_add_prod->Render(),
                    text(" "),
                    btn_rem_prod->Render(),
                    filler(),
                    btn_back->Render()
                }) | size(WIDTH, EQUAL, 25)
            }) | flex,
            text(message) | color(Color::Red) | center
            }) | border;
        });

    screen.Loop(renderer);
}
// --- 4. Main Edit Dispatcher ---
inline void CitySimulator::runEditObjectView(const string& objectID, const string& objectType) {
    // 1. Dispatch SCHOOL
    if (objectType == "SCHOOL") {
        if (islamabad && islamabad->getSchoolManager()) {
            School* s = islamabad->getSchoolManager()->findSchoolByID(objectID);
            if (s) { runEditSchoolView(s); return; }
        }
    }
    // 2. Dispatch HOSPITAL
    else if (objectType == "HOSPITAL") {
        if (islamabad && islamabad->getMedicalManager()) {
            Hospital* h = islamabad->getMedicalManager()->findHospitalByID(objectID);
            if (h) { runEditHospitalView(h); return; }
        }
    }
    // 3. Dispatch SHOP
    else if (objectType == "SHOP") {
        if (islamabad && islamabad->getCommercialManager()) {
            CommercialManager* cm = islamabad->getCommercialManager();
            // Need to find which mall this shop is in
            for (int i = 0; i < cm->malls.getSize(); i++) {
                Shop* s = cm->malls[i]->findShopByID(objectID);
                if (s) { runEditShopView(s, cm->malls[i]); return; }
            }
        }
    }

    // Fallback for generic types or un-implemented ones
    auto screen = ScreenInteractive::Fullscreen();
    auto renderer = Renderer([&] {
        return vbox({
            text("GENERIC EDITOR: " + objectID) | bold | center,
            separator(),
            text("Type: " + objectType) | center,
            text("Specific editor not implemented yet.") | dim | center,
            text("Press Esc to return") | dim | center
            }) | border | center;
        });
    auto component = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Escape) { screen.Exit(); return true; }
        return false;
        });
    screen.Loop(component);
}

// ============================================================================
// MANAGEMENT VIEW (The Big Boss Panel)
// ============================================================================

inline void CitySimulator::runManagementMenu() {
    auto screen = ScreenInteractive::Fullscreen();

    // State
    int selectedCategoryIdx = 0;
    int selectedItemIdx = 0;
    int focusPanel = 0; // 0=Tabs, 1=List, 2=Actions

    std::vector<string> categories = { "All", "Nodes", "Malls", "Shops", "Schools", "Hospitals", "Pharmacies" };

    // Struct to hold flattened list of manageable objects
    struct ManageableItem {
        string id;
        string name;
        string type;
        string extraInfo;
    };
    std::vector<ManageableItem> currentItems;

    // Helper to refresh the list based on category
    auto refreshList = [&]() {
        currentItems.clear();
        string cat = categories[selectedCategoryIdx];

        // 1. Nodes (Stops/Corners)
        if (cat == "All" || cat == "Nodes") {
            CityGraph* g = islamabad->getCityGraph();
            for (int i = 0; i < g->getNodeCount(); i++) {
                CityNode* n = g->getNode(i);
                if (n) currentItems.push_back({ n->databaseID, n->name, n->type, n->sector });
            }
        }

        // 2. Commercial
        if (islamabad->getCommercialManager()) {
            CommercialManager* cm = islamabad->getCommercialManager();
            if (cat == "All" || cat == "Malls") {
                for (int i = 0; i < cm->malls.getSize(); i++)
                    currentItems.push_back({ cm->malls[i]->id, cm->malls[i]->name, "MALL", cm->malls[i]->getSector() });
            }
            if (cat == "All" || cat == "Shops") {
                for (int i = 0; i < cm->malls.getSize(); i++) {
                    Mall* m = cm->malls[i];
                    for (int j = 0; j < m->shops.getSize(); j++)
                        currentItems.push_back({ m->shops[j]->id, m->shops[j]->name, "SHOP", m->name });
                }
            }
        }

        // 3. Schools
        if ((cat == "All" || cat == "Schools") && islamabad->getSchoolManager()) {
            SchoolManager* sm = islamabad->getSchoolManager();
            for (int i = 0; i < sm->schools.getSize(); i++)
                currentItems.push_back({ sm->schools[i]->id, sm->schools[i]->name, "SCHOOL", sm->schools[i]->getSector() });
        }

        // 4. Medical
        if (islamabad->getMedicalManager()) {
            MedicalManager* mm = islamabad->getMedicalManager();
            if (cat == "All" || cat == "Hospitals") {
                for (int i = 0; i < mm->hospitals.getSize(); i++)
                    currentItems.push_back({ mm->hospitals[i]->id, mm->hospitals[i]->name, "HOSPITAL", mm->hospitals[i]->sector });
            }
            if (cat == "All" || cat == "Pharmacies") {
                for (int i = 0; i < mm->pharmacies.getSize(); i++)
                    currentItems.push_back({ mm->pharmacies[i]->id, mm->pharmacies[i]->name, "PHARMACY", mm->pharmacies[i]->sector });
            }
        }
        };

    // Initial load
    refreshList();

    // Helper for placeholders
    auto showPlaceholder = [&](string title, string msg) {
        auto pScreen = ScreenInteractive::Fullscreen();
        auto pRenderer = Renderer([&] {
            return vbox({
                text(title) | bold | center | bgcolor(Color::Red) | color(Color::White),
                separator(),
                text(msg) | center,
                text(""),
                text("Press Enter to return") | dim | center
                }) | border | center;
            });
        auto pComp = CatchEvent(pRenderer, [&](Event e) {
            if (e == Event::Return || e == Event::Escape) {
                pScreen.Exit();
                return true;
            }
            return false;
            });
        pScreen.Loop(pComp);
        };

    auto renderer = Renderer([&] {
        // Ensure index bounds
        if (selectedItemIdx >= (int)currentItems.size()) selectedItemIdx = std::max(0, (int)currentItems.size() - 1);

        // ===== TOP TABS =====
        Elements tabs;
        for (int i = 0; i < (int)categories.size(); i++) {
            auto tab = text(" " + categories[i] + " ");
            if (i == selectedCategoryIdx) tab = tab | bold | bgcolor(Color::Cyan) | color(Color::Black);
            else tab = tab | color(Color::GrayLight);
            tabs.push_back(tab);
        }

        // ===== LEFT PANEL (Object List) =====
        Elements listElements;
        int startIdx = std::max(0, selectedItemIdx - 10);
        int endIdx = std::min((int)currentItems.size(), startIdx + 22);

        for (int i = startIdx; i < endIdx; i++) {
            const auto& item = currentItems[i];
            string label = "[" + item.id + "] - " + item.name.substr(0, 20) + " - [" + item.type + "]";
            auto row = text(label);
            if (i == selectedItemIdx) {
                row = row | bold;
                if (focusPanel == 1) row = row | bgcolor(Color::Blue) | color(Color::White);
                else row = row | color(Color::Green);
            }
            listElements.push_back(row);
        }
        auto leftPanel = vbox(listElements) | border | flex;
        if (focusPanel == 1) leftPanel = leftPanel | color(Color::Cyan);

        // ===== MIDDLE PANEL (Actions) =====
        auto btnStyle = [&](string label, bool selected) {
            return text(label) | center | (selected ? (bgcolor(Color::Red) | bold) : dim) | border;
            };
        // Visuals only - logic is in event loop
        // We highlight buttons based on key presses theoretically, but since this is keyboard-driven, 
        // we mainly show them as available options.
        auto midPanel = vbox({
            text("ACTIONS") | bold | center,
            separator(),
            btnStyle("[ EDIT ] (E)", false), // E key
            text(" "),
            btnStyle("[ ADD ] (+)", false),  // + key
            text(" "),
            btnStyle("[ DELETE ] (Del)", false) // Del key
            }) | border | size(WIDTH, EQUAL, 20);

        // ===== RIGHT PANEL (Details) =====
        Elements details;
        if (!currentItems.empty() && selectedItemIdx < (int)currentItems.size()) {
            const auto& sel = currentItems[selectedItemIdx];
            details.push_back(text("OBJECT DETAILS") | bold | center | color(Color::Yellow));
            details.push_back(separator());
            details.push_back(hbox({ text("ID: ") | bold, text(sel.id) | color(Color::Cyan) }));
            details.push_back(hbox({ text("Name: ") | bold, text(sel.name) | color(Color::White) }));
            details.push_back(hbox({ text("Type: ") | bold, text(sel.type) | color(Color::Magenta) }));
            details.push_back(hbox({ text("Loc/Info: ") | bold, text(sel.extraInfo) | color(Color::Green) }));
            details.push_back(separator());
            details.push_back(text("Press 'E' to Edit full details") | dim | center);
        }
        else {
            details.push_back(text("No item selected") | dim | center);
        }
        auto rightPanel = vbox(details) | border | flex;

        return vbox({
            text(" MANAGEMENT CONSOLE (ADMIN) ") | bold | center | bgcolor(Color::Red) | color(Color::White),
            hbox(tabs) | center,
            separator(),
            hbox({ leftPanel, midPanel, rightPanel }) | flex,
            separator(),
            text("Tab: Switch Panel | Arrows: Navigate | E: Edit | +: Add | Del: Delete | Esc: Back") | dim | center
            });
        });

    auto component = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Tab) { focusPanel = (focusPanel + 1) % 2; return true; } // Toggle Tabs/List
        if (e == Event::Escape) { currentState = SimulatorState::MAIN_MENU; screen.Exit(); return true; }

        // Global Hotkeys (Work regardless of focus panel for ease of use)

        // ADD
        if (e == Event::Character('+') || e == Event::Character('=')) { // '=' is unshifted '+'
            showPlaceholder("ADD OBJECT", "Functionality to ADD a new object is under construction.");
            refreshList(); // Refresh in case add logic is implemented later
            return true;
        }

        // DELETE
        if (e == Event::Delete || e == Event::Special({ 127 }) || e == Event::Character('x')) { // 127 is usually del/backspace on some terms, 'x' as backup
            if (!currentItems.empty() && selectedItemIdx < (int)currentItems.size()) {
                string msg = "Functionality to DELETE [" + currentItems[selectedItemIdx].name + "] is under construction.";
                showPlaceholder("DELETE OBJECT", msg);
                refreshList(); // Refresh in case delete logic is implemented later
            }
            return true;
        }

        // EDIT
        if (e == Event::Character('e') || e == Event::Character('E')) {
            if (!currentItems.empty() && selectedItemIdx < (int)currentItems.size()) {
                runEditObjectView(currentItems[selectedItemIdx].id, currentItems[selectedItemIdx].type);
                refreshList();
            }
            return true;
        }

        if (focusPanel == 0) { // Tabs
            if (e == Event::ArrowLeft) {
                selectedCategoryIdx = (selectedCategoryIdx - 1 + categories.size()) % categories.size();
                selectedItemIdx = 0; refreshList(); return true;
            }
            if (e == Event::ArrowRight) {
                selectedCategoryIdx = (selectedCategoryIdx + 1) % categories.size();
                selectedItemIdx = 0; refreshList(); return true;
            }
            if (e == Event::ArrowDown) { focusPanel = 1; return true; }
        }
        else if (focusPanel == 1) { // List
            if (e == Event::ArrowUp) {
                if (selectedItemIdx > 0) selectedItemIdx--;
                else focusPanel = 0;
                return true;
            }
            if (e == Event::ArrowDown) {
                if (selectedItemIdx < (int)currentItems.size() - 1) selectedItemIdx++;
                return true;
            }
            // Enter basically acts like Edit here
            if (e == Event::Return) {
                if (!currentItems.empty()) {
                    runEditObjectView(currentItems[selectedItemIdx].id, currentItems[selectedItemIdx].type);
                }
                return true;
            }
        }

        return false;
        });

    screen.Loop(component);
}

#endif // CITY_SIMULATOR_ENHANCED_H