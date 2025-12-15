#pragma once
#include <string>
#include "Coordinate.h"

using std::string;

class Location {
public:
    string sector;      
    Coordinate coord;   

    Location();
    Location(const string& sector, const Coordinate& coord);
    Location(const string& sector, double x, double y);
    Location(const Location& other);
    Location& operator=(const Location& other);
    ~Location();
};

// ==================== Implemenation ====================
Location::Location()
    : sector(""), coord() {
}

Location::Location(const string& sector, const Coordinate& coord)
    : sector(sector), coord(coord) {
}

Location::Location(const string& sector, double x, double y)
    : sector(sector), coord(x, y) {
}

Location::Location(const Location& other)
    : sector(other.sector), coord(other.coord) {
}

Location& Location::operator=(const Location& other) {
    if (this != &other) {
        sector = other.sector;
        coord = other.coord;
    }
    return *this;
}

Location::~Location() {}
