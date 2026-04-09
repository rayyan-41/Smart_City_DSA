# 🏙️ Smart City — Islamabad Redefined

> A robust implementation of a **Smart City** landscape that uses Graphs, Trees, and other data structures to model and manage a fully simulated urban environment.

---

## 📖 Overview

**Smart City DSA** is a C++ terminal application that simulates the complete operational backbone of a modern city — modelled on Islamabad. The project demonstrates how advanced data structures can architect complex, real-world systems: from routing ambulances through the city graph to managing thousands of citizens across a hierarchical housing tree.

An interactive **FTXUI**-powered terminal UI lets you explore the live city graph, query the city database, manage every subsystem, and run a search engine across all city resources.

---

## 💡 Motivation

Urban management is inherently a data-structure problem:

- **Where is the nearest hospital?** → Dijkstra's shortest path on a weighted graph.
- **Who lives in Sector F-7?** → N-ary tree traversal (Sector → Street → House → Citizen).
- **Which pharmacy stocks Paracetamol?** → O(1) hash-table lookup by medicine name.
- **Which ER patient is most critical?** → Min-heap priority queue.

This project maps every one of those real-world concerns onto a concrete data structure, providing a working proof-of-concept that DSA theory translates directly into scalable system design.

---

## ✨ Key Features

| Feature | Description |
|---|---|
| **Interactive City Graph** | Live visual map of nodes (stops, hospitals, schools, malls, …) and weighted roads rendered in the terminal |
| **Shortest-Path Routing** | Dijkstra's algorithm finds optimal routes between any two points in the city |
| **Nearest-Facility Search** | Locate the closest hospital, school, pharmacy, or any facility type from any node |
| **Population Management** | Full N-ary hierarchy (Sector → Street → House → Citizen) with CNIC-indexed lookups |
| **Education System** | School → Department → Class tree; subject-based school discovery |
| **Transport System** | City buses, school buses, and ambulances with linked-list routes and circular-queue passenger simulation |
| **Healthcare System** | Hospitals with min-heap Emergency Room queues; pharmacies with medicine/formula hash lookups |
| **Commercial System** | Mall → Shop → Product hierarchy; category and product name hash-indexed search |
| **City Management** | Add/remove/edit every entity (citizens, schools, buses, hospitals, shops, …) at runtime |
| **Search Engine** | Unified cross-subsystem search across all city resources |
| **CSV Dataset Loader** | Seed the city from bundled CSV files (Demo Mode or Full Mode) |

---

## 🏗️ Architecture

```
Smart_City/
├── main.cpp                    ← Entry point — creates CitySimulator and calls run()
├── SmartCity.h                 ← Central hub: owns all managers and the city graph
├── CityGraph.h / CityUtils.h  ← Graph header redirects (actual code in source/CityGrid/)
│
├── data_structures/            ← All custom, generic data structures (no STL containers)
│   ├── Vector.h                ← Dynamic array
│   ├── LinkedLists.h           ← Singly linked list
│   ├── Stack.h                 ← Stack (used for travel history)
│   ├── Queue.h                 ← Standard queue
│   ├── CircularQueue.h         ← Circular queue (bus-stop passenger simulation)
│   ├── PriorityQueue.h         ← Min-heap (hospital ER dispatch)
│   ├── HashTable.h             ← Hash table with separate chaining
│   ├── BST.h                   ← Binary search tree
│   ├── NaryTree.h              ← N-ary tree (housing hierarchy)
│   └── CustomSTL.h             ← Convenience include for all DS headers
│
├── utils/
│   ├── Coordinate.h            ← Lat/lon coordinate helpers
│   ├── ID_Generator.h          ← Auto-incrementing ID generation
│   ├── Location.h              ← Base location type
│   └── ModuleUtils.h           ← Shared utility helpers
│
├── source/
│   ├── CityGrid/               ← Graph infrastructure
│   │   ├── CityGraph.h         ← Adjacency-list graph + Dijkstra + road management
│   │   └── CityUtils.h         ← Node/edge structs, sector grid definitions
│   │
│   ├── HousingSystem/          ← Population & housing
│   │   ├── Citizen.h           ← Citizen entity (CNIC, name, age, address, status)
│   │   ├── HousingHierarchy.h  ← Sector / Street / House tree nodes
│   │   └── PopulationManager.h ← N-ary tree + CNIC hash-table management
│   │
│   ├── SchoolSystem/           ← Education
│   │   ├── School.h / Department.h / Class.h
│   │   ├── Student.h / Faculty.h
│   │   └── SchoolManager.h     ← School tree + subject hash lookup
│   │
│   ├── TransportSystem/        ← Mobility
│   │   ├── Vehicle.h / Bus.h / SchoolBus.h / Ambulance.h
│   │   └── TransportManager.h  ← Route linked lists, circular-queue stops, ambulance dispatch
│   │
│   ├── MedicalSystem/          ← Healthcare
│   │   ├── Hospital.h / Pharmacy.h / Doctor.h / Patient.h / Medicine.h
│   │   └── MedicalManager.h    ← Priority-queue ER, medicine/formula hash lookup
│   │
│   ├── CommercialSystem/       ← Commerce
│   │   ├── Mall.h / Shop.h / Product.h
│   │   └── CommercialManager.h ← Mall tree + product/category hash lookup
│   │
│   └── Simulator/              ← FTXUI interactive terminal UI
│       ├── CitySimulator.h     ← Main simulator loop & state machine
│       ├── CityManagement.h    ← CRUD operations exposed to the UI
│       ├── CityGraphView.h     ← Terminal rendering of the city graph
│       ├── CityDatabaseView.h  ← Browse city entities
│       ├── CityManagementView.h← Management menu views
│       ├── CityEditorViews.h   ← Add/edit dialogs
│       ├── CitySearchEngineView.h ← Search UI
│       └── assets/             ← PNG icons (hospital, school, mall, pharmacy, stop)
│
├── termgl/                     ← Custom lightweight terminal graphics library (C/C++)
│
└── dataset/                    ← Seed CSV files
    ├── stops.csv               ← Bus stops / graph nodes
    ├── schools.csv             ← Schools
    ├── hospitals.csv           ← Hospitals
    ├── pharmacies.csv          ← Pharmacies
    ├── buses.csv               ← Bus routes & vehicles
    ├── schoolbuses.csv         ← School bus routes
    ├── ambulances.csv          ← Ambulance fleet
    ├── malls.csv               ← Malls
    ├── shops.csv               ← Shops & products
    └── population.csv          ← Citizen records
```

---

## 🧩 Data Structures & Algorithms

| Data Structure | Where Used | Purpose |
|---|---|---|
| **Graph** (Adjacency List) | `CityGraph` | Models the city road network with weighted edges |
| **Dijkstra's Algorithm** | `CityGraph::findShortestPath` | Optimal routing between any two nodes |
| **N-ary Tree** | `PopulationManager` | Sector → Street → House → Citizen housing hierarchy |
| **3-level Tree** | `SchoolManager` | School → Department → Class education hierarchy |
| **Hash Table** (separate chaining) | All managers | O(1) lookup by ID, name, medicine, product, category |
| **Priority Queue** (min-heap) | `Hospital` (ER queue), ambulance dispatch | Severity-ordered patient admission |
| **Singly Linked List** | Bus / SchoolBus / Ambulance routes | Sequential stop traversal |
| **Circular Queue** | `BusStopQueue` | Passenger waiting simulation at each stop |
| **Stack** | `SmartCity::travelHistory` | Log of citizen travel records |
| **BST** | `data_structures/BST.h` | Sorted, searchable collections |
| **Dynamic Array (Vector)** | Everywhere | Custom resizable array replacing `std::vector` |

---

## 🔧 Prerequisites

| Requirement | Details |
|---|---|
| **OS** | Windows 10/11 (the project is a Visual Studio solution) |
| **Compiler** | MSVC (Visual Studio 2019 or 2022) — C++17 or later |
| **IDE** | Visual Studio 2019 / 2022 |
| **Dependencies** | [FTXUI](https://github.com/ArthurSonzogni/FTXUI) — must be available to the linker (see below) |

> **Note:** FTXUI is the only external dependency. The project bundles its own `termgl` graphics library and all custom data structures — no other third-party libraries are required.

---

## 🚀 Build & Run

### 1. Clone the repository

```bash
git clone https://github.com/rayyan-41/Smart_City_DSA.git
cd Smart_City_DSA
```

### 2. Install FTXUI

The recommended approach is to install FTXUI via [vcpkg](https://vcpkg.io/):

```powershell
# Install vcpkg if not already available
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install FTXUI
.\vcpkg.exe install ftxui:x64-windows

# Integrate with Visual Studio
.\vcpkg.exe integrate install
```

### 3. Open the solution in Visual Studio

```
DSA_Final_Project.sln
```

Open this file with Visual Studio 2019 or 2022.

### 4. Set the working directory

The application loads CSV datasets at runtime using relative paths. You must set the **working directory** to the project output folder that contains the `dataset/` folder, or copy the `Smart_City/dataset/` folder next to the built executable.

In Visual Studio:
- Right-click the project → **Properties**
- **Debugging → Working Directory** → set to `$(ProjectDir)` (i.e., `Smart_City/`)

### 5. Build and run

- Set the build configuration to **Release** or **Debug** (x64 recommended).
- Press **F5** (Start Debugging) or **Ctrl+F5** (Start Without Debugging).

The terminal UI will launch and display the animated intro sequence.

---

## 🖥️ Usage & Expected Behaviour

### Startup

On launch you will see a multi-phase animated intro followed by the **Welcome Screen** with ASCII art:

```
 ██╗███████╗██╗      █████╗ ███╗   ███╗ █████╗ ██████╗  █████╗ ██████╗
 ...
```

### Main Menu

```
╔══════════════════════════╗
║     ISLAMABAD REDEFINED  ║
╠══════════════════════════╣
║  [1] Load City (Demo)    ║
║  [2] Load City (Full)    ║
║  [3] Exit                ║
╚══════════════════════════╝
```

- **Demo Mode** — loads a small representative subset of each CSV (fast startup, good for exploration).
- **Full Mode** — loads the complete bundled dataset.

### Views

| View | Key | Description |
|---|---|---|
| **Graph View** | `G` | Renders the city graph. Select source/destination nodes, press Enter to run Dijkstra and highlight the shortest path. |
| **Database View** | `D` | Browse all entities: stops, schools, hospitals, pharmacies, buses, citizens, malls. |
| **Management Menu** | `M` | Add, remove, or edit entities across all six subsystems. |
| **Search View** | `S` | Type a query to search across all city resources simultaneously. |

### Example: Finding the shortest path

1. Open **Graph View**.
2. Use arrow keys to select a **start node** (e.g., a bus stop in F-7).
3. Select an **end node** (e.g., a hospital in G-9).
4. Press **Enter** — the path is highlighted in colour and the distance is displayed.

### Example: Emergency dispatch

From **Management Menu → Medical → Emergency Admit**:
- Enter a patient's CNIC, disease, and severity (1–10).
- The patient is inserted into the hospital's **min-heap priority queue**.
- The most critical patient is always served next.

---

## 📂 Repository Structure

```
Smart_City_DSA/
├── DSA_Final_Project.sln       ← Visual Studio solution file
├── README.md
├── .gitignore                  ← Visual Studio gitignore template
├── .gitattributes
└── Smart_City/                 ← All C/C++ source code
    ├── main.cpp
    ├── SmartCity.h
    ├── CityGraph.h
    ├── CityUtils.h
    ├── data_structures/        ← Custom generic data structures
    ├── utils/                  ← Shared utilities (ID gen, coordinates)
    ├── source/                 ← Six city subsystems + simulator UI
    ├── termgl/                 ← Terminal graphics library
    └── dataset/                ← Seed CSV files
```

---

## 🤝 Contributing

Contributions are welcome! Please follow these steps:

1. **Fork** the repository and create a feature branch:
   ```bash
   git checkout -b feature/your-feature-name
   ```
2. Make your changes, keeping them focused on a single concern.
3. Ensure your code compiles cleanly with no new warnings under Visual Studio (C++17, x64).
4. **Do not** modify the bundled data structures in `data_structures/` without good reason — these are the pedagogical core of the project.
5. Open a **Pull Request** against `master` with a clear description of what you changed and why.

### Code style guidelines

- Follow the existing pattern of inline implementations inside `.h` files (header-only style).
- Use `#pragma once` guards.
- Keep public APIs documented with brief block comments (see `MedicalManager.h` as an example).
- Prefer the custom `Vector<T>` over `std::vector` and the custom `HashTable<K,V>` over `std::unordered_map`.

---

## 🗺️ Future Improvements Roadmap

- [ ] **Cross-platform build** — Add a `CMakeLists.txt` so the project builds on Linux/macOS without Visual Studio.
- [ ] **Dynamic graph loading** — Allow users to import custom city maps from any CSV/JSON without recompiling.
- [ ] **A* pathfinding** — Supplement Dijkstra with A* for heuristic-guided, faster routing on large graphs.
- [ ] **Red-Black Tree** — Replace the BST with a self-balancing RB-tree for guaranteed O(log n) operations.
- [ ] **Real-time simulation mode** — Animate bus movements along routes and citizen commutes on the graph view.
- [ ] **Persistence layer** — Save and reload the city state (add/remove edits) between sessions.
- [ ] **Unit tests** — Add a test harness for every data structure and manager class.
- [ ] **Emergency alert system** — Automatically dispatch the nearest available ambulance when a critical patient is admitted.
- [ ] **Traffic simulation** — Model road congestion as dynamic edge weights affecting Dijkstra's output.

---

## 📜 License

No explicit license file is currently included in this repository.

> **All rights reserved** — © 2024 the contributors of `rayyan-41/Smart_City_DSA`.
> Contact the repository owner before using, copying, or redistributing any part of this code.

---

## 👥 Authors

This project was developed as a final project for a Data Structures & Algorithms course.

| Student ID |
|---|
| 24I-0546 |
| 24I-0581 |
| 24I-0767 |
