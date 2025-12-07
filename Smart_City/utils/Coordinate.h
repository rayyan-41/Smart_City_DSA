#pragma once
#include <string>
using std::string;

class Coordinate {
public:
    double x;   
    double y;   

    Coordinate();
    Coordinate(double x, double y);
    Coordinate(const Coordinate& other);
    Coordinate& operator=(const Coordinate& other);
    ~Coordinate();
};

Coordinate::Coordinate() : x(0.0), y(0.0) {}

Coordinate::Coordinate(double x, double y) : x(x), y(y) {}

Coordinate::Coordinate(const Coordinate& other)
    : x(other.x), y(other.y) {
}

Coordinate& Coordinate::operator=(const Coordinate& other) {
    if (this != &other) {
        x = other.x;
        y = other.y;
    }
    return *this;
}

Coordinate::~Coordinate() {}
