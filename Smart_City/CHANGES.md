# **"Sims City" Transformation \- Detailed Implementation Plan**

This document serves as the master blueprint for converting the current static Smart City project into a dynamic, agent-based simulation. The changes are ordered by dependency: we must build the road capacity logic before vehicles can get stuck in traffic, and we must define citizen needs before they can decide to travel.

## **Phase 1: The Infrastructure (Physics & Capacity)**

**Goal:** Make the graph "aware" of physical space. Roads are no longer infinite; they have limits, and traffic slows down when capacity is reached.

### **1\. Modify Smart\_City/source/CityGrid/CityGraph.h**

#### **A. Update the Edge Struct**

Currently, edges just store destination and distance. We need to turn them into containers for vehicles.

* **Add Field:** int capacity.  
  * *Logic:* Defines how many vehicles fit on this road segment.  
  * *Default:* 10 for internal sector roads, 40 for main highways (sector boundaries).  
* **Add Field:** int currentLoad.  
  * *Logic:* Tracks real-time vehicle count. Incremented when a vehicle enters, decremented when it leaves.  
* **Add Field:** double dynamicWeight.  
  * *Logic:* Used for pathfinding. dynamicWeight \= length \* (1 \+ (currentLoad / capacity)^2). This makes congested roads exponentially "longer" to the pathfinding algorithm.

#### **B. Update CityGraph Class**

* **Add Method:** bool tryEnterEdge(int fromNode, int toNode)  
  * *Logic:* Finds the edge between these nodes. If currentLoad \< capacity, increment currentLoad and return true. Else, return false (vehicle must wait).  
* **Add Method:** void leaveEdge(int fromNode, int toNode)  
  * *Logic:* Decrements currentLoad.  
* **Add Method:** void updateTrafficWeights()  
  * *Logic:* Iterates over all adjacency lists and recalculates dynamicWeight for every edge based on current load. Call this once per simulation tick.

## **Phase 2: The Agents (State & Memory)**

**Goal:** Give entities memory and internal stats so they aren't just empty data shells.

### **2\. Modify Smart\_City/source/HousingSystem/Citizen.h**

#### **A. Implement Needs System**

* **Add Struct:** Needs  
  * double hunger (0-100, increases over time).  
  * double energy (0-100, decreases over time).  
  * double social (0-100, decays, replenished by visiting parks/malls).  
  * double wallet (Cash on hand).  
* **Add Enum:** CitizenState  
  * IDLE\_HOME, SLEEPING, WORKING, COMMUTING, WAITING\_FOR\_BUS, WALKING, SHOPPING, EMERGENCY.

#### **B. Implement Pathing Memory**

* **Add Field:** Vector\<int\> currentPath  
  * Stores the list of nodes the citizen plans to walk/travel.  
* **Add Field:** int currentPathIndex  
* **Add Field:** string currentVehicleID  
  * If empty, citizen is walking. If set, citizen is inside that vehicle.

### **3\. Modify Smart\_City/source/TransportSystem/Vehicle.h**

#### **A. Implement Spatial Awareness**

* **Add Field:** int nextNodeID  
  * The node the vehicle is currently trying to reach.  
* **Add Field:** double progressOnEdge  
  * 0.0 to 1.0. Represents physical position on the road segment.  
* **Add Field:** bool isStuck  
  * True if tryEnterEdge returned false.

## **Phase 3: The Brain (Decision Making)**

**Goal:** Create a centralized AI system that governs citizen behavior based on the needs defined in Phase 2\.

### **4\. Create New File: Smart\_City/source/Simulator/AIManager.h**

#### **A. Class AIManager**

This is the "Dungeon Master" of the simulation.

* **Method:** void updateCitizens(int timeOfDay)  
  * *Logic:* Iterate through PopulationManager::masterList.  
  * *Needs Decay:* hunger \+= 0.1, energy \-= 0.05.  
  * *Decision Tree (GOAP-lite):*  
    1. **Check Critical:** If hunger \> 80, change state to FINDING\_FOOD. Find nearest Restaurant or Mall. Calculate path.  
    2. **Check Routine:** If time \== 0800 AND age \< 18, change state to GOING\_TO\_SCHOOL.  
    3. **Check Work:** If time \== 0900 AND age \> 22, change state to GOING\_TO\_WORK.  
    4. **Check Home:** If time \== 1800 OR energy \< 20, change state to GOING\_HOME.  
* **Method:** void calculateMultimodalPath(Citizen\* c, int destNode)  
  * *Logic:* This is the "Smart" part.  
  * If distance \< 1.5km: Return walking path (A\* on graph).  
  * If distance \> 1.5km:  
    1. Walk to nearest Bus Stop.  
    2. Wait for Bus (State: WAITING\_FOR\_BUS).  
    3. Bus logic takes over.  
    4. Alight at stop nearest to destination.  
    5. Walk remaining distance.

### **5\. Update Smart\_City/source/CityGrid/SmartCity.h**

* Include AIManager.h.  
* Add AIManager\* aiManager member.  
* Initialize it in initialize() and delete in destructor.  
* Call aiManager-\>updateCitizens() inside simulateStep().

## **Phase 4: The Physics (Movement & Transport Logic)**

**Goal:** Enforce the rules of the road and implement demand-responsive transport.

### **6\. Modify Smart\_City/source/TransportSystem/TransportManager.h**

#### **A. Implement "Rickshaw" Logic (Feeders)**

* **Add Container:** Vector\<Vehicle\*\> rickshaws.  
* **Add Method:** void spawnRickshaws(int count).  
* **AI Behavior (in simulateStep):**  
  * Rickshaws wander randomly within a specific Sector.  
  * **Scanner:** If a Rickshaw is at a node where a Citizen is WAITING\_FOR\_BUS (and the citizen is NOT at a stop), pick them up.  
  * **Drop-off:** Drive them to the nearest STOP node.

#### **B. Update simulateBusStep() (The Traffic Jam Logic)**

* *Current:* Just updates index.  
* *New Logic:*  
  1. Calculates speed based on road congestion (Edge::dynamicWeight).  
  2. Increment progressOnEdge.  
  3. If progress \>= 1.0 (reached end of road):  
     * Call CityGraph::tryEnterEdge(currentNode, nextNode).  
     * **If True:** Move vehicle, call CityGraph::leaveEdge(prev, current).  
     * **If False:** Vehicle stops (isStuck \= true). Queues form naturally behind it.

## **Phase 5: The Visualization (Rendering the Simulation)**

**Goal:** Show the user exactly what is happening inside the brain and physics engine.

### **7\. Modify Smart\_City/source/Simulator/CityGraphView.h**

#### **A. Remove Fake Traffic**

* Delete trafficVehicles vector and initializeTraffic. We will no longer render random particles.

#### **B. Render Real Agents**

* **Method:** renderCitizens()  
  * Iterate PopulationManager::masterList.  
  * If state \== WALKING, interpolate position between nodes based on simulation tick.  
  * Draw small pixel. Color based on state:  
    * Red: Angry/Hungry.  
    * Green: Happy/Idle.  
    * Blue: Commuting.  
* **Method:** renderVehicles()  
  * Iterate TransportManager::buses/ambulances/rickshaws.  
  * Draw sprite at progressOnEdge between currentNode and nextNode.  
  * If isStuck, draw a small "\!" icon above the vehicle.

#### **C. Implement "God Mode" Inspection**

* **Input Handling:** In run() loop, detect left click.  
* **Logic:**  
  * Calculate distance from mouse click to all Vehicles and Citizens.  
  * If dist \< threshold, select that entity.  
* **UI Panel:** Draw a box in the sidebar showing:  
  * **Name:** "Ali Khan"  
  * **Status:** "Hungry (85%) \- Walking to Centaurus Mall"  
  * **Inventory:** "Wallet: 500 PKR"  
  * **Thought:** "I hope I don't miss the bus."

#### **D. Heatmap Overlay**

* **Toggle:** "Show Congestion".  
* **Render Logic:** When drawing edges, check edge.currentLoad / edge.capacity.  
  * 0-50%: Draw Grey.  
  * 50-80%: Draw Yellow.  
  * 80-100%: Draw Red (Thick).

## **Implementation Order Checklist**

1. **CityGraph.h**: Add capacity/load logic.  
2. **Citizen.h**: Add needs/states.  
3. **Vehicle.h**: Add spatial awareness.  
4. **AIManager.h**: Implement the brain.  
5. **SmartCity.h**: Hook up the AI.  
6. **TransportManager.h**: Connect graph capacity to vehicle movement.