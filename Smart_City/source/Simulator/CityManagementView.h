#pragma once
#ifndef CITY_MANAGEMENT_VIEW_H
#define CITY_MANAGEMENT_VIEW_H

#include <string>
#include <vector>
#include <algorithm>

#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/component/event.hpp"

#include "../../SmartCity.h"
#include "CityManagement.h"
#include "CityEditorViews.h"

using namespace ftxui;
using std::string;

// ============================================================================
// MANAGEMENT VIEW COMPONENT
// ============================================================================

class CityManagementView {
private:
    SmartCity* islamabad;
    CityManagement* cityMgmt;
    CityEditorViews* editorViews;

    struct ManageableItem {
        string id;
        string name;
        string type;
        string extraInfo;
    };

public:
    CityManagementView(SmartCity* city, CityManagement* mgmt, CityEditorViews* editors);
    bool run(); // Returns true if should go back to main menu

private:
    void refreshList(std::vector<ManageableItem>& items, const std::vector<string>& categories, int categoryIdx);
    void showPlaceholder(const string& title, const string& msg);
};

inline CityManagementView::CityManagementView(SmartCity* city, CityManagement* mgmt, CityEditorViews* editors)
    : islamabad(city), cityMgmt(mgmt), editorViews(editors) {
}

inline void CityManagementView::showPlaceholder(const string& title, const string& msg) {
    auto pScreen = ScreenInteractive::Fullscreen();
    auto pRenderer = Renderer([&] {
        return vbox({
            text(title) | bold | center | bgcolor(ftxui::Color::Red) | color(ftxui::Color::White),
            separator(),
            text(msg) | center,
            text(""),
            text("Press Enter to return") | dim | center
            }) | border | center;
        });
    auto pComp = CatchEvent(pRenderer, [&](Event e) {
        if (e == Event::Return || e == Event::Escape) {
            pScreen.Exit();
            return true;
        }
        return false;
        });
    pScreen.Loop(pComp);
}

inline void CityManagementView::refreshList(std::vector<ManageableItem>& items,
    const std::vector<string>& categories, int categoryIdx) {
    items.clear();
    string cat = categories[categoryIdx];

    // 1. Nodes (Stops/Corners)
    if (cat == "All" || cat == "Nodes") {
        CityGraph* g = islamabad->getCityGraph();
        for (int i = 0; i < g->getNodeCount(); i++) {
            CityNode* n = g->getNode(i);
            if (n) items.push_back({ n->databaseID, n->name, n->type, n->sector });
        }
    }

    // 2. Commercial
    if (islamabad->getCommercialManager()) {
        CommercialManager* cm = islamabad->getCommercialManager();
        if (cat == "All" || cat == "Malls") {
            for (int i = 0; i < cm->malls.getSize(); i++)
                items.push_back({ cm->malls[i]->id, cm->malls[i]->name, "MALL", cm->malls[i]->getSector() });
        }
        if (cat == "All" || cat == "Shops") {
            for (int i = 0; i < cm->malls.getSize(); i++) {
                Mall* m = cm->malls[i];
                for (int j = 0; j < m->shops.getSize(); j++)
                    items.push_back({ m->shops[j]->id, m->shops[j]->name, "SHOP", m->name });
            }
        }
    }

    // 3. Schools
    if ((cat == "All" || cat == "Schools") && islamabad->getSchoolManager()) {
        SchoolManager* sm = islamabad->getSchoolManager();
        for (int i = 0; i < sm->schools.getSize(); i++)
            items.push_back({ sm->schools[i]->id, sm->schools[i]->name, "SCHOOL", sm->schools[i]->getSector() });
    }

    // 4. Medical
    if (islamabad->getMedicalManager()) {
        MedicalManager* mm = islamabad->getMedicalManager();
        if (cat == "All" || cat == "Hospitals") {
            for (int i = 0; i < mm->hospitals.getSize(); i++)
                items.push_back({ mm->hospitals[i]->id, mm->hospitals[i]->name, "HOSPITAL", mm->hospitals[i]->sector });
        }
        if (cat == "All" || cat == "Pharmacies") {
            for (int i = 0; i < mm->pharmacies.getSize(); i++)
                items.push_back({ mm->pharmacies[i]->id, mm->pharmacies[i]->name, "PHARMACY", mm->pharmacies[i]->sector });
        }
    }
}

inline bool CityManagementView::run() {
    auto screen = ScreenInteractive::Fullscreen();
    bool goBack = true;

    int selectedCategoryIdx = 0;
    int selectedItemIdx = 0;
    int focusPanel = 0; // 0=Tabs, 1=List

    std::vector<string> categories = { "All", "Nodes", "Malls", "Shops", "Schools", "Hospitals", "Pharmacies" };
    std::vector<ManageableItem> currentItems;

    // Initial load
    refreshList(currentItems, categories, selectedCategoryIdx);

    auto renderer = Renderer([&] {
        // Ensure index bounds
        if (selectedItemIdx >= (int)currentItems.size())
            selectedItemIdx = std::max(0, (int)currentItems.size() - 1);

        // ===== TOP TABS =====
        Elements tabs;
        for (int i = 0; i < (int)categories.size(); i++) {
            auto tab = text(" " + categories[i] + " ");
            if (i == selectedCategoryIdx)
                tab = tab | bold | bgcolor(ftxui::Color::Cyan) | color(ftxui::Color::Black);
            else
                tab = tab | color(ftxui::Color::GrayLight);
            tabs.push_back(tab);
        }

        // ===== LEFT PANEL (Object List) =====
        Elements listElements;
        int startIdx = std::max(0, selectedItemIdx - 10);
        int endIdx = std::min((int)currentItems.size(), startIdx + 22);

        for (int i = startIdx; i < endIdx; i++) {
            const auto& item = currentItems[i];
            string label = "[" + item.id + "] - " + item.name.substr(0, 20) + " - [" + item.type + "]";
            auto row = text(label);
            if (i == selectedItemIdx) {
                row = row | bold;
                if (focusPanel == 1) row = row | bgcolor(ftxui::Color::Blue) | color(ftxui::Color::White);
                else row = row | color(ftxui::Color::Green);
            }
            listElements.push_back(row);
        }
        auto leftPanel = vbox(listElements) | border | flex;
        if (focusPanel == 1) leftPanel = leftPanel | color(ftxui::Color::Cyan);

        // ===== MIDDLE PANEL (Actions) =====
        auto btnStyle = [&](string label, bool selected) {
            return text(label) | center | (selected ? (bgcolor(ftxui::Color::Red) | bold) : dim) | border;
            };

        auto midPanel = vbox({
            text("ACTIONS") | bold | center,
            separator(),
            btnStyle("[ EDIT ] (E)", false),
            text(" "),
            btnStyle("[ ADD ] (+)", false),
            text(" "),
            btnStyle("[ DELETE ] (Del)", false)
            }) | border | size(WIDTH, EQUAL, 20);

        // ===== RIGHT PANEL (Details) =====
        Elements details;
        if (!currentItems.empty() && selectedItemIdx < (int)currentItems.size()) {
            const auto& sel = currentItems[selectedItemIdx];
            details.push_back(text("OBJECT DETAILS") | bold | center | color(ftxui::Color::Yellow));
            details.push_back(separator());
            details.push_back(hbox({ text("ID: ") | bold, text(sel.id) | color(ftxui::Color::Cyan) }));
            details.push_back(hbox({ text("Name: ") | bold, text(sel.name) | color(ftxui::Color::White) }));
            details.push_back(hbox({ text("Type: ") | bold, text(sel.type) | color(ftxui::Color::Magenta) }));
            details.push_back(hbox({ text("Loc/Info: ") | bold, text(sel.extraInfo) | color(ftxui::Color::Green) }));
            details.push_back(separator());
            details.push_back(text("Press 'E' to Edit full details") | dim | center);
        }
        else {
            details.push_back(text("No item selected") | dim | center);
        }
        auto rightPanel = vbox(details) | border | flex;

        return vbox({
            text(" MANAGEMENT CONSOLE (ADMIN) ") | bold | center | bgcolor(ftxui::Color::Red) | color(ftxui::Color::White),
            hbox(tabs) | center,
            separator(),
            hbox({ leftPanel, midPanel, rightPanel }) | flex,
            separator(),
            text("Tab: Switch Panel | Arrows: Navigate | E: Edit | +: Add | Del: Delete | Esc: Back") | dim | center
            });
        });

    auto component = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Tab) { focusPanel = (focusPanel + 1) % 2; return true; }
        if (e == Event::Escape) { goBack = true; screen.Exit(); return true; }

        // ADD
        if (e == Event::Character('+') || e == Event::Character('=')) {
            if (editorViews) {
                editorViews->runManagementAddForm(categories[selectedCategoryIdx]);
                refreshList(currentItems, categories, selectedCategoryIdx);
            }
            return true;
        }

        // DELETE
        if (e == Event::Delete || e == Event::Special({ 127 }) || e == Event::Character('x')) {
            if (!currentItems.empty() && selectedItemIdx < (int)currentItems.size()) {
                auto& item = currentItems[selectedItemIdx];
                bool deleted = false;
                if (item.type == "SCHOOL") deleted = cityMgmt->removeSchool(item.id);
                else if (item.type == "HOSPITAL") deleted = cityMgmt->removeHospital(item.id);
                else if (item.type == "PHARMACY") deleted = cityMgmt->removePharmacy(item.id);
                else if (item.type == "MALL") { /* Not implemented */ }

                if (deleted) {
                    refreshList(currentItems, categories, selectedCategoryIdx);
                }
                else {
                    showPlaceholder("DELETE FAILED", "Could not delete object (Type not supported or dependency)");
                }
            }
            return true;
        }

        // EDIT
        if (e == Event::Character('e') || e == Event::Character('E')) {
            if (!currentItems.empty() && selectedItemIdx < (int)currentItems.size() && editorViews) {
                editorViews->runEditObjectView(currentItems[selectedItemIdx].id, currentItems[selectedItemIdx].type);
                refreshList(currentItems, categories, selectedCategoryIdx);
            }
            return true;
        }

        if (focusPanel == 0) { // Tabs
            if (e == Event::ArrowLeft) {
                selectedCategoryIdx = (selectedCategoryIdx - 1 + categories.size()) % categories.size();
                selectedItemIdx = 0;
                refreshList(currentItems, categories, selectedCategoryIdx);
                return true;
            }
            if (e == Event::ArrowRight) {
                selectedCategoryIdx = (selectedCategoryIdx + 1) % categories.size();
                selectedItemIdx = 0;
                refreshList(currentItems, categories, selectedCategoryIdx);
                return true;
            }
            if (e == Event::ArrowDown) { focusPanel = 1; return true; }
        }
        else if (focusPanel == 1) { // List
            if (e == Event::ArrowUp) {
                if (selectedItemIdx > 0) selectedItemIdx--;
                else focusPanel = 0;
                return true;
            }
            if (e == Event::ArrowDown) {
                if (selectedItemIdx < (int)currentItems.size() - 1) selectedItemIdx++;
                return true;
            }
            if (e == Event::Return) {
                if (!currentItems.empty() && editorViews) {
                    editorViews->runEditObjectView(currentItems[selectedItemIdx].id, currentItems[selectedItemIdx].type);
                }
                return true;
            }
        }

        return false;
        });

    screen.Loop(component);
    return goBack;
}

#endif // CITY_MANAGEMENT_VIEW_H
