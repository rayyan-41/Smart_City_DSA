#ifndef HOUSING_NODES_H
#define HOUSING_NODES_H

#include <string>
#include "Citizen_Node.h"
using namespace std;

struct IndividualNode {
    CitizenNode* data;
    IndividualNode* next;
    IndividualNode(CitizenNode* c) : data(c), next(NULL) {}
};

struct FamilyNode {
    int uniqueID; 
    int familyID;
    IndividualNode* membersHead;
    FamilyNode* next;
    FamilyNode(int id) : uniqueID(-1), familyID(id), membersHead(NULL), next(NULL) {}
};

struct HouseNode {
    int uniqueID;
    string houseNo;
    string street;
    FamilyNode* familyHead;
    HouseNode* next;
    HouseNode(string h, string s) : uniqueID(-1), houseNo(h), street(s), familyHead(NULL), next(NULL) {}
};

struct SectorNode {
    int uniqueID; 
    string sectorName;
    HouseNode* houseHead;
    SectorNode* next;
    SectorNode(string n) : uniqueID(-1), sectorName(n), houseHead(NULL), next(NULL) {}
};

#endif