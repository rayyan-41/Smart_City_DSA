#ifndef CITY_SIMULATOR_ENHANCED_H
#define CITY_SIMULATOR_ENHANCED_H

#include <string>
#include <thread>
#include <chrono>
#include <iostream>
#include <functional>
#include <fstream>
#include <atomic>
#include <sstream>
#include <cmath>
#include <algorithm> 
#include <iomanip>

#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/component/event.hpp"

#include "../../SmartCity.h"
#include "../../data_structures/Vector.h"
#include "CityManagement.h"
#include "../../termgl/Termgl.h"

using namespace ftxui;
using std::string;

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

    GraphNode2D() : id(-1), lat(0), lon(0), pos(), name(""), type(""), sector(""),
        color(termgl::Color::White()), isCorner(false), isOnPath(false),
        isVisited(false), isStart(false), isEnd(false) {
    }
};

struct GraphEdge2D {
    int fromID, toID;
    bool isOnPath;

    GraphEdge2D(int f = -1, int t = -1) : fromID(f), toID(t), isOnPath(false) {}
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

    TrafficVehicle() : edgeFromID(-1), edgeToID(-1), progress(0), speed(0.02), color(termgl::Color::Yellow()) {}
};


namespace GraphRenderConfig {
    constexpr int ROAD_THICKNESS = 2;
    constexpr int PATH_THICKNESS = 4;

    constexpr int FACILITY_RADIUS = 5;
    constexpr int CORNER_RADIUS = 3;
    constexpr int PATH_NODE_RADIUS = 6;
    constexpr int START_END_RADIUS = 8;
    constexpr int HOUSE_RADIUS = 2;
    constexpr int TRAFFIC_RADIUS = 4;
}


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
        double normX = (lon - minLon) / (maxLon - minLon);
        double normY = (maxLat - lat) / (maxLat - minLat);

        normX = (normX - 0.5) * zoom + 0.5 + offsetX;
        normY = (normY - 0.5) * zoom + 0.5 + offsetY;

        double pad = 0.05;
        double canvasX = pad * canvasWidth + normX * canvasWidth * (1.0 - 2 * pad);
        double canvasY = pad * canvasHeight + normY * canvasHeight * (1.0 - 2 * pad);

        return Point2D(canvasX, canvasY);
    }

    void zoomIn() { zoom *= 1.1; if (zoom > 10.0) zoom = 10.0; }
    void zoomOut() { zoom /= 1.1; if (zoom < 0.1) zoom = 0.1; }
    void panLeft() { offsetX -= 0.05 / zoom; }
    void panRight() { offsetX += 0.05 / zoom; }
    void panUp() { offsetY -= 0.05 / zoom; }
    void panDown() { offsetY += 0.05 / zoom; }
    void resetView() { zoom = 1.0; offsetX = 0; offsetY = 0; }

    double getZoom() const { return zoom; }
    double getOffsetX() const { return offsetX; }
    double getOffsetY() const { return offsetY; }
};



enum class SimulatorState {
    INTRO_PHASE_1, INTRO_PHASE_2, INTRO_PHASE_3,
    WELCOME_ANIMATION, MAIN_MENU, CSV_SELECTION, LOADING,
    GRAPH_VIEW, DATABASE_VIEW, MANAGEMENT_MENU,
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



class CitySimulator {
private:
    SmartCity* islamabad;
    CityManagement* cityMgmt;
    SimulatorState currentState;
    CSVLoadMode loadMode;
    bool cityInitialized;

    Vector<GraphNode2D> graphNodes;
    Vector<GraphEdge2D> graphEdges;
    Vector<SectorRegion> sectorRegions;
    Vector<int> nodeIdToIndex;
    Vector<TrafficVehicle> trafficVehicles;
    GraphViewport viewport;

    int mouseX, mouseY;
    int hoveredNodeID;
    string hoveredSector;

    bool showCorners;
    bool showRoads;
    bool showSectorBounds;
    bool showHouses;
    bool showTraffic;
    bool trafficPaused;

    DijkstraMode dijkstraMode;
    int dijkstraStartNode;
    int dijkstraEndNode;
    string dijkstraTargetType;
    Vector<int> dijkstraPath;
    double dijkstraDistance;
    int dijkstraNodeSelection;
    int dijkstraEndNodeSelection;
    Vector<int> selectableNodes;

    string stopsCSV, schoolsCSV, hospitalsCSV, pharmaciesCSV;
    string busesCSV, populationCSV, mallsCSV, shopsCSV, ambulancesCSV;

    int intersectionCounter;

    termgl::Color getNodeColor(const string& type) {
        if (type == "CORNER") return termgl::Color::Grey();
        if (type == "STOP") return termgl::Color::Green();
        if (type == "SCHOOL") return termgl::Color::Blue();
        if (type == "HOSPITAL") return termgl::Color::Red();
        if (type == "PHARMACY") return termgl::Color(255, 0, 255);
        if (type == "MALL") return termgl::Color::Yellow();
        if (type == "MOSQUE") return termgl::Color::Cyan();
        if (type == "PARK") return termgl::Color(0, 100, 0);
        if (type == "POLICE_STATION") return termgl::Color(100, 0, 0);
        if (type == "FIRE_STATION") return termgl::Color(255, 100, 0);
        if (type == "LIBRARY") return termgl::Color(0, 0, 100);
        if (type == "ATM") return termgl::Color(200, 200, 0);
        if (type == "RESTAURANT") return termgl::Color(255, 165, 0);
        if (type == "HOUSE") return termgl::Color(100, 100, 100);
        return termgl::Color::White();
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
    void runEditObjectView(const string& objectID, const string& objectType);

    void runAddFacilityForm(const string& sector);
    void runAddOfferingForm(CityNode* node);

    void runInputForm(const string& title, const std::vector<string>& labels, std::function<void(std::vector<string>)> onConfirm);
    void runManagementAddForm(const string& category);
    Citizen* runPopulationSelector(const string& title);
    void runEditSchoolView(School* school);
    void runEditHospitalView(Hospital* hospital);
    void runEditPharmacyView(Pharmacy* pharmacy);
    void runEditMallView(Mall* mall);
    void runEditShopView(Shop* shop, Mall* mall);


    void renderGraph(termgl::Window& window);
    void updateHoverState(int mx, int my);
    string getHoverInfo();

    void clearDijkstraVisualization();
    void runDijkstraAlgorithm();
    void runDijkstraPointToPoint();
    void buildSelectableNodesList();

    void initializeTraffic();
    void updateTraffic();

    void sleepMs(int ms);
    bool fileExists(const string& path);
};


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
            gNode.name = "Intersection " + std::to_string(intersectionCounter);
        }
        else {
            gNode.name = node->name;
        }

        nodeIdToIndex[node->id] = (int)graphNodes.getSize();
        graphNodes.push_back(gNode);
    }

    for (int i = 0; i < graph->getNodeCount(); i++) {
        CityNode* node = graph->getNode(i);
        if (!node) continue;

        const LinkedList<Edge>& roads = node->getRoads();
        for (int j = 0; j < roads.size(); j++) {
            Edge edge = roads.at(j);
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
        region.minLat = SECTOR_GRID[i].minLat;
        region.maxLat = SECTOR_GRID[i].maxLat;
        region.minLon = SECTOR_GRID[i].minLon;
        region.maxLon = SECTOR_GRID[i].maxLon;
        region.isHovered = false;
        sectorRegions.push_back(region);
    }

    initializeTraffic();
}

inline void CitySimulator::renderGraph(termgl::Window& window) {
    int cw = window.getWidth();
    int ch = window.getHeight();
    viewport.setCanvasSize(cw, ch);

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

    if (showSectorBounds) {
        for (int i = 0; i < sectorRegions.getSize(); i++) {
            const SectorRegion& region = sectorRegions[i];
            int x1 = (int)region.topLeft.x;
            int y1 = (int)region.topLeft.y;
            int w = (int)(region.bottomRight.x - region.topLeft.x);
            int h = (int)(region.bottomRight.y - region.topLeft.y);

            termgl::Color boundColor = region.isHovered ? termgl::Color::Yellow() : termgl::Color::Grey();
            if (dijkstraPath.getSize() > 0) boundColor = termgl::Color::Grey();

            window.drawRect(x1, y1, w, h, boundColor);
            if (region.isHovered) {
                window.drawText(x1 + 5, y1 + 5, region.name, termgl::Color::Yellow());
            }
        }
    }

    if (showRoads) {
        for (int i = 0; i < graphEdges.getSize(); i++) {
            const GraphEdge2D& edge = graphEdges[i];
            if (edge.isOnPath) continue;

            if (dijkstraPath.getSize() > 0 && dijkstraMode == DijkstraMode::COMPLETE) continue;

            int idx1 = (edge.fromID < nodeIdToIndex.getSize()) ? nodeIdToIndex[edge.fromID] : -1;
            int idx2 = (edge.toID < nodeIdToIndex.getSize()) ? nodeIdToIndex[edge.toID] : -1;
            if (idx1 >= 0 && idx2 >= 0 && idx1 < graphNodes.getSize() && idx2 < graphNodes.getSize()) {
                const GraphNode2D& n1 = graphNodes[idx1];
                const GraphNode2D& n2 = graphNodes[idx2];
                int x1 = (int)n1.pos.x, y1 = (int)n1.pos.y;
                int x2 = (int)n2.pos.x, y2 = (int)n2.pos.y;

                window.drawLine(x1, y1, x2, y2, termgl::Color(50, 50, 60));
            }
        }

        for (int i = 0; i < graphEdges.getSize(); i++) {
            const GraphEdge2D& edge = graphEdges[i];
            if (!edge.isOnPath) continue;

            int idx1 = (edge.fromID < nodeIdToIndex.getSize()) ? nodeIdToIndex[edge.fromID] : -1;
            int idx2 = (edge.toID < nodeIdToIndex.getSize()) ? nodeIdToIndex[edge.toID] : -1;
            if (idx1 >= 0 && idx2 >= 0 && idx1 < graphNodes.getSize() && idx2 < graphNodes.getSize()) {
                const GraphNode2D& n1 = graphNodes[idx1];
                const GraphNode2D& n2 = graphNodes[idx2];
                int x1 = (int)n1.pos.x, y1 = (int)n1.pos.y;
                int x2 = (int)n2.pos.x, y2 = (int)n2.pos.y;

                window.drawLine(x1, y1, x2, y2, termgl::Color::Green());
                window.drawLine(x1 + 1, y1 + 1, x2 + 1, y2 + 1, termgl::Color::Green());
            }
        }
    }

    if (showTraffic && !trafficPaused && dijkstraPath.getSize() == 0) {
        for (int i = 0; i < trafficVehicles.getSize(); i++) {
            const TrafficVehicle& vehicle = trafficVehicles[i];
            int idx1 = (vehicle.edgeFromID < nodeIdToIndex.getSize()) ? nodeIdToIndex[vehicle.edgeFromID] : -1;
            int idx2 = (vehicle.edgeToID < nodeIdToIndex.getSize()) ? nodeIdToIndex[vehicle.edgeToID] : -1;
            if (idx1 >= 0 && idx2 >= 0) {
                const GraphNode2D& n1 = graphNodes[idx1];
                const GraphNode2D& n2 = graphNodes[idx2];

                int vx = (int)(n1.pos.x + (n2.pos.x - n1.pos.x) * vehicle.progress);
                int vy = (int)(n1.pos.y + (n2.pos.y - n1.pos.y) * vehicle.progress);

                window.fillCircle(vx, vy, 3, vehicle.color);
            }
        }
    }

    if (showHouses && dijkstraPath.getSize() == 0) {
        for (int i = 0; i < graphNodes.getSize(); i++) {
            const GraphNode2D& node = graphNodes[i];
            if (node.type != "HOUSE") continue;
            int x = (int)node.pos.x;
            int y = (int)node.pos.y;
            window.drawPixel(x, y, termgl::Color(80, 80, 80));
        }
    }

    if (showCorners) {
        for (int i = 0; i < graphNodes.getSize(); i++) {
            const GraphNode2D& node = graphNodes[i];
            if (!node.isCorner) continue;

            if (dijkstraPath.getSize() > 0 && !node.isOnPath) continue;

            int x = (int)node.pos.x;
            int y = (int)node.pos.y;
            termgl::Color c = termgl::Color::Grey();
            if (node.isOnPath) c = termgl::Color::Green();
            else if (dijkstraPath.getSize() > 0) continue;

            window.fillCircle(x, y, 2, c);
        }
    }

    for (int i = 0; i < graphNodes.getSize(); i++) {
        const GraphNode2D& node = graphNodes[i];
        if (node.isCorner || node.type == "HOUSE") continue;

        if (dijkstraPath.getSize() > 0 && !node.isOnPath && !node.isStart && !node.isEnd) continue;

        int x = (int)node.pos.x;
        int y = (int)node.pos.y;

        termgl::Color nodeColor = node.color;

        int radius = 4;
        if (node.isStart) { nodeColor = termgl::Color::Cyan(); radius = 7; }
        else if (node.isEnd) { nodeColor = termgl::Color::Yellow(); radius = 7; }
        else if (node.isOnPath) { nodeColor = termgl::Color::Green(); radius = 5; }
        else if (node.isVisited && dijkstraPath.getSize() == 0) nodeColor = termgl::Color(255, 165, 0);

        if (node.id == hoveredNodeID) {
            nodeColor = termgl::Color::White();
            radius += 2;
            window.drawText(x + 10, y - 10, node.name, termgl::Color::White());
        }

        window.fillCircle(x, y, radius, nodeColor);
    }
}

inline void CitySimulator::updateHoverState(int mx, int my) {
    mouseX = mx;
    mouseY = my;
    hoveredNodeID = -1;
    hoveredSector = "";

    double minDist = 15.0;
    for (int i = 0; i < graphNodes.getSize(); i++) {
        const GraphNode2D& node = graphNodes[i];
        if (node.isCorner && !showCorners) continue;
        if (node.type == "HOUSE" && !showHouses) continue;

        double dx = node.pos.x - mx;
        double dy = node.pos.y - my;
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

inline string CitySimulator::getHoverInfo() {
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



inline void CitySimulator::clearDijkstraVisualization() {
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

inline void CitySimulator::buildSelectableNodesList() {
    selectableNodes.clear();
    for (int i = 0; i < graphNodes.getSize(); i++) {
        if (!graphNodes[i].isCorner) {
            selectableNodes.push_back(graphNodes[i].id);
        }
    }
}

inline void CitySimulator::runDijkstraAlgorithm() {
    if (!islamabad || !islamabad->getCityGraph()) return;
    if (dijkstraStartNode < 0) return;

    CityGraph* graph = islamabad->getCityGraph();

    if (dijkstraTargetType == "SCHOOL" || dijkstraTargetType == "HOSPITAL" ||
        dijkstraTargetType == "PHARMACY" || dijkstraTargetType == "STOP") {
        dijkstraEndNode = graph->findNearestFacility(dijkstraStartNode, dijkstraTargetType);
    }

    if (dijkstraEndNode < 0) return;

    dijkstraPath = graph->findShortestPath(dijkstraStartNode, dijkstraEndNode, dijkstraDistance);

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

inline void CitySimulator::runDijkstraPointToPoint() {
    if (!islamabad || !islamabad->getCityGraph()) return;
    if (dijkstraStartNode < 0 || dijkstraEndNode < 0) return;

    CityGraph* graph = islamabad->getCityGraph();

    dijkstraPath = graph->findShortestPath(dijkstraStartNode, dijkstraEndNode, dijkstraDistance);

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


inline void CitySimulator::initializeTraffic() {
    trafficVehicles.clear();

    if (graphEdges.empty()) return;

    int numVehicles = std::min(50, (int)graphEdges.getSize() / 2);

    for (int i = 0; i < numVehicles; i++) {
        TrafficVehicle vehicle;
        int edgeIdx = rand() % graphEdges.getSize();
        vehicle.edgeFromID = graphEdges[edgeIdx].fromID;
        vehicle.edgeToID = graphEdges[edgeIdx].toID;
        vehicle.progress = (rand() % 100) / 100.0;
        vehicle.speed = 0.005 + (rand() % 20) / 1000.0;

        int colorChoice = rand() % 4;
        if (colorChoice == 0) vehicle.color = termgl::Color::Yellow();
        else if (colorChoice == 1) vehicle.color = termgl::Color(255, 100, 0);
        else if (colorChoice == 2) vehicle.color = termgl::Color::Cyan();
        else vehicle.color = termgl::Color::Red();

        trafficVehicles.push_back(vehicle);
    }
}

inline void CitySimulator::updateTraffic() {
    if (trafficPaused || trafficVehicles.empty()) return;

    for (int i = 0; i < trafficVehicles.getSize(); i++) {
        TrafficVehicle& vehicle = trafficVehicles[i];
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
            else if (!graphEdges.empty()) {
                int edgeIdx = rand() % graphEdges.getSize();
                vehicle.edgeFromID = graphEdges[edgeIdx].fromID;
                vehicle.edgeToID = graphEdges[edgeIdx].toID;
            }
        }
    }
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
        case SimulatorState::SEARCH_VIEW: runSearchView(); break;
        case SimulatorState::EXIT: break;
        }
    }
    std::cout << "\nThank you for using Islamabad Redefined!\n" << std::endl;
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
            lines.push_back(text(ASCIIArt::REDEFINED_TITLE[i]) | color(ftxui::Color::GrayLight));
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


inline void CitySimulator::runMainMenu() {
    auto screen = ScreenInteractive::Fullscreen();
    std::vector<string> options;

    if (cityInitialized) {
        options = { "Graph View [1]", "Database [2]", "Search Engine [S]", "Management [3]", "Exit" };
    }
    else {
        options = { "Initialize City", "Exit" };
    }
    int sel = 0;

    auto renderer = Renderer([&] {
        Elements items;
        for (int i = 0; i < (int)options.size(); i++) {
            auto item = text((i == sel ? " > " : "   ") + options[i]);
            if (i == sel) item = item | bold | color(ftxui::Color::Green);
            items.push_back(item);
        }

        string statusText = cityInitialized ? "CITY LOADED" : "NOT INITIALIZED";
        ftxui::Color statusColor = cityInitialized ? ftxui::Color::Green : ftxui::Color::Yellow;

        Elements titleArt;
        for (int i = 0; i < 6; i++) {
            titleArt.push_back(text(ASCIIArt::ISLAMABAD_TITLE[i]) | color(ftxui::Color::Green));
        }

        auto menuBox = vbox({
            text("MAIN MENU") | bold | center | color(ftxui::Color::Cyan),
            separator(),
            text(statusText) | center | color(statusColor),
            separator(),
            vbox(items),
            }) | border | size(WIDTH, EQUAL, 35);

        return vbox({
            filler(),
            vbox(titleArt) | center,
            text("R E D E F I N E D") | bold | center | color(ftxui::Color::GrayLight),
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
                else if (sel == 1) currentState = SimulatorState::DATABASE_VIEW;
                else if (sel == 2) currentState = SimulatorState::SEARCH_VIEW;
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
            if (e == Event::Character('s') || e == Event::Character('S')) {
                currentState = SimulatorState::SEARCH_VIEW; screen.Exit(); return true;
            }
        }
        if (e == Event::Escape) { currentState = SimulatorState::EXIT; screen.Exit(); return true; }
        return false;
        });
    screen.Loop(comp);
}



inline void CitySimulator::runCSVSelection() {
    auto screen = ScreenInteractive::Fullscreen();
    int sel = 0;

    auto renderer = Renderer([&] {
        Elements csvList;
        csvList.push_back(text("DATASET FILES") | bold | color(ftxui::Color::Cyan));
        csvList.push_back(separator());

        auto fileRow = [&](const string& label, const string& path) {
            bool exists = fileExists(path);
            ftxui::Color statusColor = exists ? ftxui::Color::Green : ftxui::Color::Red;
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
            if (i == sel) item = item | bold | color(ftxui::Color::Green);
            csvList.push_back(item);
        }

        auto box = vbox(csvList) | border | size(WIDTH, EQUAL, 58);
        return vbox({ filler(), text("CITY INITIALIZATION") | bold | center | color(ftxui::Color::Green),
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
    std::atomic<int> totalSteps{ 14 };

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

        string stageDesc = (s < (int)loadingStages.size()) ? loadingStages[s] : "Completing...";

        return vbox({
            filler(),
            text("LOADING ISLAMABAD") | bold | center | color(ftxui::Color::Green),
            text(""),
            hbox({text("["), text(progressBar) | color(ftxui::Color::Green), text("]")}) | center,
            text(std::to_string((s * 100) / totalSteps.load()) + "%") | center,
            text(""),
            text(stageDesc) | center | color(ftxui::Color::Cyan),
            text(""),
            text(done.load() ? "Press Enter to continue" : "Please wait...") | center | dim,
            filler()
            });
        });

    std::thread t([&]() {
        islamabad = new SmartCity();
        islamabad->setDatasetPaths(stopsCSV, schoolsCSV, hospitalsCSV, pharmaciesCSV,
            busesCSV, populationCSV, mallsCSV, shopsCSV, ambulancesCSV);
        step.store(1); screen.PostEvent(Event::Custom); sleepMs(80);

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

inline void CitySimulator::runGraphView() {
    buildGraphVisualization();
    // Prepare data structures for Dijkstra selection
    buildSelectableNodesList();

    termgl::Window window(1600, 900, "Islamabad City Simulator - Interactive Map", true);
    window.setFramerateLimit(60);

    int width = window.getWidth();
    int height = window.getHeight();

    // Partition Setup:
    // Left (70%): Map
    // Right (30%): Side Panel (Info / Legend / Controls)
    int mapPartition = window.addPartition(0, 0, (int)(width * 0.75), height, "City Map");
    int sidePartition = window.addPartition((int)(width * 0.75), 0, (int)(width * 0.25), height, "Control Panel");

    bool running = true;
    bool inDijkstraMode = false;

    // Dijkstra State Variables
    std::vector<string> targetTypes = {
        "Nearest School",
        "Nearest Hospital",
        "Nearest Pharmacy",
        "Nearest Bus Stop",
        "Custom Location"
    };
    int targetSel = 0;
    int scrollOffset = 0;

    // Initialize Viewport Size for Hover Logic
    viewport.setCanvasSize(window.getWidth() * 0.75, window.getHeight());

    while (running && window.processEvents()) {
        // --- GLOBAL INPUTS ---
        if (window.isKeyPressed(VK_ESCAPE)) {
            if (inDijkstraMode) {
                // Exit Dijkstra Mode
                inDijkstraMode = false;
                clearDijkstraVisualization();
                dijkstraMode = DijkstraMode::SELECT_START;
                dijkstraNodeSelection = 0;
                dijkstraEndNodeSelection = 0;
                targetSel = 0;
                scrollOffset = 0;
            }
            else {
                // Exit Graph View
                running = false;
                currentState = SimulatorState::MAIN_MENU;
            }
        }

        if (window.isKeyPressed('D') && !inDijkstraMode) {
            inDijkstraMode = true;
            dijkstraMode = DijkstraMode::SELECT_START;
            dijkstraNodeSelection = 0;
            scrollOffset = 0;
            clearDijkstraVisualization();
        }

        // --- NAVIGATION INPUTS (Always Active) ---
        if (window.isKeyDown(VK_UP) && (!inDijkstraMode || dijkstraMode == DijkstraMode::COMPLETE)) viewport.panUp();
        if (window.isKeyDown(VK_DOWN) && (!inDijkstraMode || dijkstraMode == DijkstraMode::COMPLETE)) viewport.panDown();
        if (window.isKeyDown(VK_LEFT)) viewport.panLeft();
        if (window.isKeyDown(VK_RIGHT)) viewport.panRight();
        if (window.isKeyDown('W')) viewport.panUp();
        if (window.isKeyDown('S')) viewport.panDown();
        if (window.isKeyDown('A')) viewport.panLeft();
        if (window.isKeyDown('D') && window.isKeyDown(VK_CONTROL)) viewport.panRight(); // Avoid conflict with 'D' toggle, require Ctrl+D or just use Arrow Right
        if (window.isKeyPressed(VK_ADD) || window.isKeyPressed('=')) viewport.zoomIn();
        if (window.isKeyPressed(VK_SUBTRACT) || window.isKeyPressed('-')) viewport.zoomOut();

        // --- VIEW TOGGLES ---
        if (window.isKeyPressed('R') && !inDijkstraMode) showRoads = !showRoads;
        if (window.isKeyPressed('C')) showCorners = !showCorners;
        if (window.isKeyPressed('S')) showSectorBounds = !showSectorBounds;
        if (window.isKeyPressed('H')) showHouses = !showHouses;
        if (window.isKeyPressed('T')) showTraffic = !showTraffic;
        if (window.isKeyPressed('P')) trafficPaused = !trafficPaused;

        // --- DIJKSTRA LOGIC INPUTS ---
        if (inDijkstraMode) {
            if (dijkstraMode == DijkstraMode::SELECT_START) {
                if (window.isKeyPressed(VK_UP) && dijkstraNodeSelection > 0) { dijkstraNodeSelection--; scrollOffset = dijkstraNodeSelection * 20; }
                if (window.isKeyPressed(VK_DOWN) && dijkstraNodeSelection < selectableNodes.getSize() - 1) { dijkstraNodeSelection++; scrollOffset = dijkstraNodeSelection * 20; }
                if (window.isKeyPressed(VK_RETURN) && !selectableNodes.empty()) {
                    dijkstraStartNode = selectableNodes[dijkstraNodeSelection];
                    int idx = nodeIdToIndex[dijkstraStartNode];
                    if (idx >= 0) graphNodes[idx].isStart = true;
                    dijkstraMode = DijkstraMode::SELECT_TARGET_TYPE;
                }
            }
            else if (dijkstraMode == DijkstraMode::SELECT_TARGET_TYPE) {
                if (window.isKeyPressed(VK_UP) && targetSel > 0) targetSel--;
                if (window.isKeyPressed(VK_DOWN) && targetSel < targetTypes.size() - 1) targetSel++;
                if (window.isKeyPressed(VK_RETURN)) {
                    if (targetSel == 4) {
                        dijkstraTargetType = "CUSTOM";
                        dijkstraMode = DijkstraMode::RUNNING;
                        dijkstraEndNodeSelection = 0;
                        scrollOffset = 0;
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
            else if (dijkstraMode == DijkstraMode::RUNNING) {
                if (window.isKeyPressed(VK_UP) && dijkstraEndNodeSelection > 0) { dijkstraEndNodeSelection--; scrollOffset = dijkstraEndNodeSelection * 20; }
                if (window.isKeyPressed(VK_DOWN) && dijkstraEndNodeSelection < selectableNodes.getSize() - 1) { dijkstraEndNodeSelection++; scrollOffset = dijkstraEndNodeSelection * 20; }
                if (window.isKeyPressed(VK_RETURN) && !selectableNodes.empty()) {
                    dijkstraEndNode = selectableNodes[dijkstraEndNodeSelection];
                    runDijkstraPointToPoint();
                    dijkstraMode = DijkstraMode::COMPLETE;
                }
            }
            else if (dijkstraMode == DijkstraMode::COMPLETE) {
                if (window.isKeyPressed('R')) {
                    clearDijkstraVisualization();
                    dijkstraMode = DijkstraMode::SELECT_START;
                    dijkstraNodeSelection = 0;
                    targetSel = 0;
                }
            }
        }

        // --- UPDATE LOGIC ---
        if (showTraffic && !trafficPaused) updateTraffic();

        // --- RENDER PASS ---
        window.setActivePartition(-1);
        window.clear(termgl::Color(10, 10, 15));
        window.drawPartitionFrames();

        // 1. Render Map
        window.setActivePartition(mapPartition);
        window.clear(termgl::Color(20, 20, 30));

        // Critical: Update viewport size for drawing AND hover logic
        // renderGraph uses window.getWidth()/getHeight() which respects the active partition
        renderGraph(window);

        // 2. Update Hover (Post-Render to ensure positions are fresh)
        // Mouse pos relative to active partition (Map)
        termgl::Vec2 mousePos = window.getMousePos();
        updateHoverState(mousePos.x, mousePos.y);

        // 3. Render Side Panel
        window.setActivePartition(sidePartition);
        window.clear(termgl::Color(30, 30, 40));

        int cy = 10; // cursor y

        if (!inDijkstraMode) {
            // === NORMAL MODE SIDEBAR ===
            window.drawText(10, cy, "INFO PANEL", termgl::Color::Cyan()); cy += 30;

            // Hover Info
            window.drawRect(5, cy, window.getWidth() - 10, 120, termgl::Color(60, 60, 70));
            window.drawText(10, cy + 10, "SELECTION:", termgl::Color::Yellow());
            string info = getHoverInfo();
            window.drawText(10, cy + 30, info, termgl::Color::White());
            cy += 140;

            // Legend
            window.drawText(10, cy, "LEGEND:", termgl::Color::Cyan()); cy += 25;
            auto drawLegendItem = [&](termgl::Color c, string label) {
                window.fillCircle(20, cy + 5, 4, c);
                window.drawText(40, cy, label, termgl::Color::White());
                cy += 20;
                };
            drawLegendItem(termgl::Color::Green(), "Stop");
            drawLegendItem(termgl::Color::Blue(), "School");
            drawLegendItem(termgl::Color::Red(), "Hospital");
            drawLegendItem(termgl::Color(255, 0, 255), "Pharmacy");
            drawLegendItem(termgl::Color::Yellow(), "Mall");
            cy += 10;

            // Controls
            window.drawText(10, cy, "CONTROLS:", termgl::Color::Cyan()); cy += 25;
            window.drawText(10, cy, "Arrows/WASD: Pan", termgl::Color::Grey()); cy += 20;
            window.drawText(10, cy, "+/-: Zoom", termgl::Color::Grey()); cy += 20;
            window.drawText(10, cy, "R: Toggle Roads", termgl::Color::Grey()); cy += 20;
            window.drawText(10, cy, "T: Toggle Traffic", termgl::Color::Grey()); cy += 20;
            window.drawText(10, cy, "D: Dijkstra Mode", termgl::Color::Green()); cy += 20;
            window.drawText(10, cy, "Esc: Menu", termgl::Color::Grey());
        }
        else {
            // === DIJKSTRA MODE SIDEBAR ===
            window.drawText(10, cy, "PATHFINDING", termgl::Color::Green()); cy += 30;

            if (dijkstraMode == DijkstraMode::SELECT_START) {
                window.drawText(10, cy, "SELECT START:", termgl::Color::Yellow()); cy += 30;
                std::vector<string> items;
                for (int i = 0; i < selectableNodes.getSize(); i++) {
                    int idx = nodeIdToIndex[selectableNodes[i]];
                    items.push_back(graphNodes[idx].name);
                }
                // List drawing logic
                int listH = window.getHeight() - cy - 50;
                int dummyScroll = dijkstraNodeSelection * 20 - (listH / 2); // Center selection
                int clicked = window.drawList(10, cy, window.getWidth() - 20, listH, items, dummyScroll);

                // Highlight selection manually since we control via keyboard
                window.drawRect(12, cy + (dijkstraNodeSelection * 20) - dummyScroll, window.getWidth() - 24, 20, termgl::Color::Green());

                // Mouse Click Support in List
                if (clicked != -1) {
                    dijkstraNodeSelection = clicked;
                    dijkstraStartNode = selectableNodes[dijkstraNodeSelection];
                    int idx = nodeIdToIndex[dijkstraStartNode];
                    if (idx >= 0) graphNodes[idx].isStart = true;
                    dijkstraMode = DijkstraMode::SELECT_TARGET_TYPE;
                }
            }
            else if (dijkstraMode == DijkstraMode::SELECT_TARGET_TYPE) {
                window.drawText(10, cy, "DESTINATION:", termgl::Color::Yellow()); cy += 30;
                for (size_t i = 0; i < targetTypes.size(); i++) {
                    termgl::Color c = (i == targetSel) ? termgl::Color::Green() : termgl::Color::White();
                    string prefix = (i == targetSel) ? "> " : "  ";
                    window.drawText(10, cy, prefix + targetTypes[i], c);
                    cy += 25;
                }
            }
            else if (dijkstraMode == DijkstraMode::RUNNING) {
                window.drawText(10, cy, "SELECT END:", termgl::Color::Yellow()); cy += 30;
                std::vector<string> items;
                for (int i = 0; i < selectableNodes.getSize(); i++) {
                    int idx = nodeIdToIndex[selectableNodes[i]];
                    items.push_back(graphNodes[idx].name);
                }
                int listH = window.getHeight() - cy - 50;
                int dummyScroll = dijkstraEndNodeSelection * 20 - (listH / 2);
                int clicked = window.drawList(10, cy, window.getWidth() - 20, listH, items, dummyScroll);

                window.drawRect(12, cy + (dijkstraEndNodeSelection * 20) - dummyScroll, window.getWidth() - 24, 20, termgl::Color::Green());

                if (clicked != -1) {
                    dijkstraEndNodeSelection = clicked;
                    dijkstraEndNode = selectableNodes[dijkstraEndNodeSelection];
                    runDijkstraPointToPoint();
                    dijkstraMode = DijkstraMode::COMPLETE;
                }
            }
            else if (dijkstraMode == DijkstraMode::COMPLETE) {
                if (dijkstraPath.getSize() > 0) {
                    window.drawText(10, cy, "RESULT: SUCCESS", termgl::Color::Green()); cy += 30;
                    window.drawText(10, cy, "Dist: " + std::to_string(dijkstraDistance).substr(0, 5) + " km", termgl::Color::White()); cy += 30;
                    window.drawText(10, cy, "Stops: " + std::to_string(dijkstraPath.getSize()), termgl::Color::White()); cy += 30;
                }
                else {
                    window.drawText(10, cy, "RESULT: FAILED", termgl::Color::Red()); cy += 30;
                    window.drawText(10, cy, "No path found.", termgl::Color::Grey()); cy += 30;
                }
                window.drawText(10, cy, "[R] New Search", termgl::Color::Yellow()); cy += 20;
                window.drawText(10, cy, "[Esc] Exit Mode", termgl::Color::Grey());
            }
        }

        window.display();
    }
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
            text("ADD NEW FACILITY") | bold | center | color(ftxui::Color::Green),
            separator(),
            text("Sector: " + sector) | center,
            hbox({text("Name: "), input_name->Render() | border}),
            hbox({text("Type: "), toggle_type->Render() | border}),
            hbox({btn_add->Render(), text("  "), btn_cancel->Render()}) | center,
            text(message) | color(ftxui::Color::Red) | center
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
            }
            catch (...) { message = "Error: Invalid Price."; }
            });
        auto component = Container::Vertical({ input_name, input_formula, input_price, btn_add, Button("Cancel", screen.ExitLoopClosure()) });
        auto renderer = Renderer(component, [&] {
            return vbox({
                text("ADD MEDICINE TO " + node->name) | bold | center | color(ftxui::Color::Magenta),
                separator(),
                hbox({text("Name: "), input_name->Render() | border}),
                hbox({text("Formula: "), input_formula->Render() | border}),
                hbox({text("Price: "), input_price->Render() | border}),
                btn_add->Render() | center,
                text(message) | color(ftxui::Color::Red) | center
                }) | border | size(WIDTH, EQUAL, 50) | center;
            });
        screen.Loop(renderer);
    }
    else {
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
    int selectedSectorIdx = 0;
    int selectedCategoryIdx = 0;
    int selectedItemIdx = 0;
    int focusPanel = 0; // 0=Sector List, 2=Item List

    std::vector<string> categories = { "All", "Stops", "Schools", "Hospitals", "Pharmacies", "Malls" };
    std::vector<string> sectorList;
    for (int i = 0; i < SECTOR_COUNT; i++) sectorList.push_back(SECTOR_GRID[i].name);

    // Unified item structure to aggregate from all managers
    struct DBItem {
        string id;
        string name;
        string type;
        string sector;
        void* objPtr; // Pointer to the actual object (School*, Mall*, etc.)
    };

    std::vector<DBItem> items;

    // Helper to refresh the items list based on selection
    auto refreshItems = [&]() {
        items.clear();
        string currentSector = sectorList[selectedSectorIdx];
        string cat = categories[selectedCategoryIdx];

        // 1. STOPS (From Graph)
        if ((cat == "All" || cat == "Stops") && islamabad->getCityGraph()) {
            CityGraph* g = islamabad->getCityGraph();
            for (int i = 0; i < g->getNodeCount(); i++) {
                CityNode* n = g->getNode(i);
                if (n && n->type == "STOP" && n->sector == currentSector) {
                    items.push_back({ n->databaseID, n->name, "STOP", n->sector, n });
                }
            }
        }

        // 2. SCHOOLS (From SchoolManager)
        if ((cat == "All" || cat == "Schools") && islamabad->getSchoolManager()) {
            auto& schools = islamabad->getSchoolManager()->schools;
            for (int i = 0; i < schools.getSize(); i++) {
                if (schools[i]->getSector() == currentSector) {
                    items.push_back({ schools[i]->id, schools[i]->name, "SCHOOL", schools[i]->getSector(), schools[i] });
                }
            }
        }

        // 3. HOSPITALS (From MedicalManager)
        if ((cat == "All" || cat == "Hospitals") && islamabad->getMedicalManager()) {
            auto& hospitals = islamabad->getMedicalManager()->hospitals;
            for (int i = 0; i < hospitals.getSize(); i++) {
                if (hospitals[i]->sector == currentSector) {
                    items.push_back({ hospitals[i]->id, hospitals[i]->name, "HOSPITAL", hospitals[i]->sector, hospitals[i] });
                }
            }
        }

        // 4. PHARMACIES (From MedicalManager)
        if ((cat == "All" || cat == "Pharmacies") && islamabad->getMedicalManager()) {
            auto& pharmacies = islamabad->getMedicalManager()->pharmacies;
            for (int i = 0; i < pharmacies.getSize(); i++) {
                if (pharmacies[i]->sector == currentSector) {
                    items.push_back({ pharmacies[i]->id, pharmacies[i]->name, "PHARMACY", pharmacies[i]->sector, pharmacies[i] });
                }
            }
        }

        // 5. MALLS (From CommercialManager)
        if ((cat == "All" || cat == "Malls") && islamabad->getCommercialManager()) {
            auto& malls = islamabad->getCommercialManager()->malls;
            for (int i = 0; i < malls.getSize(); i++) {
                if (malls[i]->getSector() == currentSector) {
                    items.push_back({ malls[i]->id, malls[i]->name, "MALL", malls[i]->getSector(), malls[i] });
                }
            }
        }
        };

    // Initial load
    refreshItems();

    auto renderer = Renderer([&] {
        // --- SECTOR PANEL ---
        Elements sectorItems;
        sectorItems.push_back(text("SECTORS") | bold | color(ftxui::Color::Cyan));
        sectorItems.push_back(separator());
        for (int i = 0; i < (int)sectorList.size() && i < 18; i++) {
            int idx = std::max(0, selectedSectorIdx - 8) + i;
            if (idx >= (int)sectorList.size()) break;
            auto item = text((idx == selectedSectorIdx ? "> " : "  ") + sectorList[idx]);
            if (idx == selectedSectorIdx) item = item | bold | (focusPanel == 0 ? bgcolor(ftxui::Color::Blue) : color(ftxui::Color::Green));
            sectorItems.push_back(item);
        }
        auto sectorPanel = vbox(sectorItems) | border | size(WIDTH, EQUAL, 14);


        // --- ITEM LIST PANEL ---
        Elements itemList;
        itemList.push_back(text("FACILITIES (" + std::to_string(items.size()) + ")") | bold | color(ftxui::Color::Yellow));
        itemList.push_back(separator());

        int totalListItems = items.size() + 1; // +1 for "Add Facility"
        if (selectedItemIdx >= totalListItems) selectedItemIdx = std::max(0, totalListItems - 1);

        int startIdx = std::max(0, selectedItemIdx - 6);
        int endIdx = std::min(totalListItems, startIdx + 14);

        for (int i = startIdx; i < endIdx; i++) {
            bool isSelected = (i == selectedItemIdx);
            string prefix = isSelected ? "> " : "  ";
            Element e;

            if (i < (int)items.size()) {
                string icon = "●";
                if (items[i].type == "STOP") icon = "◎";
                else if (items[i].type == "SCHOOL") icon = "◆";
                else if (items[i].type == "HOSPITAL") icon = "✚";
                else if (items[i].type == "PHARMACY") icon = "⚕";
                else if (items[i].type == "MALL") icon = "◈";

                string displayName = items[i].name.length() > 20 ? items[i].name.substr(0, 17) + "..." : items[i].name;
                e = text(prefix + icon + " " + displayName);
            }
            else {
                e = text(prefix + "[+] Add Facility") | color(ftxui::Color::Yellow);
            }

            if (isSelected) e = e | bold | (focusPanel == 2 ? bgcolor(ftxui::Color::Blue) : color(ftxui::Color::Green));
            itemList.push_back(e);
        }
        auto itemPanel = vbox(itemList) | border | size(WIDTH, EQUAL, 35);


        // --- DETAIL PANEL ---
        Elements details;
        details.push_back(text("DETAILS") | bold | color(ftxui::Color::Magenta));
        details.push_back(separator());

        if (selectedItemIdx < (int)items.size()) {
            DBItem& item = items[selectedItemIdx];
            details.push_back(hbox({ text("ID:   ") | bold, text(item.id) | color(ftxui::Color::Yellow) }));
            details.push_back(hbox({ text("Name: ") | bold, text(item.name) }));
            details.push_back(hbox({ text("Type: ") | bold, text(item.type) | color(ftxui::Color::Cyan) }));
            details.push_back(separator());

            if (item.type == "SCHOOL") {
                School* s = (School*)item.objPtr;
                details.push_back(hbox({ text("Rating: "), text(std::to_string(s->rating).substr(0, 3) + "/5.0") | color(ftxui::Color::Yellow) }));
                details.push_back(hbox({ text("Students: "), text(std::to_string(s->getTotalEnrolledStudents())) | color(ftxui::Color::Green) }));
                details.push_back(hbox({ text("Faculty: "), text(std::to_string(s->getTotalFaculty())) | color(ftxui::Color::Green) }));
                details.push_back(separator());
                details.push_back(text("Departments:") | dim);
                for (int k = 0; k < std::min(5, s->departments.getSize()); k++) {
                    details.push_back(text(" - " + s->departments[k]->name));
                }
                if (s->departments.getSize() > 5) details.push_back(text(" ... +" + std::to_string(s->departments.getSize() - 5) + " more") | dim);

            }
            else if (item.type == "MALL") {
                Mall* m = (Mall*)item.objPtr;
                details.push_back(hbox({ text("Total Shops: "), text(std::to_string(m->getShopCount())) | color(ftxui::Color::Green) }));
                details.push_back(hbox({ text("Products: "), text(std::to_string(m->getTotalProductCount())) }));
                details.push_back(separator());
                details.push_back(text("Shops:") | dim);
                for (int k = 0; k < std::min(6, m->shops.getSize()); k++) {
                    Shop* shop = m->shops[k];
                    details.push_back(text(" - " + shop->name + " (" + shop->category + ")"));
                }
                if (m->shops.getSize() > 6) details.push_back(text(" ... +" + std::to_string(m->shops.getSize() - 6) + " more") | dim);
            }
            else if (item.type == "HOSPITAL") {
                Hospital* h = (Hospital*)item.objPtr;
                details.push_back(hbox({ text("Beds: "), text(std::to_string(h->getAvailableBeds()) + "/" + std::to_string(h->totalBeds)) | color(ftxui::Color::Red) }));
                details.push_back(hbox({ text("Patients: "), text(std::to_string(h->getOccupiedBeds())) }));
                details.push_back(separator());
                details.push_back(text("Specializations:") | dim);
                for (int k = 0; k < std::min(5, h->specializations.getSize()); k++) {
                    details.push_back(text(" - " + h->specializations[k]));
                }
            }
            else if (item.type == "PHARMACY") {
                Pharmacy* p = (Pharmacy*)item.objPtr;
                details.push_back(hbox({ text("Meds Count: "), text(std::to_string(p->getMedicineCount())) }));
                if (p->getMedicineCount() > 0) {
                    details.push_back(separator());
                    details.push_back(text("Sample Inventory:") | dim);
                    for (int k = 0; k < std::min(4, p->getMedicineCount()); k++) {
                        const Medicine* m = p->getMedicine(k);
                        details.push_back(text(" - " + m->name + " (" + std::to_string((int)m->price) + " Rs)"));
                    }
                }
            }
            else if (item.type == "STOP") {
                CityNode* n = (CityNode*)item.objPtr;
                if (islamabad->getTransportManager()) {
                    int w = islamabad->getTransportManager()->getWaitingCount(n->id);
                    details.push_back(hbox({ text("Waiting: "), text(std::to_string(w) + " passengers") | color(ftxui::Color::Yellow) }));
                }
            }

            // POPULATION SUMMARY
            details.push_back(separator());
            details.push_back(text("SECTOR RESIDENTS") | bold);
            Vector<Citizen*> residents = islamabad->getPopulationManager()->getCitizensInSector(sectorList[selectedSectorIdx]);
            details.push_back(text("Total Residents: " + std::to_string(residents.getSize())));
        }
        else {
            // "Add Facility" Selected
            details.push_back(text("Create New Facility") | center);
            details.push_back(text("in " + sectorList[selectedSectorIdx]) | bold | center | color(ftxui::Color::Green));
            details.push_back(text(""));
            details.push_back(text("Press Enter to open") | dim | center);
            details.push_back(text("creation wizard.") | dim | center);
        }

        auto detailPanel = vbox(details) | border | flex;

        // --- TABS ---
        Elements tabs;
        for (int i = 0; i < (int)categories.size(); i++) {
            auto tab = text(" " + categories[i] + " ");
            if (i == selectedCategoryIdx) tab = tab | bold | bgcolor(ftxui::Color::Green) | color(ftxui::Color::Black);
            else tab = tab | color(ftxui::Color::GrayLight);
            tabs.push_back(tab);
        }

        auto helpBar = hbox({
             text("Arrows: Navigate "),
             text("Tab: Switch Category "),
             text("Enter: Select/Edit "),
             text("Esc: Back")
            }) | center | dim;

        return vbox({
            text(" DATABASE VIEW ") | bold | center | bgcolor(ftxui::Color::Green) | color(ftxui::Color::Black),
            hbox(tabs) | center,
            separator(),
            hbox({sectorPanel, itemPanel, detailPanel}) | flex,
            separator(),
            helpBar
            });
        });

    auto comp = CatchEvent(renderer, [&](Event e) {
        if (e == Event::ArrowUp) {
            if (focusPanel == 0 && selectedSectorIdx > 0) { selectedSectorIdx--; refreshItems(); selectedItemIdx = 0; }
            else if (focusPanel == 2 && selectedItemIdx > 0) selectedItemIdx--;
            return true;
        }
        if (e == Event::ArrowDown) {
            if (focusPanel == 0 && selectedSectorIdx < (int)sectorList.size() - 1) { selectedSectorIdx++; refreshItems(); selectedItemIdx = 0; }
            else if (focusPanel == 2 && selectedItemIdx < (int)items.size()) selectedItemIdx++;
            return true;
        }
        if (e == Event::ArrowLeft) { focusPanel = 0; return true; }
        if (e == Event::ArrowRight) { focusPanel = 2; return true; }
        if (e == Event::Tab) {
            selectedCategoryIdx = (selectedCategoryIdx + 1) % categories.size();
            refreshItems();
            selectedItemIdx = 0;
            return true;
        }

        if (e == Event::Return) {
            if (focusPanel == 0) {
                focusPanel = 2; // Move focus to items
            }
            else if (focusPanel == 2) {
                if (selectedItemIdx < (int)items.size()) {
                    // Edit Selected Item
                    runEditObjectView(items[selectedItemIdx].id, items[selectedItemIdx].type);
                }
                else {
                    // Add Facility
                    runAddFacilityForm(sectorList[selectedSectorIdx]);
                    refreshItems();
                }
            }
            return true;
        }

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
    string query = "";
    int selected = 0;
    std::vector<string> menu_entries;
    string message = "";

    auto toLower = [](const string& s) -> string {
        string lower = s;
        for (char& c : lower) if (c >= 'A' && c <= 'Z') c += ('a' - 'A');
        return lower;
        };

    auto performSearch = [&]() {
        menu_entries.clear();
        selected = 0;
        if (query.length() < 2) return;
        string q = toLower(query);

        // 1. Search Facilities (Graph Nodes)
        if (islamabad && islamabad->getCityGraph()) {
            CityGraph* g = islamabad->getCityGraph();
            for (int i = 0; i < g->getNodeCount(); i++) {
                CityNode* n = g->getNode(i);
                if (!n || n->type == "CORNER") continue;
                if (toLower(n->name).find(q) != string::npos) {
                    menu_entries.push_back("[ID: " + n->databaseID + "] " + n->name + " (" + n->type + ") in " + n->sector);
                }
            }
        }
        // 2. Search Commercial (Malls, Shops, Products)
        if (islamabad && islamabad->getCommercialManager()) {
            CommercialManager* cm = islamabad->getCommercialManager();
            for (int i = 0; i < cm->malls.getSize(); i++) {
                Mall* m = cm->malls[i];
                if (toLower(m->name).find(q) != string::npos) {
                    menu_entries.push_back("[ID: " + m->id + "] " + m->name + " (" + m->getSector() + ")");
                }
                for (int j = 0; j < m->shops.getSize(); j++) {
                    Shop* s = m->shops[j];
                    if (toLower(s->name).find(q) != string::npos) {
                        menu_entries.push_back("[ID: " + s->id + "] " + s->name + " @ " + m->name);
                    }
                }
            }
        }
        // 3. Search Medical (Medicines)
        if (islamabad && islamabad->getMedicalManager()) {
            MedicalManager* mm = islamabad->getMedicalManager();
            for (int i = 0; i < mm->pharmacies.getSize(); i++) {
                Pharmacy* p = mm->pharmacies[i];
                if (toLower(p->name).find(q) != string::npos) {
                    menu_entries.push_back("[ID: " + p->id + "] " + p->name + " @ " + p->sector);
                }
            }
        }
        if (menu_entries.size() > 50) menu_entries.resize(50);
        };

    InputOption input_opt;
    input_opt.on_change = performSearch;
    auto input_component = Input(&query, input_opt);

    MenuOption menu_opt;
    menu_opt.on_enter = [&] {
        if (selected >= 0 && selected < (int)menu_entries.size()) {
            string selection = menu_entries[selected];
            size_t start = selection.find("[ID: ");
            if (start != string::npos) {
                start += 5;
                size_t end = selection.find("]", start);
                if (end != string::npos) {
                    string extractedID = selection.substr(start, end - start);

                    // Determine Type (simple heuristic)
                    string type = "UNKNOWN";
                    if (selection.find("(SCHOOL)") != string::npos) type = "SCHOOL";
                    else if (selection.find("(HOSPITAL)") != string::npos) type = "HOSPITAL";
                    else if (selection.find("(PHARMACY)") != string::npos) type = "PHARMACY";
                    else if (selection.find("(MALL)") != string::npos) type = "MALL";
                    else if (selection.find("@") != string::npos) { // Likely shop or pharmacy
                        if (extractedID.find("S") != string::npos && extractedID.length() < 6) type = "SHOP"; // Shop IDs like S1
                        else if (extractedID.find("P") != string::npos && extractedID.length() < 6) type = "PHARMACY"; // Pharmacy IDs like P01
                    }

                    // Attempt to launch editor
                    if (type != "UNKNOWN") {
                        runEditObjectView(extractedID, type);
                        return; // Return after edit to search view
                    }

                    message = "Selected Item ID: " + extractedID + " (Press Esc to return)";
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
    auto container = Container::Vertical({ input_component, menu_component | vscroll_indicator | frame | flex });

    auto renderer = Renderer(container, [&] {
        return vbox({
            text(" SEARCH ENGINE ") | bold | center | bgcolor(ftxui::Color::Blue) | color(ftxui::Color::White),
            separator(),
            hbox({ text(" FIND: "), input_component->Render() | flex }),
            separator(),
            menu_entries.empty() ? text("Type to search...") | dim | center : menu_component->Render() | flex,
            separator(),
            (message.empty() ? text("Select an item and press Enter to Edit") | dim | center
                             : text(message) | bold | color(ftxui::Color::Green) | center),
            text("Esc: Back") | dim | center
            }) | border | size(WIDTH, EQUAL, 80) | size(HEIGHT, EQUAL, 40) | center;
        });

    auto component = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Escape) { currentState = SimulatorState::DATABASE_VIEW; screen.Exit(); return true; }
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
            text(title) | bold | center | bgcolor(ftxui::Color::Blue) | color(ftxui::Color::White),
            separator(),
            vbox(fields) | flex,
            separator(),
            hbox({ btn_confirm->Render(), text("  "), btn_cancel->Render() }) | center
            }) | border | size(WIDTH, EQUAL, 60) | center;
        });

    screen.Loop(renderer);
}
// ============================================================================
// HELPER: MANAGEMENT ADD FORM
// ============================================================================
inline void CitySimulator::runManagementAddForm(const string& category) {
    auto screen = ScreenInteractive::Fullscreen();
    string name_val, sector_val;
    int type_selected = 0;

    std::vector<string> types;
    if (category == "Schools") types = { "School" };
    else if (category == "Hospitals") types = { "Hospital" };
    else if (category == "Pharmacies") types = { "Pharmacy" };
    else if (category == "Nodes" || category == "All") types = { "Pharmacy", "School", "Hospital", "Bus Stop" };
    else {
        types = { "Pharmacy", "School", "Hospital", "Bus Stop" };
    }

    string message = "";

    Component input_name = Input(&name_val, "Enter Name");
    Component input_sector = Input(&sector_val, "Enter Sector (e.g. F-10)");
    Component toggle_type = Toggle(&types, &type_selected);

    Component btn_create = Button("Create", [&] {
        if (name_val.empty() || sector_val.empty()) {
            message = "Error: All fields required.";
            return;
        }

        string type = types[type_selected];
        string res = "";

        if (type == "School") res = cityMgmt->addSchool(name_val, sector_val, 3.0, {}, {});
        else if (type == "Hospital") res = cityMgmt->addHospital(name_val, sector_val, 50, {});
        else if (type == "Pharmacy") res = cityMgmt->addPharmacy(name_val, sector_val);
        else if (type == "Bus Stop") {
            int id = cityMgmt->addBusStopInSector(name_val, sector_val);
            if (id != -1) res = "STOP";
        }

        if (!res.empty()) {
            screen.Exit();
        }
        else {
            message = "Creation Failed (Check Sector validity)";
        }
        });

    Component btn_cancel = Button("Cancel", screen.ExitLoopClosure());

    auto container = Container::Vertical({
        input_name, input_sector, toggle_type,
        Container::Horizontal({ btn_create, btn_cancel })
        });

    auto renderer = Renderer(container, [&] {
        return vbox({
            text("ADD NEW OBJECT (" + category + ")") | bold | center | bgcolor(ftxui::Color::Blue) | color(ftxui::Color::White),
            separator(),
            hbox({ text("Name:   ") | size(WIDTH, EQUAL, 10), input_name->Render() }),
            hbox({ text("Sector: ") | size(WIDTH, EQUAL, 10), input_sector->Render() }),
            separator(),
            text("Type:") | bold,
            toggle_type->Render(),
            separator(),
            hbox({ btn_create->Render(), text("  "), btn_cancel->Render() }) | center,
            text(message) | color(ftxui::Color::Red) | center
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

        int count = 0;
        for (int i = 0; i < pm->masterList.getSize(); ++i) {
            Citizen* c = pm->masterList[i];
            if (!c) continue;

            bool match = query.empty();
            if (!match) {
                string q = query;
                string n = c->name;
                string id = c->cnic;
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
            text(" POPULATION REGISTRY - " + title) | bold | center | bgcolor(ftxui::Color::Green) | color(ftxui::Color::Black),
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


inline void CitySimulator::runEditSchoolView(School* school) {
    if (!school) return;
    auto screen = ScreenInteractive::Fullscreen();
    string message = "";

    int tab_index = 0;
    std::vector<string> tabs = { "Info", "Departments", "Faculty", "Students" };


    Component btn_rename = Button("Rename School", [&] {
        runInputForm("Rename School", { "New Name" }, [&](std::vector<string> res) {
            if (!res[0].empty()) { school->name = res[0]; message = "Renamed to " + res[0]; }
            });
        });

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

    Component btn_hire_fac = Button("Hire Faculty", [&] {
        Citizen* c = runPopulationSelector("Select Citizen to Hire");
        if (c) {
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

        auto stats = vbox({
            hbox({text("ID: ") | bold, text(school->id)}),
            hbox({text("Name: ") | bold, text(school->name)}),
            hbox({text("Sector: ") | bold, text(school->location.sector)}),
            hbox({text("Students: ") | bold, text(std::to_string(school->getTotalEnrolledStudents()))}),
            hbox({text("Faculty: ") | bold, text(std::to_string(school->getTotalFaculty()))}),
            }) | border | size(WIDTH, EQUAL, 40);

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
            if (school->departments.getSize() > 0) {
                Department* d = school->departments[0];
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
            text(" SCHOOL ADMINISTRATION PORTAL ") | bold | center | bgcolor(ftxui::Color::Blue) | color(ftxui::Color::White),
            hbox({ stats, separator(), content | flex }),
            separator(),
            c_tabs->Render() | center,
            separator(),
            btn_back->Render() | center,
            text(message) | bold | color(ftxui::Color::Red) | center
            }) | border | center;
        });

    screen.Loop(renderer);
}

inline void CitySimulator::runEditHospitalView(Hospital* hospital) {
    if (!hospital) return;
    auto screen = ScreenInteractive::Fullscreen();
    string message = "";
    int tab_index = 0;
    std::vector<string> tabs = { "Info", "Doctors", "Patients" };


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
                    Doctor d(c, res[0]);
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
            text(" HOSPITAL ADMIN: " + hospital->name) | bold | center | bgcolor(ftxui::Color::Red) | color(ftxui::Color::White),
            hbox({
                vbox({
                    hbox({text("Beds: "), text(std::to_string(hospital->getOccupiedBeds()) + "/" + std::to_string(hospital->totalBeds))})
                }) | border | size(WIDTH, EQUAL, 30),
                content | flex
            }) | flex,
            separator(),
            c_tabs->Render() | center,
            btn_back->Render() | center,
            text(message) | color(ftxui::Color::Yellow) | center
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
            inv.push_back(hbox({ text(p->name), filler(), text("Rs " + std::to_string(p->price)) | color(ftxui::Color::Green) }));
        }

        return vbox({
            text(" SHOP INVENTORY: " + shop->name) | bold | center | bgcolor(ftxui::Color::Yellow) | color(ftxui::Color::Black),
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
            text(message) | color(ftxui::Color::Red) | center
            }) | border;
        });

    screen.Loop(renderer);
}

// --- 4. Edit PHARMACY ---
inline void CitySimulator::runEditPharmacyView(Pharmacy* pharmacy) {
    if (!pharmacy) return;
    auto screen = ScreenInteractive::Fullscreen();
    string message = "";

    // Medicine Management
    Component btn_add_med = Button("Add Medicine", [&] {
        runInputForm("New Medicine", { "Name", "Formula", "Price" }, [&](std::vector<string> res) {
            if (!res[0].empty() && !res[2].empty()) {
                float p = 0.0f; try { p = std::stof(res[2]); }
                catch (...) {}
                Medicine med(res[0], res[1], p);
                pharmacy->addMedicine(med);
                message = "Added " + res[0];
            }
            });
        });

    Component btn_rem_med = Button("Remove Medicine", [&] {
        runInputForm("Remove Medicine", { "Name" }, [&](std::vector<string> res) {
            if (pharmacy->removeMedicine(res[0])) message = "Removed " + res[0];
            else message = "Medicine not found.";
            });
        });

    Component btn_back = Button("Back", screen.ExitLoopClosure());

    auto container = Container::Vertical({ btn_add_med, btn_rem_med, btn_back });

    auto renderer = Renderer(container, [&] {
        Elements inv;
        for (int i = 0; i < pharmacy->inventory.getSize(); ++i) {
            const Medicine* m = pharmacy->getMedicine(i);
            inv.push_back(hbox({ text(m->name), filler(), text(m->formula) | dim, filler(), text("Rs " + std::to_string((int)m->price)) | color(ftxui::Color::Green) }));
        }

        return vbox({
            text(" PHARMACY INVENTORY: " + pharmacy->name) | bold | center | bgcolor(ftxui::Color::Magenta) | color(ftxui::Color::White),
            separator(),
            hbox({
                vbox({
                    hbox({text("ID: ") | bold, text(pharmacy->id)}),
                    hbox({text("Sector: ") | bold, text(pharmacy->sector)}),
                    hbox({text("Items: ") | bold, text(std::to_string(pharmacy->getMedicineCount()))})
                }) | border | size(WIDTH, EQUAL, 25),
                vbox(inv) | flex | border,
                vbox({
                    text("Actions") | bold | center,
                    separator(),
                    btn_add_med->Render(),
                    text(" "),
                    btn_rem_med->Render(),
                    filler(),
                    btn_back->Render()
                }) | size(WIDTH, EQUAL, 25)
            }) | flex,
            text(message) | color(ftxui::Color::Red) | center
            }) | border;
        });

    screen.Loop(renderer);
}

// --- 5. Edit MALL ---
inline void CitySimulator::runEditMallView(Mall* mall) {
    if (!mall) return;
    auto screen = ScreenInteractive::Fullscreen();
    string message = "";
    int selected_shop_idx = 0;

    Component btn_add_shop = Button("Add Shop", [&] {
        runInputForm("New Shop", { "Shop Name", "Category" }, [&](std::vector<string> res) {
            if (!res[0].empty()) {
                string newID = "SHOP-" + std::to_string(mall->getShopCount() + 100); // Simple ID gen
                Shop* s = new Shop(newID, res[0], res[1]);
                mall->addShop(s);
                message = "Added Shop: " + res[0];
            }
            });
        });

    Component btn_rem_shop = Button("Remove Shop", [&] {
        runInputForm("Remove Shop", { "Shop ID" }, [&](std::vector<string> res) {
            bool found = false;
            for (int i = 0; i < mall->shops.getSize(); ++i) {
                if (mall->shops[i]->id == res[0]) {
                    delete mall->shops[i];
                    mall->shops.erase(i);
                    found = true;
                    message = "Removed Shop ID: " + res[0];
                    break;
                }
            }
            if (!found) message = "Shop ID not found.";
            });
        });

    Component btn_edit_shop = Button("Edit Selected Shop", [&] {
        if (selected_shop_idx >= 0 && selected_shop_idx < mall->shops.getSize()) {
            runEditShopView(mall->shops[selected_shop_idx], mall);
        }
        else {
            message = "No shop selected.";
        }
        });

    Component btn_back = Button("Back", screen.ExitLoopClosure());

    std::vector<string> shop_names;


    auto menu_shops = Menu(&shop_names, &selected_shop_idx);

    auto container = Container::Vertical({
        menu_shops | vscroll_indicator | frame | flex,
        btn_add_shop, btn_rem_shop, btn_edit_shop, btn_back
        });

    auto renderer = Renderer(container, [&] {
        // Sync shop names
        shop_names.clear();
        for (int i = 0; i < mall->shops.getSize(); ++i) {
            shop_names.push_back(mall->shops[i]->name + " (" + mall->shops[i]->category + ")");
        }

        return vbox({
            text(" MALL MANAGEMENT: " + mall->name) | bold | center | bgcolor(ftxui::Color::Yellow) | color(ftxui::Color::Black),
            separator(),
            hbox({
                vbox({
                    hbox({text("ID: ") | bold, text(mall->id)}),
                    hbox({text("Sector: ") | bold, text(mall->getSector())}),
                    hbox({text("Shops: ") | bold, text(std::to_string(mall->getShopCount()))})
                }) | border | size(WIDTH, EQUAL, 30),
                vbox({
                    text("Shops Directory") | bold | center,
                    separator(),
                    menu_shops->Render() | flex
                }) | flex | border,
                vbox({
                    text("Actions") | bold | center,
                    separator(),
                    btn_add_shop->Render(),
                    text(" "),
                    btn_rem_shop->Render(),
                    text(" "),
                    btn_edit_shop->Render(),
                    filler(),
                    btn_back->Render()
                }) | size(WIDTH, EQUAL, 25)
            }) | flex,
            text(message) | color(ftxui::Color::Red) | center
            }) | border;
        });

    screen.Loop(renderer);
}

inline void CitySimulator::runEditObjectView(const string& objectID, const string& objectType) {
    if (objectType == "SCHOOL") {
        if (islamabad && islamabad->getSchoolManager()) {
            School* s = islamabad->getSchoolManager()->findSchoolByID(objectID);
            if (s) { runEditSchoolView(s); return; }
        }
    }
    else if (objectType == "HOSPITAL") {
        if (islamabad && islamabad->getMedicalManager()) {
            Hospital* h = islamabad->getMedicalManager()->findHospitalByID(objectID);
            if (h) { runEditHospitalView(h); return; }
        }
    }
    else if (objectType == "SHOP") {
        if (islamabad && islamabad->getCommercialManager()) {
            CommercialManager* cm = islamabad->getCommercialManager();
            for (int i = 0; i < cm->malls.getSize(); i++) {
                Shop* s = cm->malls[i]->findShopByID(objectID);
                if (s) { runEditShopView(s, cm->malls[i]); return; }
            }
        }
    }
    else if (objectType == "PHARMACY") {
        if (islamabad && islamabad->getMedicalManager()) {
            MedicalManager* mm = islamabad->getMedicalManager();
            for (int i = 0; i < mm->pharmacies.getSize(); ++i) {
                if (mm->pharmacies[i]->id == objectID) {
                    runEditPharmacyView(mm->pharmacies[i]);
                    return;
                }
            }
        }
    }
    else if (objectType == "MALL") {
        if (islamabad && islamabad->getCommercialManager()) {
            CommercialManager* cm = islamabad->getCommercialManager();
            if (cm->mallLookup.contains(objectID)) {
                Mall** m = cm->mallLookup.get(objectID);
                if (m && *m) {
                    runEditMallView(*m);
                    return;
                }
            }
            for (int i = 0; i < cm->malls.getSize(); ++i) {
                if (cm->malls[i]->id == objectID) {
                    runEditMallView(cm->malls[i]);
                    return;
                }
            }
        }
    }

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


inline void CitySimulator::runManagementMenu() {
    auto screen = ScreenInteractive::Fullscreen();

    int selectedCategoryIdx = 0;
    int selectedItemIdx = 0;
    int focusPanel = 0; // 0=Tabs, 1=List, 2=Actions

    std::vector<string> categories = { "All", "Nodes", "Malls", "Shops", "Schools", "Hospitals", "Pharmacies" };

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
                text(title) | bold | center | bgcolor(ftxui::Color::Red) | color(ftxui::Color::White),
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
            if (i == selectedCategoryIdx) tab = tab | bold | bgcolor(ftxui::Color::Cyan) | color(ftxui::Color::Black);
            else tab = tab | color(ftxui::Color::GrayLight);
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
                if (focusPanel == 1) row = row | bgcolor(ftxui::Color::Blue) | color(ftxui::Color::White);
                else row = row | color(ftxui::Color::Green);
            }
            listElements.push_back(row);
        }
        auto leftPanel = vbox(listElements) | border | flex;
        if (focusPanel == 1) leftPanel = leftPanel | color(ftxui::Color::Cyan);

        // ===== MIDDLE PANEL (Actions) =====
        auto btnStyle = [&](string label, bool selected) {
            return text(label) | center | (selected ? (bgcolor(ftxui::Color::Red) | bold) : dim) | border;
            };

        auto midPanel = vbox({
            text("ACTIONS") | bold | center,
            separator(),
            btnStyle("[ EDIT ] (E)", false), // E key
            text(" "),
            btnStyle("[ ADD ] (+)", false),  // + key
            text(" "),
            btnStyle("[ DELETE ] (Del)", false) // Del key
            }) | border | size(WIDTH, EQUAL, 20);

        Elements details;
        if (!currentItems.empty() && selectedItemIdx < (int)currentItems.size()) {
            const auto& sel = currentItems[selectedItemIdx];
            details.push_back(text("OBJECT DETAILS") | bold | center | color(ftxui::Color::Yellow));
            details.push_back(separator());
            details.push_back(hbox({ text("ID: ") | bold, text(sel.id) | color(ftxui::Color::Cyan) }));
            details.push_back(hbox({ text("Name: ") | bold, text(sel.name) | color(ftxui::Color::White) }));
            details.push_back(hbox({ text("Type: ") | bold, text(sel.type) | color(ftxui::Color::Magenta) }));
            details.push_back(hbox({ text("Loc/Info: ") | bold, text(sel.extraInfo) | color(ftxui::Color::Green) }));
            details.push_back(separator());
            details.push_back(text("Press 'E' to Edit full details") | dim | center);
        }
        else {
            details.push_back(text("No item selected") | dim | center);
        }
        auto rightPanel = vbox(details) | border | flex;

        return vbox({
            text(" MANAGEMENT CONSOLE (ADMIN) ") | bold | center | bgcolor(ftxui::Color::Red) | color(ftxui::Color::White),
            hbox(tabs) | center,
            separator(),
            hbox({ leftPanel, midPanel, rightPanel }) | flex,
            separator(),
            text("Tab: Switch Panel | Arrows: Navigate | E: Edit | +: Add | Del: Delete | Esc: Back") | dim | center
            });
        });

    auto component = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Tab) { focusPanel = (focusPanel + 1) % 2; return true; }
        if (e == Event::Escape) { currentState = SimulatorState::MAIN_MENU; screen.Exit(); return true; }


        if (e == Event::Character('+') || e == Event::Character('=')) {
            runManagementAddForm(categories[selectedCategoryIdx]);
            refreshList();
            return true;
        }

        // DELETE
        if (e == Event::Delete || e == Event::Special({ 127 }) || e == Event::Character('x')) {
            if (!currentItems.empty() && selectedItemIdx < (int)currentItems.size()) {
                auto& item = currentItems[selectedItemIdx];
                bool deleted = false;
                if (item.type == "SCHOOL") deleted = cityMgmt->removeSchool(item.id);
                else if (item.type == "HOSPITAL") deleted = cityMgmt->removeHospital(item.id);
                else if (item.type == "PHARMACY") deleted = cityMgmt->removePharmacy(item.id);
                else if (item.type == "MALL") {}

                if (deleted) {
                    refreshList(); // Update list~
                }
                else {
                    showPlaceholder("DELETE FAILED", "Could not delete object (Type not supported or dependency)");
                }
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