// Prevent Windows min/max macros and std::byte conflicts
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "source/Simulator/CitySimulator.h"
#include "termgl/Termgl.h"
#include "termgl/Termgl_Video.h"
#include <iostream>
#include "SmartCity.h"

int main() {
    CitySimulator simulator;
    simulator.run();
    return 0;
}