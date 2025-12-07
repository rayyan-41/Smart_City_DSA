/*
 * ============================================================================
 * ENHANCED CITY SIMULATOR - With Interactive 2D Graph Visualization
 * ============================================================================
 */

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

    GraphNode2D() : id(-1), pos(), name(""), type(""), sector(""),
        color(Color::White), isCorner(false) {
    }
};

struct GraphEdge2D {
    int fromID, toID;
    GraphEdge2D(int f, int t) : fromID(f), toID(t) {}
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
// GRAPH VIEWPORT - Handles coordinate transformation for visualization
// ============================================================================
// 
// Coordinate System:
//   Geographic: Latitude increases NORTH, Longitude increases EAST
//   Screen/Canvas: X increases RIGHT, Y increases DOWN
//
// Transformation:
//   Longitude -> X (direct mapping, west=left, east=right)
//   Latitude -> Y (INVERTED, north=top, south=bottom)
//
// This creates a proper rectangular map with NORTH at TOP
// ============================================================================
class GraphViewport {
private:
    // Geographic bounds (Islamabad)
    double minLat, maxLat, minLon, maxLon;
    // Canvas dimensions
    int canvasWidth, canvasHeight;
    // Pan offset (in normalized coordinates)
    double offsetX, offsetY;
    // Zoom level
    double zoom;

public:
    GraphViewport() : 
        minLat(BASE_LAT), maxLat(MAX_LAT),      // 33.60 to 33.74
        minLon(BASE_LON), maxLon(MAX_LON),      // 72.96 to 73.10
        canvasWidth(160), canvasHeight(80), 
        offsetX(0), offsetY(0), zoom(1.0) {
    }

    void setBounds(double minLa, double maxLa, double minLo, double maxLo) {
        minLat = minLa; maxLat = maxLa; minLon = minLo; maxLon = maxLo;
    }

    void setCanvasSize(int w, int h) {
        canvasWidth = w; canvasHeight = h;
    }

    /**
     * Convert geographic coordinates to canvas pixel coordinates
     * 
     * Geographic System:     Canvas System:
     *     N (high lat)           (0,0)-----> X (lon)
     *        ^                      |
     *        |                      |
     *  W <---+---> E                v
     *        |                      Y (inverted lat)
     *        v
     *     S (low lat)
     * 
     * Result: North at top, East at right - proper rectangular map
     */
    Point2D geoToCanvas(double lat, double lon) const {
        // Normalize longitude to X: 0 (west) to 1 (east)
        double normX = (lon - minLon) / (maxLon - minLon);
        
        // Normalize latitude to Y: 0 (north/top) to 1 (south/bottom)
        // INVERT: higher latitude = lower Y value (north at top)
        double normY = (maxLat - lat) / (maxLat - minLat);
        
        // Apply zoom (scale around center)
        normX = (normX - 0.5) * zoom + 0.5 + offsetX;
        normY = (normY - 0.5) * zoom + 0.5 + offsetY;
        
        // Apply padding for border spacing
        double padX = 0.03;
        double padY = 0.03;
        
        // Convert to canvas pixel coordinates
        double canvasX = padX * canvasWidth + normX * canvasWidth * (1.0 - 2 * padX);
        double canvasY = padY * canvasHeight + normY * canvasHeight * (1.0 - 2 * padY);
        
        return Point2D(canvasX, canvasY);
    }

    /**
     * Convert canvas pixel coordinates back to geographic coordinates
     * (for mouse hover/click detection)
     */
    void canvasToGeo(int canvasX, int canvasY, double& lat, double& lon) const {
        double padX = 0.03;
        double padY = 0.03;
        
        // Reverse canvas to normalized
        double normX = (canvasX - padX * canvasWidth) / (canvasWidth * (1.0 - 2 * padX));
        double normY = (canvasY - padY * canvasHeight) / (canvasHeight * (1.0 - 2 * padY));
        
        // Reverse zoom and pan
        normX = (normX - 0.5 - offsetX) / zoom + 0.5;
        normY = (normY - 0.5 - offsetY) / zoom + 0.5;
        
        // Convert to geographic
        lon = minLon + normX * (maxLon - minLon);
        lat = maxLat - normY * (maxLat - minLat);  // Invert Y back to latitude
    }

    // Navigation controls
    void zoomIn() { zoom *= 1.2; if (zoom > 5.0) zoom = 5.0; }
    void zoomOut() { zoom /= 1.2; if (zoom < 0.5) zoom = 0.5; }
    void panLeft() { offsetX -= 0.1 / zoom; }
    void panRight() { offsetX += 0.1 / zoom; }
    void panUp() { offsetY -= 0.1 / zoom; }
    void panDown() { offsetY += 0.1 / zoom; }
    void resetView() { zoom = 1.0; offsetX = 0; offsetY = 0; }

    // Getters
    double getZoom() const { return zoom; }
    double getMinLat() const { return minLat; }
    double getMaxLat() const { return maxLat; }
    double getMinLon() const { return minLon; }
    double getMaxLon() const { return maxLon; }
    int getCanvasWidth() const { return canvasWidth; }
    int getCanvasHeight() const { return canvasHeight; }
};

// ============================================================================
// ENUMS
// ============================================================================

enum class SimulatorState {
    INTRO_PHASE_1,
    INTRO_PHASE_2,
    INTRO_PHASE_3,
    WELCOME_ANIMATION,
    MAIN_MENU,
    CSV_SELECTION,
    LOADING,
    GRAPH_VIEW,
    DATABASE_VIEW,
    MANAGEMENT_MENU,
    EXIT
};

enum class CSVLoadMode {
    DEMO_MODE,
    FULL_MODE
};

namespace ASCIIArt {
    const string ISLAMABAD_TITLE[] = {
        R"( ██╗███████╗██╗      █████╗ ███╗   ███╗ █████╗ ██████╗  █████╗ ██████╗ )",
        R"( ██║██╔════╝██║     ██╔══██╗████╗ ████║██╔══██╗██╔══██╗██╔══██╗██╔══██╗)",
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

    string stopsCSV, schoolsCSV, hospitalsCSV, pharmaciesCSV;
    string busesCSV, populationCSV, mallsCSV, shopsCSV, ambulancesCSV;

    Color getNodeColor(const string& type) {
        if (type == "CORNER") return Color::White;
        if (type == "STOP") return Color::GreenLight;
        if (type == "SCHOOL") return Color::Blue;
        if (type == "HOSPITAL") return Color::Red;
        if (type == "PHARMACY") return Color::Magenta;
        if (type == "MOSQUE") return Color::Cyan;
        if (type == "PARK") return Color::Green;
        if (type == "POLICE_STATION") return Color::RedLight;
        if (type == "FIRE_STATION") return Color::Orange1;
        if (type == "LIBRARY") return Color::Blue1;
        if (type == "ATM") return Color::Yellow;
        if (type == "RESTAURANT") return Color::Orange3;
        return Color::GrayLight;
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

    Canvas renderGraphToCanvas(int width, int height);
    void updateHoverState(int mx, int my);
    string getHoverInfo();

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
    showCorners(true), showRoads(true), showSectorBounds(false) {

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
// GRAPH VISUALIZATION
// ============================================================================

inline void CitySimulator::buildGraphVisualization() {
    graphNodes.clear();
    graphEdges.clear();
    sectorRegions.clear();
    nodeIdToIndex.clear();

    if (!islamabad || !islamabad->getCityGraph()) return;
    CityGraph* graph = islamabad->getCityGraph();
    viewport.setBounds(33.60, 33.74, 72.96, 73.10);

    int maxId = 0;
    for (int i = 0; i < graph->getNodeCount(); i++) {
        CityNode* node = graph->getNode(i);
        if (node && node->id > maxId) maxId = node->id;
    }
    nodeIdToIndex.resize(maxId + 1, -1);

    for (int i = 0; i < graph->getNodeCount(); i++) {
        CityNode* node = graph->getNode(i);
        if (!node) continue;
        GraphNode2D gNode;
        gNode.id = node->id;
        gNode.pos = viewport.geoToCanvas(node->lat, node->lon);
        gNode.name = node->name;
        gNode.type = node->type;
        gNode.sector = node->sector;
        gNode.color = getNodeColor(node->type);
        gNode.isCorner = (node->type == "CORNER");
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
                graphEdges.push_back(GraphEdge2D(node->id, edge.destinationID));
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
        region.center = Point2D((tl.x + br.x) / 2, (tl.y + br.y) / 2);
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
            
            // Draw rectangle
            canvas.DrawBlockLine(x1, y1, x2, y1, stylize(boundColor)); // Top
            canvas.DrawBlockLine(x2, y1, x2, y2, stylize(boundColor)); // Right
            canvas.DrawBlockLine(x2, y2, x1, y2, stylize(boundColor)); // Bottom
            canvas.DrawBlockLine(x1, y2, x1, y1, stylize(boundColor)); // Left
        }
    }

    // Draw roads
    if (showRoads) {
        for (const auto& edge : graphEdges) {
            int idx1 = (edge.fromID < (int)nodeIdToIndex.size()) ? nodeIdToIndex[edge.fromID] : -1;
            int idx2 = (edge.toID < (int)nodeIdToIndex.size()) ? nodeIdToIndex[edge.toID] : -1;
            if (idx1 >= 0 && idx2 >= 0 && idx1 < (int)graphNodes.size() && idx2 < (int)graphNodes.size()) {
                const GraphNode2D& n1 = graphNodes[idx1];
                const GraphNode2D& n2 = graphNodes[idx2];
                int x1 = (int)n1.pos.x, y1 = (int)n1.pos.y;
                int x2 = (int)n2.pos.x, y2 = (int)n2.pos.y;
                
                // Use different colors for corner-to-corner vs facility connections
                Color roadColor = (n1.isCorner && n2.isCorner) ? Color::GrayLight : Color::GrayDark;
                canvas.DrawBlockLine(x1, y1, x2, y2, stylize(roadColor));
            }
        }
    }

    // Draw corners
    if (showCorners) {
        for (const auto& node : graphNodes) {
            if (!node.isCorner) continue;
            int x = (int)node.pos.x;
            int y = (int)node.pos.y;
            canvas.DrawBlock(x, y, true, stylize(Color::White));
        }
    }

    // Draw facilities (larger markers)
    for (const auto& node : graphNodes) {
        if (node.isCorner) continue;
        int x = (int)node.pos.x;
        int y = (int)node.pos.y;
        
        // Determine if this node is hovered
        bool isHovered = (node.id == hoveredNodeID);
        Color nodeColor = isHovered ? Color::White : node.color;
        
        // Draw a 3x3 block for visibility
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                int px = x + dx * 2;
                int py = y + dy * 2;
                if (px >= 0 && px < cw && py >= 0 && py < ch) {
                    canvas.DrawBlock(px, py, true, stylize(nodeColor));
                }
            }
        }
        
        // Draw highlight ring for hovered node
        if (isHovered) {
            for (int dx = -2; dx <= 2; dx++) {
                for (int dy = -2; dy <= 2; dy++) {
                    if (std::abs(dx) == 2 || std::abs(dy) == 2) {
                        int px = x + dx * 2;
                        int py = y + dy * 2;
                        if (px >= 0 && px < cw && py >= 0 && py < ch) {
                            canvas.DrawBlock(px, py, true, stylize(Color::Yellow));
                        }
                    }
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

    // Adjust for canvas position (account for border)
    int canvasX = (mx - 1) * 2;
    int canvasY = (my - 2) * 4;

    // Find nearest node within threshold
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

    // Find sector under cursor
    for (auto& region : sectorRegions) {
        region.isHovered = region.contains(Point2D(canvasX, canvasY));
        if (region.isHovered) {
            hoveredSector = region.name;
        }
    }
}

inline string CitySimulator::getHoverInfo() {
    std::stringstream ss;

    // If hovering over a specific node
    if (hoveredNodeID >= 0 && hoveredNodeID < (int)nodeIdToIndex.size()) {
        int idx = nodeIdToIndex[hoveredNodeID];
        if (idx >= 0 && idx < (int)graphNodes.size()) {
            const GraphNode2D& node = graphNodes[idx];
            ss << "NODE\n";
            ss << "----------\n";
            ss << "Name: " << node.name << "\n";
            ss << "Type: " << node.type << "\n";
            ss << "Sector: " << node.sector << "\n";
            ss << "ID: " << node.id;
            return ss.str();
        }
    }
    
    // If hovering over a sector
    if (!hoveredSector.empty()) {
        int nodeCount = 0, stopCount = 0, schoolCount = 0, hospitalCount = 0, pharmacyCount = 0;
        for (const auto& node : graphNodes) {
            if (node.sector == hoveredSector && !node.isCorner) {
                nodeCount++;
                if (node.type == "STOP") stopCount++;
                else if (node.type == "SCHOOL") schoolCount++;
                else if (node.type == "HOSPITAL") hospitalCount++;
                else if (node.type == "PHARMACY") pharmacyCount++;
            }
        }
        
        ss << "SECTOR\n";
        ss << "----------\n";
        ss << "Name: " << hoveredSector << "\n";
        ss << "Facilities: " << nodeCount << "\n";
        ss << "Stops: " << stopCount << "\n";
        ss << "Schools: " << schoolCount << "\n";
        ss << "Hospitals: " << hospitalCount << "\n";
        ss << "Pharmacies: " << pharmacyCount;
        return ss.str();
    }
    
    // Default info
    ss << "HOVER\n";
    ss << "----------\n";
    ss << "Move mouse\n";
    ss << "over nodes\n";
    ss << "or sectors\n";
    ss << "for details";
    return ss.str();
}

// ============================================================================
// DEBUG MODE
// ============================================================================

inline void CitySimulator::runDebugMode() {
    std::cout << "=== DEBUG MODE ===" << std::endl;
    std::cout << "Loading city data..." << std::endl;

    islamabad = new SmartCity();
    islamabad->setDatasetPaths(
        stopsCSV, schoolsCSV, hospitalsCSV, pharmaciesCSV,
        busesCSV, populationCSV, mallsCSV, shopsCSV, ambulancesCSV
    );
    
    islamabad->initialize();
    cityInitialized = true;
    cityMgmt = new CityManagement(islamabad);

    CityStats stats = islamabad->getCityStats();
    std::cout << "Loaded: " << stats.totalNodes << " nodes" << std::endl;
    std::cout << "Press Enter for graph view..." << std::endl;
    std::cin.get();

    runGraphView();
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
// MAIN MENU
// ============================================================================

inline void CitySimulator::runMainMenu() {
    auto screen = ScreenInteractive::Fullscreen();
    std::vector<string> options;
    
    if (cityInitialized) {
        options = {"Graph View [1]", "Database [2]", "City Management [3]", "Exit"};
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
        
        // Status indicator
        string statusText = cityInitialized ? "CITY LOADED" : "NOT INITIALIZED";
        Color statusColor = cityInitialized ? Color::Green : Color::Yellow;
        
        Elements titleArt = {
            text("  ██╗███████╗██╗      █████╗ ███╗   ███╗ █████╗ ██████╗  █████╗ ██████╗ ") | color(Color::Green),
            text("  ██║██╔════╝██║     ██╔══██╗████╗ ████║██╔══██╗██╔══██╗██╔══██╗██╔══██╗") | color(Color::Green),
            text("  ██║███████╗██║     ███████║██╔████╔██║███████║██████╔╝███████║██║  ██║") | color(Color::GreenLight),
            text("  ██║╚════██║██║     ██╔══██║██║╚██╔╝██║██╔══██║██╔══██╗██╔══██║██║  ██║") | color(Color::GreenLight),
            text("  ██║███████║███████╗██║  ██║██║ ╚═╝ ██║██║  ██║██████╔╝██║  ██║██████╔╝") | color(Color::Green),
            text("  ╚═╝╚══════╝╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝╚═════╝ ╚═╝  ╚═╝╚═════╝ ") | color(Color::Green),
        };
        
        auto menuBox = vbox({
            text("MAIN MENU") | bold | center | color(Color::Cyan),
            separator(),
            text(statusText) | center | color(statusColor),
            separator(),
            vbox(items),
            separator(),
            text("Up/Down: Navigate") | dim,
            text("Enter: Select") | dim,
            text("Esc: Exit") | dim,
        }) | border | size(WIDTH, EQUAL, 35);
        
        Elements infoItems;
        if (cityInitialized) {
            CityStats stats = islamabad->getCityStats();
            infoItems = {
                text("CITY INFO") | bold | color(Color::Cyan),
                separator(),
                text("Sectors: " + std::to_string(SECTOR_COUNT)),
                text("Nodes: " + std::to_string(stats.totalNodes)),
                text("Schools: " + std::to_string(stats.totalSchools)),
                text("Hospitals: " + std::to_string(stats.totalHospitals)),
                text("Buses: " + std::to_string(stats.totalBuses)),
                text("Citizens: " + std::to_string(stats.totalCitizens)),
            };
        } else {
            infoItems = {
                text("QUICK START") | bold | color(Color::Cyan),
                separator(),
                text("Select 'Initialize'"),
                text("to load city data"),
                text("from CSV files."),
                separator(),
                text("Then use 1/2/3"),
                text("for quick navigation"),
            };
        }
        
        auto infoBox = vbox(infoItems) | border | size(WIDTH, EQUAL, 25);
        
        return vbox({ 
            filler(),
            vbox(titleArt) | center,
            text("R E D E F I N E D") | bold | center | color(Color::GrayLight),
            text(""),
            hbox({ filler(), menuBox, text("  "), infoBox, filler() }),
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
                else if (sel == 1) currentState = SimulatorState::DATABASE_VIEW;
                else if (sel == 2) currentState = SimulatorState::MANAGEMENT_MENU;
                else currentState = SimulatorState::EXIT;
            }
            screen.Exit(); 
            return true;
        }
        
        // Quick navigation after initialization
        if (cityInitialized) {
            if (e == Event::Character('1')) { currentState = SimulatorState::GRAPH_VIEW; screen.Exit(); return true; }
            if (e == Event::Character('2')) { currentState = SimulatorState::DATABASE_VIEW; screen.Exit(); return true; }
            if (e == Event::Character('3')) { currentState = SimulatorState::MANAGEMENT_MENU; screen.Exit(); return true; }
        }
        
        if (e == Event::Escape) { currentState = SimulatorState::EXIT; screen.Exit(); return true; }
        return false;
    });
    screen.Loop(comp);
}

// ============================================================================
// CSV SELECTION
// ============================================================================

inline void CitySimulator::runCSVSelection() {
    auto screen = ScreenInteractive::Fullscreen();
    std::vector<string> options = { "Demo Mode", "Full Mode", "Back" };
    int sel = 0;

    auto renderer = Renderer([&] {
        Elements items;
        for (int i = 0; i < (int)options.size(); i++)
            items.push_back(text((i == sel ? " > " : "   ") + options[i]));
        
        auto box = vbox({
            text("SELECT MODE") | bold | center,
            separator(),
            vbox(items),
        }) | border | size(WIDTH, EQUAL, 30);
        
        return vbox({ filler(), hbox({ filler(), box, filler() }), filler() });
    });

    auto comp = CatchEvent(renderer, [&](Event e) {
        if (e == Event::ArrowUp) { sel = (sel - 1 + options.size()) % options.size(); return true; }
        if (e == Event::ArrowDown) { sel = (sel + 1) % options.size(); return true; }
        if (e == Event::Return) {
            if (sel == 2) currentState = SimulatorState::MAIN_MENU;
            else { loadMode = (sel == 0) ? CSVLoadMode::DEMO_MODE : CSVLoadMode::FULL_MODE; currentState = SimulatorState::LOADING; }
            screen.Exit(); return true;
        }
        if (e == Event::Escape) { currentState = SimulatorState::MAIN_MENU; screen.Exit(); return true; }
        return false;
    });
    screen.Loop(comp);
}

// ============================================================================
// LOADING SCREEN - Enhanced with Green Progress Bars
// ============================================================================

inline void CitySimulator::runLoadingScreen() {
    auto screen = ScreenInteractive::Fullscreen();
    std::atomic<int> step{ 0 };
    std::atomic<int> totalSteps{ 12 };
    std::atomic<bool> done{ false };

    auto renderer = Renderer([&] {
        int s = step.load();
        int total = totalSteps.load();
        
        // Calculate percentage
        int percent = (s * 100) / total;
        
        // Create progress bar with blocks
        int barWidth = 40;
        int filledWidth = (s * barWidth) / total;
        string progressBar = "";
        for (int i = 0; i < barWidth; i++) {
            if (i < filledWidth) {
                progressBar += "█";
            } else {
                progressBar += "░";
            }
        }
        
        // Loading stages
        std::vector<string> stages = {
            "Initializing City Graph...",
            "Loading Sector Frames...",
            "Loading Bus Stops...",
            "Loading Schools...",
            "Adding Schools to Graph...",
            "Loading Hospitals...",
            "Loading Pharmacies...",
            "Loading Transport System...",
            "Loading Population Data...",
            "Generating Pickup Points...",
            "Loading Commercial Data...",
            "Finalizing..."
        };
        
        string currentStage = (s < (int)stages.size()) ? stages[s] : "Complete!";
        
        // Build stage indicators
        Elements stageList;
        for (int i = 0; i < (int)stages.size(); i++) {
            string prefix = "  ";
            Color stageColor = Color::GrayDark;
            if (i < s) {
                prefix = "+ ";
                stageColor = Color::Green;
            } else if (i == s) {
                prefix = "> ";
                stageColor = Color::GreenLight;
            } else {
                prefix = "o ";
                stageColor = Color::GrayDark;
            }
            stageList.push_back(text(prefix + stages[i]) | color(stageColor));
        }
        
        Elements titleArt = {
            text("  ██╗███████╗██╗      █████╗ ███╗   ███╗ █████╗ ██████╗  █████╗ ██████╗ ") | color(Color::Green),
            text("  ██║██╔════╝██║     ██╔══██╗████╗ ████║██╔══██╗██╔══██╗██╔══██╗██╔══██╗") | color(Color::Green),
            text("  ██║███████╗██║     ███████║██╔████╔██║███████║██████╔╝███████║██║  ██║") | color(Color::GreenLight),
            text("  ██║╚════██║██║     ██╔══██║██║╚██╔╝██║██╔══██║██╔══██╗██╔══██║██║  ██║") | color(Color::GreenLight),
            text("  ██║███████║███████╗██║  ██║██║ ╚═╝ ██║██║  ██║██████╔╝██║  ██║██████╔╝") | color(Color::Green),
            text("  ╚═╝╚══════╝╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝╚═════╝ ╚═╝  ╚═╝╚═════╝ ") | color(Color::Green),
        };
        
        Element progressElement = hbox({
            text(" [") | color(Color::White),
            text(progressBar) | color(Color::Green),
            text("] ") | color(Color::White),
            text(std::to_string(percent) + "%") | bold | color(Color::GreenLight),
        });
        
        Element stageBox = vbox(stageList);
        
        Element content = vbox({
            text(""),
            vbox(titleArt) | center,
            text(""),
            separator() | color(Color::Green),
            text(""),
            text("LOADING CITY DATA") | bold | center | color(Color::White),
            text(""),
            progressElement | center,
            text(""),
            text(currentStage) | center | color(Color::Yellow),
            text(""),
            separator() | color(Color::GrayDark),
            text(""),
            stageBox | center,
            text(""),
            separator() | color(Color::Green),
            text(done.load() ? "Press Enter to continue..." : "Please wait...") | center | dim,
        }) | border | color(Color::Green) | size(WIDTH, EQUAL, 80);
        
        return vbox({ 
            filler(), 
            hbox({ filler(), content, filler() }), 
            filler() 
        });
    });

    std::thread t([&]() {
        // Step 0: Initialize graph
        step.store(0);
        screen.PostEvent(Event::Custom);
        islamabad = new SmartCity();
        islamabad->setDatasetPaths(stopsCSV, schoolsCSV, hospitalsCSV, pharmaciesCSV,
                                   busesCSV, populationCSV, mallsCSV, shopsCSV, ambulancesCSV);
        sleepMs(200);
        
        // Step 1: Sector frames
        step.store(1);
        screen.PostEvent(Event::Custom);
        sleepMs(150);
        
        // Step 2-11: Actual initialization (done inside SmartCity::initialize)
        // We simulate progress here
        for (int i = 2; i <= 11; i++) {
            step.store(i);
            screen.PostEvent(Event::Custom);
            if (i == 5) {
                // This is where the actual initialization happens
                islamabad->initialize();
                cityInitialized = true;
                cityMgmt = new CityManagement(islamabad);
            }
            sleepMs(120);
        }
        
        // Step 12: Done
        step.store(12);
        screen.PostEvent(Event::Custom);
        sleepMs(300);
        
        done.store(true);
        screen.PostEvent(Event::Custom);
    });

    auto comp = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Return && done.load()) { 
            currentState = SimulatorState::MAIN_MENU; 
            screen.Exit(); 
            return true; 
        }
        return false;
    });
    screen.Loop(comp);
    if (t.joinable()) t.join();
}

// ============================================================================
// GRAPH VIEW - Main City Map Visualization
// ============================================================================

inline void CitySimulator::runGraphView() {
    auto screen = ScreenInteractive::Fullscreen();
    
    // Build initial visualization
    buildGraphVisualization();

    auto renderer = Renderer([&] {
        int termW = Terminal::Size().dimx;
        int termH = Terminal::Size().dimy;
        
        // Calculate canvas size (leave room for info panel)
        int canvasW = termW - 30;
        int canvasH = termH - 4;
        
        Canvas c = renderGraphToCanvas(canvasW, canvasH);

        // Count node types
        int corners = 0, stops = 0, schools = 0, hospitals = 0, pharmacies = 0;
        for (const auto& n : graphNodes) {
            if (n.isCorner) corners++;
            else if (n.type == "STOP") stops++;
            else if (n.type == "SCHOOL") schools++;
            else if (n.type == "HOSPITAL") hospitals++;
            else if (n.type == "PHARMACY") pharmacies++;
        }

        // Hover info panel
        string hoverText = getHoverInfo();
        Elements hoverLines;
        std::istringstream iss(hoverText);
        string line;
        while (std::getline(iss, line)) {
            hoverLines.push_back(text(line));
        }

        // Info panel
        auto infoPanel = vbox({
            text("LEGEND") | bold | color(Color::Cyan),
            separator(),
            hbox({text("■") | color(Color::GreenLight), text(" Stop")}),
            hbox({text("■") | color(Color::Blue), text(" School")}),
            hbox({text("■") | color(Color::Red), text(" Hospital")}),
            hbox({text("■") | color(Color::Magenta), text(" Pharmacy")}),
            hbox({text("□") | color(Color::White), text(" Corner")}),
            separator(),
            text("STATISTICS") | bold | color(Color::Cyan),
            text("Stops: " + std::to_string(stops)),
            text("Schools: " + std::to_string(schools)),
            text("Hospitals: " + std::to_string(hospitals)),
            text("Pharmacies: " + std::to_string(pharmacies)),
            text("Corners: " + std::to_string(corners)),
            text("Roads: " + std::to_string(graphEdges.size())),
            separator(),
            text("HOVER INFO") | bold | color(Color::Yellow),
            vbox(hoverLines),
            separator(),
            text("CONTROLS") | bold | color(Color::Cyan),
            text("+/-: Zoom"),
            text("Arrows: Pan"),
            text("R: Toggle Roads"),
            text("C: Toggle Corners"),
            text("S: Sector Bounds"),
            text("0: Reset View"),
            separator(),
            text("NAVIGATION") | bold | color(Color::Green),
            text("1: Graph View") | bold,
            text("2: Database"),
            text("3: Management"),
            text("Esc: Main Menu"),
        }) | border | size(WIDTH, EQUAL, 24);

        // Title bar with sector orientation
        auto titleBar = hbox({
            text("ISLAMABAD CITY MAP") | bold | color(Color::Green),
            text("  "),
            text("[N]") | color(Color::Yellow),
            text(" North at Top "),
            text("Zoom: " + std::to_string((int)(viewport.getZoom() * 100)) + "%") | dim,
        });

        return vbox({
            titleBar | center,
            hbox({ 
                canvas(c) | border | flex, 
                text(" "), 
                infoPanel 
            }) | flex,
        });
    });

    auto comp = CatchEvent(renderer, [&](Event e) {
        // Zoom controls
        if (e == Event::Character('+') || e == Event::Character('=')) { 
            viewport.zoomIn(); 
            buildGraphVisualization();
            return true; 
        }
        if (e == Event::Character('-')) { 
            viewport.zoomOut(); 
            buildGraphVisualization();
            return true; 
        }
        
        // Pan controls
        if (e == Event::ArrowLeft) { viewport.panLeft(); buildGraphVisualization(); return true; }
        if (e == Event::ArrowRight) { viewport.panRight(); buildGraphVisualization(); return true; }
        if (e == Event::ArrowUp) { viewport.panUp(); buildGraphVisualization(); return true; }
        if (e == Event::ArrowDown) { viewport.panDown(); buildGraphVisualization(); return true; }
        
        // Toggle controls
        if (e == Event::Character('r') || e == Event::Character('R')) { showRoads = !showRoads; return true; }
        if (e == Event::Character('c') || e == Event::Character('C')) { showCorners = !showCorners; return true; }
        if (e == Event::Character('s') || e == Event::Character('S')) { showSectorBounds = !showSectorBounds; return true; }
        if (e == Event::Character('0')) { viewport.resetView(); buildGraphVisualization(); return true; }
        
        // Quick view switching (1, 2, 3 keys)
        if (e == Event::Character('1')) { 
            currentState = SimulatorState::GRAPH_VIEW; 
            screen.Exit(); 
            return true; 
        }
        if (e == Event::Character('2')) { 
            currentState = SimulatorState::DATABASE_VIEW; 
            screen.Exit(); 
            return true; 
        }
        if (e == Event::Character('3')) { 
            currentState = SimulatorState::MANAGEMENT_MENU; 
            screen.Exit(); 
            return true; 
        }
        
        // Mouse handling for hover
        if (e.is_mouse()) {
            updateHoverState(e.mouse().x, e.mouse().y);
            return true;
        }
        
        // Exit
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
// DATABASE VIEW - Data Browser
// ============================================================================

inline void CitySimulator::runDatabaseView() {
    auto screen = ScreenInteractive::Fullscreen();
    std::vector<string> categories = { "Sectors", "Nodes", "Schools", "Hospitals", "Pharmacies", "Buses", "Citizens" };
    int catSel = 0;
    int itemSel = 0;
    int scrollOffset = 0;
    const int maxVisibleItems = 15;

    auto renderer = Renderer([&] {
        CityStats stats; 
        if (islamabad) stats = islamabad->getCityStats();
        
        // Category menu
        Elements catMenu;
        for (int i = 0; i < (int)categories.size(); i++) {
            auto item = text((i == catSel ? " > " : "   ") + categories[i]);
            if (i == catSel) item = item | bold | color(Color::Green);
            catMenu.push_back(item);
        }
        
        // Item list based on selected category
        Elements itemList;
        std::vector<string> items;
        
        if (islamabad && islamabad->getCityGraph()) {
            CityGraph* graph = islamabad->getCityGraph();
            
            switch (catSel) {
                case 0: // Sectors
                    for (int i = 0; i < SECTOR_COUNT; i++) {
                        items.push_back(SECTOR_GRID[i].name);
                    }
                    break;
                case 1: // Nodes
                    for (int i = 0; i < graph->getNodeCount(); i++) {
                        CityNode* node = graph->getNode(i);
                        if (node && node->type != "CORNER") {
                            items.push_back(node->name + " [" + node->sector + "]");
                        }
                    }
                    break;
                case 2: // Schools
                    if (islamabad->getSchoolManager()) {
                        for (int i = 0; i < islamabad->getSchoolManager()->schools.getSize(); i++) {
                            School* s = islamabad->getSchoolManager()->schools[i];
                            items.push_back(s->name + " [" + s->getSector() + "]");
                        }
                    }
                    break;
                case 3: // Hospitals
                    if (islamabad->getMedicalManager()) {
                        for (int i = 0; i < islamabad->getMedicalManager()->hospitals.getSize(); i++) {
                            Hospital* h = islamabad->getMedicalManager()->hospitals[i];
                            items.push_back(h->name + " [" + h->sector + "]");
                        }
                    }
                    break;
                case 4: // Pharmacies
                    if (islamabad->getMedicalManager()) {
                        for (int i = 0; i < islamabad->getMedicalManager()->pharmacies.getSize(); i++) {
                            Pharmacy* p = islamabad->getMedicalManager()->pharmacies[i];
                            items.push_back(p->name + " [" + p->sector + "]");
                        }
                    }
                    break;
                case 5: // Buses
                    if (islamabad->getTransportManager()) {
                        auto buses = islamabad->getTransportManager()->getAllBuses();
                        for (int i = 0; i < buses.getSize(); i++) {
                            items.push_back(buses[i]->getBusNo() + " - " + buses[i]->getCompany());
                        }
                    }
                    break;
                case 6: // Citizens
                    items.push_back("Total: " + std::to_string(stats.totalCitizens));
                    items.push_back("Sectors: " + std::to_string(stats.totalSectors));
                    items.push_back("Streets: " + std::to_string(stats.totalStreets));
                    items.push_back("Houses: " + std::to_string(stats.totalHouses));
                    break;
            }
        }
        
        // Clamp selection
        if (itemSel >= (int)items.size()) itemSel = items.size() > 0 ? items.size() - 1 : 0;
        
        // Build visible items with scroll
        if (itemSel < scrollOffset) scrollOffset = itemSel;
        if (itemSel >= scrollOffset + maxVisibleItems) scrollOffset = itemSel - maxVisibleItems + 1;
        
        for (int i = scrollOffset; i < std::min((int)items.size(), scrollOffset + maxVisibleItems); i++) {
            auto item = text((i == itemSel ? " > " : "   ") + items[i]);
            if (i == itemSel) item = item | bold | color(Color::Yellow);
            itemList.push_back(item);
        }
        
        if (items.empty()) {
            itemList.push_back(text("   (no items)") | dim);
        }
        
        // Stats panel
        auto statsPanel = vbox({
            text("CITY STATISTICS") | bold | color(Color::Cyan),
            separator(),
            text("Graph Nodes: " + std::to_string(stats.totalNodes)),
            text("Bus Stops: " + std::to_string(stats.busStops)),
            text("Schools: " + std::to_string(stats.totalSchools)),
            text("Hospitals: " + std::to_string(stats.totalHospitals)),
            text("Pharmacies: " + std::to_string(stats.totalPharmacies)),
            separator(),
            text("Buses: " + std::to_string(stats.totalBuses)),
            text("School Buses: " + std::to_string(stats.totalSchoolBuses)),
            text("Ambulances: " + std::to_string(stats.totalAmbulances)),
            separator(),
            text("Citizens: " + std::to_string(stats.totalCitizens)),
        }) | border | size(WIDTH, EQUAL, 25);

        // Navigation help
        auto navPanel = vbox({
            text("NAVIGATION") | bold | color(Color::Green),
            separator(),
            text("1: Graph View"),
            text("2: Database"),
            text("3: Management"),
            separator(),
            text("Tab: Switch Panel"),
            text("Up/Down: Navigate"),
            text("Esc: Main Menu"),
        }) | border | size(WIDTH, EQUAL, 20);
        
        return vbox({ 
            text("DATABASE BROWSER") | bold | center | color(Color::Green),
            separator(),
            hbox({ 
                vbox({
                    text("CATEGORIES") | bold,
                    separator(),
                    vbox(catMenu),
                }) | border | size(WIDTH, EQUAL, 18),
                text(" "),
                vbox({
                    text("ITEMS (" + std::to_string(items.size()) + ")") | bold,
                    separator(),
                    vbox(itemList) | flex,
                }) | border | flex,
                text(" "),
                vbox({statsPanel, navPanel}),
            }) | flex,
        });
    });

    bool inCategoryPanel = true;
    
    auto comp = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Tab) {
            inCategoryPanel = !inCategoryPanel;
            return true;
        }
        
        if (e == Event::ArrowUp) { 
            if (inCategoryPanel) {
                catSel = (catSel - 1 + categories.size()) % categories.size();
                itemSel = 0;
                scrollOffset = 0;
            } else {
                if (itemSel > 0) itemSel--;
            }
            return true; 
        }
        if (e == Event::ArrowDown) { 
            if (inCategoryPanel) {
                catSel = (catSel + 1) % categories.size();
                itemSel = 0;
                scrollOffset = 0;
            } else {
                itemSel++;
            }
            return true; 
        }
        if (e == Event::ArrowLeft && !inCategoryPanel) {
            inCategoryPanel = true;
            return true;
        }
        if (e == Event::ArrowRight && inCategoryPanel) {
            inCategoryPanel = false;
            return true;
        }
        
        // Quick view switching
        if (e == Event::Character('1')) { 
            currentState = SimulatorState::GRAPH_VIEW; 
            screen.Exit(); 
            return true; 
        }
        if (e == Event::Character('2')) { 
            // Already in Database View
            return true; 
        }
        if (e == Event::Character('3')) { 
            currentState = SimulatorState::MANAGEMENT_MENU; 
            screen.Exit(); 
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
// CITY MANAGEMENT MENU - With Quick Navigation
// ============================================================================

inline void CitySimulator::runManagementMenu() {
    auto screen = ScreenInteractive::Fullscreen();
    std::vector<string> items = { 
        "School Management",
        "Transport Management", 
        "Medical Management",
        "Citizen Management",
        "Location Management",
        "View Statistics",
        "Back to Menu" 
    };
    int sel = 0;
    string statusMsg = "";
    int subMenu = -1;

    std::vector<string> schoolItems = {"Add School", "Remove School", "Add Department", "Hire Faculty", "View Schools", "Back"};
    std::vector<string> transportItems = {"Add Bus Route", "Register School Bus", "Add Bus Stop", "View Routes", "Back"};
    std::vector<string> medicalItems = {"Add Hospital", "Add Pharmacy", "Register Ambulance", "Add Medicine", "View Medical", "Back"};
    std::vector<string> citizenItems = {"Add Citizen", "Enroll Student", "View Citizens", "View Unemployed", "Back"};
    std::vector<string> locationItems = {"Add Bus Stop", "Connect Roads", "View Graph Stats", "Back"};

    int subSel = 0;

    auto getActiveItems = [&]() -> std::vector<string>& {
        switch(subMenu) {
            case 0: return schoolItems;
            case 1: return transportItems;
            case 2: return medicalItems;
            case 3: return citizenItems;
            case 4: return locationItems;
            default: return items;
        }
    };

    auto renderer = Renderer([&] {
        CityStats stats; 
        if (islamabad) stats = islamabad->getCityStats();
        
        auto& activeItems = getActiveItems();
        int activeSel = (subMenu == -1) ? sel : subSel;
        
        Elements menu;
        for (int i = 0; i < (int)activeItems.size(); i++) {
            auto item = text((i == activeSel ? " > " : "   ") + activeItems[i]);
            if (i == activeSel) item = item | bold | color(Color::Green);
            menu.push_back(item);
        }
        
        string menuTitle = "CITY MANAGEMENT";
        if (subMenu == 0) menuTitle = "SCHOOL MANAGEMENT";
        else if (subMenu == 1) menuTitle = "TRANSPORT MANAGEMENT";
        else if (subMenu == 2) menuTitle = "MEDICAL MANAGEMENT";
        else if (subMenu == 3) menuTitle = "CITIZEN MANAGEMENT";
        else if (subMenu == 4) menuTitle = "LOCATION MANAGEMENT";
        
        auto menuBox = vbox({
            text(menuTitle) | bold | center | color(Color::Cyan),
            separator(),
            vbox(menu),
            separator(),
            text(statusMsg) | color(Color::Yellow),
        }) | border | size(WIDTH, EQUAL, 35);

        auto statsBox = vbox({
            text("STATS") | bold | color(Color::Cyan),
            separator(),
            text("Schools: " + std::to_string(stats.totalSchools)),
            text("Hospitals: " + std::to_string(stats.totalHospitals)),
            text("Buses: " + std::to_string(stats.totalBuses)),
            text("Citizens: " + std::to_string(stats.totalCitizens)),
        }) | border | size(WIDTH, EQUAL, 22);

        auto navBox = vbox({
            text("NAV") | bold | color(Color::Green),
            separator(),
            text("1: Graph"),
            text("2: Database"),
            text("3: Management") | bold,
            text("Esc: Back"),
        }) | border | size(WIDTH, EQUAL, 16);
        
        return vbox({ 
            text("ISLAMABAD CITY MANAGEMENT") | bold | center | color(Color::Green),
            separator(),
            hbox({ filler(), menuBox, text(" "), statsBox, text(" "), navBox, filler() }) | flex,
        });
    });

    auto comp = CatchEvent(renderer, [&](Event e) {
        auto& activeItems = getActiveItems();
        int& activeSel = (subMenu == -1) ? sel : subSel;
        
        if (e == Event::ArrowUp) { activeSel = (activeSel - 1 + activeItems.size()) % activeItems.size(); return true; }
        if (e == Event::ArrowDown) { activeSel = (activeSel + 1) % activeItems.size(); return true; }
        
        if (e == Event::Character('1')) { currentState = SimulatorState::GRAPH_VIEW; screen.Exit(); return true; }
        if (e == Event::Character('2')) { currentState = SimulatorState::DATABASE_VIEW; screen.Exit(); return true; }
        if (e == Event::Character('3')) { return true; }
        
        if (e == Event::Return) {
            if (subMenu == -1) {
                if (sel == (int)items.size() - 1) { currentState = SimulatorState::MAIN_MENU; screen.Exit(); return true; }
                if (sel == 5) { 
                    if (cityMgmt) {
                        auto mgmtStats = cityMgmt->getManagementStats();
                        statusMsg = std::to_string(mgmtStats.totalSchools) + " schools, " + std::to_string(mgmtStats.totalBuses) + " buses";
                    }
                    return true;
                }
                subMenu = sel; subSel = 0; statusMsg = ""; return true;
            }
            
            if (cityMgmt) {
                if (subSel == (int)activeItems.size() - 1) { subMenu = -1; statusMsg = ""; return true; }
                
                if (subMenu == 0) { // School
                    if (subSel == 0) { Vector<string> d,s; d.push_back("Science"); s.push_back("Math"); string id = cityMgmt->addSchool("School", "G-9", 4.0f, d, s); statusMsg = id.empty() ? "Failed" : "Added: " + id; }
                    else if (subSel == 4) { statusMsg = "Schools: " + std::to_string(cityMgmt->getAllSchools().getSize()); }
                }
                else if (subMenu == 1) { // Transport
                    if (subSel == 2) { int id = cityMgmt->addBusStopInSector("Stop", "G-9"); statusMsg = id >= 0 ? "Added" : "Failed"; }
                    else if (subSel == 3) { statusMsg = "Buses: " + std::to_string(cityMgmt->getAllBuses().getSize()); }
                }
                else if (subMenu == 2) { // Medical
                    if (subSel == 0) { Vector<string> sp; sp.push_back("Emergency"); string id = cityMgmt->addHospital("Hospital", "F-8", 50, sp); statusMsg = id.empty() ? "Failed" : "Added: " + id; }
                    else if (subSel == 4) { statusMsg = "Hospitals: " + std::to_string(cityMgmt->getAllHospitals().getSize()); }
                }
                else if (subMenu == 3) { // Citizen
                    if (subSel == 0) { string cnic = cityMgmt->addCitizen("Citizen", 25, "G-9", 1, 1); statusMsg = cnic.empty() ? "Failed" : "Added: " + cnic; }
                    else if (subSel == 2) { statusMsg = "Citizens: " + std::to_string(cityMgmt->getTotalCitizenCount()); }
                }
                else if (subMenu == 4) { // Location
                    if (subSel == 0) { int id = cityMgmt->addBusStopInSector("Stop", "F-7"); statusMsg = id >= 0 ? "Added" : "Failed"; }
                    else if (subSel == 2) { auto s = cityMgmt->getManagementStats(); statusMsg = "Stops: " + std::to_string(s.totalStops); }
                }
            }
            return true;
        }
        if (e == Event::Escape) { 
            if (subMenu != -1) { subMenu = -1; statusMsg = ""; }
            else { currentState = SimulatorState::MAIN_MENU; screen.Exit(); }
            return true; 
        }
        return false;
    });
    screen.Loop(comp);
}

#endif // CITY_SIMULATOR_ENHANCED_H

