//#include <iostream>
//#include <cstdlib>
//#include <ctime>
//#include "source/HousingSystem/PopulationManager.h"
//#include "source/SchoolSystem/SchoolManager.h"
//#include "source/MedicalSystem/MedicalManager.h"
//#include "CityGraph.h"
//#include "source/Graphics.h"
//#include <chrono>
//#include <thread>
//using namespace std;
//#include <fstream>
//#include <sstream>
//
//void LoadStopsData(CityGraph& graph, string filename) {
//    ifstream file(filename);
//    string line;
//
//    // Skip Header
//    getline(file, line);
//
//    while (getline(file, line)) {
//        if (line.empty()) continue;
//
//        stringstream ss(line);
//        string stopID, name, coords, segment;
//
//        // 1. Read StopID (until comma)
//        getline(ss, stopID, ',');
//
//        // 2. Read Name (until comma)
//        getline(ss, name, ',');
//
//        // 3. Read Coordinates (Handle Quotes)
//        // Checks if next char is a quote
//        if (ss.peek() == '"') {
//            ss.ignore(); // skip first quote
//            getline(ss, coords, '"'); // read until closing quote
//            ss.ignore(); // skip closing quote
//        }
//        else {
//            getline(ss, coords, ',');
//        }
//
//        // 4. Parse Lat/Lon from the coords string "33.684, 73.025"
//        size_t commaPos = coords.find(',');
//        if (commaPos != string::npos) {
//            double lat = stod(coords.substr(0, commaPos));
//            double lon = stod(coords.substr(commaPos + 1));
//
//            // Use your CityGraph's addLocation
//            // Format: dbID, stopID, name, type, lat, lon
//            graph.addLocation(stopID, stopID, name, "Transport", lat, lon);
//        }
//    }
//}
//
//
//int main() {
//	cin.get(); // Pause before starting
//    // 1. Initialize Engine (Full HD approximate for terminal)
//    // Using 600x300 ensures it fits on most maximized terminals
//    const int W = 1100;
//    const int H = 700;
//    SquarePixelEngine engine(W, H);
//
//    // 2. Initialize Logic
//    CityGraph city;
//
//    // 3. Load Data
//    // Ensure "stops.csv" is in the same folder as the exe
//    LoadStopsData(city, "dataset/stops.csv");
//
//    // Manually add some roads for testing (since CSV only has nodes)
//    // Connect G-10 (Index 0) to F-10 (Index 1)
//    if (city.getNodeCount() > 1) {
//        city.addRoad(0, 1);
//        city.addRoad(1, 2); // F-10 to PIMS
//        city.addRoad(2, 5); // PIMS to Blue Area
//    }
//
//    // 4. Initialize Visualizer
//    CityVisualizer viz(city, engine);
//
//    // CRITICAL: Calculate scaling based on loaded data
//    viz.CalculateBounds(W, H);
//
//    // 5. Game Loop
//    while (true) {
//        // Clear background to Dark Grey
//        engine.Clear({ 20, 20, 20 });
//
//        // Draw the City
//        viz.Draw();
//
//        // Update Screen
//        engine.Display();
//
//        // Slow down slightly to reduce CPU usage
//        std::this_thread::sleep_for(std::chrono::milliseconds(50));
//    }
//
//    return 0;
//}

#include "source/Simulator/CitySimulator.h"

int main() {
    CitySimulator simulator;
    
    // Use debug mode to bypass intro and go straight to graph view
    simulator.runDebugMode();
    
    // Normal mode (uncomment to use):
    // simulator.run();
    
    return 0;
}
