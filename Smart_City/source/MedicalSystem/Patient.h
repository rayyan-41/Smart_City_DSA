#pragma once
#include <string>
#include "ModuleUtils.h" 

using std::string;

struct Patient {
    string id;
    string name;
    int age;
    string disease;

    // Severity: 1 (Critical) to 10 (Stable)
    // The Min-Heap Priority Queue uses this to triage patients.
    int severity;

    // Graph Integration:
    // When an ambulance is dispatched, it needs the patient's graph node
    // to calculate the shortest path (Dijkstra).
    Location location;
    string graphNodeID;

    Patient()
        : id(""), name(""), age(0), disease(""), severity(10), graphNodeID("") {
    }

    Patient(string id, string name, int age, string disease, int severity,
        string graphNodeID = "", double x = 0.0, double y = 0.0)
        : id(id), name(name), age(age), disease(disease), severity(severity),
        location("Unknown", x, y), graphNodeID(graphNodeID) {
    }

    // ---------------------------------------------------------
    // OPERATOR OVERLOADING FOR PRIORITY QUEUE (MIN-HEAP)
    // ---------------------------------------------------------

    // The PriorityQueue (Min-Heap) logic:
    // If we want severity 1 to be at the TOP, then 1 must be considered "smaller" 
    // than 10, and the heap must organize based on "smaller is higher priority".

    bool operator<(const Patient& other) const {
        // Returns true if this priority is LOWER than other
        // Severity 10 (Stable) < Severity 1 (Critical)
        return severity > other.severity;
    }

    bool operator>(const Patient& other) const {
        // Returns true if this priority is HIGHER than other
        // Severity 1 (Critical) > Severity 10 (Stable)
        return severity < other.severity;
    }

    bool operator==(const Patient& other) const {
        return id == other.id;
    }

    bool operator!=(const Patient& other) const {
        return !(*this == other);
    }
};