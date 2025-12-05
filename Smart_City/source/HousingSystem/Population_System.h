#ifndef POPULATION_SYSTEM_H
#define POPULATION_SYSTEM_H

#include <iostream>
#include <fstream>
#include "String_Helper.h"
#include "Citizen_Node.h"
#include "House_Nodes.h"
#include "Hash_Tables.h"

using namespace std;

class PopulationSystem {
private:
    SectorNode* cityRoot;
    CitizenHashTable populationDB; // Segregated Hash Table

    void AddToTree(CitizenNode* c, string sector, string street, string house);

public:
    PopulationSystem();
    void LoadPopulation(string filename);
    void SearchCitizen(string cnic);
    void ShowPopulationTree();
};

// --- Implementation ---

PopulationSystem::PopulationSystem() : cityRoot(NULL) {}

void PopulationSystem::AddToTree(CitizenNode* c, string sector, string street, string house) {
    // 1. Sector Logic
    SectorNode* currSec = cityRoot;
    SectorNode* prevSec = NULL;
    while (currSec != NULL && currSec->sectorName != sector) {
        prevSec = currSec;
        currSec = currSec->next;
    }
    if (currSec == NULL) {
        currSec = new SectorNode(sector);
        if (prevSec == NULL) cityRoot = currSec;
        else prevSec->next = currSec;
    }

    // 2. House Logic
    HouseNode* currHouse = currSec->houseHead;
    HouseNode* prevHouse = NULL;
    while (currHouse != NULL && (currHouse->houseNo != house || currHouse->street != street)) {
        prevHouse = currHouse;
        currHouse = currHouse->next;
    }
    if (currHouse == NULL) {
        currHouse = new HouseNode(house, street);
        currHouse->familyHead = new FamilyNode(1); // Default family
        if (prevHouse == NULL) currSec->houseHead = currHouse;
        else prevHouse->next = currHouse;
    }

    // 3. Family Logic
    FamilyNode* fam = currHouse->familyHead;
    IndividualNode* newIndiv = new IndividualNode(c);
    newIndiv->next = fam->membersHead;
    fam->membersHead = newIndiv;
}

void PopulationSystem::LoadPopulation(string filename) {
    ifstream file(filename);
    if (!file.is_open()) return;
    string line;
    getline(file, line);

    while (getline(file, line)) {
        StringList tokens;
        StringHelper::SplitCSV(line, tokens);
        if (tokens.count >= 7) {
            CitizenNode* newCit = new CitizenNode(tokens.tokens[0], tokens.tokens[1],
                StringHelper::StrToInt(tokens.tokens[2]), tokens.tokens[6]);

            populationDB.Insert(newCit);
            AddToTree(newCit, tokens.tokens[3], tokens.tokens[4], tokens.tokens[5]);
        }
    }
}

void PopulationSystem::SearchCitizen(string cnic) {
    CitizenNode* temp = populationDB.Search(cnic);
    if (temp) {
        cout << "Citizen: " << temp->name << ", Age: " << temp->age << endl;
    }
    else {
        cout << "Citizen not found." << endl;
    }
}

void PopulationSystem::ShowPopulationTree() {
    SectorNode* s = cityRoot;
    while (s != NULL) {
        cout << "[Sector " << s->sectorName << "]" << endl;
        HouseNode* h = s->houseHead;
        while (h != NULL) {
            cout << "  House " << h->houseNo << ", St " << h->street << endl;
            h = h->next;
        }
        s = s->next;
    }
}

#endif