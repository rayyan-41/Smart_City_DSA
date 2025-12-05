#ifndef CITIZEN_NODE_H
#define CITIZEN_NODE_H

#include <string>
using namespace std;

struct CitizenNode {
    int uniqueID; 
    string cnic;
    string name;
    int age;
    string occupation;
    CitizenNode* nextHash;

    CitizenNode(string c, string n, int a, string o)
        : uniqueID(-1), cnic(c), name(n), age(a), occupation(o), nextHash(NULL) {
    }
};

#endif