#pragma once
#ifndef CITY_SEARCH_ENGINE_VIEW_H
#define CITY_SEARCH_ENGINE_VIEW_H

#include <string>
#include <vector>
#include <algorithm>

#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/component/event.hpp"

#include "../../SmartCity.h"
#include "CityEditorViews.h"

using namespace ftxui;
using std::string;

// ============================================================================
// SEARCH ENGINE VIEW COMPONENT
// ============================================================================

class CitySearchEngineView {
private:
    SmartCity* islamabad;
    CityEditorViews* editorViews;

    static string toLower(const string& s) {
        string lower = s;
        for (char& c : lower) {
            if (c >= 'A' && c <= 'Z') c += ('a' - 'A');
        }
        return lower;
    }

public:
    CitySearchEngineView(SmartCity* city, CityEditorViews* editors);
    bool run(); // Returns true if should go back to database view
};

inline CitySearchEngineView::CitySearchEngineView(SmartCity* city, CityEditorViews* editors)
    : islamabad(city), editorViews(editors) {
}

inline bool CitySearchEngineView::run() {
    auto screen = ScreenInteractive::Fullscreen();
    string query = "";
    int selected = 0;
    std::vector<string> menu_entries;
    string message = "";
    bool goBack = true;

    auto performSearch = [&]() {
        menu_entries.clear();
        selected = 0;
        if (query.length() < 2) return;
        string q = toLower(query);

        // 1. Search Facilities (Graph Nodes)
        if (islamabad && islamabad->getCityGraph()) {
            CityGraph* g = islamabad->getCityGraph();
            for (int i = 0; i < g->getNodeCount(); i++) {
                CityNode* n = g->getNode(i);
                if (!n || n->type == "CORNER") continue;
                if (toLower(n->name).find(q) != string::npos) {
                    menu_entries.push_back("[ID: " + n->databaseID + "] " + n->name + " (" + n->type + ") in " + n->sector);
                }
            }
        }

        // 2. Search Commercial (Malls, Shops, Products)
        if (islamabad && islamabad->getCommercialManager()) {
            CommercialManager* cm = islamabad->getCommercialManager();
            for (int i = 0; i < cm->malls.getSize(); i++) {
                Mall* m = cm->malls[i];
                if (toLower(m->name).find(q) != string::npos) {
                    menu_entries.push_back("[ID: " + m->id + "] " + m->name + " (" + m->getSector() + ")");
                }
                for (int j = 0; j < m->shops.getSize(); j++) {
                    Shop* s = m->shops[j];
                    if (toLower(s->name).find(q) != string::npos) {
                        menu_entries.push_back("[ID: " + s->id + "] " + s->name + " @ " + m->name);
                    }
                }
            }
        }

        // 3. Search Medical (Medicines, Pharmacies)
        if (islamabad && islamabad->getMedicalManager()) {
            MedicalManager* mm = islamabad->getMedicalManager();
            for (int i = 0; i < mm->pharmacies.getSize(); i++) {
                Pharmacy* p = mm->pharmacies[i];
                if (toLower(p->name).find(q) != string::npos) {
                    menu_entries.push_back("[ID: " + p->id + "] " + p->name + " @ " + p->sector);
                }
            }
        }

        // 4. Search Schools
        if (islamabad && islamabad->getSchoolManager()) {
            auto& schools = islamabad->getSchoolManager()->schools;
            for (int i = 0; i < schools.getSize(); i++) {
                if (toLower(schools[i]->name).find(q) != string::npos) {
                    menu_entries.push_back("[ID: " + schools[i]->id + "] " + schools[i]->name + " (SCHOOL) in " + schools[i]->getSector());
                }
            }
        }

        // 5. Search Hospitals
        if (islamabad && islamabad->getMedicalManager()) {
            auto& hospitals = islamabad->getMedicalManager()->hospitals;
            for (int i = 0; i < hospitals.getSize(); i++) {
                if (toLower(hospitals[i]->name).find(q) != string::npos) {
                    menu_entries.push_back("[ID: " + hospitals[i]->id + "] " + hospitals[i]->name + " (HOSPITAL) in " + hospitals[i]->sector);
                }
            }
        }

        if (menu_entries.size() > 50) menu_entries.resize(50);
        };

    InputOption input_opt;
    input_opt.on_change = performSearch;
    auto input_component = Input(&query, input_opt);

    MenuOption menu_opt;
    menu_opt.on_enter = [&] {
        if (selected >= 0 && selected < (int)menu_entries.size()) {
            string selection = menu_entries[selected];
            size_t start = selection.find("[ID: ");
            if (start != string::npos) {
                start += 5;
                size_t end = selection.find("]", start);
                if (end != string::npos) {
                    string extractedID = selection.substr(start, end - start);

                    // Determine Type (simple heuristic)
                    string type = "UNKNOWN";
                    if (selection.find("(SCHOOL)") != string::npos) type = "SCHOOL";
                    else if (selection.find("(HOSPITAL)") != string::npos) type = "HOSPITAL";
                    else if (selection.find("(PHARMACY)") != string::npos) type = "PHARMACY";
                    else if (selection.find("(MALL)") != string::npos) type = "MALL";
                    else if (selection.find("(STOP)") != string::npos) type = "STOP";
                    else if (selection.find("@") != string::npos) {
                        if (extractedID.find("S") != string::npos && extractedID.length() < 6) type = "SHOP";
                        else if (extractedID.find("P") != string::npos && extractedID.length() < 6) type = "PHARMACY";
                    }

                    // Attempt to launch editor
                    if (type != "UNKNOWN" && editorViews) {
                        editorViews->runEditObjectView(extractedID, type);
                        return;
                    }

                    message = "Selected Item ID: " + extractedID + " (Press Esc to return)";
                }
                else {
                    message = "ID format not recognized.";
                }
            }
            else {
                message = "No ID associated with this entry.";
            }
        }
        };
    auto menu_component = Menu(&menu_entries, &selected, menu_opt);
    auto container = Container::Vertical({ input_component, menu_component | vscroll_indicator | frame | flex });

    auto renderer = Renderer(container, [&] {
        return vbox({
            text(" SEARCH ENGINE ") | bold | center | bgcolor(ftxui::Color::Blue) | color(ftxui::Color::White),
            separator(),
            hbox({ text(" FIND: "), input_component->Render() | flex }),
            separator(),
            menu_entries.empty() ? text("Type to search (min 2 chars)...") | dim | center : menu_component->Render() | flex,
            separator(),
            (message.empty() ? text("Select an item and press Enter to Edit") | dim | center
                             : text(message) | bold | color(ftxui::Color::Green) | center),
            text("Esc: Back to Database") | dim | center
            }) | border | size(WIDTH, EQUAL, 80) | size(HEIGHT, EQUAL, 40) | center;
        });

    auto component = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Escape) {
            goBack = true;
            screen.Exit();
            return true;
        }
        return false;
        });
    screen.Loop(component);

    return goBack;
}

#endif // CITY_SEARCH_ENGINE_VIEW_H
