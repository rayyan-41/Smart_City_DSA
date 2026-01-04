#ifndef CITY_SIMULATOR_ENHANCED_H
#define CITY_SIMULATOR_ENHANCED_H

#include <string>
#include <thread>
#include <chrono>
#include <iostream>
#include <functional>
#include <fstream>
#include <atomic>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <iomanip>

#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/component/event.hpp"

#include "../../SmartCity.h"
#include "../../data_structures/Vector.h"
#include "CityManagement.h"
#include "CityGraphView.h"
#include "CityDatabaseView.h"
#include "CityManagementView.h"
#include "CityEditorViews.h"
#include "CitySearchEngineView.h"

using namespace ftxui;
using std::string;

// ============================================================================
// ENUMS & STATE
// ============================================================================

enum class SimulatorState {
    INTRO_PHASE_1, INTRO_PHASE_2, INTRO_PHASE_3,
    WELCOME_ANIMATION, MAIN_MENU, CSV_SELECTION, LOADING,
    GRAPH_VIEW, DATABASE_VIEW, MANAGEMENT_MENU,
    SEARCH_VIEW,
    EXIT
};

enum class CSVLoadMode { DEMO_MODE, FULL_MODE };

// ============================================================================
// ASCII ART
// ============================================================================

namespace ASCIIArt {
    const string ISLAMABAD_TITLE[] = {
        R"( ██╗███████╗██╗      █████╗ ███╗   ███╗ █████╗ ██████╗  █████╗ ██████╗ )",
        R"( ██║██╔════╝██║     ██╔══██╗████╗ ████║██╔══██╗██╔══██╗██╔══██║██╔══██╗)",
        R"( ██║███████╗██║     ███████║██╔████╔██║███████║██████╔╝███████║██║  ██║)",
        R"( ██║╚════██║██║     ██╔══██║██║╚██╔╝██║██╔══██║██╔══██╗██╔══██║██║  ██║)",
        R"( ██║███████║███████╗██║  ██║██║ ╚═╝ ██║██║  ██║██████╔╝██║  ██║██████╔╝)",
        R"( ╚═╝╚══════╝╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝╚═════╝ ╚═╝  ╚═╝╚═════╝ )",
    };
    const int ISLAMABAD_TITLE_HEIGHT = 6;

    const string REDEFINED_TITLE[] = {
        R"( ██████╗ ███████╗██████╗ ███████╗███████╗██╗███╗   ██╗███████╗██████╗ )",
        R"( ██╔══██╗██╔════╝██╔══██╗██╔════╝██╔════╝██║████╗  ██║██╔════╝██╔══██╗)",
        R"( ██████╔╝█████╗  ██║  ██║█████╗  █████╗  ██║██╔██╗ ██║█████╗  ██║  ██║)",
        R"( ██╔══██╗██╔══╝  ██║  ██║██╔══╝  ██╔══╝  ██║██║╚██╗██║██╔══╝  ██║  ██║)",
        R"( ██║  ██║███████╗██████╔╝███████╗██║     ██║██║ ╚████║███████╗██████╔╝)",
        R"( ╚═╝  ╚═╝╚══════╝╚═════╝ ╚══════╝╚═╝     ╚═╝╚═╝  ╚═══╝╚══════╝╚═════╝ )",
    };
    const int REDEFINED_TITLE_HEIGHT = 6;
}

// ============================================================================
// CITY SIMULATOR CLASS
// ============================================================================

class CitySimulator {
private:
    SmartCity* islamabad;
    CityManagement* cityMgmt;
    CityEditorViews* editorViews;
    SimulatorState currentState;
    CSVLoadMode loadMode;
    bool cityInitialized;

    string stopsCSV, schoolsCSV, hospitalsCSV, pharmaciesCSV;
    string busesCSV, populationCSV, mallsCSV, shopsCSV, ambulancesCSV;

public:
    CitySimulator();
    ~CitySimulator();

    void run();
    void runDebugMode();

private:
    // Intro & Menu Phases
    void runIntroPhase1();
    void runIntroPhase2();
    void runIntroPhase3();
    void runWelcomeAnimation();
    void runMainMenu();
    void runCSVSelection();
    void runLoadingScreen();

    // View Dispatchers
    void runGraphView();
    void runDatabaseView();
    void runSearchView();
    void runManagementMenu();

    // Utility
    void sleepMs(int ms);
    bool fileExists(const string& path);
};

// ============================================================================
// IMPLEMENTATION
// ============================================================================

inline CitySimulator::CitySimulator()
    : islamabad(nullptr), cityMgmt(nullptr), editorViews(nullptr),
    currentState(SimulatorState::INTRO_PHASE_1),
    loadMode(CSVLoadMode::DEMO_MODE),
    cityInitialized(false) {

    stopsCSV = "dataset/stops.csv";
    schoolsCSV = "dataset/schools.csv";
    hospitalsCSV = "dataset/hospitals.csv";
    pharmaciesCSV = "dataset/pharmacies.csv";
    busesCSV = "dataset/buses.csv";
    populationCSV = "dataset/population.csv";
    mallsCSV = "dataset/malls.csv";
    shopsCSV = "dataset/shops.csv";
    ambulancesCSV = "dataset/ambulances.csv";
}

inline CitySimulator::~CitySimulator() {
    delete editorViews;
    delete cityMgmt;
    delete islamabad;
}

inline void CitySimulator::sleepMs(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline bool CitySimulator::fileExists(const string& path) {
    std::ifstream f(path);
    return f.good();
}

// ============================================================================
// MAIN LOOP
// ============================================================================

inline void CitySimulator::run() {
    while (currentState != SimulatorState::EXIT) {
        switch (currentState) {
        case SimulatorState::INTRO_PHASE_1:     runIntroPhase1(); break;
        case SimulatorState::INTRO_PHASE_2:     runIntroPhase2(); break;
        case SimulatorState::INTRO_PHASE_3:     runIntroPhase3(); break;
        case SimulatorState::WELCOME_ANIMATION: runWelcomeAnimation(); break;
        case SimulatorState::MAIN_MENU:         runMainMenu(); break;
        case SimulatorState::CSV_SELECTION:     runCSVSelection(); break;
        case SimulatorState::LOADING:           runLoadingScreen(); break;
        case SimulatorState::GRAPH_VIEW:        runGraphView(); break;
        case SimulatorState::DATABASE_VIEW:     runDatabaseView(); break;
        case SimulatorState::MANAGEMENT_MENU:   runManagementMenu(); break;
        case SimulatorState::SEARCH_VIEW:       runSearchView(); break;
        case SimulatorState::EXIT: break;
        }
    }
    std::cout << "\nThank you for using Islamabad Redefined!\n" << std::endl;
}

inline void CitySimulator::runDebugMode() {
    std::cout << "=== DEBUG MODE ===" << std::endl;
    islamabad = new SmartCity();
    islamabad->setDatasetPaths(stopsCSV, schoolsCSV, hospitalsCSV, pharmaciesCSV,
        busesCSV, populationCSV, mallsCSV, shopsCSV, ambulancesCSV);
    islamabad->initialize();
    cityInitialized = true;
    cityMgmt = new CityManagement(islamabad);
    editorViews = new CityEditorViews(islamabad, cityMgmt);
    runGraphView();
}

// ============================================================================
// INTRO PHASES
// ============================================================================

inline void CitySimulator::runIntroPhase1() {
    auto screen = ScreenInteractive::Fullscreen();
    const string fullText = "A product of Rayyan's Emporium";
    std::atomic<int> charIndex{ 0 };
    std::atomic<bool> done{ false };

    auto renderer = Renderer([&] {
        string txt = fullText.substr(0, std::min(charIndex.load(), (int)fullText.length()));
        if (charIndex.load() < (int)fullText.length()) txt += "_";
        return vbox({ filler(), text(txt) | bold | center, filler() });
        });

    std::thread t([&]() {
        for (int i = 0; i <= (int)fullText.length(); i++) {
            charIndex.store(i); screen.PostEvent(Event::Custom); sleepMs(50);
        }
        sleepMs(800); done.store(true); screen.PostEvent(Event::Custom);
        });

    auto comp = CatchEvent(renderer, [&](Event e) {
        if (done.load()) { currentState = SimulatorState::INTRO_PHASE_2; screen.Exit(); return true; }
        return false;
        });
    screen.Loop(comp);
    if (t.joinable()) t.join();
}

inline void CitySimulator::runIntroPhase2() {
    auto screen = ScreenInteractive::Fullscreen();
    const string fullText = "Created by Rayyan, Omar and Aryan";
    std::atomic<int> charIndex{ 0 };
    std::atomic<bool> done{ false };

    auto renderer = Renderer([&] {
        string txt = fullText.substr(0, std::min(charIndex.load(), (int)fullText.length()));
        if (charIndex.load() < (int)fullText.length()) txt += "_";
        return vbox({ filler(), text(txt) | bold | center, filler() });
        });

    std::thread t([&]() {
        for (int i = 0; i <= (int)fullText.length(); i++) {
            charIndex.store(i); screen.PostEvent(Event::Custom); sleepMs(45);
        }
        sleepMs(800); done.store(true); screen.PostEvent(Event::Custom);
        });

    auto comp = CatchEvent(renderer, [&](Event e) {
        if (done.load()) { currentState = SimulatorState::INTRO_PHASE_3; screen.Exit(); return true; }
        return false;
        });
    screen.Loop(comp);
    if (t.joinable()) t.join();
}

inline void CitySimulator::runIntroPhase3() {
    auto screen = ScreenInteractive::Fullscreen();
    std::atomic<bool> done{ false };

    auto renderer = Renderer([&] {
        Elements lines;
        for (int i = 0; i < ASCIIArt::ISLAMABAD_TITLE_HEIGHT; i++)
            lines.push_back(text(ASCIIArt::ISLAMABAD_TITLE[i]) | bold);
        lines.push_back(text(""));
        for (int i = 0; i < ASCIIArt::REDEFINED_TITLE_HEIGHT; i++)
            lines.push_back(text(ASCIIArt::REDEFINED_TITLE[i]) | color(ftxui::Color::GrayLight));
        return vbox({ filler(), vbox(lines) | center, filler(),
                     text(done.load() ? "Press Enter" : "") | center | dim, filler() });
        });

    std::thread t([&]() { sleepMs(1500); done.store(true); screen.PostEvent(Event::Custom); });

    auto comp = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Return && done.load()) { currentState = SimulatorState::MAIN_MENU; screen.Exit(); return true; }
        return false;
        });
    screen.Loop(comp);
    if (t.joinable()) t.join();
}

inline void CitySimulator::runWelcomeAnimation() {
    currentState = SimulatorState::MAIN_MENU;
}

// ============================================================================
// MAIN MENU
// ============================================================================

inline void CitySimulator::runMainMenu() {
    auto screen = ScreenInteractive::Fullscreen();
    std::vector<string> options;

    if (cityInitialized) {
        options = { "Graph View [1]", "Database [2]", "Search Engine [S]", "Management [3]", "Exit" };
    }
    else {
        options = { "Initialize City", "Exit" };
    }
    int sel = 0;

    auto renderer = Renderer([&] {
        Elements items;
        for (int i = 0; i < (int)options.size(); i++) {
            auto item = text((i == sel ? " > " : "   ") + options[i]);
            if (i == sel) item = item | bold | color(ftxui::Color::Green);
            items.push_back(item);
        }

        string statusText = cityInitialized ? "CITY LOADED" : "NOT INITIALIZED";
        ftxui::Color statusColor = cityInitialized ? ftxui::Color::Green : ftxui::Color::Yellow;

        Elements titleArt;
        for (int i = 0; i < 6; i++) {
            titleArt.push_back(text(ASCIIArt::ISLAMABAD_TITLE[i]) | color(ftxui::Color::Green));
        }

        auto menuBox = vbox({
            text("MAIN MENU") | bold | center | color(ftxui::Color::Cyan),
            separator(),
            text(statusText) | center | color(statusColor),
            separator(),
            vbox(items),
            }) | border | size(WIDTH, EQUAL, 35);

        return vbox({
            filler(),
            vbox(titleArt) | center,
            text("R E D E F I N E D") | bold | center | color(ftxui::Color::GrayLight),
            text(""),
            hbox({ filler(), menuBox, filler() }),
            filler()
            });
        });

    auto comp = CatchEvent(renderer, [&](Event e) {
        if (e == Event::ArrowUp) { sel = (sel - 1 + options.size()) % options.size(); return true; }
        if (e == Event::ArrowDown) { sel = (sel + 1) % options.size(); return true; }
        if (e == Event::Return) {
            if (!cityInitialized) {
                currentState = (sel == 0) ? SimulatorState::CSV_SELECTION : SimulatorState::EXIT;
            }
            else {
                if (sel == 0) currentState = SimulatorState::GRAPH_VIEW;
                else if (sel == 1) currentState = SimulatorState::DATABASE_VIEW;
                else if (sel == 2) currentState = SimulatorState::SEARCH_VIEW;
                else if (sel == 3) currentState = SimulatorState::MANAGEMENT_MENU;
                else currentState = SimulatorState::EXIT;
            }
            screen.Exit();
            return true;
        }
        if (cityInitialized) {
            if (e == Event::Character('1')) { currentState = SimulatorState::GRAPH_VIEW; screen.Exit(); return true; }
            if (e == Event::Character('2')) { currentState = SimulatorState::DATABASE_VIEW; screen.Exit(); return true; }
            if (e == Event::Character('3')) { currentState = SimulatorState::MANAGEMENT_MENU; screen.Exit(); return true; }
            if (e == Event::Character('s') || e == Event::Character('S')) {
                currentState = SimulatorState::SEARCH_VIEW; screen.Exit(); return true;
            }
        }
        if (e == Event::Escape) { currentState = SimulatorState::EXIT; screen.Exit(); return true; }
        return false;
        });
    screen.Loop(comp);
}

// ============================================================================
// CSV SELECTION
// ============================================================================

inline void CitySimulator::runCSVSelection() {
    auto screen = ScreenInteractive::Fullscreen();
    int sel = 0;

    auto renderer = Renderer([&] {
        Elements csvList;
        csvList.push_back(text("DATASET FILES") | bold | color(ftxui::Color::Cyan));
        csvList.push_back(separator());

        auto fileRow = [&](const string& label, const string& path) {
            bool exists = fileExists(path);
            ftxui::Color statusColor = exists ? ftxui::Color::Green : ftxui::Color::Red;
            string statusIcon = exists ? "[OK]" : "[MISSING]";
            return hbox({
                text(label + ": ") | bold | size(WIDTH, EQUAL, 14),
                text(path) | dim | size(WIDTH, EQUAL, 30),
                text(" " + statusIcon) | color(statusColor)
                });
            };

        csvList.push_back(fileRow("Stops", stopsCSV));
        csvList.push_back(fileRow("Schools", schoolsCSV));
        csvList.push_back(fileRow("Hospitals", hospitalsCSV));
        csvList.push_back(fileRow("Pharmacies", pharmaciesCSV));
        csvList.push_back(fileRow("Buses", busesCSV));
        csvList.push_back(fileRow("Population", populationCSV));
        csvList.push_back(fileRow("Malls", mallsCSV));
        csvList.push_back(fileRow("Shops", shopsCSV));
        csvList.push_back(fileRow("Ambulances", ambulancesCSV));
        csvList.push_back(separator());

        std::vector<string> options = { "Begin Initialization", "Back to Menu" };
        for (int i = 0; i < (int)options.size(); i++) {
            auto item = text((i == sel ? " > " : "   ") + options[i]);
            if (i == sel) item = item | bold | color(ftxui::Color::Green);
            csvList.push_back(item);
        }

        auto box = vbox(csvList) | border | size(WIDTH, EQUAL, 58);
        return vbox({ filler(), text("CITY INITIALIZATION") | bold | center | color(ftxui::Color::Green),
            text(""), hbox({ filler(), box, filler() }), text(""),
            text("Press Enter to select, Esc to go back") | center | dim, filler() });
        });

    auto comp = CatchEvent(renderer, [&](Event e) {
        if (e == Event::ArrowUp) { sel = (sel - 1 + 2) % 2; return true; }
        if (e == Event::ArrowDown) { sel = (sel + 1) % 2; return true; }
        if (e == Event::Return) {
            if (sel == 0) currentState = SimulatorState::LOADING;
            else currentState = SimulatorState::MAIN_MENU;
            screen.Exit(); return true;
        }
        if (e == Event::Escape) { currentState = SimulatorState::MAIN_MENU; screen.Exit(); return true; }
        return false;
        });
    screen.Loop(comp);
}

// ============================================================================
// LOADING SCREEN
// ============================================================================

inline void CitySimulator::runLoadingScreen() {
    auto screen = ScreenInteractive::Fullscreen();
    std::atomic<int> step{ 0 };
    std::atomic<bool> done{ false };
    std::atomic<int> totalSteps{ 14 };

    std::vector<string> loadingStages = {
        "Initializing city graph...",
        "Loading sector frames...",
        "Loading bus stops...",
        "Loading schools...",
        "Loading hospitals...",
        "Loading pharmacies...",
        "Loading buses...",
        "Loading ambulances...",
        "Loading school buses...",
        "Loading population data...",
        "Loading malls...",
        "Loading shops...",
        "Setting up transport queues...",
        "Finalizing initialization..."
    };

    auto renderer = Renderer([&] {
        int s = step.load();
        int barWidth = 50;
        int filledWidth = (s * barWidth) / totalSteps.load();
        string progressBar = "";
        for (int i = 0; i < barWidth; i++) {
            progressBar += (i < filledWidth) ? "█" : "░";
        }

        string stageDesc = (s < (int)loadingStages.size()) ? loadingStages[s] : "Completing...";

        return vbox({
            filler(),
            text("LOADING ISLAMABAD") | bold | center | color(ftxui::Color::Green),
            text(""),
            hbox({text("["), text(progressBar) | color(ftxui::Color::Green), text("]")}) | center,
            text(std::to_string((s * 100) / totalSteps.load()) + "%") | center,
            text(""),
            text(stageDesc) | center | color(ftxui::Color::Cyan),
            text(""),
            text(done.load() ? "Press Enter to continue" : "Please wait...") | center | dim,
            filler()
            });
        });

    std::thread t([&]() {
        islamabad = new SmartCity();
        islamabad->setDatasetPaths(stopsCSV, schoolsCSV, hospitalsCSV, pharmaciesCSV,
            busesCSV, populationCSV, mallsCSV, shopsCSV, ambulancesCSV);
        step.store(1); screen.PostEvent(Event::Custom); sleepMs(80);

        for (int i = 2; i <= 6; i++) {
            step.store(i); screen.PostEvent(Event::Custom); sleepMs(60);
        }

        islamabad->initialize();
        cityInitialized = true;

        for (int i = 7; i <= 12; i++) {
            step.store(i); screen.PostEvent(Event::Custom); sleepMs(50);
        }

        cityMgmt = new CityManagement(islamabad);
        editorViews = new CityEditorViews(islamabad, cityMgmt);

        step.store(13); screen.PostEvent(Event::Custom); sleepMs(50);
        step.store(14); screen.PostEvent(Event::Custom); sleepMs(100);

        done.store(true); screen.PostEvent(Event::Custom);
        });

    auto comp = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Return && done.load()) { currentState = SimulatorState::MAIN_MENU; screen.Exit(); return true; }
        return false;
        });
    screen.Loop(comp);
    if (t.joinable()) t.join();
}

// ============================================================================
// VIEW DISPATCHERS
// ============================================================================

inline void CitySimulator::runGraphView() {
    CityGraphView graphView(islamabad);
    graphView.run();
    currentState = SimulatorState::MAIN_MENU;
}

inline void CitySimulator::runDatabaseView() {
    auto screen = ScreenInteractive::Fullscreen();
    CityDatabaseView dbView(islamabad, this);

    int selectedSectorIdx = 0;
    int selectedCategoryIdx = 0;
    int selectedItemIdx = 0;
    int focusPanel = 0;

    std::vector<string> categories = { "All", "Stops", "Schools", "Hospitals", "Pharmacies", "Malls" };
    std::vector<string> sectorList;
    for (int i = 0; i < SECTOR_COUNT; i++) sectorList.push_back(SECTOR_GRID[i].name);

    struct DBItem {
        string id;
        string name;
        string type;
        string sector;
        void* objPtr;
    };
    std::vector<DBItem> items;

    auto refreshItems = [&]() {
        items.clear();
        string currentSector = sectorList[selectedSectorIdx];
        string cat = categories[selectedCategoryIdx];

        if ((cat == "All" || cat == "Stops") && islamabad->getCityGraph()) {
            CityGraph* g = islamabad->getCityGraph();
            for (int i = 0; i < g->getNodeCount(); i++) {
                CityNode* n = g->getNode(i);
                if (n && n->type == "STOP" && n->sector == currentSector) {
                    items.push_back({ n->databaseID, n->name, "STOP", n->sector, n });
                }
            }
        }
        if ((cat == "All" || cat == "Schools") && islamabad->getSchoolManager()) {
            auto& schools = islamabad->getSchoolManager()->schools;
            for (int i = 0; i < schools.getSize(); i++) {
                if (schools[i]->getSector() == currentSector) {
                    items.push_back({ schools[i]->id, schools[i]->name, "SCHOOL", schools[i]->getSector(), schools[i] });
                }
            }
        }
        if ((cat == "All" || cat == "Hospitals") && islamabad->getMedicalManager()) {
            auto& hospitals = islamabad->getMedicalManager()->hospitals;
            for (int i = 0; i < hospitals.getSize(); i++) {
                if (hospitals[i]->sector == currentSector) {
                    items.push_back({ hospitals[i]->id, hospitals[i]->name, "HOSPITAL", hospitals[i]->sector, hospitals[i] });
                }
            }
        }
        if ((cat == "All" || cat == "Pharmacies") && islamabad->getMedicalManager()) {
            auto& pharmacies = islamabad->getMedicalManager()->pharmacies;
            for (int i = 0; i < pharmacies.getSize(); i++) {
                if (pharmacies[i]->sector == currentSector) {
                    items.push_back({ pharmacies[i]->id, pharmacies[i]->name, "PHARMACY", pharmacies[i]->sector, pharmacies[i] });
                }
            }
        }
        if ((cat == "All" || cat == "Malls") && islamabad->getCommercialManager()) {
            auto& malls = islamabad->getCommercialManager()->malls;
            for (int i = 0; i < malls.getSize(); i++) {
                if (malls[i]->getSector() == currentSector) {
                    items.push_back({ malls[i]->id, malls[i]->name, "MALL", malls[i]->getSector(), malls[i] });
                }
            }
        }
        };

    refreshItems();

    auto renderer = Renderer([&] {
        // Sector Panel
        Elements sectorItems;
        sectorItems.push_back(text("SECTORS") | bold | color(ftxui::Color::Cyan));
        sectorItems.push_back(separator());
        for (int i = 0; i < (int)sectorList.size() && i < 18; i++) {
            int idx = std::max(0, selectedSectorIdx - 8) + i;
            if (idx >= (int)sectorList.size()) break;
            auto item = text((idx == selectedSectorIdx ? "> " : "  ") + sectorList[idx]);
            if (idx == selectedSectorIdx) item = item | bold | (focusPanel == 0 ? bgcolor(ftxui::Color::Blue) : color(ftxui::Color::Green));
            sectorItems.push_back(item);
        }
        auto sectorPanel = vbox(sectorItems) | border | size(WIDTH, EQUAL, 14);

        // Item Panel
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
                string icon = "●";
                if (items[i].type == "STOP") icon = "◎";
                else if (items[i].type == "SCHOOL") icon = "◆";
                else if (items[i].type == "HOSPITAL") icon = "✚";
                else if (items[i].type == "PHARMACY") icon = "⚕";
                else if (items[i].type == "MALL") icon = "◈";
                string displayName = items[i].name.length() > 20 ? items[i].name.substr(0, 17) + "..." : items[i].name;
                e = text(prefix + icon + " " + displayName);
            }
            else {
                e = text(prefix + "[+] Add Facility") | color(ftxui::Color::Yellow);
            }
            if (isSelected) e = e | bold | (focusPanel == 2 ? bgcolor(ftxui::Color::Blue) : color(ftxui::Color::Green));
            itemList.push_back(e);
        }
        auto itemPanel = vbox(itemList) | border | size(WIDTH, EQUAL, 35);

        // Detail Panel
        Elements details;
        details.push_back(text("DETAILS") | bold | color(ftxui::Color::Magenta));
        details.push_back(separator());
        if (selectedItemIdx < (int)items.size()) {
            DBItem& item = items[selectedItemIdx];
            details.push_back(hbox({ text("ID:   ") | bold, text(item.id) | color(ftxui::Color::Yellow) }));
            details.push_back(hbox({ text("Name: ") | bold, text(item.name) }));
            details.push_back(hbox({ text("Type: ") | bold, text(item.type) | color(ftxui::Color::Cyan) }));
            details.push_back(separator());
            details.push_back(text("SECTOR RESIDENTS") | bold);
            Vector<Citizen*> residents = islamabad->getPopulationManager()->getCitizensInSector(sectorList[selectedSectorIdx]);
            details.push_back(text("Total Residents: " + std::to_string(residents.getSize())));
        }
        else {
            details.push_back(text("Create New Facility") | center);
            details.push_back(text("in " + sectorList[selectedSectorIdx]) | bold | center | color(ftxui::Color::Green));
        }
        auto detailPanel = vbox(details) | border | flex;

        // Tabs
        Elements tabs;
        for (int i = 0; i < (int)categories.size(); i++) {
            auto tab = text(" " + categories[i] + " ");
            if (i == selectedCategoryIdx) tab = tab | bold | bgcolor(ftxui::Color::Green) | color(ftxui::Color::Black);
            else tab = tab | color(ftxui::Color::GrayLight);
            tabs.push_back(tab);
        }

        auto helpBar = hbox({
             text("Arrows: Navigate "),
             text("Tab: Switch Category "),
             text("Enter: Select/Edit "),
             text("Esc: Back")
            }) | center | dim;

        return vbox({
            text(" DATABASE VIEW ") | bold | center | bgcolor(ftxui::Color::Green) | color(ftxui::Color::Black),
            hbox(tabs) | center,
            separator(),
            hbox({sectorPanel, itemPanel, detailPanel}) | flex,
            separator(),
            helpBar
            });
        });

    auto comp = CatchEvent(renderer, [&](Event e) {
        if (e == Event::ArrowUp) {
            if (focusPanel == 0 && selectedSectorIdx > 0) { selectedSectorIdx--; refreshItems(); selectedItemIdx = 0; }
            else if (focusPanel == 2 && selectedItemIdx > 0) selectedItemIdx--;
            return true;
        }
        if (e == Event::ArrowDown) {
            if (focusPanel == 0 && selectedSectorIdx < (int)sectorList.size() - 1) { selectedSectorIdx++; refreshItems(); selectedItemIdx = 0; }
            else if (focusPanel == 2 && selectedItemIdx < (int)items.size()) selectedItemIdx++;
            return true;
        }
        if (e == Event::ArrowLeft) { focusPanel = 0; return true; }
        if (e == Event::ArrowRight) { focusPanel = 2; return true; }
        if (e == Event::Tab) {
            selectedCategoryIdx = (selectedCategoryIdx + 1) % categories.size();
            refreshItems();
            selectedItemIdx = 0;
            return true;
        }
        if (e == Event::Return) {
            if (focusPanel == 0) {
                focusPanel = 2;
            }
            else if (focusPanel == 2) {
                if (selectedItemIdx < (int)items.size()) {
                    editorViews->runEditObjectView(items[selectedItemIdx].id, items[selectedItemIdx].type);
                }
                else {
                    editorViews->runAddFacilityForm(sectorList[selectedSectorIdx]);
                    refreshItems();
                }
            }
            return true;
        }
        if (e == Event::Escape) { currentState = SimulatorState::MAIN_MENU; screen.Exit(); return true; }
        return false;
        });
    screen.Loop(comp);
}

inline void CitySimulator::runSearchView() {
    CitySearchEngineView searchView(islamabad, editorViews);
    searchView.run();
    currentState = SimulatorState::DATABASE_VIEW;
}

inline void CitySimulator::runManagementMenu() {
    CityManagementView mgmtView(islamabad, cityMgmt, editorViews);
    mgmtView.run();
    currentState = SimulatorState::MAIN_MENU;
}

#endif // CITY_SIMULATOR_ENHANCED_H