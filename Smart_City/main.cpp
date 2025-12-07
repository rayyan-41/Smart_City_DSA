#include <iostream>
#include "source/Simulator/CitySimulator.h"

int main() {
    try {
        CitySimulator simulator;
        simulator.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
