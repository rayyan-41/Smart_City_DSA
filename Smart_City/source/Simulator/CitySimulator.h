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
    DIJKSTRA_VIEW,  // New state for Dijkstra visualization
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
    void runManagementMenu();
    void runDijkstraView();  // New Dijkstra visualization

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
        } else {
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
        } else if (node.isEnd) {
            nodeColor = Color::Yellow;
        } else if (node.isOnPath) {
            nodeColor = Color::GreenLight;
        } else if (node.isVisited) {
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
        options = {"Graph View [1]", "Dijkstra Pathfinding [D]", "Database [2]", "Management [3]", "Exit"};
    } else {
        options = {"Initialize City", "Exit"};
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
            } else {
                if (sel == 0) currentState = SimulatorState::GRAPH_VIEW;
                else if (sel == 1) currentState = SimulatorState::DIJKSTRA_VIEW;
                else if (sel == 2) currentState = SimulatorState::DATABASE_VIEW;
                else if (sel == 3) currentState = SimulatorState::MANAGEMENT_MENU;
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
        
        std::vector<string> options = {"Begin Initialization", "Back to Menu"};
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
    
    std::vector<string> targetTypes = {"Nearest School", "Nearest Hospital", "Nearest Pharmacy", "Nearest Bus Stop"};
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
            
        } else if (dijkstraMode == DijkstraMode::SELECT_TARGET_TYPE) {
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
            
        } else if (dijkstraMode == DijkstraMode::COMPLETE) {
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
            controlItems.push_back(text(" " + std::to_string(dijkstraDistance).substr(0,5) + " km") | color(Color::Yellow));
            controlItems.push_back(text("Stops:") | bold);
            controlItems.push_back(text(" " + std::to_string(dijkstraPath.getSize())) | color(Color::Yellow));
            controlItems.push_back(separator());
            controlItems.push_back(text("R: Reset") | dim);
            controlItems.push_back(text("Esc: Back") | dim);
        }
        
        // Legend
        controlItems.push_back(separator());
        controlItems.push_back(text("LEGEND") | bold);
        controlItems.push_back(hbox({text("■") | color(Color::Cyan), text(" Start")}));
        controlItems.push_back(hbox({text("■") | color(Color::Yellow), text(" End")}));
        controlItems.push_back(hbox({text("■") | color(Color::GreenLight), text(" Path")}));

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
// DATABASE VIEW - Robust Sector Browser with Details Panel
// ============================================================================

inline void CitySimulator::runDatabaseView() {
    auto screen = ScreenInteractive::Fullscreen();
    
    // State variables
    int selectedSectorIdx = 0;
    int selectedCategoryIdx = 0;
    int selectedItemIdx = 0;
    int detailScrollOffset = 0;
    
    // Categories for filtering
    std::vector<string> categories = {"All", "Stops", "Schools", "Hospitals", "Pharmacies", "Malls"};
    
    // Build sector list (E-7 to I-12)
    std::vector<string> sectorList;
    for (int i = 0; i < SECTOR_COUNT; i++) {
        sectorList.push_back(SECTOR_GRID[i].name);
    }
    
    // Current view mode: 0=sector list, 1=category, 2=items, 3=details
    int focusPanel = 0;
    
    auto renderer = Renderer([&] {
        string currentSector = sectorList[selectedSectorIdx];
        string currentCategory = categories[selectedCategoryIdx];
        
        // ===== LEFT PANEL: Sector List =====
        Elements sectorItems;
        sectorItems.push_back(text("SECTORS") | bold | color(Color::Cyan));
        sectorItems.push_back(separator());
        
        // Show sectors with scroll window
        int sectorStartIdx = std::max(0, selectedSectorIdx - 8);
        int sectorEndIdx = std::min((int)sectorList.size(), sectorStartIdx + 18);
        
        for (int i = sectorStartIdx; i < sectorEndIdx; i++) {
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
        
        // ===== CATEGORY TABS =====
        Elements categoryTabs;
        for (int i = 0; i < (int)categories.size(); i++) {
            auto tab = text(" " + categories[i] + " ");
            if (i == selectedCategoryIdx) {
                tab = tab | bold | bgcolor(Color::Green) | color(Color::Black);
            } else {
                tab = tab | color(Color::GrayLight);
            }
            categoryTabs.push_back(tab);
            if (i < (int)categories.size() - 1) categoryTabs.push_back(text(" "));
        }
        
        // ===== MIDDLE PANEL: Items List =====
        Elements itemList;
        std::vector<CityNode*> filteredNodes;
        
        if (islamabad && islamabad->getCityGraph()) {
            CityGraph* graph = islamabad->getCityGraph();
            for (int i = 0; i < graph->getNodeCount(); i++) {
                CityNode* node = graph->getNode(i);
                if (node && node->sector == currentSector && node->type != "CORNER") {
                    bool include = false;
                    if (currentCategory == "All") include = true;
                    else if (currentCategory == "Stops" && node->type == "STOP") include = true;
                    else if (currentCategory == "Schools" && node->type == "SCHOOL") include = true;
                    else if (currentCategory == "Hospitals" && node->type == "HOSPITAL") include = true;
                    else if (currentCategory == "Pharmacies" && node->type == "PHARMACY") include = true;
                    else if (currentCategory == "Malls" && node->type == "MALL") include = true;
                    
                    if (include) filteredNodes.push_back(node);
                }
            }
        }
        
        if (selectedItemIdx >= (int)filteredNodes.size()) {
            selectedItemIdx = std::max(0, (int)filteredNodes.size() - 1);
        }
        
        itemList.push_back(text("FACILITIES (" + std::to_string(filteredNodes.size()) + ")") | bold | color(Color::Yellow));
        itemList.push_back(separator());
        
        if (filteredNodes.empty()) {
            itemList.push_back(text("No items found") | dim);
        } else {
            int itemStartIdx = std::max(0, selectedItemIdx - 6);
            int itemEndIdx = std::min((int)filteredNodes.size(), itemStartIdx + 14);
            
            for (int i = itemStartIdx; i < itemEndIdx; i++) {
                CityNode* node = filteredNodes[i];
                string prefix = (i == selectedItemIdx) ? "▶ " : "  ";
                string displayName = node->name;
                if (displayName.length() > 22) displayName = displayName.substr(0, 19) + "...";
                
                // Type icon
                string icon = "●";
                if (node->type == "STOP") icon = "◎";
                else if (node->type == "SCHOOL") icon = "◆";
                else if (node->type == "HOSPITAL") icon = "✚";
                else if (node->type == "PHARMACY") icon = "⚕";
                else if (node->type == "MALL") icon = "◈";
                
                auto item = text(prefix + icon + " " + displayName);
                if (i == selectedItemIdx) {
                    item = item | bold;
                    if (focusPanel == 2) item = item | bgcolor(Color::Blue) | color(Color::White);
                    else item = item | color(Color::Green);
                }
                itemList.push_back(item);
            }
        }
        
        auto itemPanel = vbox(itemList) | border | size(WIDTH, EQUAL, 30);
        if (focusPanel == 2) itemPanel = itemPanel | color(Color::Cyan);
        
        // ===== RIGHT PANEL: Details =====
        Elements detailItems;
        detailItems.push_back(text("DETAILS") | bold | color(Color::Magenta));
        detailItems.push_back(separator());
        
        if (!filteredNodes.empty() && selectedItemIdx < (int)filteredNodes.size()) {
            CityNode* selectedNode = filteredNodes[selectedItemIdx];
            
            detailItems.push_back(hbox({text("Name: ") | bold, text(selectedNode->name) | color(Color::White)}));
            detailItems.push_back(hbox({text("Type: ") | bold, text(selectedNode->type) | color(Color::Cyan)}));
            detailItems.push_back(hbox({text("Sector: ") | bold, text(selectedNode->sector) | color(Color::Green)}));
            detailItems.push_back(hbox({text("ID: ") | bold, text(selectedNode->databaseID) | color(Color::Yellow)}));
            detailItems.push_back(separator());
            
            // Position info
            std::stringstream posStream;
            posStream << std::fixed << std::setprecision(2) << selectedNode->lat << ", " << selectedNode->lon;
            detailItems.push_back(hbox({text("Position: ") | bold, text(posStream.str()) | dim}));
            detailItems.push_back(hbox({text("Connections: ") | bold, text(std::to_string(selectedNode->roads.size())) | color(Color::Orange1)}));
            
            // Type-specific details
            detailItems.push_back(separator());
            
            if (selectedNode->type == "SCHOOL") {
                // Get school details
                if (islamabad && islamabad->getSchoolManager()) {
                    School* school = nullptr;
                    // Search for school by name since findSchoolByName doesn't exist
                    for (int s = 0; s < islamabad->getSchoolManager()->schools.getSize(); s++) {
                        if (islamabad->getSchoolManager()->schools[s]->name == selectedNode->name) {
                            school = islamabad->getSchoolManager()->schools[s];
                            break;
                        }
                    }
                    if (school) {
                        detailItems.push_back(text("SCHOOL INFO") | bold | color(Color::Blue));
                        detailItems.push_back(hbox({text("Rating: ") | bold, text(std::to_string(school->rating).substr(0,3) + "/5.0") | color(Color::Yellow)}));
                        
                        // Build subjects string from vector
                        string subjectsStr = "";
                        for (int si = 0; si < school->subjects.getSize(); si++) {
                            if (si > 0) subjectsStr += ", ";
                            subjectsStr += school->subjects[si];
                            if (subjectsStr.length() > 25) {
                                subjectsStr = subjectsStr.substr(0, 22) + "...";
                                break;
                            }
                        }
                        if (subjectsStr.empty()) subjectsStr = "N/A";
                        detailItems.push_back(hbox({text("Subjects: ") | bold, text(subjectsStr) | color(Color::Cyan)}));
                        
                        // Department count
                        int deptCount = 0;
                        int studentCount = 0;
                        for (int d = 0; d < school->departments.getSize(); d++) {
                            deptCount++;
                            for (int c = 0; c < school->departments[d]->classes.getSize(); c++) {
                                studentCount += school->departments[d]->classes[c]->students.getSize();
                            }
                        }
                        detailItems.push_back(hbox({text("Departments: ") | bold, text(std::to_string(deptCount)) | color(Color::Green)}));
                        detailItems.push_back(hbox({text("Students: ") | bold, text(std::to_string(studentCount)) | color(Color::Green)}));
                    }
                }
            }
            else if (selectedNode->type == "HOSPITAL") {
                // Get hospital details
                if (islamabad && islamabad->getMedicalManager()) {
                    Hospital* hospital = islamabad->getMedicalManager()->findHospitalByID(selectedNode->databaseID);
                    if (hospital) {
                        detailItems.push_back(text("HOSPITAL INFO") | bold | color(Color::Red));
                        detailItems.push_back(hbox({text("Total Beds: ") | bold, text(std::to_string(hospital->totalBeds)) | color(Color::Yellow)}));
                        detailItems.push_back(hbox({text("Available: ") | bold, text(std::to_string(hospital->getAvailableBeds())) | color(Color::Green)}));
                        detailItems.push_back(hbox({text("Specializations: ") | bold}));
                        // Build specializations string from vector
                        string specStr = "";
                        for (int sp = 0; sp < hospital->specializations.getSize(); sp++) {
                            if (sp > 0) specStr += ", ";
                            specStr += hospital->specializations[sp];
                            if (specStr.length() > 25) {
                                specStr = specStr.substr(0, 22) + "...";
                                break;
                            }
                        }
                        if (specStr.empty()) specStr = "General";
                        detailItems.push_back(text("  " + specStr) | color(Color::Cyan));
                        detailItems.push_back(hbox({text("ER Patients: ") | bold, text(std::to_string(hospital->getERQueueSize())) | color(Color::Orange1)}));
                    }
                }
            }
            else if (selectedNode->type == "PHARMACY") {
                // Get pharmacy details
                if (islamabad && islamabad->getMedicalManager()) {
                    Pharmacy* pharmacy = nullptr;
                    // Search for pharmacy by ID since findPharmacyByID doesn't exist
                    for (int p = 0; p < islamabad->getMedicalManager()->pharmacies.getSize(); p++) {
                        if (islamabad->getMedicalManager()->pharmacies[p]->id == selectedNode->databaseID) {
                            pharmacy = islamabad->getMedicalManager()->pharmacies[p];
                            break;
                        }
                    }
                    if (pharmacy) {
                        detailItems.push_back(text("PHARMACY INFO") | bold | color(Color::Magenta));
                        detailItems.push_back(hbox({text("Medicines: ") | bold, text(std::to_string(pharmacy->getMedicineCount())) | color(Color::Cyan)}));
                        // Show first medicine if available
                        if (pharmacy->getMedicineCount() > 0) {
                            const Medicine* firstMed = pharmacy->getMedicine(0);
                            if (firstMed) {
                                detailItems.push_back(hbox({text("Sample: ") | bold, text(firstMed->name.substr(0, 15)) | color(Color::Yellow)}));
                                detailItems.push_back(hbox({text("Formula: ") | bold, text(firstMed->formula) | color(Color::Yellow)}));
                                detailItems.push_back(hbox({text("Price: ") | bold, text("Rs. " + std::to_string((int)firstMed->price)) | color(Color::Green)}));
                            }
                        }
                    }
                }
            }
            else if (selectedNode->type == "STOP") {
                detailItems.push_back(text("BUS STOP INFO") | bold | color(Color::GreenLight));
                if (!selectedNode->operatingHours.empty()) {
                    detailItems.push_back(hbox({text("Hours: ") | bold, text(selectedNode->operatingHours) | color(Color::Cyan)}));
                }
                // Show waiting passengers
                if (islamabad && islamabad->getTransportManager()) {
                    int waiting = islamabad->getTransportManager()->getWaitingCount(selectedNode->id);
                    detailItems.push_back(hbox({text("Waiting: ") | bold, text(std::to_string(waiting) + " passengers") | color(Color::Yellow)}));
                }
            }
            
            // Show residents in this sector
            detailItems.push_back(separator());
            detailItems.push_back(text("SECTOR POPULATION") | bold | color(Color::Orange1));
            
            if (islamabad && islamabad->getPopulationManager()) {
                Vector<Citizen*> residents = islamabad->getPopulationManager()->getCitizensInSector(currentSector);
                detailItems.push_back(hbox({text("Residents: ") | bold, text(std::to_string(residents.getSize())) | color(Color::Green)}));
                
                // Show first few residents
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
            
        } else {
            detailItems.push_back(text("Select an item to") | dim);
            detailItems.push_back(text("view details") | dim);
        }
        
        auto detailPanel = vbox(detailItems) | border | flex;
        if (focusPanel == 3) detailPanel = detailPanel | color(Color::Cyan);
        
        // ===== SECTOR STATS BAR =====
        int stopCount = 0, schoolCount = 0, hospitalCount = 0, pharmacyCount = 0, mallCount = 0;
        if (islamabad && islamabad->getCityGraph()) {
            CityGraph* graph = islamabad->getCityGraph();
            for (int i = 0; i < graph->getNodeCount(); i++) {
                CityNode* node = graph->getNode(i);
                if (node && node->sector == currentSector) {
                    if (node->type == "STOP") stopCount++;
                    else if (node->type == "SCHOOL") schoolCount++;
                    else if (node->type == "HOSPITAL") hospitalCount++;
                    else if (node->type == "PHARMACY") pharmacyCount++;
                    else if (node->type == "MALL") mallCount++;
                }
            }
        }
        
        auto statsBar = hbox({
            text("◎ " + std::to_string(stopCount)) | color(Color::GreenLight),
            text("  "),
            text("◆ " + std::to_string(schoolCount)) | color(Color::Blue),
            text("  "),
            text("✚ " + std::to_string(hospitalCount)) | color(Color::Red),
            text("  "),
            text("⚕ " + std::to_string(pharmacyCount)) | color(Color::Magenta),
            text("  "),
            text("◈ " + std::to_string(mallCount)) | color(Color::Yellow),
        });
        
        // ===== HELP BAR =====
        auto helpBar = hbox({
            text("↑↓") | bold | color(Color::Cyan), text(": Navigate  "),
            text("←→") | bold | color(Color::Cyan), text(": Panel  "),
            text("Tab") | bold | color(Color::Cyan), text(": Category  "),
            text("Enter") | bold | color(Color::Cyan), text(": Select  "),
            text("Esc") | bold | color(Color::Cyan), text(": Back"),
        }) | center;
        
        // ===== MAIN LAYOUT =====
        return vbox({
            hbox({
                text(" DATABASE BROWSER ") | bold | bgcolor(Color::Green) | color(Color::Black),
                text(" "),
                text(currentSector) | bold | color(Color::Cyan),
                filler(),
                statsBar,
            }),
            separator(),
            hbox(categoryTabs) | center,
            separator(),
            hbox({
                sectorPanel,
                text(" "),
                itemPanel,
                text(" "),
                detailPanel,
            }) | flex,
            separator(),
            helpBar,
        });
    });

    auto comp = CatchEvent(renderer, [&](Event e) {
        // Navigation
        if (e == Event::ArrowUp) {
            if (focusPanel == 0 && selectedSectorIdx > 0) selectedSectorIdx--;
            else if ( focusPanel == 2 && selectedItemIdx > 0) selectedItemIdx--;
            return true;
        }
        if (e == Event::ArrowDown) {
            if (focusPanel == 0 && selectedSectorIdx < (int)sectorList.size() - 1) selectedSectorIdx++;
            else if (focusPanel == 2) selectedItemIdx++;
            return true;
        }
        if (e == Event::ArrowLeft) {
            if (focusPanel > 0) focusPanel--;
            else focusPanel = 2;
            return true;
        }
        if (e == Event::ArrowRight) {
            if (focusPanel < 2) focusPanel++;
            else focusPanel = 0;
            return true;
        }
        
        // Tab to change category
        if (e == Event::Tab) {
            selectedCategoryIdx = (selectedCategoryIdx + 1) % categories.size();
            selectedItemIdx = 0;
            return true;
        }
        if (e == Event::TabReverse) {
            selectedCategoryIdx = (selectedCategoryIdx - 1 + categories.size()) % categories.size();
            selectedItemIdx = 0;
            return true;
        }
        
        // Enter to focus on items or details
        if (e == Event::Return) {
            if (focusPanel == 0) focusPanel = 2;
            else if (focusPanel == 2) focusPanel = 0;
            return true;
        }
        
        // Number keys to select category
        if (e == Event::Character('1')) { selectedCategoryIdx = 0; selectedItemIdx = 0; return true; }
        if (e == Event::Character('2')) { selectedCategoryIdx = 1; selectedItemIdx = 0; return true; }
        if (e == Event::Character('3')) { selectedCategoryIdx = 2; selectedItemIdx = 0; return true; }
        if (e == Event::Character('4')) { selectedCategoryIdx = 3; selectedItemIdx = 0; return true; }
        if (e == Event::Character('5')) { selectedCategoryIdx = 4; selectedItemIdx = 0; return true; }
        if (e == Event::Character('6')) { selectedCategoryIdx = 5; selectedItemIdx = 0; return true; }
        
        // Escape to go back
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
// MANAGEMENT MENU - Placeholder for city management functions
// ============================================================================

inline void CitySimulator::runManagementMenu() {
    auto screen = ScreenInteractive::Fullscreen();
    std::vector<string> options = {"View Statistics", "Manage Transport", "Manage Facilities", "Back to Main Menu"};
    int sel = 0;

    auto renderer = Renderer([&] {
        Elements items;
        for (int i = 0; i < (int)options.size(); i++) {
            auto item = text((i == sel ? " > " : "   ") + options[i]);
            if (i == sel) item = item | bold | color(Color::Green);
            items.push_back(item);
        }
        
        auto menuBox = vbox({
            text("MANAGEMENT MENU") | bold | center | color(Color::Cyan),
            separator(),
            vbox(items),
            separator(),
            text("Feature coming soon...") | dim | center,
        }) | border | size(WIDTH, EQUAL, 35);
        
        return vbox({ 
            filler(),
            hbox({ filler(), menuBox, filler() }),
            filler() 
        });
    });

    auto comp = CatchEvent(renderer, [&](Event e) {
        if (e == Event::ArrowUp) { sel = (sel - 1 + options.size()) % options.size(); return true; }
        if (e == Event::ArrowDown) { sel = (sel + 1) % options.size(); return true; }
        if (e == Event::Return) {
            if (sel == 3) { // Back to Main Menu
                currentState = SimulatorState::MAIN_MENU;
                screen.Exit();
            }
            // Other options can be implemented later
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

#endif // CITY_SIMULATOR_ENHANCED_H


