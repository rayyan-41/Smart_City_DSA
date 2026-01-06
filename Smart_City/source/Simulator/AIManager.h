#pragma once
#ifndef AI_MANAGER_H
#define AI_MANAGER_H

#include "../HousingSystem/Citizen.h"
#include "../HousingSystem/PopulationManager.h"
#include "../CityGrid/CityGraph.h"
#include "../CityGrid/CityUtils.h"
#include "../TransportSystem/TransportManager.h"

// ==================== AI CONSTANTS ====================
constexpr double WALKING_DISTANCE_THRESHOLD = 1.5;  // km - beyond this, use transport
constexpr double CITIZEN_WALK_SPEED = 0.05;          // Progress per tick while walking
constexpr double VEHICLE_BASE_SPEED = 0.1;           // Progress per tick for vehicles

// ==================== TIME CONSTANTS ====================
constexpr int SCHOOL_START_HOUR = 8;
constexpr int WORK_START_HOUR = 9;
constexpr int SCHOOL_END_HOUR = 14;
constexpr int WORK_END_HOUR = 17;
constexpr int SLEEP_START_HOUR = 22;
constexpr int WAKE_UP_HOUR = 6;

// ==================== AI MANAGER ====================
// The "Dungeon Master" of the simulation - governs citizen behavior
// based on needs, time of day, and available resources.

class AIManager {
private:
    CityGraph* cityGraph;
    PopulationManager* populationManager;
    TransportManager* transportManager;
    
    int currentSimHour;      // 0-23
    int currentSimMinute;    // 0-59
    int totalSimTicks;       // Total simulation ticks elapsed
    
public:
    AIManager(CityGraph* graph, PopulationManager* popMgr, TransportManager* transMgr)
        : cityGraph(graph), populationManager(popMgr), transportManager(transMgr),
          currentSimHour(6), currentSimMinute(0), totalSimTicks(0) {}
    
    ~AIManager() = default;
    
    // ==================== TIME MANAGEMENT ====================
    void setTime(int hour, int minute) {
        currentSimHour = hour % 24;
        currentSimMinute = minute % 60;
    }
    
    void advanceTime(int minutes) {
        currentSimMinute += minutes;
        while (currentSimMinute >= 60) {
            currentSimMinute -= 60;
            currentSimHour = (currentSimHour + 1) % 24;
        }
    }
    
    int getHour() const { return currentSimHour; }
    int getMinute() const { return currentSimMinute; }
    bool isNightTime() const { return currentSimHour >= 22 || currentSimHour < 6; }
    bool isWorkHours() const { return currentSimHour >= 9 && currentSimHour < 17; }
    bool isSchoolHours() const { return currentSimHour >= 8 && currentSimHour < 14; }
    
    // ==================== MAIN UPDATE LOOP ====================
    void updateCitizens(double deltaTime) {
        if (!populationManager) return;
        
        totalSimTicks++;
        
        // Update all citizens using the masterList Vector
        Vector<Citizen*>& citizens = populationManager->masterList;
        for (int i = 0; i < citizens.getSize(); i++) {
            if (citizens[i]) {
                updateSingleCitizen(*citizens[i], deltaTime);
            }
        }
    }
    
    // ==================== INDIVIDUAL CITIZEN AI ====================
    void updateSingleCitizen(Citizen& citizen, double deltaTime) {
        // 1. Decay needs over time
        citizen.needs.decay(deltaTime);
        
        // 2. Update movement if walking/commuting
        if (citizen.state == CitizenState::WALKING) {
            updateWalkingCitizen(citizen);
        }
        else if (citizen.state == CitizenState::COMMUTING) {
            // Movement handled by vehicle
        }
        // 3. Make decisions based on state
        else {
            makeDecision(citizen);
        }
    }
    
    void updateWalkingCitizen(Citizen& citizen) {
        if (!citizen.path.hasPath()) {
            // No path, return to idle
            citizen.state = CitizenState::IDLE_HOME;
            return;
        }
        
        // Advance along path
        bool reachedNode = citizen.path.advance(CITIZEN_WALK_SPEED);
        
        if (reachedNode) {
            citizen.currentNodeID = citizen.path.getCurrentNodeID();
            
            // Update position from graph
            if (cityGraph) {
                CityNode* node = cityGraph->getNode(citizen.currentNodeID);
                if (node) {
                    citizen.lat = node->lat;
                    citizen.lon = node->lon;
                }
            }
        }
        
        // Check if path is complete
        if (citizen.path.isComplete()) {
            arriveAtDestination(citizen);
        }
        else {
            // Interpolate position for rendering
            interpolateCitizenPosition(citizen);
        }
    }
    
    void interpolateCitizenPosition(Citizen& citizen) {
        if (!cityGraph) return;
        
        int currentNode = citizen.path.getCurrentNodeID();
        int nextNode = citizen.path.getNextNodeID();
        
        if (currentNode < 0 || nextNode < 0) return;
        
        CityNode* curr = cityGraph->getNode(currentNode);
        CityNode* next = cityGraph->getNode(nextNode);
        
        if (curr && next) {
            double t = citizen.path.progressOnEdge;
            citizen.lat = curr->lat + t * (next->lat - curr->lat);
            citizen.lon = curr->lon + t * (next->lon - curr->lon);
        }
    }
    
    void arriveAtDestination(Citizen& citizen) {
        string destType = citizen.path.destinationType;
        citizen.path.clear();
        
        if (destType == FacilityType::RESTAURANT || destType == FacilityType::MALL) {
            citizen.state = CitizenState::EATING;
            citizen.needs.eat();
        }
        else if (destType == FacilityType::HOSPITAL) {
            citizen.state = CitizenState::AT_HOSPITAL;
            citizen.needs.heal();
        }
        else if (destType == FacilityType::SCHOOL) {
            citizen.state = CitizenState::AT_SCHOOL;
        }
        else if (destType == "HOME") {
            citizen.state = CitizenState::IDLE_HOME;
            citizen.currentNodeID = citizen.homeNodeID;
        }
        else if (destType == "WORK") {
            citizen.state = CitizenState::WORKING;
        }
        else if (destType == FacilityType::PARK) {
            citizen.needs.socialize();
            citizen.state = CitizenState::IDLE_HOME;  // Will return home
        }
        else {
            citizen.state = CitizenState::IDLE_HOME;
        }
    }
    
    // ==================== DECISION MAKING (GOAP-lite) ====================
    void makeDecision(Citizen& citizen) {
        // Priority-based decision tree
        
        // 1. CRITICAL: Health emergency
        if (citizen.needs.isCritical()) {
            if (citizen.state != CitizenState::EMERGENCY) {
                citizen.state = CitizenState::EMERGENCY;
                findPathToFacility(citizen, FacilityType::HOSPITAL);
            }
            return;
        }
        
        // 2. CRITICAL: Starving
        if (citizen.needs.isCriticallyHungry() && citizen.needs.canAfford(200)) {
            if (citizen.state != CitizenState::WALKING && citizen.state != CitizenState::EATING) {
                findPathToFacility(citizen, FacilityType::RESTAURANT);
                citizen.state = CitizenState::WALKING;
            }
            return;
        }
        
        // 3. EXHAUSTED: Go home and sleep
        if (citizen.needs.isExhausted()) {
            if (citizen.state != CitizenState::SLEEPING && citizen.currentNodeID != citizen.homeNodeID) {
                findPathHome(citizen);
                citizen.state = CitizenState::WALKING;
            } else if (citizen.currentNodeID == citizen.homeNodeID) {
                citizen.state = CitizenState::SLEEPING;
                citizen.needs.sleep();
            }
            return;
        }
        
        // 4. TIME-BASED: Sleep at night
        if (isNightTime() && citizen.state == CitizenState::IDLE_HOME) {
            citizen.state = CitizenState::SLEEPING;
            citizen.needs.sleep();
            return;
        }
        
        // 5. Wake up in morning
        if (currentSimHour == WAKE_UP_HOUR && citizen.state == CitizenState::SLEEPING) {
            citizen.state = CitizenState::IDLE_HOME;
        }
        
        // 6. ROUTINE: Students go to school
        if (citizen.isStudent() && currentSimHour == SCHOOL_START_HOUR) {
            if (citizen.state == CitizenState::IDLE_HOME && citizen.schoolNodeID != -1) {
                calculateMultimodalPath(citizen, citizen.schoolNodeID, "SCHOOL");
                citizen.state = CitizenState::WALKING;
            }
            return;
        }
        
        // 7. ROUTINE: Workers go to work
        if (citizen.isWorker() && currentSimHour == WORK_START_HOUR) {
            if (citizen.state == CitizenState::IDLE_HOME && citizen.workplaceNodeID != -1) {
                calculateMultimodalPath(citizen, citizen.workplaceNodeID, "WORK");
                citizen.state = CitizenState::WALKING;
            }
            return;
        }
        
        // 8. ROUTINE: Return from school
        if (citizen.isStudent() && currentSimHour == SCHOOL_END_HOUR) {
            if (citizen.state == CitizenState::AT_SCHOOL) {
                findPathHome(citizen);
                citizen.state = CitizenState::WALKING;
            }
            return;
        }
        
        // 9. ROUTINE: Return from work
        if (citizen.isWorker() && currentSimHour == WORK_END_HOUR) {
            if (citizen.state == CitizenState::WORKING) {
                findPathHome(citizen);
                citizen.state = CitizenState::WALKING;
            }
            return;
        }
        
        // 10. MODERATE: Hungry (but not starving)
        if (citizen.needs.isHungry() && citizen.needs.canAfford(200)) {
            if (citizen.state == CitizenState::IDLE_HOME) {
                findPathToFacility(citizen, FacilityType::RESTAURANT);
                citizen.state = CitizenState::WALKING;
            }
            return;
        }
        
        // 11. SOCIAL: Visit park if lonely
        if (citizen.needs.isLonely() && citizen.state == CitizenState::IDLE_HOME) {
            findPathToFacility(citizen, FacilityType::PARK);
            citizen.state = CitizenState::WALKING;
            return;
        }
    }
    
    // ==================== PATH CALCULATION ====================
    void findPathToFacility(Citizen& citizen, const string& facilityType) {
        if (!cityGraph || citizen.currentNodeID < 0) return;
        
        int nearestFacility = cityGraph->findNearestFacility(citizen.currentNodeID, facilityType);
        if (nearestFacility >= 0) {
            calculateMultimodalPath(citizen, nearestFacility, facilityType);
        }
    }
    
    void findPathHome(Citizen& citizen) {
        if (citizen.homeNodeID >= 0) {
            calculateMultimodalPath(citizen, citizen.homeNodeID, "HOME");
        }
    }
    
    void calculateMultimodalPath(Citizen& citizen, int destNodeID, const string& destType) {
        if (!cityGraph || citizen.currentNodeID < 0 || destNodeID < 0) return;
        
        // Get start and end nodes
        CityNode* startNode = cityGraph->getNode(citizen.currentNodeID);
        CityNode* endNode = cityGraph->getNode(destNodeID);
        
        if (!startNode || !endNode) return;
        
        // Calculate distance
        double distance = GeometryUtils::getGridDistance(
            startNode->lat, startNode->lon,
            endNode->lat, endNode->lon
        );
        
        if (distance < WALKING_DISTANCE_THRESHOLD) {
            // Short distance: Walk directly
            double pathDist;
            Vector<int> path = cityGraph->findShortestPath(citizen.currentNodeID, destNodeID, pathDist);
            
            citizen.path.clear();
            citizen.path.nodes = path;
            citizen.path.currentIndex = 0;
            citizen.path.progressOnEdge = 0.0;
            citizen.path.destinationNodeID = destNodeID;
            citizen.path.destinationType = destType;
        }
        else {
            // Long distance: Need transport (simplified - just walk for now)
            // TODO: Implement bus stop finding and waiting logic
            
            // For now, find nearest bus stop, then walk
            int nearestStop = cityGraph->findNearestFacility(citizen.currentNodeID, FacilityType::STOP);
            
            if (nearestStop >= 0) {
                // Walk to bus stop
                double pathDist;
                Vector<int> pathToStop = cityGraph->findShortestPath(citizen.currentNodeID, nearestStop, pathDist);
                
                citizen.path.clear();
                citizen.path.nodes = pathToStop;
                citizen.path.currentIndex = 0;
                citizen.path.progressOnEdge = 0.0;
                citizen.path.destinationNodeID = nearestStop;
                citizen.path.destinationType = "BUS_STOP";
                
                // After reaching stop, citizen will enter WAITING_FOR_BUS state
                // The rest is handled by transport system
            }
            else {
                // No stops available, just walk the whole way
                double pathDist;
                Vector<int> path = cityGraph->findShortestPath(citizen.currentNodeID, destNodeID, pathDist);
                
                citizen.path.clear();
                citizen.path.nodes = path;
                citizen.path.currentIndex = 0;
                citizen.path.progressOnEdge = 0.0;
                citizen.path.destinationNodeID = destNodeID;
                citizen.path.destinationType = destType;
            }
        }
    }
    
    // ==================== STATISTICS ====================
    int getWalkingCitizenCount() const {
        if (!populationManager) return 0;
        int count = 0;
        const Vector<Citizen*>& citizens = populationManager->masterList;
        for (int i = 0; i < citizens.getSize(); i++) {
            if (citizens[i] && citizens[i]->state == CitizenState::WALKING) count++;
        }
        return count;
    }
    
    int getWaitingCitizenCount() const {
        if (!populationManager) return 0;
        int count = 0;
        const Vector<Citizen*>& citizens = populationManager->masterList;
        for (int i = 0; i < citizens.getSize(); i++) {
            if (citizens[i] && (citizens[i]->state == CitizenState::WAITING_FOR_BUS ||
                citizens[i]->state == CitizenState::WAITING_FOR_RIDE)) count++;
        }
        return count;
    }
    
    int getCommutingCitizenCount() const {
        if (!populationManager) return 0;
        int count = 0;
        const Vector<Citizen*>& citizens = populationManager->masterList;
        for (int i = 0; i < citizens.getSize(); i++) {
            if (citizens[i] && citizens[i]->state == CitizenState::COMMUTING) count++;
        }
        return count;
    }
};

#endif // AI_MANAGER_H
