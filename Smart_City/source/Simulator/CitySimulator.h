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
    double lat, lon;        // Store original geo coordinates (immutable)
    Point2D pos;            // Computed canvas position (recomputed on render)
    string name;
    string type;
    string sector;
    Color color;
    bool isCorner;
    bool isOnPath;      // For Dijkstra visualization
    bool isVisited;     // For Dijkstra visualization
    bool isStart;       // Starting node
    bool isEnd;         // Destination node

    GraphNode2D() : id(-1), lat(0), lon(0), pos(), name(""), type(""), sector(""),
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
    double minLat, maxLat, minLon, maxLon;  // Store original geo bounds
    Point2D topLeft, bottomRight;            // Computed canvas positions
    Point2D center;
    bool isHovered;

    SectorRegion() : name(""), minLat(0), maxLat(0), minLon(0), maxLon(0),
        topLeft(), bottomRight(), center(), isHovered(false) {}

    bool contains(Point2D p) const {
        return p.x >= topLeft.x && p.x <= bottomRight.x &&
            p.y >= topLeft.y && p.y <= bottomRight.y;
    }
};

// Traffic vehicle for animation
struct TrafficVehicle {
    int edgeFromID, edgeToID;
    double progress;        // 0.0 to 1.0 along the edge
    double speed;           // Progress per frame
    Color color;
    
    TrafficVehicle() : edgeFromID(-1), edgeToID(-1), progress(0), speed(0.02), color(Color::Yellow) {}
};

// ============================================================================
// GRAPH RENDERING CONSTANTS
// ============================================================================
namespace GraphRenderConfig {
    // Edge (road) thickness - number of parallel lines to draw
    constexpr int ROAD_THICKNESS = 2;          // Normal roads
    constexpr int PATH_THICKNESS = 3;          // Dijkstra path roads (thicker)
    
    // Vertex (node) size - radius in canvas units
    constexpr int FACILITY_RADIUS = 3;         // Facilities like schools, hospitals
    constexpr int CORNER_RADIUS = 2;           // Sector corners/intersections
    constexpr int PATH_NODE_RADIUS = 4;        // Nodes on Dijkstra path (larger)
    constexpr int START_END_RADIUS = 5;        // Start/End nodes (largest)
    constexpr int HOUSE_RADIUS = 1;            // Houses (small dots)
    constexpr int TRAFFIC_RADIUS = 2;          // Traffic vehicles
}

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

    // Get canvas dimensions
    int getCanvasWidth() const { return canvasWidth; }
    int getCanvasHeight() const { return canvasHeight; }

    // Simple linear mapping - keeps natural tilt of geographic data
    // This is called at RENDER time, not build time
    Point2D geoToCanvas(double lat, double lon) const {
        // Map longitude to X (west to east = left to right)
        double normX = (lon - minLon) / (maxLon - minLon);
        // Map latitude to Y (north to south = top to bottom, so invert)
        double normY = (maxLat - lat) / (maxLat - minLat);

        // Apply zoom centered on view
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
    void panLeft() { offsetX -= 0.05 / zoom; }
    void panRight() { offsetX += 0.05 / zoom; }
    void panUp() { offsetY -= 0.05 / zoom; }
    void panDown() { offsetY += 0.05 / zoom; }
    void resetView() { zoom = 1.0; offsetX = 0; offsetY = 0; }

    double getZoom() const { return zoom; }
    double getOffsetX() const { return offsetX; }
    double getOffsetY() const { return offsetY; }
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
    std::vector<TrafficVehicle> trafficVehicles;  // Traffic simulation
    GraphViewport viewport;

    int mouseX, mouseY;
    int hoveredNodeID;
    string hoveredSector;

    bool showCorners;
    bool showRoads;
    bool showSectorBounds;
    bool showHouses;          // Toggle houses display
    bool showTraffic;         // Toggle traffic animation
    bool trafficPaused;       // Pause traffic

    // Dijkstra visualization state
    DijkstraMode dijkstraMode;
    int dijkstraStartNode;
    int dijkstraEndNode;
    string dijkstraTargetType;  // "SCHOOL", "HOSPITAL", "STOP", "SECTOR", "CUSTOM"
    Vector<int> dijkstraPath;
    double dijkstraDistance;
    int dijkstraNodeSelection;      // For selecting start node
    int dijkstraEndNodeSelection;   // For selecting end node (point-to-point)
    std::vector<int> selectableNodes;  // Nodes user can select from

    string stopsCSV, schoolsCSV, hospitalsCSV, pharmaciesCSV;
    string busesCSV, populationCSV, mallsCSV, shopsCSV, ambulancesCSV;

    // Counter for naming intersections
    int intersectionCounter;

    Color getNodeColor(const string& type) {
        if (type == "CORNER") return Color::White;  // Corners are white
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
        if (type == "HOUSE") return Color::GrayLight;
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
    void runDijkstraPointToPoint();  // New: point-to-point pathfinding
    void buildSelectableNodesList();

    // Traffic simulation
    void initializeTraffic();
    void updateTraffic();

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
    showCorners(true), showRoads(true), showSectorBounds(false),
    showHouses(false), showTraffic(false), trafficPaused(false),
    dijkstraMode(DijkstraMode::SELECT_START),
    dijkstraStartNode(-1), dijkstraEndNode(-1),
    dijkstraTargetType(""), dijkstraDistance(0.0),
    dijkstraNodeSelection(0), dijkstraEndNodeSelection(0),
    intersectionCounter(0) {

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

    // Build graph nodes - store GEO coordinates, not canvas positions
    for (int i = 0; i < graph->getNodeCount(); i++) {
        CityNode* node = graph->getNode(i);
        if (!node) continue;

        GraphNode2D gNode;
        gNode.id = node->id;
        gNode.lat = node->lat;      // Store original lat
        gNode.lon = node->lon;      // Store original lon
        gNode.pos = Point2D(0, 0);  // Will be computed at render time
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

    // Build sector regions - store GEO bounds, not canvas positions
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

    // Initialize traffic if enabled
    initializeTraffic();
}

// ============================================================================
// GRAPH RENDERING - Optimized and Centralized
// ============================================================================

inline Canvas CitySimulator::renderGraphToCanvas(int width, int height) {
    Canvas canvas(width * 2, height * 4);
    viewport.setCanvasSize(width * 2, height * 4);

    int cw = width * 2;
    int ch = height * 4;

    auto stylize = [](Color c) -> Canvas::Stylizer {
        return [c](Pixel& p) { p.foreground_color = c; };
    };

    // Helper function to draw a thick line (multiple parallel lines)
    auto drawThickLine = [&](int x1, int y1, int x2, int y2, int thickness, Color c) {
        double dx = x2 - x1;
        double dy = y2 - y1;
        double len = std::sqrt(dx * dx + dy * dy);
        if (len < 0.001) return;
        
        double perpX = -dy / len;
        double perpY = dx / len;
        
        int halfThick = thickness / 2;
        for (int t = -halfThick; t <= halfThick; t++) {
            int offsetX = (int)(perpX * t);
            int offsetY = (int)(perpY * t);
            canvas.DrawBlockLine(x1 + offsetX, y1 + offsetY, 
                                 x2 + offsetX, y2 + offsetY, stylize(c));
        }
    };

    // Helper function to draw a filled circle (for vertices)
    auto drawFilledCircle = [&](int cx, int cy, int radius, Color c) {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (dx * dx + dy * dy <= radius * radius) {
                    int px = cx + dx;
                    int py = cy + dy;
                    if (px >= 0 && px < cw && py >= 0 && py < ch) {
                        canvas.DrawBlock(px, py, true, stylize(c));
                    }
                }
            }
        }
    };

    // Compute canvas positions for all nodes (at render time, not build time)
    for (auto& node : graphNodes) {
        node.pos = viewport.geoToCanvas(node.lat, node.lon);
    }

    // Compute canvas positions for sector regions
    for (auto& region : sectorRegions) {
        Point2D tl = viewport.geoToCanvas(region.maxLat, region.minLon);
        Point2D br = viewport.geoToCanvas(region.minLat, region.maxLon);
        region.topLeft = Point2D(std::min(tl.x, br.x), std::min(tl.y, br.y));
        region.bottomRight = Point2D(std::max(tl.x, br.x), std::max(tl.y, br.y));
        region.center = Point2D((region.topLeft.x + region.bottomRight.x) / 2,
                                (region.topLeft.y + region.bottomRight.y) / 2);
    }

    // Draw sector boundaries if enabled
    if (showSectorBounds) {
        for (const auto& region : sectorRegions) {
            int x1 = (int)region.topLeft.x;
            int y1 = (int)region.topLeft.y;
            int x2 = (int)region.bottomRight.x;
            int y2 = (int)region.bottomRight.y;

            // Clamp to canvas bounds
            x1 = std::max(0, std::min(cw - 1, x1));
            y1 = std::max(0, std::min(ch - 1, y1));
            x2 = std::max(0, std::min(cw - 1, x2));
            y2 = std::max(0, std::min(ch - 1, y2));

            Color boundColor = region.isHovered ? Color::Yellow : Color::GrayDark;
            canvas.DrawBlockLine(x1, y1, x2, y1, stylize(boundColor));
            canvas.DrawBlockLine(x2, y1, x2, y2, stylize(boundColor));
            canvas.DrawBlockLine(x2, y2, x1, y2, stylize(boundColor));
            canvas.DrawBlockLine(x1, y2, x1, y1, stylize(boundColor));
        }
    }

    // Draw roads (edges) with proportional thickness
    if (showRoads) {
        // First pass: draw non-path roads
        for (const auto& edge : graphEdges) {
            if (edge.isOnPath) continue;
            
            int idx1 = (edge.fromID < (int)nodeIdToIndex.size()) ? nodeIdToIndex[edge.fromID] : -1;
            int idx2 = (edge.toID < (int)nodeIdToIndex.size()) ? nodeIdToIndex[edge.toID] : -1;
            if (idx1 >= 0 && idx2 >= 0 && idx1 < (int)graphNodes.size() && idx2 < (int)graphNodes.size()) {
                const GraphNode2D& n1 = graphNodes[idx1];
                const GraphNode2D& n2 = graphNodes[idx2];
                int x1 = (int)n1.pos.x, y1 = (int)n1.pos.y;
                int x2 = (int)n2.pos.x, y2 = (int)n2.pos.y;

                drawThickLine(x1, y1, x2, y2, GraphRenderConfig::ROAD_THICKNESS, Color::GrayDark);
            }
        }
        
        // Second pass: draw path roads on top (green, thicker)
        for (const auto& edge : graphEdges) {
            if (!edge.isOnPath) continue;
            
            int idx1 = (edge.fromID < (int)nodeIdToIndex.size()) ? nodeIdToIndex[edge.fromID] : -1;
            int idx2 = (edge.toID < (int)nodeIdToIndex.size()) ? nodeIdToIndex[edge.toID] : -1;
            if (idx1 >= 0 && idx2 >= 0 && idx1 < (int)graphNodes.size() && idx2 < (int)graphNodes.size()) {
                const GraphNode2D& n1 = graphNodes[idx1];
                const GraphNode2D& n2 = graphNodes[idx2];
                int x1 = (int)n1.pos.x, y1 = (int)n1.pos.y;
                int x2 = (int)n2.pos.x, y2 = (int)n2.pos.y;

                drawThickLine(x1, y1, x2, y2, GraphRenderConfig::PATH_THICKNESS, Color::Green);
            }
        }
    }

    // Draw traffic vehicles if enabled
    if (showTraffic && !trafficPaused) {
        for (const auto& vehicle : trafficVehicles) {
            int idx1 = (vehicle.edgeFromID < (int)nodeIdToIndex.size()) ? nodeIdToIndex[vehicle.edgeFromID] : -1;
            int idx2 = (vehicle.edgeToID < (int)nodeIdToIndex.size()) ? nodeIdToIndex[vehicle.edgeToID] : -1;
            if (idx1 >= 0 && idx2 >= 0) {
                const GraphNode2D& n1 = graphNodes[idx1];
                const GraphNode2D& n2 = graphNodes[idx2];
                
                // Interpolate position along edge
                int vx = (int)(n1.pos.x + (n2.pos.x - n1.pos.x) * vehicle.progress);
                int vy = (int)(n1.pos.y + (n2.pos.y - n1.pos.y) * vehicle.progress);
                
                if (vx >= 0 && vx < cw && vy >= 0 && vy < ch) {
                    drawFilledCircle(vx, vy, GraphRenderConfig::TRAFFIC_RADIUS, vehicle.color);
                }
            }
        }
    }

    // Draw houses if enabled
    if (showHouses) {
        for (const auto& node : graphNodes) {
            if (node.type != "HOUSE") continue;
            int x = (int)node.pos.x;
            int y = (int)node.pos.y;
            if (x >= 0 && x < cw && y >= 0 && y < ch) {
                drawFilledCircle(x, y, GraphRenderConfig::HOUSE_RADIUS, Color::GrayLight);
            }
        }
    }

    // Draw corners (intersections) - white colored with fixed size
    if (showCorners) {
        for (const auto& node : graphNodes) {
            if (!node.isCorner) continue;
            int x = (int)node.pos.x;
            int y = (int)node.pos.y;
            if (x >= 0 && x < cw && y >= 0 && y < ch) {
                Color c = Color::White;
                int radius = GraphRenderConfig::CORNER_RADIUS;
                
                if (node.isOnPath) {
                    c = Color::GreenLight;
                    radius = GraphRenderConfig::PATH_NODE_RADIUS;
                }
                
                drawFilledCircle(x, y, radius, c);
            }
        }
    }

    // Draw facilities - fixed width with color coding
    for (const auto& node : graphNodes) {
        if (node.isCorner || node.type == "HOUSE") continue;
        int x = (int)node.pos.x;
        int y = (int)node.pos.y;
        if (x < 0 || x >= cw || y < 0 || y >= ch) continue;

        Color nodeColor = node.color;
        int radius = GraphRenderConfig::FACILITY_RADIUS;

        // Override colors and sizes for Dijkstra visualization
        if (node.isStart) {
            nodeColor = Color::Cyan;
            radius = GraphRenderConfig::START_END_RADIUS;
        }
        else if (node.isEnd) {
            nodeColor = Color::Yellow;
            radius = GraphRenderConfig::START_END_RADIUS;
        }
        else if (node.isOnPath) {
            nodeColor = Color::GreenLight;
            radius = GraphRenderConfig::PATH_NODE_RADIUS;
        }
        else if (node.isVisited) {
            nodeColor = Color::Orange1;
        }

        // Hovered node highlight
        if (node.id == hoveredNodeID) {
            nodeColor = Color::White;
            radius = GraphRenderConfig::PATH_NODE_RADIUS;
        }

        drawFilledCircle(x, y, radius, nodeColor);
    }

    return canvas;
}

inline void CitySimulator::updateHoverState(int mx, int my) {
    mouseX = mx;
    mouseY = my;
    hoveredNodeID = -1;
    hoveredSector = "";

    // Convert mouse position to canvas coordinates
    int canvasX = (mx - 1) * 2;
    int canvasY = (my - 2) * 4;

    double minDist = 25.0;  // Increased hover distance
    for (const auto& node : graphNodes) {
        if (node.isCorner && !showCorners) continue;
        if (node.type == "HOUSE" && !showHouses) continue;
        
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
            ss << node.name.substr(0, 18) << "\n";
            ss << "Type: " << node.type << "\n";
            ss << "Sector: " << node.sector;
            return ss.str();
        }
    }

    if (!hoveredSector.empty()) {
        ss << "SECTOR:\n" << hoveredSector;
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

inline void CitySimulator::runDijkstraPointToPoint() {
    if (!islamabad || !islamabad->getCityGraph()) return;
    if (dijkstraStartNode < 0 || dijkstraEndNode < 0) return;

    CityGraph* graph = islamabad->getCityGraph();

    // Run Dijkstra between two specific points
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
// TRAFFIC SIMULATION
// ============================================================================

inline void CitySimulator::initializeTraffic() {
    trafficVehicles.clear();
    
    if (graphEdges.empty()) return;
    
    // Create some random traffic vehicles on edges
    int numVehicles = std::min(20, (int)graphEdges.size() / 3);
    
    for (int i = 0; i < numVehicles; i++) {
        TrafficVehicle vehicle;
        int edgeIdx = rand() % graphEdges.size();
        vehicle.edgeFromID = graphEdges[edgeIdx].fromID;
        vehicle.edgeToID = graphEdges[edgeIdx].toID;
        vehicle.progress = (rand() % 100) / 100.0;
        vehicle.speed = 0.01 + (rand() % 30) / 1000.0;  // Variable speeds
        
        // Different vehicle colors
        int colorChoice = rand() % 4;
        if (colorChoice == 0) vehicle.color = Color::Yellow;
        else if (colorChoice == 1) vehicle.color = Color::Orange1;
        else if (colorChoice == 2) vehicle.color = Color::Cyan;
        else vehicle.color = Color::RedLight;
        
        trafficVehicles.push_back(vehicle);
    }
}

inline void CitySimulator::updateTraffic() {
    if (trafficPaused || trafficVehicles.empty()) return;
    
    for (auto& vehicle : trafficVehicles) {
        vehicle.progress += vehicle.speed;
        
        // When vehicle reaches end, pick a new random edge
        if (vehicle.progress >= 1.0) {
            vehicle.progress = 0.0;
            
            // Try to find a connected edge for smooth traffic flow
            int currentEndID = vehicle.edgeToID;
            std::vector<int> connectedEdges;
            
            for (int i = 0; i < (int)graphEdges.size(); i++) {
                if (graphEdges[i].fromID == currentEndID || graphEdges[i].toID == currentEndID) {
                    connectedEdges.push_back(i);
                }
            }
            
            if (!connectedEdges.empty()) {
                int nextEdgeIdx = connectedEdges[rand() % connectedEdges.size()];
                if (graphEdges[nextEdgeIdx].fromID == currentEndID) {
                    vehicle.edgeFromID = graphEdges[nextEdgeIdx].fromID;
                    vehicle.edgeToID = graphEdges[nextEdgeIdx].toID;
                } else {
                    vehicle.edgeFromID = graphEdges[nextEdgeIdx].toID;
                    vehicle.edgeToID = graphEdges[nextEdgeIdx].fromID;
                }
            } else if (!graphEdges.empty()) {
                // Pick random edge if no connection found
                int edgeIdx = rand() % graphEdges.size();
                vehicle.edgeFromID = graphEdges[edgeIdx].fromID;
                vehicle.edgeToID = graphEdges[edgeIdx].toID;
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
    buildGraphVisualization();  // Build once at start

    // Traffic update thread
    std::atomic<bool> running{ true };
    std::thread trafficThread([&]() {
        while (running.load()) {
            if (showTraffic && !trafficPaused) {
                updateTraffic();
                screen.PostEvent(Event::Custom);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });

    auto renderer = Renderer([&] {
        int termW = Terminal::Size().dimx;
        int termH = Terminal::Size().dimy;
        int canvasW = termW - 25;
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
            hbox({text("■") | color(Color::White), text(" Corner")}),
            separator(),
            text("INFO") | bold | color(Color::Yellow),
            vbox(hoverLines),
            separator(),
            text("KEYS") | bold | color(Color::Cyan),
            text("+/-: Zoom"),
            text("Arrows: Pan"),
            text("0: Reset view"),
            text("R: Roads " + string(showRoads ? "[ON]" : "[off]")),
            text("C: Corners " + string(showCorners ? "[ON]" : "[off]")),
            text("S: Sectors " + string(showSectorBounds ? "[ON]" : "[off]")),
            text("H: Houses " + string(showHouses ? "[ON]" : "[off]")),
            text("T: Traffic " + string(showTraffic ? "[ON]" : "[off]")),
            text("D: Dijkstra"),
            separator(),
            text("Esc: Menu"),
            }) | border | size(WIDTH, EQUAL, 22);

        return vbox({
            text("ISLAMABAD MAP") | bold | center | color(Color::Green),
            hbox({ canvas(c) | border | flex, text(" "), infoPanel }) | flex,
            });
        });

    auto comp = CatchEvent(renderer, [&](Event e) {
        // Zoom - NO rebuild needed, positions computed at render time
        if (e == Event::Character('+') || e == Event::Character('=') || e == Event::Character('i')) { 
            viewport.zoomIn(); 
            return true; 
        }
        if (e == Event::Character('-') || e == Event::Character('o')) { 
            viewport.zoomOut(); 
            return true; 
        }
        // Pan - NO rebuild needed
        if (e == Event::ArrowLeft) { viewport.panLeft(); return true; }
        if (e == Event::ArrowRight) { viewport.panRight(); return true; }
        if (e == Event::ArrowUp) { viewport.panUp(); return true; }
        if (e == Event::ArrowDown) { viewport.panDown(); return true; }
        // Reset view
        if (e == Event::Character('0')) { viewport.resetView(); return true; }
        // Toggle options
        if (e == Event::Character('r') || e == Event::Character('R')) { showRoads = !showRoads; return true; }
        if (e == Event::Character('c') || e == Event::Character('C')) { showCorners = !showCorners; return true; }
        if (e == Event::Character('s') || e == Event::Character('S')) { showSectorBounds = !showSectorBounds; return true; }
        if (e == Event::Character('h') || e == Event::Character('H')) { showHouses = !showHouses; return true; }
        if (e == Event::Character('t') || e == Event::Character('T')) { showTraffic = !showTraffic; return true; }
        if (e == Event::Character('p') || e == Event::Character('P')) { trafficPaused = !trafficPaused; return true; }
        if (e == Event::Character('d') || e == Event::Character('D')) { 
            running.store(false);
            currentState = SimulatorState::DIJKSTRA_VIEW; 
            screen.Exit(); 
            return true; 
        }
        if (e.is_mouse()) { updateHoverState(e.mouse().x, e.mouse().y); return true; }
        if (e == Event::Escape) { 
            running.store(false);
            currentState = SimulatorState::MAIN_MENU; 
            screen.Exit(); 
            return true; 
        }
        return false;
        });
    screen.Loop(comp);
    
    running.store(false);
    if (trafficThread.joinable()) trafficThread.join();
}

inline void CitySimulator::runDijkstraView() {
    auto screen = ScreenInteractive::Fullscreen();
    if (graphNodes.empty()) buildGraphVisualization();
    clearDijkstraVisualization();
    buildSelectableNodesList();

    dijkstraMode = DijkstraMode::SELECT_START;
    dijkstraNodeSelection = 0;
    dijkstraEndNodeSelection = 0;

    std::vector<string> targetTypes = { 
        "Nearest School", 
        "Nearest Hospital", 
        "Nearest Pharmacy", 
        "Nearest Bus Stop",
        ">>> Custom Location <<<" // New option for point-to-point
    };
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
            controlItems.push_back(text("SELECT START POINT") | bold | color(Color::Yellow));
            controlItems.push_back(separator());

            int startIdx = std::max(0, dijkstraNodeSelection - 5);
            int endIdx = std::min((int)selectableNodes.size(), startIdx + 10);

            for (int i = startIdx; i < endIdx; i++) {
                int nodeId = selectableNodes[i];
                int idx = nodeIdToIndex[nodeId];
                if (idx >= 0 && idx < (int)graphNodes.size()) {
                    string nodeName = graphNodes[idx].name;
                    string nodeType = graphNodes[idx].type;
                    if (nodeName.length() > 16) nodeName = nodeName.substr(0, 13) + "...";
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
            controlItems.push_back(text("SELECT DESTINATION") | bold | color(Color::Yellow));
            controlItems.push_back(separator());

            if (dijkstraStartNode >= 0 && dijkstraStartNode < (int)nodeIdToIndex.size()) {
                int idx = nodeIdToIndex[dijkstraStartNode];
                if (idx >= 0) {
                    controlItems.push_back(text("From: ") | bold);
                    controlItems.push_back(text(" " + graphNodes[idx].name.substr(0, 18)) | color(Color::Cyan));
                }
            }
            controlItems.push_back(separator());

            for (int i = 0; i < (int)targetTypes.size(); i++) {
                auto item = text((i == targetSel ? "> " : "  ") + targetTypes[i]);
                if (i == targetSel) item = item | bold | color(Color::Green);
                if (i == 4) item = item | color(Color::Yellow); // Highlight custom option
                controlItems.push_back(item);
            }
            controlItems.push_back(separator());
            controlItems.push_back(text("Up/Down: Select") | dim);
            controlItems.push_back(text("Enter: Continue") | dim);
        }
        else if (dijkstraMode == DijkstraMode::RUNNING) {
            // This mode is for selecting custom end point
            controlItems.push_back(text("SELECT END POINT") | bold | color(Color::Yellow));
            controlItems.push_back(separator());

            if (dijkstraStartNode >= 0 && dijkstraStartNode < (int)nodeIdToIndex.size()) {
                int idx = nodeIdToIndex[dijkstraStartNode];
                if (idx >= 0) {
                    controlItems.push_back(text("From: " + graphNodes[idx].name.substr(0, 15)) | color(Color::Cyan));
                }
            }
            controlItems.push_back(separator());

            int startIdx = std::max(0, dijkstraEndNodeSelection - 5);
            int endIdx = std::min((int)selectableNodes.size(), startIdx + 10);

            for (int i = startIdx; i < endIdx; i++) {
                int nodeId = selectableNodes[i];
                int idx = nodeIdToIndex[nodeId];
                if (idx >= 0 && idx < (int)graphNodes.size()) {
                    string nodeName = graphNodes[idx].name;
                    if (nodeName.length() > 16) nodeName = nodeName.substr(0, 13) + "...";
                    auto item = text((i == dijkstraEndNodeSelection ? "> " : "  ") + nodeName);
                    if (i == dijkstraEndNodeSelection) item = item | bold | color(Color::Green);
                    controlItems.push_back(item);
                }
            }
            controlItems.push_back(separator());
            controlItems.push_back(text("Up/Down: Select") | dim);
            controlItems.push_back(text("Enter: Find Path") | dim);
        }
        else if (dijkstraMode == DijkstraMode::COMPLETE) {
            if (dijkstraPath.getSize() > 0) {
                controlItems.push_back(text("PATH FOUND!") | bold | color(Color::Green));
            } else {
                controlItems.push_back(text("NO PATH FOUND") | bold | color(Color::Red));
            }
            controlItems.push_back(separator());

            if (dijkstraStartNode >= 0 && dijkstraStartNode < (int)nodeIdToIndex.size()) {
                int idx = nodeIdToIndex[dijkstraStartNode];
                if (idx >= 0) {
                    controlItems.push_back(text("From:") | bold);
                    controlItems.push_back(text(" " + graphNodes[idx].name.substr(0, 18)));
                }
            }
            if (dijkstraEndNode >= 0 && dijkstraEndNode < (int)nodeIdToIndex.size()) {
                int idx = nodeIdToIndex[dijkstraEndNode];
                if (idx >= 0) {
                    controlItems.push_back(text("To:") | bold);
                    controlItems.push_back(text(" " + graphNodes[idx].name.substr(0, 18)));
                }
            }
            controlItems.push_back(separator());

            if (dijkstraPath.getSize() > 0) {
                std::stringstream distStr;
                distStr << std::fixed << std::setprecision(2) << dijkstraDistance;
                controlItems.push_back(text("Distance:") | bold);
                controlItems.push_back(text(" " + distStr.str() + " km") | color(Color::Yellow));
                controlItems.push_back(text("Nodes:") | bold);
                controlItems.push_back(text(" " + std::to_string(dijkstraPath.getSize())) | color(Color::Yellow));
            }
            controlItems.push_back(separator());
            controlItems.push_back(text("R: New Search") | dim);
            controlItems.push_back(text("+/-: Zoom") | dim);
            controlItems.push_back(text("Esc: Back") | dim);
        }

        // Legend
        controlItems.push_back(separator());
        controlItems.push_back(text("LEGEND") | bold);
        controlItems.push_back(hbox({ text("●") | color(Color::Cyan), text(" Start") }));
        controlItems.push_back(hbox({ text("●") | color(Color::Yellow), text(" End") }));
        controlItems.push_back(hbox({ text("●") | color(Color::GreenLight), text(" Path") }));
        controlItems.push_back(hbox({ text("━") | color(Color::Green), text(" Route") }));

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
                if (targetSel == 4) {
                    // Custom location - go to end point selection
                    dijkstraTargetType = "CUSTOM";
                    dijkstraMode = DijkstraMode::RUNNING;
                } else {
                    // Nearest facility
                    if (targetSel == 0) dijkstraTargetType = "SCHOOL";
                    else if (targetSel == 1) dijkstraTargetType = "HOSPITAL";
                    else if (targetSel == 2) dijkstraTargetType = "PHARMACY";
                    else dijkstraTargetType = "STOP";

                    runDijkstraAlgorithm();
                    dijkstraMode = DijkstraMode::COMPLETE;
                }
                return true;
            }
        }
        else if (dijkstraMode == DijkstraMode::RUNNING) {
            // Custom end point selection
            if (e == Event::ArrowUp && dijkstraEndNodeSelection > 0) {
                dijkstraEndNodeSelection--;
                return true;
            }
            if (e == Event::ArrowDown && dijkstraEndNodeSelection < (int)selectableNodes.size() - 1) {
                dijkstraEndNodeSelection++;
                return true;
            }
            if (e == Event::Return && !selectableNodes.empty()) {
                dijkstraEndNode = selectableNodes[dijkstraEndNodeSelection];
                runDijkstraPointToPoint();
                dijkstraMode = DijkstraMode::COMPLETE;
                return true;
            }
        }
        else if (dijkstraMode == DijkstraMode::COMPLETE) {
            if (e == Event::Character('r') || e == Event::Character('R')) {
                clearDijkstraVisualization();
                dijkstraMode = DijkstraMode::SELECT_START;
                dijkstraNodeSelection = 0;
                dijkstraEndNodeSelection = 0;
                return true;
            }
        }

        // Common controls - zoom without rebuild
        if (e == Event::Character('+') || e == Event::Character('=')) { viewport.zoomIn(); return true; }
        if (e == Event::Character('-')) { viewport.zoomOut(); return true; }
        if (e == Event::Character('0')) { viewport.resetView(); return true; }
        if (e == Event::ArrowLeft && dijkstraMode == DijkstraMode::COMPLETE) { viewport.panLeft(); return true; }
        if (e == Event::ArrowRight && dijkstraMode == DijkstraMode::COMPLETE) { viewport.panRight(); return true; }
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
    string name_val;
    int type_selected = 0;
    std::vector<string> types = { "Pharmacy", "School", "Hospital", "Bus Stop" };
    string message = "";

    Component input_name = Input(&name_val, "Enter Name");
    Component toggle_type = Toggle(&types, &type_selected);

    Component btn_add = Button("Create Facility", [&] {
        if (name_val.empty()) { message = "Error: Name cannot be empty!"; return; }
        string newID = "";
        string type = types[type_selected];
        if (type == "Pharmacy") newID = cityMgmt->addPharmacy(name_val, sector);
        else if (type == "School") newID = cityMgmt->addSchool(name_val, sector, 3.0, {}, {});
        else if (type == "Hospital") newID = cityMgmt->addHospital(name_val, sector, 50, {});
        else if (type == "Bus Stop") {
            int id = cityMgmt->addBusStopInSector(name_val, sector);
            if (id != -1) newID = "STOP-" + std::to_string(id);
        }
        if (!newID.empty() || type == "Bus Stop") { screen.Exit(); }
        else { message = "Error: Creation failed."; }
    });

    Component btn_cancel = Button("Cancel", screen.ExitLoopClosure());
    auto component = Container::Vertical({ input_name, toggle_type, btn_add, btn_cancel });

    auto renderer = Renderer(component, [&] {
        return vbox({
            text("ADD NEW FACILITY") | bold | center | color(Color::Green),
            separator(),
            text("Sector: " + sector) | center,
            hbox({text("Name: "), input_name->Render() | border}),
            hbox({text("Type: "), toggle_type->Render() | border}),
            hbox({btn_add->Render(), text("  "), btn_cancel->Render()}) | center,
            text(message) | color(Color::Red) | center
        }) | border | size(WIDTH, EQUAL, 60) | center;
    });
    screen.Loop(renderer);
}

// ============================================================================
// ADD OFFERING FORM
// ============================================================================
inline void CitySimulator::runAddOfferingForm(CityNode* node) {
    if (!node) return;
    auto screen = ScreenInteractive::Fullscreen();
    string message = "";

    if (node->type == "PHARMACY") {
        string med_name, med_formula, med_price_str;
        Component input_name = Input(&med_name, "Medicine Name");
        Component input_formula = Input(&med_formula, "Formula");
        Component input_price = Input(&med_price_str, "Price");
        Component btn_add = Button("Add Medicine", [&] {
            try {
                float price = std::stof(med_price_str);
                if (cityMgmt->addMedicineToPharmacy(node->databaseID, med_name, med_formula, price)) screen.Exit();
                else message = "Error: Could not add medicine.";
            } catch (...) { message = "Error: Invalid Price."; }
        });
        auto component = Container::Vertical({ input_name, input_formula, input_price, btn_add, Button("Cancel", screen.ExitLoopClosure()) });
        auto renderer = Renderer(component, [&] {
            return vbox({
                text("ADD MEDICINE TO " + node->name) | bold | center | color(Color::Magenta),
                separator(),
                hbox({text("Name: "), input_name->Render() | border}),
                hbox({text("Formula: "), input_formula->Render() | border}),
                hbox({text("Price: "), input_price->Render() | border}),
                btn_add->Render() | center,
                text(message) | color(Color::Red) | center
            }) | border | size(WIDTH, EQUAL, 50) | center;
        });
        screen.Loop(renderer);
    } else {
        auto renderer = Renderer([&] {
            return vbox({ text("No offerings for " + node->type) | center, text("Press Enter to go back") | dim | center }) | border | center;
        });
        auto comp = CatchEvent(renderer, [&](Event e) { if (e == Event::Return) screen.Exit(); return true; });
        screen.Loop(comp);
    }
}

// ============================================================================
// DATABASE VIEW
// ============================================================================
inline void CitySimulator::runDatabaseView() {
    auto screen = ScreenInteractive::Fullscreen();
    int selectedSectorIdx = 0, selectedCategoryIdx = 0, selectedItemIdx = 0, focusPanel = 0;
    std::vector<string> categories = { "All", "Stops", "Schools", "Hospitals", "Pharmacies", "Malls" };
    std::vector<string> sectorList;
    for (int i = 0; i < SECTOR_COUNT; i++) sectorList.push_back(SECTOR_GRID[i].name);

    auto renderer = Renderer([&] {
        string currentSector = sectorList[selectedSectorIdx];
        string currentCategory = categories[selectedCategoryIdx];

        // Sector panel
        Elements sectorItems;
        sectorItems.push_back(text("SECTORS") | bold | color(Color::Cyan));
        sectorItems.push_back(separator());
        for (int i = 0; i < (int)sectorList.size() && i < 15; i++) {
            int idx = std::max(0, selectedSectorIdx - 7) + i;
            if (idx >= (int)sectorList.size()) break;
            auto item = text((idx == selectedSectorIdx ? "> " : "  ") + sectorList[idx]);
            if (idx == selectedSectorIdx) item = item | bold | (focusPanel == 0 ? bgcolor(Color::Blue) : color(Color::Green));
            sectorItems.push_back(item);
        }
        auto sectorPanel = vbox(sectorItems) | border | size(WIDTH, EQUAL, 14);

        // Get filtered nodes
        std::vector<CityNode*> filteredNodes;
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

        // Item panel
        Elements itemList;
        itemList.push_back(text("ITEMS (" + std::to_string(filteredNodes.size()) + ")") | bold | color(Color::Yellow));
        itemList.push_back(separator());
        int totalItems = filteredNodes.size() + 1;
        if (selectedItemIdx >= totalItems) selectedItemIdx = totalItems - 1;
        for (int i = 0; i < std::min(12, totalItems); i++) {
            int idx = std::max(0, selectedItemIdx - 6) + i;
            if (idx >= totalItems) break;
            Element item;
            if (idx < (int)filteredNodes.size()) {
                item = text(string(idx == selectedItemIdx ? "> " : "  ") + filteredNodes[idx]->name.substr(0, 20));
            } else {
                item = text(string(idx == selectedItemIdx ? "> " : "  ") + string("[+] Add Facility")) | color(Color::Yellow);
            }
            if (idx == selectedItemIdx) item = item | bold | (focusPanel == 2 ? bgcolor(Color::Blue) : color(Color::Green));
            itemList.push_back(item);
        }
        auto itemPanel = vbox(itemList) | border | size(WIDTH, EQUAL, 28);

        // Detail panel
        Elements detailItems;
        detailItems.push_back(text("DETAILS") | bold | color(Color::Magenta));
        detailItems.push_back(separator());
        if (selectedItemIdx < (int)filteredNodes.size()) {
            CityNode* n = filteredNodes[selectedItemIdx];
            detailItems.push_back(text("Name: " + n->name));
            detailItems.push_back(text("Type: " + n->type));
            detailItems.push_back(text("Sector: " + n->sector));
        } else {
            detailItems.push_back(text("Press Enter to add"));
        }
        auto detailPanel = vbox(detailItems) | border | flex;

        // Category tabs
        Elements categoryTabs;
        for (int i = 0; i < (int)categories.size(); i++) {
            auto tab = text(" " + categories[i] + " ");
            if (i == selectedCategoryIdx) tab = tab | bold | bgcolor(Color::Green);
            categoryTabs.push_back(tab);
        }

        return vbox({
            text(" DATABASE VIEW ") | bold | center | bgcolor(Color::Green) | color(Color::Black),
            hbox(categoryTabs) | center,
            separator(),
            hbox({sectorPanel, itemPanel, detailPanel}) | flex,
            separator(),
            text("Arrows: Navigate | Tab: Category | S: Search | Esc: Back") | dim | center
        });
    });

    auto comp = CatchEvent(renderer, [&](Event e) {
        if (e == Event::ArrowUp) {
            if (focusPanel == 0 && selectedSectorIdx > 0) selectedSectorIdx--;
            else if (focusPanel == 2 && selectedItemIdx > 0) selectedItemIdx--;
            return true;
        }
        if (e == Event::ArrowDown) {
            if (focusPanel == 0 && selectedSectorIdx < (int)sectorList.size() - 1) selectedSectorIdx++;
            else if (focusPanel == 2) selectedItemIdx++;
            return true;
        }
        if (e == Event::ArrowLeft) { focusPanel = std::max(0, focusPanel - 2); return true; }
        if (e == Event::ArrowRight) { focusPanel = std::min(2, focusPanel + 2); return true; }
        if (e == Event::Tab) { selectedCategoryIdx = (selectedCategoryIdx + 1) % categories.size(); selectedItemIdx = 0; return true; }
        if (e == Event::Character('s') || e == Event::Character('S')) { currentState = SimulatorState::SEARCH_VIEW; screen.Exit(); return true; }
        if (e == Event::Escape) { currentState = SimulatorState::MAIN_MENU; screen.Exit(); return true; }
        return false;
    });
    screen.Loop(comp);
}

// ============================================================================
// SEARCH VIEW
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
                if (cityMgmt->admitPatient(c->cnic, hospital->id, sev, res[0])) message = "Admitted " + c->name;
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