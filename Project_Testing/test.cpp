#include "pch.h"
#include <gtest/gtest.h>
#include <string>
#include <fstream>
#include <cstdio> // For remove()
#include <iostream>

// Include your project logic
#include "../Smart_City/CityMap.h"

using namespace std;

// ==========================================
// TEST FIXTURE: CityGraphTest
// ==========================================
// This class sets up a fresh graph for every single test case.
class CityGraphTest : public ::testing::Test {
protected:
    CityMapGraph graph;

    // Helper to create temporary CSV files for testing
    void createTempCSV(string filename, string content) {
        ofstream file(filename);
        file << content;
        file.close();
    }

    void SetUp() override {
        // Code here runs before each test
        // Graph is automatically re-initialized by constructor
    }

    void TearDown() override {
        // Cleanup temp files if any exist
        remove("test_stops.csv");
        remove("test_buildings.csv");
        remove("bad_stops.csv");
    }
};

// ==========================================
// 1. CORE LOGIC & GEOMETRY TESTS
// ==========================================

TEST_F(CityGraphTest, AddLocationBasic) {
    // Add a simple node in F-8
    int id = graph.addLocation("S1", "Test Spot", "STOP", 33.710, 73.040);

    EXPECT_NE(id, -1);
    EXPECT_EQ(graph.getIDByName("Test Spot"), id);
    EXPECT_EQ(graph.getIDByString("S1"), id);
}

TEST_F(CityGraphTest, SectorResolutionAndFrameGeneration) {
    // 1. Ensure G-11 Frame does not exist initially
    EXPECT_EQ(graph.getIDByName("G-11 Corner 1"), -1);

    // 2. Add a node in G-11 (Bounds: 33.660-33.680, 72.970-72.990)
    // This should trigger the automatic frame generation
    int houseID = graph.addLocation("H1", "My House", "HOUSE", 33.670, 72.980);

    EXPECT_NE(houseID, -1);

    // 3. Verify Corners were created
    int c1 = graph.getIDByName("G-11 Corner 1");
    int c2 = graph.getIDByName("G-11 Corner 2");
    int c3 = graph.getIDByName("G-11 Corner 3");
    int c4 = graph.getIDByName("G-11 Corner 4");

    EXPECT_NE(c1, -1);
    EXPECT_NE(c2, -1);
    EXPECT_NE(c3, -1);
    EXPECT_NE(c4, -1);

    // 4. Verify connectivity (Frame should be a loop)
    // Note: We can't directly check edges without exposing 'nodes', 
    // but we can check via Dijkstra connectivity later.
}

TEST_F(CityGraphTest, AddRoadLogic) {
    int id1 = graph.addLocation("A", "Node A", "STOP", 33.700, 73.030);
    int id2 = graph.addLocation("B", "Node B", "STOP", 33.701, 73.031);

    // Add road
    graph.addRoad(id1, id2);

    // Verify distance calculation logic (Haversine)
    // Small distance check
    double dist = GeometryUtils::getHaversineDistance(33.700, 73.030, 33.701, 73.031);
    EXPECT_GT(dist, 0.0);
    EXPECT_LT(dist, 1.0); // Should be very close (meters)
}

// ==========================================
// 2. CSV PARSING TESTS
// ==========================================

TEST_F(CityGraphTest, LoadStopsCSV_Valid) {
    string csvContent =
        "StopID,Name,Coordinates\n"
        "Stop1,G-10 Markaz,\"33.684, 73.025\"\n"
        "Stop2,F-10 Park,\"33.691, 73.019\"\n";

    createTempCSV("test_stops.csv", csvContent);

    graph.loadStopsCSV("test_stops.csv");

    // Verify loading
    EXPECT_NE(graph.getIDByString("Stop1"), -1);
    EXPECT_NE(graph.getIDByString("Stop2"), -1);
    EXPECT_EQ(graph.getIDByName("G-10 Markaz"), graph.getIDByString("Stop1"));
}

TEST_F(CityGraphTest, LoadStopsCSV_Malformed) {
    string badContent =
        "StopID,Name,Coordinates\n"
        "StopBad1\n" // Missing columns
        "StopBad2,NameOnly,\"33.684\n" // Missing closing quote/coord
        ",,,\n"; // Empty garbage

    createTempCSV("bad_stops.csv", badContent);

    // Should handle gracefully without crashing
    graph.loadStopsCSV("bad_stops.csv");

    // Ensure bad data wasn't loaded
    EXPECT_EQ(graph.getIDByString("StopBad1"), -1);
}

TEST_F(CityGraphTest, LoadBuildingsCSV_AutoGenerateCoords) {
    // Simulating schools.csv which usually lacks coords or has just 'Sector'
    // Format assumed: ID,Name,Sector,...
    string csvContent =
        "ID,Name,Sector,Rating\n"
        "S1,Smart School,F-6,5.0\n"
        "S2,Bad Sector School,Z-99,1.0\n"; // Z-99 is invalid sector

    createTempCSV("test_buildings.csv", csvContent);

    graph.loadBuildingsCSV("test_buildings.csv", "SCHOOL");

    // 1. Verify F-6 school loaded
    int s1 = graph.getIDByName("Smart School");
    EXPECT_NE(s1, -1);

    // 2. Verify F-6 Frame was created as a side effect
    EXPECT_NE(graph.getIDByName("F-6 Corner 1"), -1);

    // 3. Verify Z-99 school failed or was skipped (since sector is unknown/invalid)
    // Your code checks: if (sector.empty()) continue; 
    // And resolveSector returns "Unknown" -> generateCoords falls back to default.
    // So it might be loaded at default coords. 
    // Let's check if it exists:
    int s2 = graph.getIDByName("Bad Sector School");
    // Depending on logic, it might be added at default (33.69, 73.04) or skipped.
    // Given the code, it adds it at fallback coords.
    EXPECT_NE(s2, -1);
}

// ==========================================
// 3. ALGORITHM TESTS (DIJKSTRA)
// ==========================================

TEST_F(CityGraphTest, Dijkstra_ConnectedPath) {
    int n1 = graph.addLocation("N1", "Start", "STOP", 33.700, 73.030);
    int n2 = graph.addLocation("N2", "Mid", "STOP", 33.705, 73.035);
    int n3 = graph.addLocation("N3", "End", "STOP", 33.710, 73.040);

    // Path: N1 -> N2 -> N3
    graph.addRoad(n1, n2);
    graph.addRoad(n2, n3);

    // Capture Output
    testing::internal::CaptureStdout();
    graph.findShortestPath(n1, n3);
    string output = testing::internal::GetCapturedStdout();

    // Verify Success
    EXPECT_NE(output.find("Shortest Distance"), string::npos);
    EXPECT_NE(output.find("Start -> Mid -> End"), string::npos);
}

TEST_F(CityGraphTest, Dijkstra_DisconnectedGraph) {
    int n1 = graph.addLocation("N1", "Island A", "STOP", 33.700, 73.030);
    int n2 = graph.addLocation("N2", "Island B", "STOP", 33.800, 73.100);

    // No road added

    testing::internal::CaptureStdout();
    graph.findShortestPath(n1, n2);
    string output = testing::internal::GetCapturedStdout();

    // Verify Failure Message
    EXPECT_NE(output.find("No path found"), string::npos);
}

TEST_F(CityGraphTest, Dijkstra_FrameConnectivity) {
    // Test if the "Frame" logic actually connects nodes
    // 1. Add a node in G-10
    int g10Node = graph.addLocation("G10", "G-10 Center", "STOP", 33.680, 73.000);

    // 2. Since G-10 frame is auto-created, G10Node should be linked to a corner.
    // 3. Let's find a path from G10Node to one of the corners.

    int corner = graph.getIDByName("G-10 Corner 1");
    ASSERT_NE(corner, -1); // Frame must exist

    testing::internal::CaptureStdout();
    graph.findShortestPath(g10Node, corner);
    string output = testing::internal::GetCapturedStdout();

    // Should find a path (direct or via other corners)
    EXPECT_NE(output.find("Shortest Distance"), string::npos);
}

// ==========================================
// 4. EDGE CASES
// ==========================================

TEST_F(CityGraphTest, InvalidIDHandling) {
    testing::internal::CaptureStdout();
    graph.findShortestPath(-1, 5);
    string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Invalid Node IDs"), string::npos);
}

TEST_F(CityGraphTest, CapacityOverflow) {
    // Fill graph to MAX (1000)
    // Note: Frames add 4 nodes + original node = 5 nodes per addLocation call (if new sector)
    // We will just loop adding manual corner types to avoid frame generation overhead for speed
    for (int i = 0; i < 1000; i++) {
        graph.addLocation("F" + to_string(i), "Fill" + to_string(i), "CORNER", 33.0, 73.0);
    }

    // Try adding 1001st
    int result = graph.addLocation("Overflow", "Bad", "STOP", 33.0, 73.0);

    EXPECT_EQ(result, -1);
}