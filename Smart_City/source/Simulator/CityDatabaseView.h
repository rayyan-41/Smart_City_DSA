#pragma once
#ifndef CITY_DATABASE_VIEW_H
#define CITY_DATABASE_VIEW_H

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/component/event.hpp"

#include "../../SmartCity.h"
#include "../../data_structures/Vector.h"

using namespace ftxui;
using std::string;

// Forward declarations
class CitySimulator;

// ============================================================================
// DATABASE VIEW COMPONENT
// ============================================================================

class CityDatabaseView {
private:
    SmartCity* islamabad;
    CitySimulator* simulator;

    int selectedSectorIdx;
    int selectedCategoryIdx;
    int selectedItemIdx;
    int focusPanel;

    std::vector<string> categories;
    std::vector<string> sectorList;

    struct DBItem {
        string id;
        string name;
        string type;
        string sector;
        void* objPtr;
    };
    std::vector<DBItem> items;

public:
    CityDatabaseView(SmartCity* city, CitySimulator* sim);
    bool run(); // Returns true if should go back to main menu

private:
    void refreshItems();
    Element renderSectorPanel();
    Element renderItemPanel();
    Element renderDetailPanel();
};

inline CityDatabaseView::CityDatabaseView(SmartCity* city, CitySimulator* sim)
    : islamabad(city), simulator(sim),
    selectedSectorIdx(0), selectedCategoryIdx(0),
    selectedItemIdx(0), focusPanel(0) {

    categories = { "All", "Stops", "Schools", "Hospitals", "Pharmacies", "Malls" };
    for (int i = 0; i < SECTOR_COUNT; i++) {
        sectorList.push_back(SECTOR_GRID[i].name);
    }
}

inline void CityDatabaseView::refreshItems() {
    items.clear();
    string currentSector = sectorList[selectedSectorIdx];
    string cat = categories[selectedCategoryIdx];

    // 1. STOPS (From Graph)
    if ((cat == "All" || cat == "Stops") && islamabad->getCityGraph()) {
        CityGraph* g = islamabad->getCityGraph();
        for (int i = 0; i < g->getNodeCount(); i++) {
            CityNode* n = g->getNode(i);
            if (n && n->type == "STOP" && n->sector == currentSector) {
                items.push_back({ n->databaseID, n->name, "STOP", n->sector, n });
            }
        }
    }

    // 2. SCHOOLS (From SchoolManager)
    if ((cat == "All" || cat == "Schools") && islamabad->getSchoolManager()) {
        auto& schools = islamabad->getSchoolManager()->schools;
        for (int i = 0; i < schools.getSize(); i++) {
            if (schools[i]->getSector() == currentSector) {
                items.push_back({ schools[i]->id, schools[i]->name, "SCHOOL", schools[i]->getSector(), schools[i] });
            }
        }
    }

    // 3. HOSPITALS (From MedicalManager)
    if ((cat == "All" || cat == "Hospitals") && islamabad->getMedicalManager()) {
        auto& hospitals = islamabad->getMedicalManager()->hospitals;
        for (int i = 0; i < hospitals.getSize(); i++) {
            if (hospitals[i]->sector == currentSector) {
                items.push_back({ hospitals[i]->id, hospitals[i]->name, "HOSPITAL", hospitals[i]->sector, hospitals[i] });
            }
        }
    }

    // 4. PHARMACIES (From MedicalManager)
    if ((cat == "All" || cat == "Pharmacies") && islamabad->getMedicalManager()) {
        auto& pharmacies = islamabad->getMedicalManager()->pharmacies;
        for (int i = 0; i < pharmacies.getSize(); i++) {
            if (pharmacies[i]->sector == currentSector) {
                items.push_back({ pharmacies[i]->id, pharmacies[i]->name, "PHARMACY", pharmacies[i]->sector, pharmacies[i] });
            }
        }
    }

    // 5. MALLS (From CommercialManager)
    if ((cat == "All" || cat == "Malls") && islamabad->getCommercialManager()) {
        auto& malls = islamabad->getCommercialManager()->malls;
        for (int i = 0; i < malls.getSize(); i++) {
            if (malls[i]->getSector() == currentSector) {
                items.push_back({ malls[i]->id, malls[i]->name, "MALL", malls[i]->getSector(), malls[i] });
            }
        }
    }
}

inline Element CityDatabaseView::renderSectorPanel() {
    Elements sectorItems;
    sectorItems.push_back(text("SECTORS") | bold | color(ftxui::Color::Cyan));
    sectorItems.push_back(separator());
    for (int i = 0; i < (int)sectorList.size() && i < 18; i++) {
        int idx = std::max(0, selectedSectorIdx - 8) + i;
        if (idx >= (int)sectorList.size()) break;
        auto item = text((idx == selectedSectorIdx ? "> " : "  ") + sectorList[idx]);
        if (idx == selectedSectorIdx) {
            item = item | bold | (focusPanel == 0 ? bgcolor(ftxui::Color::Blue) : color(ftxui::Color::Green));
        }
        sectorItems.push_back(item);
    }
    return vbox(sectorItems) | border | size(WIDTH, EQUAL, 14);
}

inline Element CityDatabaseView::renderItemPanel() {
    Elements itemList;
    itemList.push_back(text("FACILITIES (" + std::to_string(items.size()) + ")") | bold | color(ftxui::Color::Yellow));
    itemList.push_back(separator());

    int totalListItems = items.size() + 1;
    if (selectedItemIdx >= totalListItems) selectedItemIdx = std::max(0, totalListItems - 1);

    int startIdx = std::max(0, selectedItemIdx - 6);
    int endIdx = std::min(totalListItems, startIdx + 14);

    for (int i = startIdx; i < endIdx; i++) {
        bool isSelected = (i == selectedItemIdx);
        string prefix = isSelected ? "> " : "  ";
        Element e;

        if (i < (int)items.size()) {
            string icon = "?";
            if (items[i].type == "STOP") icon = "?";
            else if (items[i].type == "SCHOOL") icon = "?";
            else if (items[i].type == "HOSPITAL") icon = "?";
            else if (items[i].type == "PHARMACY") icon = "?";
            else if (items[i].type == "MALL") icon = "?";

            string displayName = items[i].name.length() > 20 ? items[i].name.substr(0, 17) + "..." : items[i].name;
            e = text(prefix + icon + " " + displayName);
        }
        else {
            e = text(prefix + "[+] Add Facility") | color(ftxui::Color::Yellow);
        }

        if (isSelected) {
            e = e | bold | (focusPanel == 2 ? bgcolor(ftxui::Color::Blue) : color(ftxui::Color::Green));
        }
        itemList.push_back(e);
    }
    return vbox(itemList) | border | size(WIDTH, EQUAL, 35);
}

inline Element CityDatabaseView::renderDetailPanel() {
    Elements details;
    details.push_back(text("DETAILS") | bold | color(ftxui::Color::Magenta));
    details.push_back(separator());

    if (selectedItemIdx < (int)items.size()) {
        DBItem& item = items[selectedItemIdx];
        details.push_back(hbox({ text("ID:   ") | bold, text(item.id) | color(ftxui::Color::Yellow) }));
        details.push_back(hbox({ text("Name: ") | bold, text(item.name) }));
        details.push_back(hbox({ text("Type: ") | bold, text(item.type) | color(ftxui::Color::Cyan) }));
        details.push_back(separator());

        if (item.type == "SCHOOL") {
            School* s = (School*)item.objPtr;
            details.push_back(hbox({ text("Rating: "), text(std::to_string(s->rating).substr(0, 3) + "/5.0") | color(ftxui::Color::Yellow) }));
            details.push_back(hbox({ text("Students: "), text(std::to_string(s->getTotalEnrolledStudents())) | color(ftxui::Color::Green) }));
            details.push_back(hbox({ text("Faculty: "), text(std::to_string(s->getTotalFaculty())) | color(ftxui::Color::Green) }));
            details.push_back(separator());
            details.push_back(text("Departments:") | dim);
            for (int k = 0; k < std::min(5, s->departments.getSize()); k++) {
                details.push_back(text(" - " + s->departments[k]->name));
            }
            if (s->departments.getSize() > 5) {
                details.push_back(text(" ... +" + std::to_string(s->departments.getSize() - 5) + " more") | dim);
            }
        }
        else if (item.type == "MALL") {
            Mall* m = (Mall*)item.objPtr;
            details.push_back(hbox({ text("Total Shops: "), text(std::to_string(m->getShopCount())) | color(ftxui::Color::Green) }));
            details.push_back(hbox({ text("Products: "), text(std::to_string(m->getTotalProductCount())) }));
            details.push_back(separator());
            details.push_back(text("Shops:") | dim);
            for (int k = 0; k < std::min(6, m->shops.getSize()); k++) {
                Shop* shop = m->shops[k];
                details.push_back(text(" - " + shop->name + " (" + shop->category + ")"));
            }
            if (m->shops.getSize() > 6) {
                details.push_back(text(" ... +" + std::to_string(m->shops.getSize() - 6) + " more") | dim);
            }
        }
        else if (item.type == "HOSPITAL") {
            Hospital* h = (Hospital*)item.objPtr;
            details.push_back(hbox({ text("Beds: "), text(std::to_string(h->getAvailableBeds()) + "/" + std::to_string(h->totalBeds)) | color(ftxui::Color::Red) }));
            details.push_back(hbox({ text("Patients: "), text(std::to_string(h->getOccupiedBeds())) }));
            details.push_back(separator());
            details.push_back(text("Specializations:") | dim);
            for (int k = 0; k < std::min(5, h->specializations.getSize()); k++) {
                details.push_back(text(" - " + h->specializations[k]));
            }
        }
        else if (item.type == "PHARMACY") {
            Pharmacy* p = (Pharmacy*)item.objPtr;
            details.push_back(hbox({ text("Meds Count: "), text(std::to_string(p->getMedicineCount())) }));
            if (p->getMedicineCount() > 0) {
                details.push_back(separator());
                details.push_back(text("Sample Inventory:") | dim);
                for (int k = 0; k < std::min(4, p->getMedicineCount()); k++) {
                    const Medicine* m = p->getMedicine(k);
                    details.push_back(text(" - " + m->name + " (" + std::to_string((int)m->price) + " Rs)"));
                }
            }
        }
        else if (item.type == "STOP") {
            CityNode* n = (CityNode*)item.objPtr;
            if (islamabad->getTransportManager()) {
                int w = islamabad->getTransportManager()->getWaitingCount(n->id);
                details.push_back(hbox({ text("Waiting: "), text(std::to_string(w) + " passengers") | color(ftxui::Color::Yellow) }));
            }
        }

        details.push_back(separator());
        details.push_back(text("SECTOR RESIDENTS") | bold);
        Vector<Citizen*> residents = islamabad->getPopulationManager()->getCitizensInSector(sectorList[selectedSectorIdx]);
        details.push_back(text("Total Residents: " + std::to_string(residents.getSize())));
    }
    else {
        details.push_back(text("Create New Facility") | center);
        details.push_back(text("in " + sectorList[selectedSectorIdx]) | bold | center | color(ftxui::Color::Green));
        details.push_back(text(""));
        details.push_back(text("Press Enter to open") | dim | center);
        details.push_back(text("creation wizard.") | dim | center);
    }

    return vbox(details) | border | flex;
}

#endif // CITY_DATABASE_VIEW_H
