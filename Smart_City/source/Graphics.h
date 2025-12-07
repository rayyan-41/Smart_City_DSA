#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <limits>
#include <list>
#include <fstream>
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

    // --- CORRECTED DRAW TEXT FUNCTION ---
    void DrawText(int x, int y, const std::string& text, Color fg = { 255, 255, 255 }) {
        // Standard ASCII 5x7 Font (Columns are bytes)
        // Space (32) to Tilde (126)
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
            // Map character to font index (32 = Space is index 0)
            int index = ch - 32;
            if (index < 0 || index >= 95) index = 31; // Default to ? if invalid

            const uint8_t* glyph = font[index];

            for (int col = 0; col < 5; col++) {
                uint8_t line = glyph[col];
                for (int row = 0; row < 7; row++) {
                    // TRANSPARENCY FIX:
                    // Only draw if the bit is 1. If 0, skip (don't draw black).
                    if (line & 0x01) {
                        DrawPixel(startX + col, y + row, fg);
                    }
                    line >>= 1;
                }
            }
            startX += 6; // 5 pixel width + 1 pixel spacing
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


class AutoScalingCityGraph {
private:
    struct Node {
        int id;
        float logicX, logicY; // The ACTUAL map coordinates (e.g. Latitude/Longitude or Meters)
        std::string name;
    };

    struct Edge {
        int toNodeID;
        int trafficWeight;
    };

    std::vector<Node> nodes;
    std::vector<std::vector<Edge>> adjList;
    SquarePixelEngine& engine;

    // Bounds for normalization
    float minX, maxX, minY, maxY;

    // Dynamic sizing factors
    int screenW, screenH;

public:
    AutoScalingCityGraph(SquarePixelEngine& eng, int w, int h)
        : engine(eng), screenW(w), screenH(h) {
        // Initialize bounds to inverse extremes so they adapt to first data point
        minX = std::numeric_limits<float>::max();
        maxX = std::numeric_limits<float>::min();
        minY = std::numeric_limits<float>::max();
        maxY = std::numeric_limits<float>::min();
    }

    // 1. Add Data in "Map Units" (Any range you want)
    void AddNode(int id, float mapX, float mapY, std::string name = "") {
        if (id >= nodes.size()) {
            nodes.resize(id + 1);
            adjList.resize(id + 1);
        }
        nodes[id] = { id, mapX, mapY, name };

        // Update bounds automatically
        if (mapX < minX) minX = mapX;
        if (mapX > maxX) maxX = mapX;
        if (mapY < minY) minY = mapY;
        if (mapY > maxY) maxY = mapY;
    }

    void AddRoad(int src, int dest, int traffic) {
        adjList[src].push_back({ dest, traffic });
        adjList[dest].push_back({ src, traffic });
    }

    // 2. The Magic: Convert Logic -> Screen
    void Draw() {
        // Safety: If only 1 node or flat graph, prevent divide by zero
        if (maxX == minX) maxX += 1.0f;
        if (maxY == minY) maxY += 1.0f;

        // CALCULATE DYNAMIC SIZES based on Resolution
        // Node = 2% of screen width (Minimum 3 pixels)
        int nodeSize = (int)(screenW * 0.02f);
        if (nodeSize < 3) nodeSize = 3;

        // Road = 25% of node size (Minimum 1 pixel)
        int roadThick = nodeSize / 4;
        if (roadThick < 1) roadThick = 1;

        // Draw Roads First
        for (int i = 0; i < nodes.size(); i++) {
            // Convert start node to screen coords
            int sx = Normalize(nodes[i].logicX, minX, maxX, screenW);
            int sy = Normalize(nodes[i].logicY, minY, maxY, screenH);

            for (const auto& edge : adjList[i]) {
                if (i < edge.toNodeID) {
                    // Convert end node
                    int ex = Normalize(nodes[edge.toNodeID].logicX, minX, maxX, screenW);
                    int ey = Normalize(nodes[edge.toNodeID].logicY, minY, maxY, screenH);

                    Color c = GetTrafficColor(edge.trafficWeight);
                    engine.DrawThickLine(sx, sy, ex, ey, roadThick, c);
                }
            }
        }

        // Draw Intersections Second
        for (const auto& node : nodes) {
            int sx = Normalize(node.logicX, minX, maxX, screenW);
            int sy = Normalize(node.logicY, minY, maxY, screenH);

            // Draw Node (Blue Square)
            // Center the square by subtracting half size
            int half = nodeSize / 2;
            // Fill a square area
            for (int dy = -half; dy <= half; dy++) {
                for (int dx = -half; dx <= half; dx++) {
                    engine.DrawPixel(sx + dx, sy + dy, { 50, 50, 255 });
                }
            }
        }
    }

private:
    // Helper: Map value from [min, max] to [0, screenDim]
    int Normalize(float val, float min, float max, int dimension) {
        // Add 10% padding so nodes aren't stuck on the exact edge of the screen
        float padding = dimension * 0.05f;
        float usableDim = dimension - (2 * padding);

        float ratio = (val - min) / (max - min);
        return (int)(padding + (ratio * usableDim));
    }

    Color GetTrafficColor(int w) {
        if (w <= 3) return { 50, 255, 50 };   // Green
        if (w <= 7) return { 255, 165, 0 };   // Orange
        return { 255, 0, 0 };                 // Red
    }
};


// Add this to your project
class CityMap {
private:
    SquarePixelEngine& engine;
    float scaleX, scaleY;

public:
    CityMap(SquarePixelEngine& eng, int mapMaxX, int mapMaxY) : engine(eng) {
        // Calculate scaling factor based on current terminal resolution
        // e.g. If map is 1000 units wide, but screen is 500 pixels wide -> scale is 0.5
        // We use hardcoded screen dimensions from the engine usually
        scaleX = 1.0f; // Simplified for now
        scaleY = 1.0f;
    }

    // Convert City Node (e.g., Node 5 at x:500, y:500) to Screen Pixel
    void DrawRoad(int x1, int y1, int x2, int y2, int congestionLevel) {
        Color roadColor;
        if (congestionLevel < 3) roadColor = { 100, 100, 100 };      // Grey (Clear)
        else if (congestionLevel < 7) roadColor = { 255, 165, 0 };   // Orange (Busy)
        else roadColor = { 255, 0, 0 };                              // Red (Jam)

        engine.DrawLine(x1, y1, x2, y2, roadColor);
    }

    void DrawHospital(int x, int y, int size) {
        // 1. Define Colors
        Color wallColor = { 200, 200, 200 }; // Light Grey
        Color crossColor = { 255, 0, 0 }; // White

        // 2. Draw the Square Base (The Ward)
        // 'y' is the top-left corner of the WALL.
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                // Using 'this->' assumes this function is inside your CityMap/Engine class
                engine.DrawPixel(x + j, y + i, wallColor);
            }
        }

        // 4. Draw the White Cross (On the Roof)
        // We calculate the visual center of the roof area
        int centerX = x + (size / 2);
        int centerY = y - (size / 2) + 1;

        // Safety: Only draw cross if the building is big enough (size >= 4)
        if (size >= 4) {
            int crossSize = size / 4;
            if (crossSize < 1) crossSize = 1;

            // Vertical bar of the cross
            for (int i = -crossSize; i <= crossSize; i++) {
                engine.DrawPixel(centerX, centerY + i, crossColor);
            }

            // Horizontal bar of the cross
            for (int i = -crossSize; i <= crossSize; i++) {
                engine.DrawPixel(centerX + i, centerY, crossColor);
            }
        }
    }
};


class CityVisualizer {
private:
    CityGraph& graph;
    SquarePixelEngine& engine;
    double scaleFactor;
    int offsetX, offsetY;

public:
    CityVisualizer(CityGraph& g, SquarePixelEngine& e) : graph(g), engine(e), scaleFactor(1.0), offsetX(0), offsetY(0) {}

    void CalculateBounds(int width, int height) {
        // Auto-fit the logical grid to the screen
        // Logical Grid is roughly: 12 Cols * 200px wide, 5 Rows * 150px high
        double mapWidth = GridUtils::MAX_SECTOR_NUM * (GridUtils::CELL_WIDTH + GridUtils::PADDING) + 200;
        double mapHeight = 5 * (GridUtils::CELL_HEIGHT + GridUtils::PADDING) + 200;

        double scaleX = width / mapWidth;
        double scaleY = height / mapHeight;

        scaleFactor = (scaleX < scaleY) ? scaleX : scaleY;

        // Center the map
        offsetX = (width - (mapWidth * scaleFactor)) / 2;
        offsetY = (height - (mapHeight * scaleFactor)) / 2;
    }

    struct Point { int x, y; };

    Point Project(double x, double y) {
        return {
            (int)(x * scaleFactor) + offsetX,
            (int)(y * scaleFactor) + offsetY
        };
    }

    void Draw() {
        int count = graph.getNodeCount();

        // 1. Draw Roads
        for (int i = 0; i < count; i++) {
            CityNode* n1 = graph.getNode(i);
            if (!n1) continue;
            Point p1 = Project(n1->lat, n1->lon); // using grid coords stored in lat/lon

            const auto& roads = n1->getRoads();
            for (int j = 0; j < roads.size(); j++) {
                // Accessing list manually or via helper
                // Assuming LinkedList exposes data via iteration or index
                // For simplicity here, assuming we can iterate edges
                // If LinkedList doesn't support index, use iterator logic
                // [Adapting to LinkedList structure from prompt]
                auto* curr = roads.getHead();
                while (curr) {
                    Edge edge = curr->data;
                    if (n1->id < edge.destinationID) { // Draw once
                        CityNode* n2 = graph.getNode(edge.destinationID);
                        if (n2) {
                            Point p2 = Project(n2->lat, n2->lon);
                            engine.DrawLine(p1.x, p1.y, p2.x, p2.y, { 100, 100, 100 });
                        }
                    }
                    curr = curr->next;
                }
            }
        }

        // 2. Draw Nodes
        for (int i = 0; i < count; i++) {
            CityNode* n = graph.getNode(i);
            if (!n) continue;
            Point p = Project(n->lat, n->lon);

            Color c = { 50, 255, 50 }; // Default Green
            int size = 3;

            if (n->type == "CORNER") { c = { 100,100,100 }; size = 2; }
            else if (n->type == "HOSPITAL") { c = { 255,0,0 }; size = 5; }
            else if (n->type == "SCHOOL") { c = { 255,255,0 }; size = 5; }
            else if (n->type == "STOP") { c = { 0,200,255 }; size = 4; }

            engine.DrawRect(p.x, p.y, size, c);

            if (n->type != "CORNER") {
                // Simple label
                // engine.DrawText(p.x, p.y - 5, n->name, {200,200,200}); 
            }
        }
    }
};