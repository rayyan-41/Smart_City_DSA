#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <limits>
#include <list>
#include <fstream>
#include <chrono>
#include <thread>
#include "../CityGraph.h"
using namespace std;


struct Color {
    uint8_t r, g, b;
    bool operator==(const Color& o) const { return r == o.r && g == o.g && b == o.b; }
    bool operator!=(const Color& o) const { return !(*this == o); }
};

class SquarePixelEngine {
private:
    int width, height;
    std::vector<Color> buffer;

public:
    SquarePixelEngine(int w, int h) : width(w), height(h) {
        buffer.resize(w * h, { 0, 0, 0 });
        std::cout << "\033[?25l"; // Hide Cursor
    }

    void Clear(Color c = { 0, 0, 0 }) {
        std::fill(buffer.begin(), buffer.end(), c);
    }

    void DrawPixel(int x, int y, Color c) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            buffer[y * width + x] = c;
        }
    }

    void DrawLine(int x0, int y0, int x1, int y1, Color c) {
        int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2;

        while (true) {
            DrawPixel(x0, y0, c);
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }

    void DrawRect(int x, int y, int size, Color c) {
        int half = size / 2;
        for (int i = -half; i <= half; i++) {
            for (int j = -half; j <= half; j++) {
                DrawPixel(x + j, y + i, c);
            }
        }
    }

    void DrawThickLine(int x0, int y0, int x1, int y1, int thickness, Color c) {
        int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2;
        while (true) {
            DrawRect(x0, y0, thickness, c);
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }

    void DrawText(int x, int y, const std::string& text, Color fg = { 255, 255, 255 }) {
        static const uint8_t font[][5] = {
            {0x00, 0x00, 0x00, 0x00, 0x00}, {0x00, 0x00, 0x5F, 0x00, 0x00}, {0x00, 0x07, 0x00, 0x07, 0x00},
            {0x14, 0x7F, 0x14, 0x7F, 0x14}, {0x24, 0x2A, 0x7F, 0x2A, 0x12}, {0x23, 0x13, 0x08, 0x64, 0x62},
            {0x36, 0x49, 0x55, 0x22, 0x50}, {0x00, 0x05, 0x03, 0x00, 0x00}, {0x00, 0x1C, 0x22, 0x41, 0x00},
            {0x00, 0x41, 0x22, 0x1C, 0x00}, {0x14, 0x08, 0x3E, 0x08, 0x14}, {0x08, 0x08, 0x3E, 0x08, 0x08},
            {0x00, 0x50, 0x30, 0x00, 0x00}, {0x08, 0x08, 0x08, 0x08, 0x08}, {0x00, 0x60, 0x60, 0x00, 0x00},
            {0x20, 0x10, 0x08, 0x04, 0x02}, {0x3E, 0x51, 0x49, 0x45, 0x3E}, {0x00, 0x42, 0x7F, 0x40, 0x00},
            {0x42, 0x61, 0x51, 0x49, 0x46}, {0x21, 0x41, 0x45, 0x4B, 0x31}, {0x18, 0x14, 0x12, 0x7F, 0x10},
            {0x27, 0x45, 0x45, 0x45, 0x39}, {0x3C, 0x4A, 0x49, 0x49, 0x30}, {0x01, 0x71, 0x09, 0x05, 0x03},
            {0x36, 0x49, 0x49, 0x49, 0x36}, {0x06, 0x49, 0x49, 0x29, 0x1E}, {0x00, 0x36, 0x36, 0x00, 0x00},
            {0x00, 0x56, 0x36, 0x00, 0x00}, {0x08, 0x14, 0x22, 0x41, 0x00}, {0x14, 0x14, 0x14, 0x14, 0x14},
            {0x00, 0x41, 0x22, 0x14, 0x08}, {0x02, 0x01, 0x51, 0x09, 0x06}, {0x32, 0x49, 0x79, 0x41, 0x3E},
            {0x7E, 0x11, 0x11, 0x11, 0x7E}, {0x7F, 0x49, 0x49, 0x49, 0x36}, {0x3E, 0x41, 0x41, 0x41, 0x22},
            {0x7F, 0x41, 0x41, 0x22, 0x1C}, {0x7F, 0x49, 0x49, 0x49, 0x41}, {0x7F, 0x09, 0x09, 0x09, 0x01},
            {0x3E, 0x41, 0x49, 0x49, 0x7A}, {0x7F, 0x08, 0x08, 0x08, 0x7F}, {0x00, 0x41, 0x7F, 0x41, 0x00},
            {0x20, 0x40, 0x41, 0x3F, 0x01}, {0x7F, 0x08, 0x14, 0x22, 0x41}, {0x7F, 0x40, 0x40, 0x40, 0x40},
            {0x7F, 0x02, 0x0C, 0x02, 0x7F}, {0x7F, 0x04, 0x08, 0x10, 0x7F}, {0x3E, 0x41, 0x41, 0x41, 0x3E},
            {0x7F, 0x09, 0x09, 0x09, 0x06}, {0x3E, 0x41, 0x51, 0x21, 0x5E}, {0x7F, 0x09, 0x19, 0x29, 0x46},
            {0x46, 0x49, 0x49, 0x49, 0x31}, {0x01, 0x01, 0x7F, 0x01, 0x01}, {0x3F, 0x40, 0x40, 0x40, 0x3F},
            {0x1F, 0x20, 0x40, 0x20, 0x1F}, {0x3F, 0x40, 0x38, 0x40, 0x3F}, {0x63, 0x14, 0x08, 0x14, 0x63},
            {0x07, 0x08, 0x70, 0x08, 0x07}, {0x61, 0x51, 0x49, 0x45, 0x43}, {0x00, 0x7F, 0x41, 0x41, 0x00},
            {0x02, 0x04, 0x08, 0x10, 0x20}, {0x00, 0x41, 0x41, 0x7F, 0x00}, {0x04, 0x02, 0x01, 0x02, 0x04},
            {0x40, 0x40, 0x40, 0x40, 0x40}, {0x00, 0x01, 0x02, 0x04, 0x00}, {0x20, 0x54, 0x54, 0x54, 0x78},
            {0x7F, 0x48, 0x44, 0x44, 0x38}, {0x38, 0x44, 0x44, 0x44, 0x20}, {0x38, 0x44, 0x44, 0x48, 0x7F},
            {0x38, 0x54, 0x54, 0x54, 0x18}, {0x08, 0x7E, 0x09, 0x01, 0x02}, {0x0C, 0x52, 0x52, 0x52, 0x3E},
            {0x7F, 0x08, 0x04, 0x04, 0x78}, {0x00, 0x44, 0x7D, 0x40, 0x00}, {0x20, 0x40, 0x44, 0x3D, 0x00},
            {0x7F, 0x10, 0x28, 0x44, 0x00}, {0x00, 0x41, 0x7F, 0x40, 0x00}, {0x7C, 0x04, 0x18, 0x04, 0x78},
            {0x7C, 0x08, 0x04, 0x04, 0x78}, {0x38, 0x44, 0x44, 0x44, 0x38}, {0xFC, 0x24, 0x24, 0x24, 0x18},
            {0x18, 0x24, 0x24, 0x18, 0xFC}, {0x7C, 0x08, 0x04, 0x04, 0x08}, {0x48, 0x54, 0x54, 0x54, 0x20},
            {0x04, 0x3F, 0x44, 0x40, 0x20}, {0x3C, 0x40, 0x40, 0x20, 0x7C}, {0x1C, 0x20, 0x40, 0x20, 0x1C},
            {0x3C, 0x40, 0x30, 0x40, 0x3C}, {0x44, 0x28, 0x10, 0x28, 0x44}, {0x0C, 0x50, 0x50, 0x50, 0x3C},
            {0x44, 0x64, 0x54, 0x4C, 0x44}, {0x00, 0x08, 0x36, 0x41, 0x00}, {0x00, 0x00, 0x7F, 0x00, 0x00},
            {0x00, 0x41, 0x36, 0x08, 0x00}, {0x10, 0x08, 0x08, 0x10, 0x08}
        };

        int startX = x;
        for (char ch : text) {
            int index = ch - 32;
            if (index < 0 || index >= 95) index = 31;

            const uint8_t* glyph = font[index];

            for (int col = 0; col < 5; col++) {
                uint8_t line = glyph[col];
                for (int row = 0; row < 7; row++) {
                    if (line & 0x01) {
                        DrawPixel(startX + col, y + row, fg);
                    }
                    line >>= 1;
                }
            }
            startX += 6;
        }
    }

    void Display() {
        std::cout << "\033[H";
        std::string frame;
        frame.reserve(width * height * 15);
        Color lastColor = { 255, 255, 255 };

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                Color current = buffer[y * width + x];
                if (current != lastColor) {
                    frame += "\033[48;2;" + std::to_string(current.r) + ";" +
                        std::to_string(current.g) + ";" + std::to_string(current.b) + "m";
                    lastColor = current;
                }
                frame += "  ";
            }
            frame += "\033[0m\n";
            lastColor = { 0,0,0 };
        }
        std::cout << frame << std::flush;
    }
};


// Simple Grid-Based City Visualizer (No rotation or complex normalization)
class CityVisualizer {
private:
    CityGraph& graph;
    SquarePixelEngine& engine;

    int screenW, screenH;

    // Simple bounds from lat/lon
    double minLat, maxLat, minLon, maxLon;

    // Node visualization states
    std::vector<Color> nodeColors;
    std::vector<bool> isHighlighted;

public:
    CityVisualizer(CityGraph& g, SquarePixelEngine& e)
        : graph(g), engine(e) {

        int count = graph.getNodeCount();
        nodeColors.resize(count, { 50, 50, 255 }); // Default blue
        isHighlighted.resize(count, false);
    }

    void CalculateBounds(int width, int height) {
        screenW = width;
        screenH = height;

        // Get bounds directly from CityUtils - these form a perfect grid
        minLat = BASE_LAT;  // 33.60
        maxLat = MAX_LAT;   // 33.74
        minLon = BASE_LON;  // 72.96
        maxLon = MAX_LON;   // 73.10
    }

    struct Point { int x; int y; };

    // Simple linear mapping: lat/lon -> screen coordinates
    // North (high lat) = top of screen (low y)
    // East (high lon) = right of screen (high x)
    Point Project(double lat, double lon) {
        // Add padding
        int padding = 20;
        int usableWidth = screenW - (2 * padding);
        int usableHeight = screenH - (2 * padding);

        // Normalize lat/lon to [0, 1]
        double normLon = (lon - minLon) / (maxLon - minLon);
        double normLat = (lat - minLat) / (maxLat - minLat);

        // Map to screen space
        // X: West (minLon) = left, East (maxLon) = right
        int x = padding + (int)(normLon * usableWidth);

        // Y: North (maxLat) = top (low y), South (minLat) = bottom (high y)
        int y = padding + (int)((1.0 - normLat) * usableHeight);

        return { x, y };
    }

    // Set node color for visualization
    void SetNodeColor(int nodeID, Color c) {
        if (nodeID >= 0 && nodeID < nodeColors.size()) {
            nodeColors[nodeID] = c;
        }
    }

    // Highlight a node
    void HighlightNode(int nodeID, bool highlight = true) {
        if (nodeID >= 0 && nodeID < isHighlighted.size()) {
            isHighlighted[nodeID] = highlight;
        }
    }

    // Reset all visualization states with comprehensive color coding
    void ResetVisualization() {
        int count = graph.getNodeCount();
        for (int i = 0; i < count; i++) {
            CityNode* n = graph.getNode(i);
            if (!n) continue;

            // Comprehensive color coding for all facility types
            if (n->type == "HOSPITAL") nodeColors[i] = { 255, 0, 0 };          // Red
            else if (n->type == "SCHOOL") nodeColors[i] = { 255, 255, 0 };     // Yellow
            else if (n->type == "STOP") nodeColors[i] = { 0, 200, 255 };       // Cyan
            else if (n->type == "CORNER") nodeColors[i] = { 60, 60, 60 };      // Dark Gray
            else if (n->type == "PHARMACY") nodeColors[i] = { 0, 255, 100 };   // Green
            else if (n->type == "MALL") nodeColors[i] = { 255, 0, 255 };       // Magenta
            else if (n->type == "SHOP") nodeColors[i] = { 255, 100, 200 };     // Pink

            // Public Facilities
            else if (n->type == "MOSQUE") nodeColors[i] = { 200, 255, 200 };   // Light Green
            else if (n->type == "PARK") nodeColors[i] = { 50, 200, 50 };       // Dark Green
            else if (n->type == "WATER_COOLER") nodeColors[i] = { 100, 150, 255 }; // Light Blue
            else if (n->type == "PLAYGROUND") nodeColors[i] = { 255, 200, 100 };   // Orange
            else if (n->type == "LIBRARY") nodeColors[i] = { 150, 100, 255 };      // Purple
            else if (n->type == "COMMUNITY_CENTER") nodeColors[i] = { 200, 150, 100 }; // Brown
            else if (n->type == "POLICE_STATION") nodeColors[i] = { 0, 0, 200 };   // Dark Blue
            else if (n->type == "FIRE_STATION") nodeColors[i] = { 200, 0, 0 };     // Dark Red
            else if (n->type == "POST_OFFICE") nodeColors[i] = { 100, 100, 150 };  // Gray Blue
            else if (n->type == "BANK") nodeColors[i] = { 255, 215, 0 };           // Gold
            else if (n->type == "ATM") nodeColors[i] = { 200, 200, 0 };            // Dark Yellow
            else if (n->type == "PETROL_STATION") nodeColors[i] = { 255, 50, 50 }; // Bright Red
            else if (n->type == "RESTAURANT") nodeColors[i] = { 255, 150, 50 };    // Orange Red
            else if (n->type == "PUBLIC_TOILET") nodeColors[i] = { 150, 150, 150 }; // Gray

            else nodeColors[i] = { 100, 100, 255 }; // Default Blue for unknown types

            isHighlighted[i] = false;
        }
    }

    // Get color for a specific node type (helper for legend)
    static Color GetTypeColor(const string& type) {
        if (type == "HOSPITAL") return { 255, 0, 0 };
        else if (type == "SCHOOL") return { 255, 255, 0 };
        else if (type == "STOP") return { 0, 200, 255 };
        else if (type == "PHARMACY") return { 0, 255, 100 };
        else if (type == "MALL") return { 255, 0, 255 };
        else if (type == "SHOP") return { 255, 100, 200 };
        else if (type == "MOSQUE") return { 200, 255, 200 };
        else if (type == "PARK") return { 50, 200, 50 };
        else if (type == "WATER_COOLER") return { 100, 150, 255 };
        else if (type == "PLAYGROUND") return { 255, 200, 100 };
        else if (type == "LIBRARY") return { 150, 100, 255 };
        else if (type == "COMMUNITY_CENTER") return { 200, 150, 100 };
        else if (type == "POLICE_STATION") return { 0, 0, 200 };
        else if (type == "FIRE_STATION") return { 200, 0, 0 };
        else if (type == "POST_OFFICE") return { 100, 100, 150 };
        else if (type == "BANK") return { 255, 215, 0 };
        else if (type == "ATM") return { 200, 200, 0 };
        else if (type == "PETROL_STATION") return { 255, 50, 50 };
        else if (type == "RESTAURANT") return { 255, 150, 50 };
        else if (type == "PUBLIC_TOILET") return { 150, 150, 150 };
        else return { 100, 100, 255 };
    }

    void Draw(bool showLegend = true) {
        int count = graph.getNodeCount();

        // Pass 1: Draw title
        engine.DrawText(10, 2, "ISLAMABAD SMART CITY (North=Top, East=Right)", { 200, 200, 200 });

        // Pass 2: Draw ALL Roads/Edges
        int totalEdges = 0;
        for (int i = 0; i < count; i++) {
            CityNode* n1 = graph.getNode(i);
            if (!n1) continue;
            Point p1 = Project(n1->lat, n1->lon);

            // Draw each edge from this node's adjacency list
            const LinkedList<Edge>& roads = n1->getRoads();
            for (int j = 0; j < roads.size(); j++) {
                Edge edge = roads[j];
                CityNode* n2 = graph.getNode(edge.destinationID);
                if (n2 && i < edge.destinationID) { // Draw each edge only once
                    Point p2 = Project(n2->lat, n2->lon);
                    totalEdges++;

                    // Color coding for different types of connections
                    Color roadColor = { 70, 70, 70 }; // Default gray

                    if (n1->type == "CORNER" && n2->type == "CORNER") {
                        roadColor = { 40, 40, 40 }; // Dark gray for sector boundaries
                    }
                    else if (edge.weight > 5.0) {
                        roadColor = { 255, 100, 0 }; // Orange for heavy traffic/long distance
                    }
                    else if (n1->type == "CORNER" || n2->type == "CORNER") {
                        roadColor = { 90, 90, 90 }; // Light gray for connections to corners
                    }
                    else {
                        roadColor = { 120, 120, 120 }; // Standard roads between facilities
                    }

                    engine.DrawLine(p1.x, p1.y, p2.x, p2.y, roadColor);
                }
            }
        }

        // Pass 3: Draw ALL Nodes (including corners for debugging)
        int visibleNodes = 0;
        for (int i = 0; i < count; i++) {
            CityNode* n = graph.getNode(i);
            if (!n) continue;
            Point p = Project(n->lat, n->lon);

            Color nodeColor = nodeColors[i];

            // Size based on node type and highlight status
            int size = 1;
            if (isHighlighted[i]) size = 3;
            else if (n->type == "HOSPITAL" || n->type == "SCHOOL" || n->type == "MALL") size = 2;
            else if (n->type == "POLICE_STATION" || n->type == "FIRE_STATION") size = 2;
            else if (n->type == "CORNER") size = 0; // Tiny dot for corners

            // Draw all nodes (even corners, but make them tiny)
            if (n->type == "CORNER") {
                // Draw tiny corner markers
                engine.DrawPixel(p.x, p.y, { 80, 80, 80 });
            }
            else {
                visibleNodes++;
                for (int dy = -size; dy <= size; dy++) {
                    for (int dx = -size; dx <= size; dx++) {
                        engine.DrawPixel(p.x + dx, p.y + dy, nodeColor);
                    }
                }

                // Draw text for highlighted nodes or major facilities
                if (isHighlighted[i] || n->type == "HOSPITAL" || n->type == "MALL") {
                    Color textColor = isHighlighted[i] ? Color{ 255, 255, 0 } : Color{ 255, 255, 255 };
                    string label = n->name;
                    if (label.length() > 10) label = label.substr(0, 10);
                    engine.DrawText(p.x + 4, p.y - 2, label, textColor);
                }
            }
        }

        // Pass 4: Draw legend and statistics
        if (showLegend) {
            DrawLegend();
        }

        // Draw graph statistics at bottom right
        string statsText = "Nodes:" + std::to_string(visibleNodes) + " Edges:" + std::to_string(totalEdges);
        engine.DrawText(screenW - 80, screenH - 10, statsText, { 200, 200, 200 });
    }

    // Draw color-coded legend with actual counts from graph
    void DrawLegend() {
        int startY = 10;
        int startX = 10;

        // Count each type and total edges
        std::map<string, int> typeCounts;
        int count = graph.getNodeCount();
        int totalEdges = 0;

        for (int i = 0; i < count; i++) {
            CityNode* n = graph.getNode(i);
            if (n) {
                typeCounts[n->type]++;
                // Count edges (each edge is counted once from both nodes, so divide by 2 later)
                totalEdges += n->getRoads().size();
            }
        }

        totalEdges /= 2; // Each edge counted twice

        // Display title
        engine.DrawText(startX, startY, "=== LEGEND ===", { 255, 255, 255 });
        int row = 2;

        // Draw legend for major types found in graph
        if (typeCounts["HOSPITAL"] > 0) {
            DrawLegendItem(startX, startY + row * 7, "Hospital", GetTypeColor("HOSPITAL"), typeCounts["HOSPITAL"]);
            row++;
        }
        if (typeCounts["SCHOOL"] > 0) {
            DrawLegendItem(startX, startY + row * 7, "School", GetTypeColor("SCHOOL"), typeCounts["SCHOOL"]);
            row++;
        }
        if (typeCounts["PHARMACY"] > 0) {
            DrawLegendItem(startX, startY + row * 7, "Pharmacy", GetTypeColor("PHARMACY"), typeCounts["PHARMACY"]);
            row++;
        }
        if (typeCounts["MALL"] > 0) {
            DrawLegendItem(startX, startY + row * 7, "Mall", GetTypeColor("MALL"), typeCounts["MALL"]);
            row++;
        }
        if (typeCounts["STOP"] > 0) {
            DrawLegendItem(startX, startY + row * 7, "Stop", GetTypeColor("STOP"), typeCounts["STOP"]);
            row++;
        }
        if (typeCounts["MOSQUE"] > 0) {
            DrawLegendItem(startX, startY + row * 7, "Mosque", GetTypeColor("MOSQUE"), typeCounts["MOSQUE"]);
            row++;
        }
        if (typeCounts["PARK"] > 0) {
            DrawLegendItem(startX, startY + row * 7, "Park", GetTypeColor("PARK"), typeCounts["PARK"]);
            row++;
        }
        if (typeCounts["POLICE_STATION"] > 0) {
            DrawLegendItem(startX, startY + row * 7, "Police", GetTypeColor("POLICE_STATION"), typeCounts["POLICE_STATION"]);
            row++;
        }
        if (typeCounts["FIRE_STATION"] > 0) {
            DrawLegendItem(startX, startY + row * 7, "Fire Stn", GetTypeColor("FIRE_STATION"), typeCounts["FIRE_STATION"]);
            row++;
        }
        if (typeCounts["BANK"] > 0) {
            DrawLegendItem(startX, startY + row * 7, "Bank", GetTypeColor("BANK"), typeCounts["BANK"]);
            row++;
        }
        if (typeCounts["ATM"] > 0) {
            DrawLegendItem(startX, startY + row * 7, "ATM", GetTypeColor("ATM"), typeCounts["ATM"]);
            row++;
        }
        if (typeCounts["RESTAURANT"] > 0) {
            DrawLegendItem(startX, startY + row * 7, "Restaurant", GetTypeColor("RESTAURANT"), typeCounts["RESTAURANT"]);
            row++;
        }

        // Show corner count (these form the sector grid)
        if (typeCounts["CORNER"] > 0) {
            row++;
            string cornerInfo = "Grid Corners: " + std::to_string(typeCounts["CORNER"]);
            engine.DrawText(startX, startY + row * 7, cornerInfo, { 150, 150, 150 });
        }
    }

    void DrawLegendItem(int x, int y, const string& label, Color color, int count) {
        // Draw colored square (3x3 pixels)
        for (int dy = 0; dy < 3; dy++) {
            for (int dx = 0; dx < 3; dx++) {
                engine.DrawPixel(x + dx, y + dy + 1, color);
            }
        }

        // Draw label with count (format: "Type: N")
        string text = label + ": " + std::to_string(count);
        engine.DrawText(x + 5, y, text, { 200, 200, 200 });
    }
};


// Dijkstra Visualizer - Shows pathfinding in real-time
class DijkstraVisualizer {
private:
    CityGraph& graph;
    CityVisualizer& visualizer;
    SquarePixelEngine& engine;
    int delayMs;

    // Colors for visualization
    Color COLOR_UNVISITED = { 50, 50, 255 };      // Blue
    Color COLOR_EXPLORING = { 0, 255, 0 };         // Green
    Color COLOR_VISITED = { 100, 100, 100 };       // Gray
    Color COLOR_PATH = { 255, 255, 0 };            // Yellow
    Color COLOR_START = { 255, 0, 255 };           // Magenta
    Color COLOR_END = { 255, 165, 0 };             // Orange

public:
    DijkstraVisualizer(CityGraph& g, CityVisualizer& v, SquarePixelEngine& e, int delay = 100)
        : graph(g), visualizer(v), engine(e), delayMs(delay) {
    }

    void SetDelay(int ms) { delayMs = ms; }

    // Visualize Dijkstra's algorithm step by step
    Vector<int> VisualizePath(int startID, int endID, double& totalDistance) {
        Vector<int> path;
        totalDistance = 0.0;

        int count = graph.getNodeCount();
        if (startID < 0 || startID >= count || endID < 0 || endID >= count) {
            return path;
        }

        // Reset visualization
        visualizer.ResetVisualization();

        // Mark start and end
        visualizer.SetNodeColor(startID, COLOR_START);
        visualizer.HighlightNode(startID, true);
        visualizer.SetNodeColor(endID, COLOR_END);
        visualizer.HighlightNode(endID, true);

        engine.Clear();
        visualizer.Draw();
        engine.Display();
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMs * 2));

        // Dijkstra's Algorithm with Visualization
        double distance[MAX_NODES];
        int parent[MAX_NODES];
        bool visited[MAX_NODES];

        for (int i = 0; i < MAX_NODES; i++) {
            distance[i] = INF;
            parent[i] = -1;
            visited[i] = false;
        }

        PriorityQueue<DijkstraNode> pq;
        distance[startID] = 0.0;
        pq.push(DijkstraNode(startID, 0.0));

        int nodesExplored = 0;

        while (!pq.empty()) {
            DijkstraNode current = pq.top();
            pq.pop();

            int u = current.nodeID;
            if (visited[u]) continue;

            nodesExplored++;

            // Visualize current node being explored
            if (u != startID && u != endID) {
                visualizer.SetNodeColor(u, COLOR_EXPLORING);
                visualizer.HighlightNode(u, true);
            }

            engine.Clear();
            visualizer.Draw();

            // Draw status text
            CityNode* node = graph.getNode(u);
            string status = "Exploring: " + (node ? node->name : "Node " + std::to_string(u));
            engine.DrawText(10, 12, status, { 0, 255, 0 });

            string stats = "Nodes explored: " + std::to_string(nodesExplored);
            engine.DrawText(10, 19, stats, { 255, 255, 255 });

            engine.Display();
            std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));

            visited[u] = true;

            // Mark as visited (unless it's start/end)
            if (u != startID && u != endID) {
                visualizer.SetNodeColor(u, COLOR_VISITED);
                visualizer.HighlightNode(u, false);
            }

            if (u == endID) break;

            if (u < 0 || u >= count || graph.getNode(u) == nullptr) continue;

            CityNode* uNode = graph.getNode(u);
            for (int i = 0; i < uNode->roads.size(); i++) {
                Edge edge = uNode->roads[i];
                int v = edge.destinationID;
                double weight = edge.weight;

                if (!visited[v] && distance[u] + weight < distance[v]) {
                    distance[v] = distance[u] + weight;
                    parent[v] = u;
                    pq.push(DijkstraNode(v, distance[v]));
                }
            }
        }

        // Reconstruct path
        if (parent[endID] != -1 || startID == endID) {
            int current = endID;
            while (current != -1) {
                path.push_back(current);
                current = parent[current];
            }

            // Reverse path
            for (int i = 0; i < path.getSize() / 2; i++) {
                int temp = path[i];
                path[i] = path[path.getSize() - 1 - i];
                path[path.getSize() - 1 - i] = temp;
            }

            totalDistance = distance[endID];

            // Visualize final path
            for (int i = 0; i < path.getSize(); i++) {
                int nodeID = path[i];
                if (nodeID != startID && nodeID != endID) {
                    visualizer.SetNodeColor(nodeID, COLOR_PATH);
                    visualizer.HighlightNode(nodeID, true);
                }
            }

            engine.Clear();
            visualizer.Draw();

            // Draw final status
            string finalStatus = "PATH FOUND! Distance: " + std::to_string((int)totalDistance) + " km";
            engine.DrawText(10, 12, finalStatus, { 0, 255, 0 });
            string pathInfo = "Path: " + std::to_string(path.getSize()) + " nodes | Explored: " + std::to_string(nodesExplored);
            engine.DrawText(10, 19, pathInfo, { 255, 255, 255 });

            engine.Display();
        }
        else {
            // No path found
            engine.Clear();
            visualizer.Draw();
            engine.DrawText(10, 12, "NO PATH FOUND!", { 255, 0, 0 });
            engine.Display();
        }

        return path;
    }
};
