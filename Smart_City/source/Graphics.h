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

    // Bounds for Auto-Scaling
    double minLat, maxLat, minLon, maxLon;
    double scaleFactor; // Pixels per degree
    int offsetX, offsetY;

public:
    CityVisualizer(CityGraph& g, SquarePixelEngine& e)
        : graph(g), engine(e) {
        ResetBounds();
    }

    void ResetBounds() {
        minLat = 90.0; maxLat = -90.0;
        minLon = 180.0; maxLon = -180.0;
    }

    // 1. Scan the graph to find the Map Boundaries
    void CalculateBounds(int screenWidth, int screenHeight) {
        ResetBounds();
        int count = graph.getNodeCount();

        if (count == 0) return;

        for (int i = 0; i < count; i++) {
            CityNode* n = graph.getNode(i);
            if (!n) continue;
            if (n->lat < minLat) minLat = n->lat;
            if (n->lat > maxLat) maxLat = n->lat;
            if (n->lon < minLon) minLon = n->lon;
            if (n->lon > maxLon) maxLon = n->lon;
        }

        // Add padding (approx 5%)
        double latRange = maxLat - minLat;
        double lonRange = maxLon - minLon;
        if (latRange == 0) latRange = 0.01; // Prevent div by zero
        if (lonRange == 0) lonRange = 0.01;

        minLat -= latRange * 0.05; maxLat += latRange * 0.05;
        minLon -= lonRange * 0.05; maxLon += lonRange * 0.05;

        // ASPECT RATIO CORRECTION
        // We want 1 degree of Lat to look proportional to 1 degree of Lon
        // Screen Y is inverted (0 is top), Lat is normal (90 is top)

        double latScale = screenHeight / (maxLat - minLat);
        double lonScale = screenWidth / (maxLon - minLon);

        // Pick the smaller scale to ensure it fits entirely
        scaleFactor = (latScale < lonScale) ? latScale : lonScale;

        // Center the map
        double mapPixelWidth = (maxLon - minLon) * scaleFactor;
        double mapPixelHeight = (maxLat - minLat) * scaleFactor;

        offsetX = (screenWidth - mapPixelWidth) / 2;
        offsetY = (screenHeight - mapPixelHeight) / 2;
    }

    // 2. Converters
    int LonToScreenX(double lon) {
        return offsetX + (int)((lon - minLon) * scaleFactor);
    }

    int LatToScreenY(double lat) {
        // INVERT LATITUDE: Higher Lat = Lower Y (Top of screen)
        return offsetY + (int)((maxLat - lat) * scaleFactor);
    }

    // 3. Render Loop
    void Draw() {
        int count = graph.getNodeCount();

        // Pass 1: Draw Roads (Edges)
        for (int i = 0; i < count; i++) {
            CityNode* n1 = graph.getNode(i);
            if (!n1) continue;

            int x1 = LonToScreenX(n1->lon);
            int y1 = LatToScreenY(n1->lat);

            // Access roads using your LinkedList logic
            // Assuming your LinkedList works like an array based on your snippet
            for (int j = 0; j < n1->roads.size(); j++) {
                // NOTE: Depending on your CustomSTL, access might be different.
                // Based on snippet: nodes[u]->roads[i]
                Edge edge = n1->roads[j];

                CityNode* n2 = graph.getNode(edge.destinationID);
                if (n2) {
                    int x2 = LonToScreenX(n2->lon);
                    int y2 = LatToScreenY(n2->lat);

                    // Traffic Color Logic
                    Color roadColor = { 100, 100, 100 }; // Grey
                    if (edge.weight > 5.0) roadColor = { 255, 100, 0 }; // Orange

                    engine.DrawLine(x1, y1, x2, y2, roadColor);
                }
            }
        }

        // Pass 2: Draw Nodes (on top of roads)
        for (int i = 0; i < count; i++) {
            CityNode* n = graph.getNode(i);
            if (!n) continue;

            int x = LonToScreenX(n->lon);
            int y = LatToScreenY(n->lat);

            Color nodeColor;

            // Color code based on your "Type" field from CSV
            if (n->type == "Hospital")     nodeColor = { 255, 0, 0 };    // Red
            else if (n->type == "School")  nodeColor = { 255, 255, 0 };  // Yellow
            else if (n->type == "Transport") nodeColor = { 0, 200, 255 }; // Cyan
            else                           nodeColor = { 50, 255, 50 };  // Green (Generic)

            // Draw a small 3x3 pixel box for the node
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    engine.DrawPixel(x + dx, y + dy, nodeColor);
                }
            }
			// Optional: Draw the name next to the node (if your engine supports text)
			engine.DrawText(x + 2, y - 2, n->name, { 255, 255, 255 });
        }
    }
};