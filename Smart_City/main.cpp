#include <iostream>
#include <cstdlib>
#include <ctime>
#include "source/HousingSystem/PopulationManager.h"
#include "source/SchoolSystem/SchoolManager.h"
#include "source/MedicalSystem/MedicalManager.h"

using namespace std;

// Utility to print citizens and their status
void printCitizens(PopulationManager& pop) {
    cout << "\n===== CURRENT CITIZENS =====\n";
    for (int i = 0; i < pop.masterList.getSize(); i++) {
        Citizen* c = pop.masterList[i];
        cout << "CNIC: " << c->cnic
            << " | Name: " << c->name
            << " | Age: " << c->age
            << " | Sector: " << c->sector
            << " | Status: " << c->currentStatus
            << endl;
    }
    cout << "Total Citizens: " << pop.masterList.getSize() << endl;
}

int main() {
    srand((unsigned)time(0));
    cout << "=== SMART CITY INTEGRATION TEST ===\n";

    // -------------------------------------------------------------------------
    // 1. POPULATION SETUP
    // -------------------------------------------------------------------------
    PopulationManager popMgr;
    cout << "\n[TEST] Creating citizens and housing hierarchy...\n";

    // Add some citizens
    popMgr.addCitizen("1234", "Ali", 20, "SectorA", 1, 1, "sdfsdf");
	popMgr.addCitizen("5678", "Zara", 30, "SectorA", 1, 1, "dfwefs");
	popMgr.addCitizen("A001", "Omar", 45, "SectorA", 1, 2, "sdfs");

    popMgr.addCitizen("Sara", 17, "SectorA", 1, 2);
    popMgr.addCitizen("Bilal", 35, "SectorB", 2, 1);
    popMgr.addCitizen("Hina", 40, "SectorB", 3, 1);
    popMgr.addCitizen("Tariq", 9, "SectorC", 5, 2);
    popMgr.addCitizen("Ahmad", 50, "SectorC", 5, 3);

    printCitizens(popMgr);

    // -------------------------------------------------------------------------
    // 2. SCHOOL SYSTEM SETUP
    // -------------------------------------------------------------------------
    cout << "\n[TEST] Building school system...\n";
    SchoolManager schoolMgr;

    // Create one school manually (simulate CSV load)
    School* s1 = new School("S01", "City Grammar", "SectorA", 4.5);
    s1->setSubjects({ "Math", "Science", "English", "Urdu" });
    s1->buildDepartmentsForSchool(); // internally adds depts/classes
    schoolMgr.addSchool(s1);

    // Enroll some citizens as students
    Citizen* cAli = popMgr.getCitizenByName("Ali");
    Citizen* cSara = popMgr.getCitizenByName("Sara");
    Citizen* cTariq = popMgr.getCitizenByName("Tariq");

    schoolMgr.addStudent("S01", "Science", cAli, 10);
    schoolMgr.addStudent("S01", "Science", cSara, 9);
    schoolMgr.addStudent("S01", "Science", cTariq, 4);

    cout << "\n[TEST] Simulating bus arrival (students go to school)...\n";
    School* school = schoolMgr.getSchoolByID("S01");
    school->processArrival(s1->findStudentByCNIC(cAli->cnic));
    school->processArrival(s1->findStudentByCNIC(cSara->cnic));

    printCitizens(popMgr);

    cout << "\n[TEST] Students leaving school...\n";
    school->processDeparture(s1->findStudentByCNIC(cAli->cnic));
    school->processDeparture(s1->findStudentByCNIC(cSara->cnic));
    printCitizens(popMgr);

    // -------------------------------------------------------------------------
    // 3. MEDICAL SYSTEM SETUP
    // -------------------------------------------------------------------------
    cout << "\n[TEST] Building hospital & pharmacy system...\n";
    MedicalManager medMgr;

    medMgr.addHospital("H01", "Shifa International", "SectorB", 10);
    medMgr.addPharmacy("P01", "HealthPlus", "SectorB");

    Hospital* hosp = medMgr.getHospitalByID("H01");

    cout << "\n[TEST] Admitting some citizens to hospital...\n";
    Citizen* cBilal = popMgr.getCitizenByName("Bilal");
    Citizen* cHina = popMgr.getCitizenByName("Hina");
    Patient p1(cBilal, "Flu", 3);
    Patient p2(cHina, "Heart Attack", 1);

    hosp->admitPatient(p1);
    hosp->admitPatient(p2);
    printCitizens(popMgr);

    cout << "\n[TEST] Discharging Bilal...\n";
    hosp->dischargePatient(p1.id);
    printCitizens(popMgr);

    cout << "\n[TEST] Simulating random disaster casualties...\n";
    hosp->simulateDisasterCasualties(&popMgr);
    printCitizens(popMgr);

    // -------------------------------------------------------------------------
    // 4. DELETION VERIFICATION
    // -------------------------------------------------------------------------
    cout << "\n[TEST] Deletion and housing hierarchy verification...\n";
    cout << "Removing a citizen manually: Tariq\n";
    popMgr.removeCitizen(cTariq->cnic);
    printCitizens(popMgr);

    cout << "\nVerifying Tariq no longer in any house or hash map:\n";
    if (popMgr.getCitizenByCNIC(cTariq->cnic) == nullptr)
        cout << "[PASS] Tariq successfully deleted.\n";
    else
        cout << "[FAIL] Tariq still found in hash map!\n";

    // -------------------------------------------------------------------------
    // 5. EDGE CASES
    // -------------------------------------------------------------------------
    cout << "\n[TEST] Adding duplicate citizen (should fail gracefully)...\n";
    bool ok = popMgr.addCitizen("Ali", 22, "SectorA", 1, 1);
    if (!ok) cout << "[PASS] Duplicate CNIC/name check prevented re-add.\n";

    cout << "\n[TEST] Admitting non-existent citizen to hospital (should fail)...\n";
    Citizen fake("0000000000000", "Ghost", 99, "SectorZ", 0, 0);
    Patient ghost(&fake, "Nothing", 5);
    bool admit = hosp->admitPatient(ghost);
    if (!admit) cout << "[PASS] Ghost patient rejected.\n";

    // -------------------------------------------------------------------------
    // 6. FINAL SUMMARY
    // -------------------------------------------------------------------------
    cout << "\n=== FINAL SYSTEM STATE ===\n";
    printCitizens(popMgr);

    cout << "\nRemaining hospitals: " << medMgr.getHospitalCount()
        << " | Pharmacies: " << medMgr.getPharmacyCount()
        << " | Schools: " << schoolMgr.getSchoolCount()
        << endl;

    cout << "\n=== INTEGRATION TEST COMPLETE ===\n";
    return 0;
}
