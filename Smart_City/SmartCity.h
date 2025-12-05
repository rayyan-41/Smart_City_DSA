#pragma once
#include "CityGraph.h"
#include "source/SchoolSystem/SchoolManager.h"
#include "source/TransportSystem/BusManager.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using namespace std;

class SmartCity {
private:
    CityGraph* cityGraph;
    
    // Managers
    SchoolManager* schoolManager;
    BusManager* busManager;
    // TODO: Add HospitalManager, PopulationManager in future
    
    // Dataset paths
    string stopsCSV;
    string schoolsCSV;
    string hospitalsCSV;
    string pharmaciesCSV;
    string busesCSV;
    
    // UI state
    bool isRunning;
    bool cityInitialized;
    bool simulationStarted;
    
    // UI Methods
    void displayWelcomeMenu();
    void displayMainMenu();
    void clearScreen();
    void pauseScreen();
    int getIntInput(const string& prompt);
    string getStringInput(const string& prompt);
    
    // Initialization
    void initializeCity();
    
    // Simulation
    void startSimulation();
    void showCityStats();
    
    // Sector Navigation
    void viewAllSectors();
    void viewSectorDetails(const string& sectorName);
    void viewSchoolDetails(School* school);
    void viewHospitalDetails(int nodeID);
    
    // Transport Module
    void transportMenu();
    void viewAllBuses();
    void viewBusDetails(const string& busNo);
    void registerNewBus();
    void findNearestFacilityMenu();
    void findBusRoute();
    
    // Pathfinding
    void runDijkstra();
    void displayPath(int startID, int endID);
    
public:
    SmartCity();
    ~SmartCity();
    
    // Configuration
    void setDatasetPaths(const string& stops, const string& schools, 
                        const string& hospitals, const string& pharmacies, const string& buses);
    
    // Main entry point
    void run();
    
    // Menu options
    void showAbout();
    void exitProgram();
};

/*------ Constructor & Destructor ------*/
inline SmartCity::SmartCity() {
    cityGraph = nullptr;
    schoolManager = nullptr;
    busManager = nullptr;
    isRunning = true;
    cityInitialized = false;
    simulationStarted = false;
    
    // Default dataset paths (relative to executable)
    stopsCSV = "dataset/stops.csv";
    schoolsCSV = "dataset/schools.csv";
    hospitalsCSV = "dataset/hospitals.csv";
    pharmaciesCSV = "dataset/pharmacies.csv";
    busesCSV = "dataset/buses.csv";
}

inline SmartCity::~SmartCity() {
    if (cityGraph != nullptr) {
        delete cityGraph;
    }
    if (schoolManager != nullptr) {
        delete schoolManager;
    }
    if (busManager != nullptr) {
        delete busManager;
    }
}

/*------ Configuration ------*/
inline void SmartCity::setDatasetPaths(const string& stops, const string& schools, 
                               const string& hospitals, const string& pharmacies, const string& buses) {
    stopsCSV = stops;
    schoolsCSV = schools;
    hospitalsCSV = hospitals;
    pharmaciesCSV = pharmacies;
    busesCSV = buses;
}

/*------ Utility Methods ------*/
inline void SmartCity::clearScreen() {
    std::cout << "\x1B[2J\x1B[H";
}

inline void SmartCity::pauseScreen() {
    cout << "\nPress Enter to continue...";
    cin.ignore(10000, '\n');
    cin.get();
}

inline int SmartCity::getIntInput(const string& prompt) {
    int value;
    cout << prompt;
    while (!(cin >> value)) {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Invalid input. " << prompt;
    }
    cin.ignore(10000, '\n');
    return value;
}

inline string SmartCity::getStringInput(const string& prompt) {
    string value;
    cout << prompt;
    getline(cin, value);
    return value;
}

/*------ Welcome Menu ------*/
inline void SmartCity::displayWelcomeMenu() {
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "   SMART CITY MANAGEMENT SYSTEM\n";
    cout << "   New Islamabad - City of the Future\n";
    cout << "========================================\n\n";
    cout << "Group Members:\n";
    cout << "  - Rayyan Ahmad Sultan\n";
    cout << "  - Omar Abdullah Khan Niazi\n";
    cout << "  - Aryan Ali Khan\n\n";
    cout << "========================================\n\n";
    cout << "Press Enter to Initialize the City...\n";
}

/*------ Main Menu ------*/
inline void SmartCity::displayMainMenu() {
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "   SMART CITY MANAGEMENT SYSTEM\n";
    if (simulationStarted) {
        cout << "   [SIMULATION RUNNING]\n";
    }
    cout << "========================================\n\n";
    cout << "Main Menu:\n\n";
    
    if (!simulationStarted) {
        cout << "  1. Start Simulation\n";
    } else {
        cout << "  1. View City Statistics\n";
        cout << "  2. View All Sectors\n";
        cout << "  3. Transport Module\n";
        cout << "  4. Run Dijkstra Pathfinding\n";
        cout << "  5. About\n";
        cout << "  6. Exit Program\n";
    }
    
    cout << "\n========================================\n";
    cout << "Enter your choice: ";
}

/*------ City Initialization ------*/
inline void SmartCity::initializeCity() {
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "   Initializing Smart City...\n";
    cout << "========================================\n\n";
    
    // Initialize graph and managers
    cout << "[1/5] Creating city infrastructure...\n";
    cityGraph = new CityGraph();
    schoolManager = new SchoolManager();
    busManager = new BusManager();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Step 1: Load stops into CityGraph
    cout << "[2/5] Loading stops.csv into CityGraph...\n";
    cityGraph->loadStopsCSV(stopsCSV);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Step 2: Load schools into SchoolManager
    cout << "[3/5] Loading schools.csv into SchoolManager...\n";
    schoolManager->loadFromCSV(schoolsCSV);
    
    // Add schools to CityGraph with same databaseID
    cout << "       Adding schools to CityGraph with linked databaseIDs...\n";
    for (int i = 0; i < schoolManager->schools.getSize(); i++) {
        School* school = schoolManager->schools[i];
        int graphID = cityGraph->addLocation(
            school->id,              
            "",                      
            school->name,            
            "SCHOOL",                
            school->location.coord.x,
            school->location.coord.y 
        );
        
        if (graphID != -1) {
            school->graphNodeID = std::to_string(graphID);
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Step 3: Load hospitals and pharmacies
    cout << "[4/5] Loading hospitals.csv and pharmacies.csv...\n";
    cityGraph->loadBuildingsCSV(hospitalsCSV, "HOSPITAL");
    cityGraph->loadBuildingsCSV(pharmaciesCSV, "PHARMACY");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Step 4: Load buses
    cout << "[5/5] Loading buses.csv into BusManager...\n";
    busManager->loadFromCSV(busesCSV);
    busManager->setCityGraph(cityGraph);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Complete
    cout << "\n✓ City Initialized Successfully!\n";
    cout << "  All systems operational.\n\n";
    cout << "City Statistics:\n";
    cout << "  - Total Nodes: " << cityGraph->getNodeCount() << "\n";
    cout << "  - Schools: " << schoolManager->schools.getSize() << "\n";
    cout << "  - Buses: " << busManager->getBusCount() << "\n";
    
    cityInitialized = true;
    pauseScreen();
}

/*------ Simulation ------*/
inline void SmartCity::startSimulation() {
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "   Starting City Simulation...\n";
    cout << "========================================\n\n";
    
    cout << "Initializing city systems...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    cout << "✓ Transportation network online\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    cout << "✓ School management system active\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    cout << "✓ Healthcare facilities operational\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    cout << "\n✓ Simulation started successfully!\n\n";
    
    simulationStarted = true;
    
    showCityStats();
    pauseScreen();
}

inline void SmartCity::showCityStats() {
    cout << "========================================\n";
    cout << "   CITY STATISTICS\n";
    cout << "========================================\n\n";
    
    // Count different node types
    int stops = 0, schools = 0, hospitals = 0, pharmacies = 0, corners = 0;
    for (int i = 0; i < cityGraph->getNodeCount(); i++) {
        CityNode* node = cityGraph->getNode(i);
        if (node != nullptr) {
            if (node->type == "STOP") stops++;
            else if (node->type == "SCHOOL") schools++;
            else if (node->type == "HOSPITAL") hospitals++;
            else if (node->type == "PHARMACY") pharmacies++;
            else if (node->type == "CORNER") corners++;
        }
    }
    
    cout << "Total Nodes: " << cityGraph->getNodeCount() << "\n\n";
    cout << "Node Distribution:\n";
    cout << "  • Bus Stops: " << stops << "\n";
    cout << "  • Schools: " << schools << "\n";
    cout << "  • Hospitals: " << hospitals << "\n";
    cout << "  • Pharmacies: " << pharmacies << "\n";
    cout << "  • Sector Corners: " << corners << "\n\n";
    
    cout << "Active Sectors: " << SECTOR_COUNT << "\n";
    cout << "Total Buses: " << busManager->getBusCount() << "\n";
    cout << "========================================\n\n";
}

/*------ Sector Navigation ------*/
inline void SmartCity::viewAllSectors() {
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "   ALL SECTORS\n";
    cout << "========================================\n\n";
    
    for (int i = 0; i < SECTOR_COUNT; i++) {
        string sectorName = SECTOR_GRID[i].name;
        
        int totalNodes = 0, schools = 0, hospitals = 0, pharmacies = 0;
        for (int j = 0; j < cityGraph->getNodeCount(); j++) {
            CityNode* node = cityGraph->getNode(j);
            if (node != nullptr && node->sector == sectorName) {
                totalNodes++;
                if (node->type == "SCHOOL") schools++;
                else if (node->type == "HOSPITAL") hospitals++;
                else if (node->type == "PHARMACY") pharmacies++;
            }
        }
        
        cout << (i + 1) << ". " << sectorName << " - " << totalNodes << " nodes";
        cout << " (S:" << schools << " H:" << hospitals << " P:" << pharmacies << ")\n";
    }
    
    cout << "\n" << (SECTOR_COUNT + 1) << ". Back to Main Menu\n";
    cout << "\n========================================\n";
    
    int choice = getIntInput("Enter sector number to view details: ");
    
    if (choice > 0 && choice <= SECTOR_COUNT) {
        viewSectorDetails(SECTOR_GRID[choice - 1].name);
    }
}

inline void SmartCity::viewSectorDetails(const string& sectorName) {
    while (true) {
        clearScreen();
        cout << "\n";
        cout << "========================================\n";
        cout << "   SECTOR: " << sectorName << "\n";
        cout << "========================================\n\n";
        
        Vector<CityNode*> stops, schools, hospitals, pharmacies;
        
        for (int i = 0; i < cityGraph->getNodeCount(); i++) {
            CityNode* node = cityGraph->getNode(i);
            if (node != nullptr && node->sector == sectorName) {
                if (node->type == "STOP") stops.push_back(node);
                else if (node->type == "SCHOOL") schools.push_back(node);
                else if (node->type == "HOSPITAL") hospitals.push_back(node);
                else if (node->type == "PHARMACY") pharmacies.push_back(node);
            }
        }
        
        cout << "Bus Stops: " << stops.getSize() << "\n";
        cout << "Schools: " << schools.getSize() << "\n";
        cout << "Hospitals: " << hospitals.getSize() << "\n";
        cout << "Pharmacies: " << pharmacies.getSize() << "\n\n";
        
        cout << "========================================\n";
        cout << "SCHOOLS IN " << sectorName << ":\n";
        cout << "========================================\n";
        for (int i = 0; i < schools.getSize(); i++) {
            cout << (i + 1) << ". " << schools[i]->name 
                 << " (ID: " << schools[i]->databaseID << ")\n";
        }
        if (schools.getSize() == 0) cout << "  No schools in this sector\n";
        
        cout << "\n========================================\n";
        cout << "HOSPITALS IN " << sectorName << ":\n";
        cout << "========================================\n";
        for (int i = 0; i < hospitals.getSize(); i++) {
            cout << (schools.getSize() + i + 1) << ". " << hospitals[i]->name 
                 << " (ID: " << hospitals[i]->databaseID << ")\n";
        }
        if (hospitals.getSize() == 0) cout << "  No hospitals in this sector\n";
        
        cout << "\n========================================\n";
        cout << "PHARMACIES IN " << sectorName << ":\n";
        cout << "========================================\n";
        for (int i = 0; i < pharmacies.getSize(); i++) {
            cout << (schools.getSize() + hospitals.getSize() + i + 1) << ". " 
                 << pharmacies[i]->name << " (ID: " << pharmacies[i]->databaseID << ")\n";
        }
        if (pharmacies.getSize() == 0) cout << "  No pharmacies in this sector\n";
        
        cout << "\n" << (schools.getSize() + hospitals.getSize() + pharmacies.getSize() + 1) 
             << ". Back\n";
        cout << "\n========================================\n";
        
        int choice = getIntInput("Enter number to view details (or back): ");
        
        if (choice < 1 || choice > schools.getSize() + hospitals.getSize() + pharmacies.getSize()) {
            break;
        }
        
        if (choice <= schools.getSize()) {
            string schoolID = schools[choice - 1]->databaseID;
            School* school = schoolManager->findSchoolByID(schoolID);
            if (school != nullptr) {
                viewSchoolDetails(school);
            }
        } else if (choice <= schools.getSize() + hospitals.getSize()) {
            int idx = choice - schools.getSize() - 1;
            viewHospitalDetails(hospitals[idx]->id);
        } else {
            int idx = choice - schools.getSize() - hospitals.getSize() - 1;
            viewHospitalDetails(pharmacies[idx]->id);
        }
    }
}

inline void SmartCity::viewSchoolDetails(School* school) {
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "   SCHOOL DETAILS\n";
    cout << "========================================\n\n";
    
    cout << "School ID: " << school->id << "\n";
    cout << "Name: " << school->name << "\n";
    cout << "Sector: " << school->location.sector << "\n";
    cout << "Rating: " << school->rating << "/5.0\n";
    cout << "Location: (" << school->location.coord.x << ", " 
         << school->location.coord.y << ")\n";
    cout << "Graph Node ID: " << school->graphNodeID << "\n\n";
    
    cout << "========================================\n";
    cout << "SUBJECTS OFFERED:\n";
    cout << "========================================\n";
    for (int i = 0; i < school->subjects.getSize(); i++) {
        cout << "  • " << school->subjects[i] << "\n";
    }
    
    cout << "\n========================================\n";
    cout << "DEPARTMENTS:\n";
    cout << "========================================\n";
    for (int i = 0; i < school->departments.getSize(); i++) {
        Department* dept = school->departments[i];
        cout << "\n" << (i + 1) << ". " << dept->name << " Department\n";
        cout << "   Subjects: " << dept->subjects.getSize() << "\n";
        cout << "   Classes: " << dept->classes.getSize() << "\n";
        cout << "   Faculty: " << dept->faculty.getSize() << "\n";
    }
    
    cout << "\n========================================\n";
    pauseScreen();
}

inline void SmartCity::viewHospitalDetails(int nodeID) {
    CityNode* node = cityGraph->getNode(nodeID);
    if (node == nullptr) return;
    
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "   " << node->type << " DETAILS\n";
    cout << "========================================\n\n";
    
    cout << "Database ID: " << node->databaseID << "\n";
    cout << "Name: " << node->name << "\n";
    cout << "Type: " << node->type << "\n";
    cout << "Sector: " << node->sector << "\n";
    cout << "Location: (" << node->lat << ", " << node->lon << ")\n";
    cout << "Graph Node ID: " << node->id << "\n\n";
    
    cout << "========================================\n";
    cout << "CONNECTIONS:\n";
    cout << "========================================\n";
    cout << "Connected to " << node->roads.size() << " other locations:\n";
    for (size_t i = 0; i < node->roads.size(); i++) {
        Edge edge = node->roads[i];
        CityNode* connected = cityGraph->getNode(edge.destinationID);
        if (connected != nullptr) {
            cout << "  • " << connected->name << " (" << edge.weight << " km)\n";
        }
    }
    
    cout << "\n========================================\n";
    pauseScreen();
}

/*------ Transport Module ------*/
inline void SmartCity::transportMenu() {
    while (true) {
        clearScreen();
        cout << "\n";
        cout << "========================================\n";
        cout << "   TRANSPORT MODULE\n";
        cout << "========================================\n\n";
        
        cout << "  1. View All Buses\n";
        cout << "  2. Register New Bus\n";
        cout << "  3. Find Nearest Facility\n";
        cout << "  4. Back to Main Menu\n\n";
        
        int choice = getIntInput("Enter your choice: ");
        
        switch (choice) {
            case 1:
                viewAllBuses();
                break;
            case 2:
                registerNewBus();
                break;
            case 3:
                findNearestFacilityMenu();
                break;
            case 4:
                return;
            default:
                cout << "Invalid choice!\n";
                pauseScreen();
                break;
        }
    }
}

inline void SmartCity::viewAllBuses() {
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "   ALL BUSES\n";
    cout << "========================================\n\n";
    
    if (busManager->getBusCount() == 0) {
        cout << "No buses registered in the system.\n\n";
    } else {
        for (int i = 0; i < busManager->getBusCount(); i++) {
            Bus* bus = busManager->getBus(i);
            cout << (i + 1) << ". Bus No: " << bus->busNo 
                 << " | Company: " << bus->company 
                 << " | Current Stop: " << bus->currentStop << "\n";
        }
        
        cout << "\n" << (busManager->getBusCount() + 1) << ". View Bus Details\n";
        cout << (busManager->getBusCount() + 2) << ". Back\n";
        cout << "\n========================================\n";
        
        int choice = getIntInput("Enter choice: ");
        
        if (choice == busManager->getBusCount() + 1) {
            string busNo = getStringInput("Enter bus number: ");
            viewBusDetails(busNo);
        }
    }
    
    pauseScreen();
}

inline void SmartCity::viewBusDetails(const string& busNo) {
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "   BUS DETAILS\n";
    cout << "========================================\n\n";
    
    Bus* bus = busManager->findBusByNumber(busNo);
    if (bus == nullptr) {
        cout << "Bus not found!\n";
    } else {
        cout << "Bus No: " << bus->busNo << "\n";
        cout << "Company: " << bus->company << "\n";
        cout << "Current Stop: " << bus->currentStop << "\n";
        cout << "Total Distance: " << bus->totalDistance << " km\n";
        cout << "Number of Stops: " << bus->getStopCount() << "\n\n";
        
        if (bus->getStopCount() > 0) {
            cout << "========================================\n";
            cout << "ROUTE:\n";
            cout << "========================================\n";
            cout << "Start: " << bus->startStopID << "\n";
            cout << "End: " << bus->endStopID << "\n\n";
            
            cout << "Full Route (" << bus->route.getSize() << " nodes):\n";
            for (int i = 0; i < bus->route.getSize(); i++) {
                CityNode* node = cityGraph->getNode(bus->route[i]);
                if (node != nullptr) {
                    cout << "  " << (i + 1) << ". " << node->name << " (" << node->type << ")\n";
                }
            }
        }
    }
    
    cout << "\n========================================\n";
    pauseScreen();
}

inline void SmartCity::registerNewBus() {
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "   REGISTER NEW BUS\n";
    cout << "========================================\n\n";
    
    string busNo = getStringInput("Enter bus number (e.g., B501): ");
    string company = getStringInput("Enter company name: ");
    string currentStop = getStringInput("Enter current stop ID: ");
    
    Bus* bus = busManager->createBus(busNo, company, currentStop);
    
    cout << "\nBus created. Now set the route:\n";
    string startStopID = getStringInput("Enter starting stop ID: ");
    string endStopID = getStringInput("Enter ending stop ID: ");
    
    int startNodeID = cityGraph->getIDByDatabaseID(startStopID);
    int endNodeID = cityGraph->getIDByDatabaseID(endStopID);
    
    if (startNodeID == -1 || endNodeID == -1) {
        cout << "\nError: Invalid stop IDs!\n";
        cout << "Bus registered but route not set.\n";
        pauseScreen();
        return;
    }
    
    cout << "\nCalculating optimal route using Dijkstra's algorithm...\n";
    double distance = 0.0;
    Vector<int> route = cityGraph->findShortestPath(startNodeID, endNodeID, distance);
    
    if (route.getSize() > 0) {
        bus->setRoute(route, distance);
        bus->setStops(startStopID, endStopID);
        
        cout << "\n✓ Bus registered successfully!\n";
        cout << "Bus No: " << busNo << "\n";
        cout << "Route Distance: " << distance << " km\n";
        cout << "Number of stops: " << route.getSize() << "\n";
    } else {
        cout << "\nError: Could not calculate route!\n";
    }
    
    pauseScreen();
}

inline void SmartCity::findNearestFacilityMenu() {
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "   FIND NEAREST FACILITY\n";
    cout << "========================================\n\n";
    
    cout << "Facility Types:\n";
    cout << "  1. School\n";
    cout << "  2. Hospital\n";
    cout << "  3. Pharmacy\n";
    cout << "  4. Bus Stop\n";
    cout << "  5. Back\n\n";
    
    int choice = getIntInput("Select facility type: ");
    
    string facilityType;
    switch (choice) {
        case 1: facilityType = "SCHOOL"; break;
        case 2: facilityType = "HOSPITAL"; break;
        case 3: facilityType = "PHARMACY"; break;
        case 4: facilityType = "STOP"; break;
        case 5: return;
        default:
            cout << "Invalid choice!\n";
            pauseScreen();
            return;
    }
    
    string fromLocation = getStringInput("\nEnter your current location (stop ID or name): ");
    
    int fromNodeID = cityGraph->getIDByDatabaseID(fromLocation);
    if (fromNodeID == -1) {
        fromNodeID = cityGraph->getIDByName(fromLocation);
    }
    
    if (fromNodeID == -1) {
        cout << "\nError: Location not found!\n";
        pauseScreen();
        return;
    }
    
    cout << "\nSearching for nearest " << facilityType << "...\n\n";
    
    int nearestID = cityGraph->findNearestFacility(fromNodeID, facilityType);
    
    if (nearestID == -1) {
        cout << "No " << facilityType << " found in the city.\n";
    } else {
        CityNode* nearest = cityGraph->getNode(nearestID);
        CityNode* from = cityGraph->getNode(fromNodeID);
        
        cout << "========================================\n";
        cout << "NEAREST " << facilityType << "\n";
        cout << "========================================\n";
        cout << "From: " << from->name << "\n";
        cout << "To: " << nearest->name << "\n";
        cout << "Database ID: " << nearest->databaseID << "\n";
        cout << "Sector: " << nearest->sector << "\n";
        cout << "Location: (" << nearest->lat << ", " << nearest->lon << ")\n";
        
        double distance = 0.0;
        Vector<int> path = cityGraph->findShortestPath(fromNodeID, nearestID, distance);
        
        cout << "\nDistance: " << distance << " km\n";
        cout << "Number of stops: " << path.getSize() << "\n";
    }
    
    cout << "\n========================================\n";
    pauseScreen();
}

inline void SmartCity::findBusRoute() {
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "   FIND BUS ROUTE\n";
    cout << "========================================\n\n";
    
    string from = getStringInput("Enter starting location (ID or name): ");
    string to = getStringInput("Enter destination (ID or name): ");
    
    int fromID = cityGraph->getIDByDatabaseID(from);
    if (fromID == -1) fromID = cityGraph->getIDByName(from);
    
    int toID = cityGraph->getIDByDatabaseID(to);
    if (toID == -1) toID = cityGraph->getIDByName(to);
    
    if (fromID == -1 || toID == -1) {
        cout << "\nError: Location(s) not found!\n";
        pauseScreen();
        return;
    }
    
    cout << "\nSearching for buses on this route...\n\n";
    
    Vector<Bus*> matchingBuses;
    for (int i = 0; i < busManager->getBusCount(); i++) {
        Bus* bus = busManager->getBus(i);
        if (bus->isOnRoute(fromID) && bus->isOnRoute(toID)) {
            matchingBuses.push_back(bus);
        }
    }
    
    if (matchingBuses.getSize() == 0) {
        cout << "No direct bus found for this route.\n";
        cout << "\nCalculating walking/transfer route...\n";
        
        double distance = 0.0;
        Vector<int> path = cityGraph->findShortestPath(fromID, toID, distance);
        
        if (path.getSize() > 0) {
            cout << "\nRecommended Route:\n";
            cout << "Distance: " << distance << " km\n";
            cout << "Stops: " << path.getSize() << "\n";
        }
    } else {
        cout << "Found " << matchingBuses.getSize() << " bus(es) for this route:\n\n";
        for (int i = 0; i < matchingBuses.getSize(); i++) {
            Bus* bus = matchingBuses[i];
            cout << (i + 1) << ". Bus " << bus->busNo 
                 << " (" << bus->company << ")\n";
        }
    }
    
    cout << "\n========================================\n";
    pauseScreen();
}

/*------ Pathfinding ------*/
inline void SmartCity::runDijkstra() {
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "   DIJKSTRA PATHFINDING\n";
    cout << "========================================\n\n";
    
    cout << "Available options:\n";
    cout << "  1. Search by Node ID\n";
    cout << "  2. Search by Database ID\n";
    cout << "  3. Search by Name\n";
    cout << "  4. Back\n\n";
    
    int choice = getIntInput("Choose search method: ");
    if (choice == 4) return;
    
    int startID = -1, endID = -1;
    
    if (choice == 1) {
        startID = getIntInput("Enter start node ID: ");
        endID = getIntInput("Enter end node ID: ");
    } else if (choice == 2) {
        string startDB = getStringInput("Enter start database ID: ");
        string endDB = getStringInput("Enter end database ID: ");
        startID = cityGraph->getIDByDatabaseID(startDB);
        endID = cityGraph->getIDByDatabaseID(endDB);
    } else if (choice == 3) {
        string startName = getStringInput("Enter start location name: ");
        string endName = getStringInput("Enter end location name: ");
        startID = cityGraph->getIDByName(startName);
        endID = cityGraph->getIDByName(endName);
    } else {
        cout << "Invalid choice!\n";
        pauseScreen();
        return;
    }
    
    if (startID == -1 || endID == -1) {
        cout << "\nError: Location not found!\n";
        pauseScreen();
        return;
    }
    
    displayPath(startID, endID);
}

inline void SmartCity::displayPath(int startID, int endID) {
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "   SHORTEST PATH\n";
    cout << "========================================\n\n";
    
    CityNode* start = cityGraph->getNode(startID);
    CityNode* end = cityGraph->getNode(endID);
    
    if (start == nullptr || end == nullptr) {
        cout << "Error: Invalid node IDs!\n";
        pauseScreen();
        return;
    }
    
    cout << "From: " << start->name << " (ID: " << startID << ")\n";
    cout << "To: " << end->name << " (ID: " << endID << ")\n\n";
    cout << "Calculating shortest path...\n\n";
    
    double distance = 0.0;
    Vector<int> path = cityGraph->findShortestPath(startID, endID, distance);
    
    cout << "========================================\n";
    cout << "PATH CALCULATED\n";
    cout << "========================================\n\n";
    
    if (path.getSize() > 0) {
        cout << "Distance: " << distance << " km\n";
        cout << "Number of stops: " << path.getSize() << "\n\n";
        cout << "Route:\n";
        for (int i = 0; i < path.getSize(); i++) {
            CityNode* node = cityGraph->getNode(path[i]);
            if (node != nullptr) {
                cout << "  " << (i + 1) << ". " << node->name << " (" << node->type << ")\n";
            }
        }
    } else {
        cout << "No path found!\n";
    }
    
    cout << "\n========================================\n";
    pauseScreen();
}

/*------ About ------*/
inline void SmartCity::showAbout() {
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "   SMART CITY MANAGEMENT SYSTEM\n";
    cout << "   New Islamabad - City of the Future\n";
    cout << "========================================\n\n";
    cout << "About This Project:\n\n";
    cout << "This project demonstrates a smart city\n";
    cout << "management system using advanced data\n";
    cout << "structures and algorithms.\n\n";
    cout << "Key Features:\n";
    cout << "  • Graph-based city infrastructure\n";
    cout << "  • School management system\n";
    cout << "  • Hospital and pharmacy tracking\n";
    cout << "  • Transport module with bus management\n";
    cout << "  • Dijkstra's pathfinding algorithm\n";
    cout << "  • Nearest facility search\n";
    cout << "  • Custom data structure implementations\n\n";
    cout << "Technologies:\n";
    cout << "  • C++ Programming Language\n";
    cout << "  • Custom STL Implementations\n";
    cout << "  • Graph Theory & Algorithms\n";
    cout << "  • Hash Tables and Priority Queues\n\n";
    cout << "========================================\n";
    
    pauseScreen();
}

inline void SmartCity::exitProgram() {
    isRunning = false;
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "   Thank you for using\n";
    cout << "   Smart City Management System!\n";
    cout << "========================================\n\n";
}

/*------ Main Run Loop ------*/
inline void SmartCity::run() {
    if (!cityInitialized) {
        displayWelcomeMenu();
        cin.get();
        initializeCity();
    }
    
    while (isRunning && cityInitialized) {
        displayMainMenu();
        
        int choice;
        cin >> choice;
        cin.ignore(10000, '\n');
        
        if (!simulationStarted) {
            if (choice == 1) {
                startSimulation();
            } else {
                cout << "\nPlease start simulation first!\n";
                pauseScreen();
            }
        } else {
            switch (choice) {
                case 1:
                    clearScreen();
                    cout << "\n";
                    showCityStats();
                    pauseScreen();
                    break;
                case 2:
                    viewAllSectors();
                    break;
                case 3:
                    transportMenu();
                    break;
                case 4:
                    runDijkstra();
                    break;
                case 5:
                    showAbout();
                    break;
                case 6:
                    exitProgram();
                    break;
                default:
                    cout << "\nInvalid choice. Please try again.\n";
                    pauseScreen();
                    break;
            }
        }
    }
}