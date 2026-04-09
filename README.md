# Smart City Management System — Islamabad

A terminal-based **Smart City simulation** built in C++ for a Data Structures & Algorithms (DSA) final project. The system models the city of Islamabad as an interconnected graph and provides a rich TUI (Terminal User Interface) to manage, visualise, and interact with six core city subsystems.

---

## Table of Contents

1. [Project Overview](#project-overview)
2. [Features](#features)
3. [Data Structures Used](#data-structures-used)
4. [Prerequisites](#prerequisites)
5. [Building the Project](#building-the-project)
6. [Running the Application](#running-the-application)
7. [Repository Structure](#repository-structure)
8. [Dataset / CSV Files](#dataset--csv-files)
9. [Usage Walkthrough](#usage-walkthrough)
10. [Troubleshooting](#troubleshooting)
11. [Contributing](#contributing)
12. [License](#license)

---

## Project Overview

The **Smart City Management System** simulates a living city — Islamabad — by loading real-world-style CSV datasets and building an in-memory graph of city nodes (bus stops, hospitals, schools, malls, etc.). It then exposes a multi-screen TUI built with [FTXUI](https://github.com/ArthurSonzogni/FTXUI) and a custom **TermGL** rendering layer.

The project was developed as a DSA final assignment (student IDs: 24I-0546, 24I-0581, 24I-0767) to demonstrate practical application of core data structures inside a realistic, domain-rich scenario.

---

## Features

| Subsystem | Highlights |
|---|---|
| **City Graph** | Adjacency-list graph of all city nodes; Dijkstra's shortest path; nearest-facility search |
| **Transport System** | Public buses, school buses, and ambulances; circular passenger queues at stops; priority dispatch |
| **Medical System** | Hospitals with priority-queue emergency rooms; pharmacies with O(1) medicine lookup by name or formula |
| **Housing / Population** | N-ary tree hierarchy (Sector → Street → House → Citizen); CNIC-based O(1) citizen lookup |
| **Commercial System** | Malls → Shops → Products; hash-table lookups for fast product/shop search |
| **School System** | Schools → Departments → Classes → Students; faculty and school-bus integration |
| **Search Engine** | Cross-system search across all city entities from a single view |
| **City Editor** | Add / remove nodes and roads interactively through the TUI |
| **Graph View** | Visual ASCII map of the city graph with selectable nodes |
| **Database View** | Tabular view of all loaded entities (stops, hospitals, schools, …) |

---

## Data Structures Used

The project implements **all data structures from scratch** (no STL containers in core logic):

| Data Structure | Where Used |
|---|---|
| **Graph (Adjacency List)** | `CityGraph` — city node network |
| **Dijkstra's Algorithm** | Shortest path, bus-route calculation, nearest-facility search |
| **N-ary Tree** | `PopulationManager` — Sector/Street/House/Citizen hierarchy |
| **3-Level Tree** | `SchoolManager` — School/Department/Class hierarchy |
| **Hash Table** (separate chaining) | O(1) lookups in Medical, Commercial, and Population systems |
| **Priority Queue** (min-heap) | Hospital ER queue; ambulance dispatch priority |
| **Circular Queue** | Passenger waiting queues at bus stops |
| **Singly Linked List** | Bus, school-bus, and ambulance route management |
| **Stack** | City-wide travel history |
| **Dynamic Array (Vector)** | General-purpose list throughout all subsystems |
| **Binary Search Tree** | Auxiliary sorted lookups |

---

## Prerequisites

| Requirement | Version / Notes |
|---|---|
| **Operating System** | Windows 10 / 11 (project targets Win32/x64 via MSVC) |
| **Visual Studio** | 2022 (solution targets VS 18 format) |
| **C++ Standard** | C++17 or later |
| **FTXUI** | Included as a dependency — must be available to the linker (see build notes below) |
| **TermGL / miniaudio / stb_image** | Already bundled inside `Smart_City/termgl/` |

> **Note:** The project uses `WIN32_LEAN_AND_MEAN` and `NOMINMAX` macros, so it is tightly coupled to the Windows MSVC toolchain. Building on Linux/macOS is not officially supported.

---

## Building the Project

### Option A — Visual Studio IDE (recommended)

1. Open `DSA_Final_Project.sln` in **Visual Studio 2022**.
2. Ensure the **FTXUI** NuGet package or vcpkg integration is configured (see [Troubleshooting](#troubleshooting) if FTXUI headers are missing).
3. Select your desired configuration: `Debug | x64` or `Release | x64`.
4. Press **Ctrl + Shift + B** (Build Solution).
5. The compiled executable will be placed in:
   - Debug: `Smart_City\x64\Debug\Smart_City.exe`
   - Release: `Smart_City\x64\Release\Smart_City.exe`

### Option B — MSBuild (command line)

```bat
cd Smart_City_DSA
msbuild DSA_Final_Project.sln /p:Configuration=Release /p:Platform=x64
```

---

## Running the Application

1. After building, navigate to the output directory that contains `Smart_City.exe`.
2. **Copy the `dataset/` folder** into the same directory as the executable (or run from within the `Smart_City/` source folder so relative paths resolve correctly):
   ```
   Smart_City.exe
   dataset/
     stops.csv
     schools.csv
     hospitals.csv
     ...
   ```
3. Run `Smart_City.exe` from a terminal (CMD or PowerShell — a terminal that supports ANSI escape codes is recommended):
   ```bat
   .\Smart_City.exe
   ```
4. The application opens with an animated intro sequence, followed by the **Main Menu**.

---

## Repository Structure

```
Smart_City_DSA/
├── DSA_Final_Project.sln           # Visual Studio solution file
└── Smart_City/
    ├── main.cpp                    # Entry point — creates and runs CitySimulator
    ├── SmartCity.h                 # Central hub connecting all subsystem managers
    ├── CityGraph.h                 # Redirect header → source/CityGrid/CityGraph.h
    ├── CityUtils.h                 # Redirect header → source/CityGrid/CityUtils.h
    ├── Smart_City.vcxproj          # MSVC project file
    │
    ├── data_structures/            # Custom DSA implementations (no STL)
    │   ├── Vector.h                # Dynamic array
    │   ├── LinkedLists.h           # Singly linked list
    │   ├── Stack.h                 # Stack (travel history)
    │   ├── Queue.h                 # Basic queue
    │   ├── CircularQueue.h         # Circular queue (bus-stop passengers)
    │   ├── PriorityQueue.h         # Min-heap (hospital ER, ambulance dispatch)
    │   ├── HashTable.h             # Hash table with separate chaining
    │   ├── BST.h                   # Binary search tree
    │   ├── NaryTree.h              # N-ary tree (population hierarchy)
    │   └── CustomSTL.h             # Shared typedefs / helpers
    │
    ├── dataset/                    # CSV data files loaded at runtime
    │   ├── stops.csv               # Bus stop nodes (name, sector, lat/lon)
    │   ├── buses.csv               # Bus fleet and routes
    │   ├── schoolbuses.csv         # School bus routes
    │   ├── ambulances.csv          # Ambulance fleet
    │   ├── hospitals.csv           # Hospital facilities
    │   ├── pharmacies.csv          # Pharmacy facilities
    │   ├── schools.csv             # School facilities
    │   ├── malls.csv               # Mall facilities
    │   ├── shops.csv               # Shops within malls
    │   └── population.csv          # Citizen / housing data
    │
    ├── source/
    │   ├── CityGrid/               # Graph layer
    │   │   ├── CityGraph.h         # Adjacency-list graph + Dijkstra
    │   │   ├── CityUtils.h         # Node/edge structs, sector grid helpers
    │   │   └── SmartCity.h         # SmartCity class implementation
    │   │
    │   ├── TransportSystem/        # Buses, school buses, ambulances
    │   │   ├── Vehicle.h           # Base vehicle class
    │   │   ├── Bus.h               # Public bus (linked-list routes)
    │   │   ├── SchoolBus.h         # School bus
    │   │   ├── Ambulance.h         # Ambulance + priority dispatch
    │   │   └── TransportManager.h  # Loads CSV; exposes transport operations
    │   │
    │   ├── MedicalSystem/          # Hospitals and pharmacies
    │   │   ├── Hospital.h          # ER priority queue, doctor/patient records
    │   │   ├── Pharmacy.h          # Medicine inventory + hash lookups
    │   │   ├── Doctor.h
    │   │   ├── Patient.h
    │   │   ├── Medicine.h
    │   │   └── MedicalManager.h    # Loads CSV; O(1) lookups
    │   │
    │   ├── HousingSystem/          # Population / housing hierarchy
    │   │   ├── HousingHierarchy.h  # Sector, Street, House, Citizen structs
    │   │   ├── Citizen.h           # Citizen model
    │   │   └── PopulationManager.h # N-ary tree + CNIC hash lookup
    │   │
    │   ├── CommercialSystem/       # Malls, shops, products
    │   │   ├── Mall.h
    │   │   ├── Shop.h
    │   │   ├── Product.h
    │   │   └── CommercialManager.h # Hash-table product/shop search
    │   │
    │   ├── SchoolSystem/           # Schools, departments, classes, students
    │   │   ├── School.h
    │   │   ├── Department.h
    │   │   ├── Class.h
    │   │   ├── Student.h
    │   │   ├── Faculty.h
    │   │   └── SchoolManager.h     # Loads CSV; tree-based hierarchy
    │   │
    │   └── Simulator/              # TUI front-end
    │       ├── CitySimulator.h     # Main simulation loop + screen dispatching
    │       ├── CityManagement.h    # Management operations (add/remove entities)
    │       ├── CityManagementView.h
    │       ├── CityGraphView.h     # ASCII city-graph visualisation
    │       ├── CityDatabaseView.h  # Tabular entity browser
    │       ├── CityEditorViews.h   # Interactive node/road editor
    │       ├── CitySearchEngineView.h # Cross-system search
    │       └── assets/             # PNG icons used in the TUI
    │           ├── hospital.png
    │           ├── pharmacy.png
    │           ├── school.png
    │           ├── mall.png
    │           └── stop.png
    │
    ├── termgl/                     # Bundled TermGL rendering library
    │   ├── Termgl.h / .cpp         # Core window & rendering
    │   ├── Termgl_Video.h / .cpp   # Video playback (FFmpeg pipe)
    │   ├── Termgl_UI.cpp           # UI widgets
    │   ├── Termgl_Input.cpp        # Keyboard/mouse input
    │   ├── Termgl_Texture.cpp      # Image/texture loading
    │   ├── miniaudio.h             # Bundled audio library (single-header)
    │   └── stb_image.h             # Bundled image-loading library (single-header)
    │
    └── utils/                      # Shared helpers
        ├── Coordinate.h            # Lat/lon coordinate type
        ├── ID_Generator.h          # Auto-incrementing ID generation
        ├── Location.h              # Named location helper
        └── ModuleUtils.h           # Miscellaneous utility functions
```

---

## Dataset / CSV Files

All CSV files live in `Smart_City/dataset/` and are loaded automatically at startup. The columns expected by each file are described below.

| File | Key Columns |
|---|---|
| `stops.csv` | Stop ID, Name, Sector, Latitude, Longitude |
| `buses.csv` | Bus ID, Route (ordered stop IDs), Capacity, Fare |
| `schoolbuses.csv` | Bus ID, School ID, Route stop IDs |
| `ambulances.csv` | Ambulance ID, Home hospital ID, Status |
| `hospitals.csv` | Hospital ID, Name, Sector, Beds, ER capacity |
| `pharmacies.csv` | Pharmacy ID, Name, Sector, Medicine inventory list |
| `schools.csv` | School ID, Name, Sector, Departments |
| `malls.csv` | Mall ID, Name, Sector |
| `shops.csv` | Shop ID, Mall ID, Name, Products |
| `population.csv` | CNIC, Name, Age, Sector, Street No., House No., Occupation |

The application offers two load modes at startup:
- **Demo Mode** — loads a small representative subset for fast exploration.
- **Full Mode** — loads all rows from every CSV file.

---

## Usage Walkthrough

### 1. Intro & Welcome Screen
On launch the simulator plays a multi-phase ASCII-art intro (Islamabad title animation). Press any key to skip each phase.

### 2. Main Menu
Use the **↑ / ↓** arrow keys to navigate and **Enter** to select:
- **Load City (Demo / Full)** — parse CSV files and build the city graph.
- **Graph View** — browse the city node graph visually.
- **Database View** — browse all loaded entities in a table.
- **Management Menu** — add/remove nodes, roads, citizens, vehicles, etc.
- **Search** — cross-system keyword search.
- **Exit**

### 3. Graph View
Displays the city as a navigable ASCII graph. Select a node to see its connections, type, sector, and coordinates. Use the shortest-path tool to find the route between any two nodes.

### 4. Database View
Tabular paged view of stops, hospitals, schools, pharmacies, malls, buses, and citizens. Filter by subsystem using the category selector.

### 5. Management Menu
Provides sub-menus for each subsystem:
- **Transport** — dispatch bus/ambulance, view routes, simulate stop queues.
- **Medical** — admit patient (ER queue), search medicine by name/formula.
- **Housing** — add/remove citizen, browse sector hierarchy.
- **Commercial** — search products, view mall → shop → product tree.
- **School** — enrol/remove student, view school hierarchy.
- **City Graph Editor** — add node, add/remove road, view adjacency list.

### 6. Search
Type a query to search all subsystems simultaneously. Results are grouped by category.

---

## Troubleshooting

| Problem | Solution |
|---|---|
| **FTXUI headers not found** | **Option 1 — vcpkg (recommended):** `vcpkg install ftxui:x64-windows` then `vcpkg integrate install`. **Option 2 — NuGet:** search for `ftxui` in the Visual Studio NuGet Package Manager and install it for the `Smart_City` project. |
| **CSV files not found at runtime** | Ensure the `dataset/` folder is in the **working directory** of the executable (usually the project root when running from VS, or alongside the `.exe` when running standalone). |
| **Garbled characters / missing box-drawing** | Use a terminal / console font that supports Unicode (e.g., *Windows Terminal* with **Cascadia Code** or **Consolas**). Set the console to UTF-8: `chcp 65001`. |
| **Crash on startup** | Run in Debug mode from Visual Studio to catch assertions. Ensure all CSV files are present and correctly formatted. |
| **Video player not working** | The `termgl::VideoPlayer` requires `ffmpeg.exe` in a `bin/` folder next to the executable. This feature is secondary to the city simulation. |
| **Build errors about `std::byte`** | Ensure `NOMINMAX` and `WIN32_LEAN_AND_MEAN` are defined before any Windows headers (already set in `main.cpp`). |

---

## Contributing

Contributions are welcome! Please follow these steps:

1. Fork the repository and create a feature branch:
   ```bash
   git checkout -b feature/your-feature-name
   ```
2. Keep each commit focused and descriptive.
3. Ensure the project builds without warnings in both Debug and Release configurations.
4. Open a Pull Request describing what was changed and why.

Code style guidelines:
- Match the existing naming conventions (PascalCase for classes, camelCase for variables).
- Keep custom data-structure implementations in `data_structures/`.
- Add new subsystem code under `source/<SubsystemName>/`.

---

## License

This project was created as an academic assignment. All rights reserved by the authors (24I-0546, 24I-0581, 24I-0767). If you wish to reuse or adapt it, please credit the original authors and your institution.
