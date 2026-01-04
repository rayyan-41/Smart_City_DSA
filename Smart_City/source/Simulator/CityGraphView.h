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
    CityGraphView(SmartCity* cityPtr) : city(cityPtr),
        mouseX(0), mouseY(0), hoveredNodeID(-1), hoveredSector(""),
        showCorners(true), showRoads(true), showSectorBounds(false),
        showHouses(false), showTraffic(false), trafficPaused(false),
        dijkstraMode(DijkstraMode::SELECT_START), dijkstraStartNode(-1),
        dijkstraEndNode(-1), dijkstraDistance(0.0),
        dijkstraNodeSelection(0), dijkstraEndNodeSelection(0),
        intersectionCounter(0) {
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

    void initializeTraffic() {
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

    void updateTraffic() {
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

    void updateHoverState(int mx, int my) {
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
                if ((edge.fromID == from && edge.toID == to) || (edge.fromID == to && edge.toID == from)) {
                    edge.isOnPath = true;
                }
            }
        }
    }

    void renderGraph(termgl::Window& window) {
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
                    window.drawLine((int)n1.pos.x, (int)n1.pos.y, (int)n2.pos.x, (int)n2.pos.y, termgl::Color(50, 50, 60));
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
                    window.drawLine((int)n1.pos.x, (int)n1.pos.y, (int)n2.pos.x, (int)n2.pos.y, termgl::Color::Green());
                    window.drawLine((int)n1.pos.x + 1, (int)n1.pos.y + 1, (int)n2.pos.x + 1, (int)n2.pos.y + 1, termgl::Color::Green());
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
                window.drawPixel((int)node.pos.x, (int)node.pos.y, termgl::Color(80, 80, 80));
            }
        }

        if (showCorners) {
            for (int i = 0; i < graphNodes.getSize(); i++) {
                const GraphNode2D& node = graphNodes[i];
                if (!node.isCorner) continue;
                if (dijkstraPath.getSize() > 0 && !node.isOnPath) continue;
                termgl::Color c = node.isOnPath ? termgl::Color::Green() : termgl::Color::Grey();
                if (dijkstraPath.getSize() > 0 && !node.isOnPath) continue;
                window.fillCircle((int)node.pos.x, (int)node.pos.y, 2, c);
            }
        }

        for (int i = 0; i < graphNodes.getSize(); i++) {
            const GraphNode2D& node = graphNodes[i];
            if (node.isCorner || node.type == "HOUSE") continue;
            if (dijkstraPath.getSize() > 0 && !node.isOnPath && !node.isStart && !node.isEnd) continue;

            termgl::Color nodeColor = node.color;
            int radius = 4;
            if (node.isStart) { nodeColor = termgl::Color::Cyan(); radius = 7; }
            else if (node.isEnd) { nodeColor = termgl::Color::Yellow(); radius = 7; }
            else if (node.isOnPath) { nodeColor = termgl::Color::Green(); radius = 5; }
            else if (node.isVisited && dijkstraPath.getSize() == 0) nodeColor = termgl::Color(255, 165, 0);

            if (node.id == hoveredNodeID) {
                nodeColor = termgl::Color::White();
                radius += 2;
                window.drawText((int)node.pos.x + 10, (int)node.pos.y - 10, node.name, termgl::Color::White());
            }
            window.fillCircle((int)node.pos.x, (int)node.pos.y, radius, nodeColor);
        }
    }

    void run() {
        buildGraphVisualization();
        buildSelectableNodesList();

        termgl::Window window(1600, 900, "Islamabad City Simulator - Interactive Map", true);
        window.setFramerateLimit(60);

        int width = window.getWidth();
        int height = window.getHeight();

        int mapPartition = window.addPartition(0, 0, (int)(width * 0.75), height, "City Map");
        int sidePartition = window.addPartition((int)(width * 0.75), 0, (int)(width * 0.25), height, "Control Panel");

        bool running = true;
        bool inDijkstraMode = false;

        std::vector<string> targetTypes = {
            "Nearest School", "Nearest Hospital", "Nearest Pharmacy", "Nearest Bus Stop", "Custom Location"
        };
        int targetSel = 0;
        int scrollOffset = 0;

        viewport.setCanvasSize(window.getWidth() * 0.75, window.getHeight());

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

            if (window.isKeyPressed('D') && !inDijkstraMode) {
                inDijkstraMode = true;
                dijkstraMode = DijkstraMode::SELECT_START;
                dijkstraNodeSelection = 0;
                scrollOffset = 0;
                clearDijkstraVisualization();
            }

            if (window.isKeyDown(VK_UP) && (!inDijkstraMode || dijkstraMode == DijkstraMode::COMPLETE)) viewport.panUp();
            if (window.isKeyDown(VK_DOWN) && (!inDijkstraMode || dijkstraMode == DijkstraMode::COMPLETE)) viewport.panDown();
            if (window.isKeyDown(VK_LEFT)) viewport.panLeft();
            if (window.isKeyDown(VK_RIGHT)) viewport.panRight();
            if (window.isKeyDown('W')) viewport.panUp();
            if (window.isKeyDown('S')) viewport.panDown();
            if (window.isKeyDown('A')) viewport.panLeft();
            if (window.isKeyDown('D') && window.isKeyDown(VK_CONTROL)) viewport.panRight();
            if (window.isKeyPressed(VK_ADD) || window.isKeyPressed('=')) viewport.zoomIn();
            if (window.isKeyPressed(VK_SUBTRACT) || window.isKeyPressed('-')) viewport.zoomOut();

            if (window.isKeyPressed('R') && !inDijkstraMode) showRoads = !showRoads;
            if (window.isKeyPressed('C')) showCorners = !showCorners;
            if (window.isKeyPressed('S')) showSectorBounds = !showSectorBounds;
            if (window.isKeyPressed('H')) showHouses = !showHouses;
            if (window.isKeyPressed('T')) showTraffic = !showTraffic;
            if (window.isKeyPressed('P')) trafficPaused = !trafficPaused;

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

            if (showTraffic && !trafficPaused) updateTraffic();

            window.setActivePartition(-1);
            window.clear(termgl::Color(10, 10, 15));
            window.drawPartitionFrames();

            window.setActivePartition(mapPartition);
            window.clear(termgl::Color(20, 20, 30));
            renderGraph(window);

            window.setActivePartition(mapPartition);
            termgl::Vec2 mousePos = window.getMousePos();
            updateHoverState(mousePos.x, mousePos.y);

            window.setActivePartition(sidePartition);
            window.clear(termgl::Color(30, 30, 40));

            int cy = 10;

            if (!inDijkstraMode) {
                window.drawText(10, cy, "INFO PANEL", termgl::Color::Cyan()); cy += 30;
                window.drawRect(5, cy, window.getWidth() - 10, 120, termgl::Color(60, 60, 70));
                window.drawText(10, cy + 10, "SELECTION:", termgl::Color::Yellow());
                string info = getHoverInfo();
                window.drawText(10, cy + 30, info, termgl::Color::White());
                cy += 140;

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

                window.drawText(10, cy, "CONTROLS:", termgl::Color::Cyan()); cy += 25;
                window.drawText(10, cy, "Arrows/WASD: Pan", termgl::Color::Grey()); cy += 20;
                window.drawText(10, cy, "+/-: Zoom", termgl::Color::Grey()); cy += 20;
                window.drawText(10, cy, "R: Toggle Roads", termgl::Color::Grey()); cy += 20;
                window.drawText(10, cy, "T: Toggle Traffic", termgl::Color::Grey()); cy += 20;
                window.drawText(10, cy, "D: Dijkstra Mode", termgl::Color::Green()); cy += 20;
                window.drawText(10, cy, "Esc: Menu", termgl::Color::Grey());
            }
            else {
                window.drawText(10, cy, "PATHFINDING", termgl::Color::Green()); cy += 30;

                if (dijkstraMode == DijkstraMode::SELECT_START) {
                    window.drawText(10, cy, "SELECT START:", termgl::Color::Yellow()); cy += 30;
                    std::vector<string> items;
                    for (int i = 0; i < selectableNodes.getSize(); i++) {
                        int idx = nodeIdToIndex[selectableNodes[i]];
                        items.push_back(graphNodes[idx].name);
                    }
                    int listH = window.getHeight() - cy - 50;
                    int dummyScroll = dijkstraNodeSelection * 20 - (listH / 2);
                    int clicked = window.drawList(10, cy, window.getWidth() - 20, listH, items, dummyScroll);
                    window.drawRect(12, cy + (dijkstraNodeSelection * 20) - dummyScroll, window.getWidth() - 24, 20, termgl::Color::Green());
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
};

#endif // CITY_GRAPH_VIEW_H