/*
 * Smart City Management System - Entry Point
 */

#include "SmartCity.h"
#include <iostream>

int main() {
    SmartCity city;
    
    if (city.initialize()) {
        CityStats stats = city.getCityStats();
        TransportStats tStats = city.getTransportStats();
        
        std::cout << "============================================" << std::endl;
        std::cout << "   SMART CITY MANAGEMENT SYSTEM" << std::endl;
        std::cout << "   New Islamabad - City of the Future" << std::endl;
        std::cout << "============================================" << std::endl;
        std::cout << std::endl;
        
        std::cout << "=== City Infrastructure ===" << std::endl;
        std::cout << "  Total Nodes: " << stats.totalNodes << std::endl;
        std::cout << "  Bus Stops: " << stats.busStops << std::endl;
        std::cout << "  Schools: " << stats.totalSchools << std::endl;
        std::cout << "  Hospitals: " << stats.totalHospitals << std::endl;
        std::cout << "  Pharmacies: " << stats.totalPharmacies << std::endl;
        std::cout << std::endl;
        
        std::cout << "=== Transport System ===" << std::endl;
        std::cout << "  Total Buses: " << tStats.totalBuses << std::endl;
        std::cout << "  Active Buses: " << tStats.activeBuses << std::endl;
        std::cout << "  Total School Buses: " << tStats.totalSchoolBuses << std::endl;
        std::cout << "  Active School Buses: " << tStats.activeSchoolBuses << std::endl;
        std::cout << "  Total Ambulances: " << tStats.totalAmbulances << std::endl;
        std::cout << "  Available Ambulances: " << tStats.availableAmbulances << std::endl;
        std::cout << "  Pending Transfers: " << tStats.pendingTransfers << std::endl;
        std::cout << std::endl;
        
        std::cout << "=== Population ===" << std::endl;
        std::cout << "  Sectors: " << stats.totalSectors << std::endl;
        std::cout << "  Streets: " << stats.totalStreets << std::endl;
        std::cout << "  Houses: " << stats.totalHouses << std::endl;
        std::cout << "  Citizens: " << stats.totalCitizens << std::endl;
        std::cout << std::endl;
        
        std::cout << "============================================" << std::endl;
        std::cout << "   City Initialized Successfully!" << std::endl;
        std::cout << "============================================" << std::endl;
        
    } else {
        std::cout << "Failed to initialize city." << std::endl;
    }
    
    return 0;
}
