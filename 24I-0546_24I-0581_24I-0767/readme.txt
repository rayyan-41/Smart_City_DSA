Smart City Management System (Islamabad Redefined)
Project Overview
This project is a comprehensive C++ application designed to simulate and manage the infrastructure of a Smart City (modeled after the grid layout of Islamabad, Pakistan). It utilizes advanced Data Structures and Algorithms (DSA) to handle various city subsystems, including Transport, Education, Healthcare, Housing, and Commerce.

The system features a visual simulator (using the FTXUI library) that renders the city map, visualizes shortest paths using Dijkstra's algorithm, and manages real-time simulation of traffic and emergency services.

Features
City Graph & Navigation: A graph-based representation of sectors (E, F, G, H, I series), roads, and intersections.

Pathfinding: Implementation of Dijkstra's Algorithm for shortest path routing (used for navigation and emergency services).

Transport System:

Buses: Scheduled routes with stop queues and passenger management.

School Buses: Home-to-school pickup logic and routing.

Ambulances: Priority-based emergency dispatch and routing to hospitals.

Medical System: Management of Hospitals, Doctors, and Patient admissions (including ER priority queues). Includes Pharmacy inventory management.

Education System: Hierarchy of Schools, Departments, Faculty, and Students.

Commercial System: Management of Malls, Shops, and Inventory using Hash Tables for fast lookups.

Housing & Population: N-ary Tree structure representing Sectors -> Streets -> Houses -> Citizens.

Visual Interface: Console-based GUI for map visualization, database querying, and statistics.

Data Structures Used
The project is built upon custom implementations of standard data structures (found in Smart_City/data_structures/):

Graph: Adjacency List with weighted edges.

Hash Table: For O(1) lookups of citizens, products, and facilities.

Priority Queue: (Min-Heap) For Dijkstra’s algorithm and ER/Ambulance dispatch.

BST (Binary Search Tree): For data sorting and retrieval.

N-ary Tree: For hierarchical population management.

Linked Lists & Circular Lists: For vehicle routes and queues.

Stack & Queue: For travel history and standard processing.

Vector: Dynamic array implementation.

Prerequisites
Operating System: Windows (Recommended due to Visual Studio solution files).

IDE: Microsoft Visual Studio 2019 or 2022.

Language Standard: C++17 or C++20.

Dependencies:

FTXUI: Functional Terminal User Interface library (Required for the visual interface). Note: Ensure the library headers and binaries are linked in the project properties.

Setup and Installation
Clone/Extract: Extract the project files into a directory. Ensure the folder structure is maintained (e.g., Smart_City/source, Smart_City/dataset, etc.).

Open Project: Locate the DSA_Final_Project.sln file and open it in Microsoft Visual Studio.

Verify Dataset Location: The application reads data from CSV files located in the Smart_City/dataset/ folder.

Ensure the Working Directory in Visual Studio is set correctly so the program can find the folder dataset/.

To check this: Right-click Project -> Properties -> Debugging -> Working Directory. It should usually be $(ProjectDir) or point to the folder containing the dataset folder.

Dependencies (FTXUI): If the project fails to build due to missing ftxui headers:

You may need to install FTXUI via vcpkg or manually link the include and lib directories in the Project Properties (C/C++ -> General -> Additional Include Directories).

Build: Select Debug or Release configuration (x64 recommended) and build the solution (Ctrl+Shift+B).

How to Run
Start the application via Visual Studio (Local Windows Debugger) or by running the executable generated in the x64/Debug or x64/Release folder.

Intro Sequence: The application will play a short intro animation.

Main Menu: You will be presented with the main dashboard.

Operational Modes
Initialize City:

On first run, select "CSV Selection" or "Initialize" to load the data from the CSV files (Population, Hospitals, Schools, etc.). Wait for the loading bar to complete.

Graph View (Press '1'):

Displays the map of Islamabad sectors.

Controls:

Arrow Keys: Pan the map.

+ / -: Zoom in and out.

R: Toggle Roads.

C: Toggle Corner nodes.

T: Toggle Traffic animation.

D: Enter Dijkstra Pathfinding mode.

Esc: Return to menu.

Dijkstra Pathfinding (Press 'D' inside Graph View):

Select a Start Node from the list.

Select a Destination Type (Nearest Hospital, School, Pharmacy, or Custom Location).

The system will calculate and highlight the shortest path on the map.

Database View (Press '2'):

Browse lists of citizens, schools, hospitals, and commercial entities filtered by Sector.

Tab: Switch between categories.

S: Open Search Engine.

Management Console (Press '3'):

View system statistics (total population, active buses, etc.).

(Future implementations allow for adding/removing structures via this menu).

Project Structure
Smart_City/main.cpp: Entry point of the application.

Smart_City/SmartCity.h: Core wrapper class connecting all managers.

Smart_City/source/: Contains the logic for specific modules.

CityGrid/: Graph logic and geometry utilities.

CommercialSystem/: Malls, shops, products.

HousingSystem/: Population and housing hierarchy.

MedicalSystem/: Hospitals, doctors, patients.

SchoolSystem/: Schools, faculty, students.

TransportSystem/: Buses, ambulances, scheduling.

Simulator/: The FTXUI visual engine code (CitySimulator.h).

Smart_City/data_structures/: Custom template classes (Vector, List, etc.).

Smart_City/dataset/: CSV files containing city data.

Authors
Rayyan Ahmad Sultan

Omar Abdullah Khan Niazi

Aryan Ali Khan