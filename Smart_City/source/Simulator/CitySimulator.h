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
// GRAPH VIEWPORT
// ============================================================================
class GraphViewport {
private:
    double minLat, maxLat, minLon, maxLon;
    int canvasWidth, canvasHeight;
    double offsetX, offsetY;
    double zoom;
    double rotationAngle;  // Rotation in radians

public:
    GraphViewport() : minLat(33.60), maxLat(33.74), minLon(72.96), maxLon(73.10),
        canvasWidth(160), canvasHeight(80), offsetX(0), offsetY(0), zoom(1.0),
        rotationAngle(-0.785398) {  // -45 degrees to straighten the diagonal grid
    }

    void setBounds(double minLa, double maxLa, double minLo, double maxLo) {
        minLat = minLa; maxLat = maxLa; minLon = minLo; maxLon = maxLo;
    }

    void setCanvasSize(int w, int h) {
        canvasWidth = w; canvasHeight = h;
    }

    void setRotation(double angle) {
        rotationAngle = angle;
    }

    Point2D geoToCanvas(double lat, double lon) const {
        // Normalize to 0-1 range
        double normX = (lon - minLon) / (maxLon - minLon);
        double normY = (lat - minLat) / (maxLat - minLat);

        // Center for rotation
        normX -= 0.5;
        normY -= 0.5;

        // Apply rotation to straighten the tilted grid
        double cosA = std::cos(rotationAngle);
        double sinA = std::sin(rotationAngle);
        double rotX = normX * cosA - normY * sinA;
        double rotY = normX * sinA + normY * cosA;

        // Scale to fit after rotation (rotation expands bounds by sqrt(2))
        double scale = 0.7;  // Shrink to fit rotated content
        rotX *= scale;
        rotY *= scale;

        // Move back from center
        rotX += 0.5;
        rotY += 0.5;

        // Flip Y axis (higher latitude = top of screen)
        rotY = 1.0 - rotY;

        // Apply zoom and pan
        rotX = (rotX - 0.5) * zoom + 0.5 + offsetX;
        rotY = (rotY - 0.5) * zoom + 0.5 + offsetY;

        // Padding
        double pad = 0.05;
        return Point2D(
            pad * canvasWidth + rotX * canvasWidth * (1.0 - 2 * pad),
            pad * canvasHeight + rotY * canvasHeight * (1.0 - 2 * pad)
        );
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
    INTRO_PHASE_1,
    INTRO_PHASE_2,
    INTRO_PHASE_3,
    WELCOME_ANIMATION,
    MAIN_MENU,
    CSV_SELECTION,
    LOADING,
    GRAPH_VIEW,
    DATABASE_VIEW,
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
    void runDebugMode();  // NEW: Bypass function for debugging
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

    Canvas renderGraphToCanvas(int width, int height);
    Canvas renderGridBackground(int width, int height);
    void updateHoverState(int mx, int my);
    string getHoverInfo();

    void sleepMs(int ms);
    bool fileExists(const string& path);
};

// ============================================================================
// IMPLEMENTATION
// ============================================================================

inline CitySimulator::CitySimulator()
    : islamabad(nullptr),
    currentState(SimulatorState::INTRO_PHASE_1),
    loadMode(CSVLoadMode::DEMO_MODE),
    cityInitialized(false),
    mouseX(0), mouseY(0),
    hoveredNodeID(-1), hoveredSector(""),
    showCorners(true), showRoads(true) {

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
    buildGraphVisualization();

    int cw = width * 2;
    int ch = height * 4;

    // Draw sector grid
    for (const auto& region : sectorRegions) {
        int x1 = std::max(0, std::min((int)region.topLeft.x, cw - 1));
        int y1 = std::max(0, std::min((int)region.topLeft.y, ch - 1));
        int x2 = std::max(0, std::min((int)region.bottomRight.x, cw - 1));
        int y2 = std::max(0, std::min((int)region.bottomRight.y, ch - 1));
        for (int x = x1; x <= x2; x += 3) {
            canvas.DrawPoint(x, y1, true);
            canvas.DrawPoint(x, y2, true);
        }
        for (int y = y1; y <= y2; y += 3) {
            canvas.DrawPoint(x1, y, true);
            canvas.DrawPoint(x2, y, true);
        }
    }

    // Draw roads using nodeIdToIndex for fast lookup
    if (showRoads) {
        for (const auto& edge : graphEdges) {
            int idx1 = (edge.fromID < (int)nodeIdToIndex.size()) ? nodeIdToIndex[edge.fromID] : -1;
            int idx2 = (edge.toID < (int)nodeIdToIndex.size()) ? nodeIdToIndex[edge.toID] : -1;
            if (idx1 >= 0 && idx2 >= 0) {
                const GraphNode2D& n1 = graphNodes[idx1];
                const GraphNode2D& n2 = graphNodes[idx2];
                int x1 = std::max(0, std::min((int)n1.pos.x, cw - 1));
                int y1 = std::max(0, std::min((int)n1.pos.y, ch - 1));
                int x2 = std::max(0, std::min((int)n2.pos.x, cw - 1));
                int y2 = std::max(0, std::min((int)n2.pos.y, ch - 1));
                canvas.DrawPointLine(x1, y1, x2, y2);
            }
        }
    }

    // Draw nodes - corners small (white), facilities larger (colored)
    for (const auto& node : graphNodes) {
        if (!showCorners && node.isCorner) continue;
        int x = std::max(0, std::min((int)node.pos.x, cw - 1));
        int y = std::max(0, std::min((int)node.pos.y, ch - 1));

        if (node.isCorner) {
            // Small 2x2 corner marker
            for (int dx = 0; dx <= 1; dx++)
                for (int dy = 0; dy <= 1; dy++)
                    if (x+dx < cw && y+dy < ch) canvas.DrawPoint(x+dx, y+dy, true);
        } else {
            // Larger circle for facilities
            int r = 3;
            for (int dx = -r; dx <= r; dx++) {
                for (int dy = -r; dy <= r; dy++) {
                    if (dx*dx + dy*dy <= r*r) {
                        int px = x + dx, py = y + dy;
                        if (px >= 0 && px < cw && py >= 0 && py < ch)
                            canvas.DrawPoint(px, py, true);
                    }
                }
            }
        }
    }

    return canvas;
}

inline Canvas CitySimulator::renderGridBackground(int width, int height) {
    Canvas canvas(width, height);
    
    int gridSpacingX = 8;
    int gridSpacingY = 8;
    
    for (int x = 0; x < width; x += gridSpacingX) {
        for (int y = 0; y < height; y++) {
            canvas.DrawPoint(x, y, true);
        }
    }
    
    for (int y = 0; y < height; y += gridSpacingY) {
        for (int x = 0; x < width; x++) {
            canvas.DrawPoint(x, y, true);
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

    double minDist = 15.0;
    for (const auto& node : graphNodes) {
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

    if (hoveredNodeID >= 0 && hoveredNodeID < (int)graphNodes.size()) {
        const GraphNode2D& node = graphNodes[hoveredNodeID];
        ss << "NODE INFORMATION\n";
        ss << "----------------\n";
        ss << "ID: " << node.id << "\n";
        ss << "Name: " << node.name << "\n";
        ss << "Type: " << node.type << "\n";
        ss << "Sector: " << node.sector;
    }
    else if (!hoveredSector.empty()) {
        int nodeCount = 0, stopCount = 0, schoolCount = 0, hospitalCount = 0;

        for (const auto& node : graphNodes) {
            if (node.sector == hoveredSector) {
                nodeCount++;
                if (node.type == "STOP") stopCount++;
                if (node.type == "SCHOOL") schoolCount++;
                if (node.type == "HOSPITAL") hospitalCount++;
            }
        }

        ss << "SECTOR: " << hoveredSector << "\n";
        ss << "----------------\n";
        ss << "Total Nodes: " << nodeCount << "\n";
        ss << "Bus Stops: " << stopCount << "\n";
        ss << "Schools: " << schoolCount << "\n";
        ss << "Hospitals: " << hospitalCount;
    }
    else {
        ss << "HOVER OVER MAP\n";
        ss << "----------------\n";
        ss << "Move mouse over\n";
        ss << "nodes or sectors\n";
        ss << "to see details";
    }

    return ss.str();
}

// ============================================================================
// INTRO PHASE 1 - Typewriter: "A product of Rayyan's Emporium"
// ============================================================================

inline void CitySimulator::runIntroPhase1() {
    auto screen = ScreenInteractive::Fullscreen();

    const string fullText = "A product of Rayyan's Emporium";
    std::atomic<int> charIndex{ 0 };
    std::atomic<bool> typingDone{ false };
    std::atomic<bool> canProceed{ false };

    auto renderer = Renderer([&] {
        int idx = charIndex.load();
        string displayText = fullText.substr(0, std::min(idx, (int)fullText.length()));
        
        if (!typingDone.load() && idx < (int)fullText.length()) {
            displayText += "_";
        }

        string hint = canProceed.load() ? "Press Enter to continue" : "";

        return vbox({
            filler(),
            text(displayText) | bold | color(Color::White) | center,
            text("") | size(HEIGHT, EQUAL, 2),
            text(hint) | center | dim,
            filler(),
        });
    });

    std::thread typewriter([&]() {
        for (int i = 0; i <= (int)fullText.length(); i++) {
            charIndex.store(i);
            screen.PostEvent(Event::Custom);
            sleepMs(50);
        }
        typingDone.store(true);
        sleepMs(500);
        canProceed.store(true);
        screen.PostEvent(Event::Custom);
    });

    auto component = CatchEvent(renderer, [&](Event event) {
        // Only Enter key advances
        if (event == Event::Return && canProceed.load()) {
            currentState = SimulatorState::INTRO_PHASE_2;
            screen.Exit();
            return true;
        }
        return false;
    });

    screen.Loop(component);
    if (typewriter.joinable()) typewriter.join();
}

// ============================================================================
// INTRO PHASE 2 - Typewriter: "Created by Rayyan, Omar and Aryan"
// ============================================================================

inline void CitySimulator::runIntroPhase2() {
    auto screen = ScreenInteractive::Fullscreen();

    const string fullText = "Created by Rayyan, Omar and Aryan";
    std::atomic<int> charIndex{ 0 };
    std::atomic<bool> typingDone{ false };
    std::atomic<bool> canProceed{ false };

    auto renderer = Renderer([&] {
        int idx = charIndex.load();
        string displayText = fullText.substr(0, std::min(idx, (int)fullText.length()));
        
        if (!typingDone.load() && idx < (int)fullText.length()) {
            displayText += "_";
        }

        string hint = canProceed.load() ? "Press Enter to continue" : "";

        return vbox({
            filler(),
            text(displayText) | bold | color(Color::White) | center,
            text("") | size(HEIGHT, EQUAL, 2),
            text(hint) | center | dim,
            filler(),
        });
    });

    std::thread typewriter([&]() {
        sleepMs(200);
        for (int i = 0; i <= (int)fullText.length(); i++) {
            charIndex.store(i);
            screen.PostEvent(Event::Custom);
            sleepMs(45);
        }
        typingDone.store(true);
        sleepMs(500);
        canProceed.store(true);
        screen.PostEvent(Event::Custom);
    });

    auto component = CatchEvent(renderer, [&](Event event) {
        if (event == Event::Return && canProceed.load()) {
            currentState = SimulatorState::INTRO_PHASE_3;
            screen.Exit();
            return true;
        }
        return false;
    });

    screen.Loop(component);
    if (typewriter.joinable()) typewriter.join();
}

// ============================================================================
// INTRO PHASE 3 - "ISLAMABAD REDEFINED" Title Screen
// ============================================================================

inline void CitySimulator::runIntroPhase3() {
    auto screen = ScreenInteractive::Fullscreen();

    std::atomic<bool> showTitle{ false };
    std::atomic<bool> canProceed{ false };
    std::atomic<int> titleRevealLine{ 0 };

    auto renderer = Renderer([&] {
        Elements islamabadLines;
        Elements redefinedLines;
        
        int revealLine = titleRevealLine.load();
        
        if (showTitle.load()) {
            for (int i = 0; i < ASCIIArt::ISLAMABAD_TITLE_HEIGHT; i++) {
                if (i <= revealLine) {
                    islamabadLines.push_back(text(ASCIIArt::ISLAMABAD_TITLE[i]) | color(Color::White) | bold);
                } else {
                    islamabadLines.push_back(text(""));
                }
            }
            
            for (int i = 0; i < ASCIIArt::REDEFINED_TITLE_HEIGHT; i++) {
                if (i <= revealLine - ASCIIArt::ISLAMABAD_TITLE_HEIGHT) {
                    redefinedLines.push_back(text(ASCIIArt::REDEFINED_TITLE[i]) | color(Color::GrayLight) | bold);
                } else {
                    redefinedLines.push_back(text(""));
                }
            }
        }

        string prompt = canProceed.load() ? "Press Enter to begin" : "";

        return vbox({
            filler(),
            vbox(islamabadLines) | center,
            text("") | size(HEIGHT, EQUAL, 1),
            vbox(redefinedLines) | center,
            text("") | size(HEIGHT, EQUAL, 3),
            text(prompt) | center | dim,
            filler(),
        });
    });

    std::thread animator([&]() {
        sleepMs(300);
        showTitle.store(true);
        
        for (int i = 0; i <= ASCIIArt::ISLAMABAD_TITLE_HEIGHT + ASCIIArt::REDEFINED_TITLE_HEIGHT; i++) {
            titleRevealLine.store(i);
            screen.PostEvent(Event::Custom);
            sleepMs(80);
        }
        
        sleepMs(400);
        canProceed.store(true);
        screen.PostEvent(Event::Custom);
    });

    auto component = CatchEvent(renderer, [&](Event event) {
        if (event == Event::Return && canProceed.load()) {
            currentState = SimulatorState::MAIN_MENU;
            screen.Exit();
            return true;
        }
        return false;
    });

    screen.Loop(component);
    if (animator.joinable()) animator.join();
}

// ============================================================================
// WELCOME ANIMATION
// ============================================================================

inline void CitySimulator::runWelcomeAnimation() {
    currentState = SimulatorState::MAIN_MENU;
}

// ============================================================================
// MAIN MENU
// ============================================================================

inline void CitySimulator::runMainMenu() {
    auto screen = ScreenInteractive::Fullscreen();

    std::vector<string> options;
    if (!cityInitialized) {
        options = { "Initialize City", "Exit" };
    } else {
        options = { "Interactive Graph View", "Database Browser", "Exit" };
    }

    int selected = 0;

    auto renderer = Renderer([&] {
        string statusText = cityInitialized ? "CITY INITIALIZED" : "Not Initialized";
        Color statusColor = cityInitialized ? Color::Green : Color::GrayDark;

        Elements menuItems;
        for (int i = 0; i < (int)options.size(); i++) {
            string prefix = (i == selected) ? " > " : "   ";
            menuItems.push_back(text(prefix + options[i]) | color(Color::White));
        }

        CityStats stats;
        if (islamabad) stats = islamabad->getCityStats();

        auto statsBox = cityInitialized ? vbox({
            text(""),
            text("  Nodes: " + std::to_string(stats.totalNodes)),
            text("  Schools: " + std::to_string(stats.totalSchools)),
            text("  Hospitals: " + std::to_string(stats.totalHospitals)),
            text("  Buses: " + std::to_string(stats.totalBuses)),
            text("  Citizens: " + std::to_string(stats.totalCitizens)),
            text(""),
        }) | border | color(Color::White) : vbox({text("")});

        auto menuBox = vbox({
            text(""),
            text("  ISLAMABAD REDEFINED") | bold | center,
            text("  Smart City System") | dim | center,
            text(""),
            separator(),
            text(""),
            text("  Status: " + statusText) | color(statusColor),
            text(""),
            separator(),
            text(""),
            vbox(menuItems),
            text(""),
            separator(),
            text(""),
            text("  Demo Mode: Uses sample data") | dim,
            text("  Full Mode: Loads all CSVs") | dim,
            text(""),
        }) | border | color(Color::White) | size(WIDTH, EQUAL, 50);

        return vbox({
            filler(),
            hbox({
                filler(),
                vbox({
                    menuBox,
                    text("") | size(HEIGHT, EQUAL, 1),
                    statsBox | size(WIDTH, EQUAL, 50),
                }),
                filler(),
            }),
            filler(),
            text("Up/Down: Navigate | Enter: Select | Esc: Exit") | center | dim,
            text("") | size(HEIGHT, EQUAL, 1),
        });
    });

    auto component = CatchEvent(renderer, [&](Event event) {
        if (event == Event::ArrowUp) {
            selected = (selected - 1 + options.size()) % options.size();
            screen.PostEvent(Event::Custom);
            return true;
        }
        if (event == Event::ArrowDown) {
            selected = (selected + 1) % options.size();
            screen.PostEvent(Event::Custom);
            return true;
        }
        if (event == Event::Return) {
            if (!cityInitialized) {
                if (selected == 0) currentState = SimulatorState::CSV_SELECTION;
                else currentState = SimulatorState::EXIT;
            } else {
                if (selected == 0) currentState = SimulatorState::GRAPH_VIEW;
                else if (selected == 1) currentState = SimulatorState::DATABASE_VIEW;
                else currentState = SimulatorState::EXIT;
            }
            screen.Exit();
            return true;
        }
        if (event == Event::Escape) {
            currentState = SimulatorState::EXIT;
            screen.Exit();
            return true;
        }
        return false;
    });

    screen.Loop(component);
}

// ============================================================================
// CSV SELECTION
// ============================================================================

inline void CitySimulator::runCSVSelection() {
    auto screen = ScreenInteractive::Fullscreen();

    std::vector<string> options = { "Demo Mode", "Full Mode", "Back to Menu" };
    int selected = 0;

    auto renderer = Renderer([&] {
        // File status checks
        auto fileStatus = [&](const string& name, const string& path) {
            bool exists = fileExists(path);
            string status = exists ? "[FOUND]" : "[MISSING]";
            Color col = exists ? Color::Green : Color::Red;
            return hbox({
                text("  " + name) | size(WIDTH, EQUAL, 25),
                text(status) | color(col),
            });
        };

        Elements menuItems;
        for (int i = 0; i < (int)options.size(); i++) {
            string prefix = (i == selected) ? " > " : "   ";
            menuItems.push_back(text(prefix + options[i]) | color(Color::White));
        }

        auto filesBox = vbox({
            text(""),
            text("  DATASET FILES") | bold,
            text(""),
            separator(),
            text(""),
            fileStatus("stops.csv", stopsCSV),
            fileStatus("schools.csv", schoolsCSV),
            fileStatus("hospitals.csv", hospitalsCSV),
            fileStatus("pharmacies.csv", pharmaciesCSV),
            fileStatus("buses.csv", busesCSV),
            fileStatus("population.csv", populationCSV),
            fileStatus("malls.csv", mallsCSV),
            fileStatus("shops.csv", shopsCSV),
            fileStatus("ambulances.csv", ambulancesCSV),
            text(""),
        }) | border | color(Color::White) | size(WIDTH, EQUAL, 50);

        auto optionsBox = vbox({
            text(""),
            text("  SELECT MODE") | bold,
            text(""),
            separator(),
            text(""),
            vbox(menuItems),
            text(""),
            separator(),
            text(""),
            text("  Demo Mode: Uses sample data") | dim,
            text("  Full Mode: Loads all CSVs") | dim,
            text(""),
        }) | border | color(Color::White) | size(WIDTH, EQUAL, 50);

        return vbox({
            filler(),
            hbox({
                filler(),
                vbox({
                    text("  INITIALIZE CITY") | bold | center,
                    text("") | size(HEIGHT, EQUAL, 1),
                    filesBox,
                    text("") | size(HEIGHT, EQUAL, 1),
                    optionsBox,
                }),
                filler(),
            }),
            filler(),
            text("Up/Down: Navigate | Enter: Select | Esc: Back") | center | dim,
            text("") | size(HEIGHT, EQUAL, 1),
        });
    });

    auto component = CatchEvent(renderer, [&](Event event) {
        if (event == Event::ArrowUp) {
            selected = (selected - 1 + options.size()) % options.size();
            screen.PostEvent(Event::Custom);
            return true;
        }
        if (event == Event::ArrowDown) {
            selected = (selected + 1) % options.size();
            screen.PostEvent(Event::Custom);
            return true;
        }
        if (event == Event::Return) {
            if (selected == 2) {
                currentState = SimulatorState::MAIN_MENU;
            } else {
                loadMode = (selected == 0) ? CSVLoadMode::DEMO_MODE : CSVLoadMode::FULL_MODE;
                currentState = SimulatorState::LOADING;
            }
            screen.Exit();
            return true;
        }
        if (event == Event::Escape) {
            currentState = SimulatorState::MAIN_MENU;
            screen.Exit();
            return true;
        }
        return false;
    });

    screen.Loop(component);
}

// ============================================================================
// LOADING SCREEN
// ============================================================================

inline void CitySimulator::runLoadingScreen() {
    auto screen = ScreenInteractive::Fullscreen();

    std::vector<string> steps = {
        "Initializing infrastructure...",
        "Loading geographic data...",
        "Building sector grid...",
        "Loading bus stops...",
        "Loading schools...",
        "Loading hospitals...",
        "Setting up transport...",
        "Loading routes...",
        "Loading population...",
        "Connecting nodes...",
        "Finalizing...",
        "Complete!"
    };

    std::atomic<int> currentStep{ 0 };
    std::atomic<bool> done{ false };

    auto renderer = Renderer([&] {
        int step = currentStep.load();
        float progress = (float)step / (float)(steps.size() - 1);
        int barWidth = 40;
        int filled = (int)(progress * barWidth);

        string bar = "[";
        for (int i = 0; i < barWidth; i++) {
            bar += (i < filled) ? "#" : "-";
        }
        bar += "]";

        string statusMsg = step < (int)steps.size() ? steps[step] : "Complete!";
        string hint = done.load() ? "Press Enter to continue" : "";

        auto loadingBox = vbox({
            text(""),
            text("  LOADING") | bold | center,
            text(""),
            separator(),
            text(""),
            text("  " + bar) | center,
            text("  " + std::to_string((int)(progress * 100)) + "%") | center,
            text(""),
            text("  " + statusMsg) | center | dim,
            text(""),
            text("  " + hint) | center,
            text(""),
        }) | border | color(Color::White) | size(WIDTH, EQUAL, 55);

        return vbox({
            filler(),
            hbox({ filler(), loadingBox, filler() }),
            filler(),
        });
    });

    std::thread loader([&]() {
        islamabad = new SmartCity();

        for (int i = 0; i < (int)steps.size(); i++) {
            currentStep.store(i);
            screen.PostEvent(Event::Custom);

            if (i == 2) {
                islamabad->setDatasetPaths(
                    stopsCSV, schoolsCSV, hospitalsCSV, pharmaciesCSV,
                    busesCSV, populationCSV, mallsCSV, shopsCSV, ambulancesCSV
                );
            }

            if (i == 6) {
                islamabad->initialize();
                cityInitialized = true;
            }

            sleepMs(250);
        }

        done.store(true);
        screen.PostEvent(Event::Custom);
    });

    auto component = CatchEvent(renderer, [&](Event event) {
        if (event == Event::Return && done.load()) {
            currentState = SimulatorState::MAIN_MENU;
            screen.Exit();
            return true;
        }
        return false;
    });

    screen.Loop(component);
    if (loader.joinable()) loader.join();
}

// ============================================================================
// INTERACTIVE GRAPH VIEW
// ============================================================================

inline void CitySimulator::runGraphView() {
    auto screen = ScreenInteractive::Fullscreen();

    buildGraphVisualization();

    auto renderer = Renderer([&] {
        int termWidth = Terminal::Size().dimx;
        int termHeight = Terminal::Size().dimy;

        int canvasWidth = termWidth - 35;
        int canvasHeight = termHeight - 8;

        Canvas graphCanvas = renderGraphToCanvas(canvasWidth, canvasHeight);

        CityStats stats;
        if (islamabad) stats = islamabad->getCityStats();

        string hoverText = getHoverInfo();

        auto infoBox = vbox({
            text("  INFO") | bold,
            separator(),
            text(""),
            paragraph(hoverText),
            text(""),
            separator(),
            text(""),
            text("  Nodes: " + std::to_string(stats.totalNodes)),
            text("  Edges: " + std::to_string(graphEdges.size())),
            text("  Zoom: " + std::to_string((int)(viewport.getZoom() * 100)) + "%"),
            text(""),
            separator(),
            text(""),
            text("  CONTROLS") | bold,
            text(""),
            text("  +/-: Zoom"),
            text("  Arrows: Pan"),
            text("  C: Corners"),
            text("  R: Roads"),
            text("  0: Reset"),
            text(""),
            text("  Esc: Back"),
        }) | border | color(Color::White) | size(WIDTH, EQUAL, 28);

        return vbox({
            text("  CITY GRAPH VISUALIZATION") | bold | center,
            text("") | size(HEIGHT, EQUAL, 1),
            hbox({
                canvas(graphCanvas) | border | color(Color::White) | flex,
                text(" "),
                infoBox,
            }) | flex,
        });
    });

    auto component = CatchEvent(renderer, [&](Event event) {
        if (event.is_mouse()) {
            if (event.mouse().motion == Mouse::Moved) {
                updateHoverState(event.mouse().x, event.mouse().y);
                screen.PostEvent(Event::Custom);
                return true;
            }
        }

        if (event == Event::Character('+') || event == Event::Character('=')) {
            viewport.zoomIn();
            screen.PostEvent(Event::Custom);
            return true;
        }
        if (event == Event::Character('-') || event == Event::Character('_')) {
            viewport.zoomOut();
            screen.PostEvent(Event::Custom);
            return true;
        }
        if (event == Event::ArrowLeft) { viewport.panLeft(); screen.PostEvent(Event::Custom); return true; }
        if (event == Event::ArrowRight) { viewport.panRight(); screen.PostEvent(Event::Custom); return true; }
        if (event == Event::ArrowUp) { viewport.panUp(); screen.PostEvent(Event::Custom); return true; }
        if (event == Event::ArrowDown) { viewport.panDown(); screen.PostEvent(Event::Custom); return true; }

        if (event == Event::Character('c') || event == Event::Character('C')) {
            showCorners = !showCorners;
            screen.PostEvent(Event::Custom);
            return true;
        }
        if (event == Event::Character('r') || event == Event::Character('R')) {
            showRoads = !showRoads;
            screen.PostEvent(Event::Custom);
            return true;
        }
        if (event == Event::Character('0')) {
            viewport.resetView();
            screen.PostEvent(Event::Custom);
            return true;
        }
        if (event == Event::Escape) {
            currentState = SimulatorState::MAIN_MENU;
            screen.Exit();
            return true;
        }
        return false;
    });

    screen.Loop(component);
}

// ============================================================================
// DATABASE VIEW
// ============================================================================

inline void CitySimulator::runDatabaseView() {
    auto screen = ScreenInteractive::Fullscreen();

    std::vector<string> categories = {
        "Sectors", "Graph Nodes", "Schools", "Hospitals",
        "Pharmacies", "Buses", "School Buses", "Ambulances",
        "Citizens", "Malls", "Shops", "Back to Menu"
    };

    int selected = 0;

    auto renderer = Renderer([&] {
        CityStats stats;
        if (islamabad) stats = islamabad->getCityStats();

        std::vector<string> counts = {
            std::to_string(SECTOR_COUNT),
            std::to_string(stats.totalNodes),
            std::to_string(stats.totalSchools),
            std::to_string(stats.totalHospitals),
            std::to_string(stats.totalPharmacies),
            std::to_string(stats.totalBuses),
            std::to_string(stats.totalSchoolBuses),
            std::to_string(stats.totalAmbulances),
            std::to_string(stats.totalCitizens),
            std::to_string(stats.totalMalls),
            "N/A",
            ""
        };

        Elements menuItems;
        for (int i = 0; i < (int)categories.size(); i++) {
            string prefix = (i == selected) ? " > " : "   ";
            string countStr = counts[i].empty() ? "" : " (" + counts[i] + ")";
            menuItems.push_back(text(prefix + categories[i] + countStr) | color(Color::White));
        }

        auto menuBox = vbox({
            text(""),
            text("  DATABASE BROWSER") | bold,
            text(""),
            separator(),
            text(""),
            vbox(menuItems),
            text(""),
        }) | border | color(Color::White) | size(WIDTH, EQUAL, 45);

        auto statsBox = vbox({
            text(""),
            text("  QUICK STATS") | bold,
            text(""),
            separator(),
            text(""),
            text("  Infrastructure:"),
            text("    Nodes: " + std::to_string(stats.totalNodes)),
            text("    Sectors: " + std::to_string(SECTOR_COUNT)),
            text(""),
            text("  Education:"),
            text("    Schools: " + std::to_string(stats.totalSchools)),
            text(""),
            text("  Healthcare:"),
            text("    Hospitals: " + std::to_string(stats.totalHospitals)),
            text("    Pharmacies: " + std::to_string(stats.totalPharmacies)),
            text(""),
            text("  Transport:"),
            text("    Buses: " + std::to_string(stats.totalBuses)),
            text("    Active: " + std::to_string(stats.activeBuses)),
            text(""),
        }) | border | color(Color::White) | size(WIDTH, EQUAL, 35);

        return vbox({
            filler(),
            hbox({
                filler(),
                menuBox,
                text("  "),
                statsBox,
                filler(),
            }),
            filler(),
            text("Up/Down: Navigate | Enter: Select | Esc: Back") | center | dim,
            text("") | size(HEIGHT, EQUAL, 1),
        });
    });

    auto component = CatchEvent(renderer, [&](Event event) {
        if (event == Event::ArrowUp) {
            selected = (selected - 1 + categories.size()) % categories.size();
            screen.PostEvent(Event::Custom);
            return true;
        }
        if (event == Event::ArrowDown) {
            selected = (selected + 1) % categories.size();
            screen.PostEvent(Event::Custom);
            return true;
        }
        if (event == Event::Return) {
            if (selected == (int)categories.size() - 1) {
                currentState = SimulatorState::MAIN_MENU;
                screen.Exit();
            }
            return true;
        }
        if (event == Event::Escape) {
            currentState = SimulatorState::MAIN_MENU;
            screen.Exit();
            return true;
        }
        return false;
    });

    screen.Loop(component);
}

// ============================================================================
// DEBUG MODE - Bypass intro and go straight to graph view
// ============================================================================

inline void CitySimulator::runDebugMode() {
    std::cout << "=== DEBUG MODE ===" << std::endl;
    std::cout << "Loading city data directly..." << std::endl;

    // Create SmartCity and load data
    islamabad = new SmartCity();
    islamabad->setDatasetPaths(
        stopsCSV, schoolsCSV, hospitalsCSV, pharmaciesCSV,
        busesCSV, populationCSV, mallsCSV, shopsCSV, ambulancesCSV
    );
    
    std::cout << "Initializing..." << std::endl;
    islamabad->initialize();
    cityInitialized = true;

    // Print debug info
    CityStats stats = islamabad->getCityStats();
    std::cout << "Loaded:" << std::endl;
    std::cout << "  Nodes: " << stats.totalNodes << std::endl;
    std::cout << "  Schools: " << stats.totalSchools << std::endl;
    std::cout << "  Hospitals: " << stats.totalHospitals << std::endl;
    std::cout << "  Buses: " << stats.totalBuses << std::endl;
    
    // Print some node info for debugging
    CityGraph* graph = islamabad->getCityGraph();
    if (graph) {
        std::cout << "\nFirst 20 nodes:" << std::endl;
        for (int i = 0; i < std::min(20, graph->getNodeCount()); i++) {
            CityNode* node = graph->getNode(i);
            if (node) {
                std::cout << "  [" << i << "] " << node->name 
                          << " (" << node->type << ") "
                          << "lat=" << node->lat << " lon=" << node->lon
                          << " sector=" << node->sector << std::endl;
            }
        }
    }

    std::cout << "\nPress Enter to open graph view..." << std::endl;
    std::cin.get();

    // Go directly to graph view
    runGraphView();

    std::cout << "\nDebug session complete." << std::endl;
}

// ============================================================================
// MAIN LOOP
// ============================================================================

inline void CitySimulator::run() {
    while (currentState != SimulatorState::EXIT) {
        switch (currentState) {
        case SimulatorState::INTRO_PHASE_1:
            runIntroPhase1();
            break;
        case SimulatorState::INTRO_PHASE_2:
            runIntroPhase2();
            break;
        case SimulatorState::INTRO_PHASE_3:
            runIntroPhase3();
            break;
        case SimulatorState::WELCOME_ANIMATION:
            runWelcomeAnimation();
            break;
        case SimulatorState::MAIN_MENU:
            runMainMenu();
            break;
        case SimulatorState::CSV_SELECTION:
            runCSVSelection();
            break;
        case SimulatorState::LOADING:
            runLoadingScreen();
            break;
        case SimulatorState::GRAPH_VIEW:
            runGraphView();
            break;
        case SimulatorState::DATABASE_VIEW:
            runDatabaseView();
            break;
        case SimulatorState::EXIT:
            break;
        }
    }

    std::cout << "\nThank you for using Islamabad Redefined!\n" << std::endl;
}

#endif // CITY_SIMULATOR_ENHANCED_H

