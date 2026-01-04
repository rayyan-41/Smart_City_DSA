#pragma once
#ifndef CITY_EDITOR_VIEWS_H
#define CITY_EDITOR_VIEWS_H

#include <string>
#include <vector>
#include <functional>
#include <sstream>

#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/component/event.hpp"

#include "../../SmartCity.h"
#include "../../data_structures/Vector.h"
#include "CityManagement.h"

using namespace ftxui;
using std::string;

// ============================================================================
// CITY EDITOR VIEWS - Forms for editing various city objects
// ============================================================================

class CityEditorViews {
private:
    SmartCity* islamabad;
    CityManagement* cityMgmt;

public:
    CityEditorViews(SmartCity* city, CityManagement* mgmt);

    // Generic Input Form Helper
    void runInputForm(const string& title, const std::vector<string>& labels,
        std::function<void(std::vector<string>)> onConfirm);

    // Population Selector
    Citizen* runPopulationSelector(const string& title);

    // Facility Forms
    void runAddFacilityForm(const string& sector);
    void runAddOfferingForm(CityNode* node);
    void runManagementAddForm(const string& category);

    // Entity Editors
    void runEditSchoolView(School* school);
    void runEditHospitalView(Hospital* hospital);
    void runEditPharmacyView(Pharmacy* pharmacy);
    void runEditMallView(Mall* mall);
    void runEditShopView(Shop* shop, Mall* mall);

    // Generic Object Editor Dispatcher
    void runEditObjectView(const string& objectID, const string& objectType);
};

// ============================================================================
// IMPLEMENTATION
// ============================================================================

inline CityEditorViews::CityEditorViews(SmartCity* city, CityManagement* mgmt)
    : islamabad(city), cityMgmt(mgmt) {
}

inline void CityEditorViews::runInputForm(const string& title, const std::vector<string>& labels,
    std::function<void(std::vector<string>)> onConfirm) {
    auto screen = ScreenInteractive::Fullscreen();
    std::vector<string> inputs(labels.size());

    Component container = Container::Vertical({});
    for (size_t i = 0; i < labels.size(); ++i) {
        container->Add(Input(&inputs[i], "Type " + labels[i] + "..."));
    }

    Component btn_confirm = Button("Confirm", [&] {
        onConfirm(inputs);
        screen.Exit();
        });

    Component btn_cancel = Button("Cancel", screen.ExitLoopClosure());

    container->Add(Container::Horizontal({ btn_confirm, btn_cancel }));

    auto renderer = Renderer(container, [&] {
        Elements fields;
        for (size_t i = 0; i < labels.size(); ++i) {
            fields.push_back(hbox({
                text(labels[i] + ": ") | bold | size(WIDTH, EQUAL, 20),
                container->ChildAt(i)->Render() | flex
                }) | border);
        }

        return vbox({
            text(title) | bold | center | bgcolor(ftxui::Color::Blue) | color(ftxui::Color::White),
            separator(),
            vbox(fields) | flex,
            separator(),
            hbox({ btn_confirm->Render(), text("  "), btn_cancel->Render() }) | center
            }) | border | size(WIDTH, EQUAL, 60) | center;
        });

    screen.Loop(renderer);
}

inline Citizen* CityEditorViews::runPopulationSelector(const string& title) {
    if (!islamabad || !islamabad->getPopulationManager()) return nullptr;

    auto screen = ScreenInteractive::Fullscreen();
    Citizen* selectedCitizen = nullptr;
    string query = "";
    int selected = 0;
    std::vector<string> entries;
    std::vector<Citizen*> displayedCitizens;

    auto refreshList = [&]() {
        entries.clear();
        displayedCitizens.clear();
        PopulationManager* pm = islamabad->getPopulationManager();

        int count = 0;
        for (int i = 0; i < pm->masterList.getSize(); ++i) {
            Citizen* c = pm->masterList[i];
            if (!c) continue;

            bool match = query.empty();
            if (!match) {
                string q = query;
                string n = c->name;
                string id = c->cnic;
                if (n.find(q) != string::npos || id.find(q) != string::npos) match = true;
            }

            if (match) {
                displayedCitizens.push_back(c);
                entries.push_back(c->name + " (" + c->cnic + ") - Age: " + std::to_string(c->age) + " - " + c->currentStatus);
                count++;
                if (count >= 50) break;
            }
        }
        if (entries.empty()) entries.push_back("No citizens found matching query.");
        };

    refreshList();

    InputOption input_opt;
    input_opt.on_change = refreshList;
    Component input = Input(&query, "Search Name or CNIC...", input_opt);

    MenuOption menu_opt;
    menu_opt.on_enter = [&] {
        if (selected >= 0 && selected < (int)displayedCitizens.size()) {
            selectedCitizen = displayedCitizens[selected];
            screen.Exit();
        }
        };
    Component menu = Menu(&entries, &selected, menu_opt);

    auto layout = Container::Vertical({
        input,
        menu | vscroll_indicator | frame | flex
        });

    auto renderer = Renderer(layout, [&] {
        return vbox({
            text(" POPULATION REGISTRY - " + title) | bold | center | bgcolor(ftxui::Color::Green) | color(ftxui::Color::Black),
            separator(),
            hbox({ text(" Filter: "), input->Render() }),
            separator(),
            menu->Render() | flex,
            separator(),
            text("Enter: Select | Esc: Cancel") | dim | center
            }) | border | size(WIDTH, EQUAL, 80) | size(HEIGHT, EQUAL, 40) | center;
        });

    auto component = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Escape) { screen.Exit(); return true; }
        return false;
        });

    screen.Loop(component);
    return selectedCitizen;
}

inline void CityEditorViews::runAddFacilityForm(const string& sector) {
    auto screen = ScreenInteractive::Fullscreen();
    string name_val;
    int type_selected = 0;
    std::vector<string> types = { "Pharmacy", "School", "Hospital", "Bus Stop" };
    string message = "";

    Component input_name = Input(&name_val, "Enter Name");
    Component toggle_type = Toggle(&types, &type_selected);

    Component btn_add = Button("Create Facility", [&] {
        if (name_val.empty()) { message = "Error: Name cannot be empty!"; return; }
        string newID = "";
        string type = types[type_selected];
        if (type == "Pharmacy") newID = cityMgmt->addPharmacy(name_val, sector);
        else if (type == "School") newID = cityMgmt->addSchool(name_val, sector, 3.0, {}, {});
        else if (type == "Hospital") newID = cityMgmt->addHospital(name_val, sector, 50, {});
        else if (type == "Bus Stop") {
            int id = cityMgmt->addBusStopInSector(name_val, sector);
            if (id != -1) newID = "STOP-" + std::to_string(id);
        }
        if (!newID.empty() || type == "Bus Stop") { screen.Exit(); }
        else { message = "Error: Creation failed."; }
        });

    Component btn_cancel = Button("Cancel", screen.ExitLoopClosure());
    auto component = Container::Vertical({ input_name, toggle_type, btn_add, btn_cancel });

    auto renderer = Renderer(component, [&] {
        return vbox({
            text("ADD NEW FACILITY") | bold | center | color(ftxui::Color::Green),
            separator(),
            text("Sector: " + sector) | center,
            hbox({text("Name: "), input_name->Render() | border}),
            hbox({text("Type: "), toggle_type->Render() | border}),
            hbox({btn_add->Render(), text("  "), btn_cancel->Render()}) | center,
            text(message) | color(ftxui::Color::Red) | center
            }) | border | size(WIDTH, EQUAL, 60) | center;
        });
    screen.Loop(renderer);
}

inline void CityEditorViews::runAddOfferingForm(CityNode* node) {
    if (!node) return;
    auto screen = ScreenInteractive::Fullscreen();
    string message = "";

    if (node->type == "PHARMACY") {
        string med_name, med_formula, med_price_str;
        Component input_name = Input(&med_name, "Medicine Name");
        Component input_formula = Input(&med_formula, "Formula");
        Component input_price = Input(&med_price_str, "Price");
        Component btn_add = Button("Add Medicine", [&] {
            try {
                float price = std::stof(med_price_str);
                if (cityMgmt->addMedicineToPharmacy(node->databaseID, med_name, med_formula, price)) screen.Exit();
                else message = "Error: Could not add medicine.";
            }
            catch (...) { message = "Error: Invalid Price."; }
            });
        auto component = Container::Vertical({ input_name, input_formula, input_price, btn_add, Button("Cancel", screen.ExitLoopClosure()) });
        auto renderer = Renderer(component, [&] {
            return vbox({
                text("ADD MEDICINE TO " + node->name) | bold | center | color(ftxui::Color::Magenta),
                separator(),
                hbox({text("Name: "), input_name->Render() | border}),
                hbox({text("Formula: "), input_formula->Render() | border}),
                hbox({text("Price: "), input_price->Render() | border}),
                btn_add->Render() | center,
                text(message) | color(ftxui::Color::Red) | center
                }) | border | size(WIDTH, EQUAL, 50) | center;
            });
        screen.Loop(renderer);
    }
    else {
        auto renderer = Renderer([&] {
            return vbox({ text("No offerings for " + node->type) | center, text("Press Enter to go back") | dim | center }) | border | center;
            });
        auto comp = CatchEvent(renderer, [&](Event e) { if (e == Event::Return) screen.Exit(); return true; });
        screen.Loop(comp);
    }
}

inline void CityEditorViews::runManagementAddForm(const string& category) {
    auto screen = ScreenInteractive::Fullscreen();
    string name_val, sector_val;
    int type_selected = 0;

    std::vector<string> types;
    if (category == "Schools") types = { "School" };
    else if (category == "Hospitals") types = { "Hospital" };
    else if (category == "Pharmacies") types = { "Pharmacy" };
    else if (category == "Nodes" || category == "All") types = { "Pharmacy", "School", "Hospital", "Bus Stop" };
    else types = { "Pharmacy", "School", "Hospital", "Bus Stop" };

    string message = "";

    Component input_name = Input(&name_val, "Enter Name");
    Component input_sector = Input(&sector_val, "Enter Sector (e.g. F-10)");
    Component toggle_type = Toggle(&types, &type_selected);

    Component btn_create = Button("Create", [&] {
        if (name_val.empty() || sector_val.empty()) {
            message = "Error: All fields required.";
            return;
        }

        string type = types[type_selected];
        string res = "";

        if (type == "School") res = cityMgmt->addSchool(name_val, sector_val, 3.0, {}, {});
        else if (type == "Hospital") res = cityMgmt->addHospital(name_val, sector_val, 50, {});
        else if (type == "Pharmacy") res = cityMgmt->addPharmacy(name_val, sector_val);
        else if (type == "Bus Stop") {
            int id = cityMgmt->addBusStopInSector(name_val, sector_val);
            if (id != -1) res = "STOP";
        }

        if (!res.empty()) screen.Exit();
        else message = "Creation Failed (Check Sector validity)";
        });

    Component btn_cancel = Button("Cancel", screen.ExitLoopClosure());

    auto container = Container::Vertical({
        input_name, input_sector, toggle_type,
        Container::Horizontal({ btn_create, btn_cancel })
        });

    auto renderer = Renderer(container, [&] {
        return vbox({
            text("ADD NEW OBJECT (" + category + ")") | bold | center | bgcolor(ftxui::Color::Blue) | color(ftxui::Color::White),
            separator(),
            hbox({ text("Name:   ") | size(WIDTH, EQUAL, 10), input_name->Render() }),
            hbox({ text("Sector: ") | size(WIDTH, EQUAL, 10), input_sector->Render() }),
            separator(),
            text("Type:") | bold,
            toggle_type->Render(),
            separator(),
            hbox({ btn_create->Render(), text("  "), btn_cancel->Render() }) | center,
            text(message) | color(ftxui::Color::Red) | center
            }) | border | size(WIDTH, EQUAL, 60) | center;
        });

    screen.Loop(renderer);
}

inline void CityEditorViews::runEditSchoolView(School* school) {
    if (!school) return;
    auto screen = ScreenInteractive::Fullscreen();
    string message = "";

    int tab_index = 0;
    std::vector<string> tabs = { "Info", "Departments", "Faculty", "Students" };

    Component btn_rename = Button("Rename School", [&] {
        runInputForm("Rename School", { "New Name" }, [&](std::vector<string> res) {
            if (!res[0].empty()) { school->name = res[0]; message = "Renamed to " + res[0]; }
            });
        });

    Component btn_add_dept = Button("Add Department", [&] {
        runInputForm("New Department", { "Department Name" }, [&](std::vector<string> res) {
            if (!res[0].empty()) {
                if (cityMgmt->addDepartmentToSchool(school->id, res[0])) message = "Added Dept: " + res[0];
                else message = "Error: Dept likely exists.";
            }
            });
        });

    Component btn_rem_dept = Button("Remove Department", [&] {
        runInputForm("Remove Department", { "Department Name" }, [&](std::vector<string> res) {
            if (cityMgmt->removeDepartmentFromSchool(school->id, res[0])) message = "Removed Dept: " + res[0];
            else message = "Error: Dept not found.";
            });
        });

    Component btn_hire_fac = Button("Hire Faculty", [&] {
        Citizen* c = runPopulationSelector("Select Citizen to Hire");
        if (c) {
            runInputForm("Employment Contract", { "Department", "Qualification", "Salary" }, [&](std::vector<string> res) {
                string dept = res[0];
                string qual = res[1];
                double sal = 0;
                try { sal = std::stod(res[2]); }
                catch (...) {}

                string resID = cityMgmt->hireCitizenAsFaculty(c->cnic, school->id, dept, qual, sal);
                if (!resID.empty()) message = "Hired " + c->name + " as " + resID;
                else message = "Hiring Failed (Dept invalid or already employed?)";
                });
        }
        });

    Component btn_enroll = Button("Enroll Student", [&] {
        Citizen* c = runPopulationSelector("Select Student to Enroll");
        if (c) {
            runInputForm("Enrollment Form", { "Department", "Class (1-10)" }, [&](std::vector<string> res) {
                string dept = res[0];
                int cls = 1;
                try { cls = std::stoi(res[1]); }
                catch (...) {}

                if (cityMgmt->enrollStudent(c->cnic, school->id, dept, cls)) message = "Enrolled " + c->name;
                else message = "Enrollment Failed (Dept/Class invalid or duplicate)";
                });
        }
        });

    Component btn_back = Button("Back to Manager", screen.ExitLoopClosure());

    auto c_info = Container::Vertical({ btn_rename });
    auto c_dept = Container::Vertical({ btn_add_dept, btn_rem_dept });
    auto c_fac = Container::Vertical({ btn_hire_fac });
    auto c_stu = Container::Vertical({ btn_enroll });
    auto c_tabs = Toggle(&tabs, &tab_index);

    auto main_container = Container::Vertical({
        c_tabs,
        Container::Tab({c_info, c_dept, c_fac, c_stu}, &tab_index),
        btn_back
        });

    auto renderer = Renderer(main_container, [&] {
        auto stats = vbox({
            hbox({text("ID: ") | bold, text(school->id)}),
            hbox({text("Name: ") | bold, text(school->name)}),
            hbox({text("Sector: ") | bold, text(school->location.sector)}),
            hbox({text("Students: ") | bold, text(std::to_string(school->getTotalEnrolledStudents()))}),
            hbox({text("Faculty: ") | bold, text(std::to_string(school->getTotalFaculty()))}),
            }) | border | size(WIDTH, EQUAL, 40);

        Element content;
        if (tab_index == 0) {
            content = vbox({ text("School Management Dashboard") | center, separator(), btn_rename->Render() | center });
        }
        else if (tab_index == 1) {
            Elements list;
            for (int i = 0; i < school->departments.getSize(); ++i) {
                Department* d = school->departments[i];
                list.push_back(text("- " + d->name + " (Classes: " + std::to_string(d->getClassCount()) + ")"));
            }
            content = vbox({
                vbox(list) | flex | border,
                hbox({ btn_add_dept->Render(), text(" "), btn_rem_dept->Render() }) | center
                });
        }
        else if (tab_index == 2) {
            Elements list;
            int limit = 0;
            for (int i = 0; i < school->departments.getSize(); ++i) {
                Department* d = school->departments[i];
                for (int j = 0; j < d->faculty.getSize(); ++j) {
                    if (limit++ > 15) { list.push_back(text("...")); break; }
                    Faculty* f = d->faculty[j];
                    list.push_back(text(f->getName() + " [" + d->name + "] - " + f->qualification));
                }
            }
            content = vbox({
                vbox(list) | flex | border,
                btn_hire_fac->Render() | center
                });
        }
        else {
            Elements list;
            int limit = 0;
            if (school->departments.getSize() > 0) {
                Department* d = school->departments[0];
                if (d->getClassCount() > 0) {
                    Class* c = d->classes[0];
                    for (int k = 0; k < c->students.getSize(); ++k) {
                        if (limit++ > 15) break;
                        list.push_back(text(c->students[k]->getName() + " (Class " + std::to_string(c->classNumber) + ")"));
                    }
                }
            }
            content = vbox({
                text("Sample Student List (First Dept/Class)") | dim,
                vbox(list) | flex | border,
                btn_enroll->Render() | center
                });
        }

        return vbox({
            text(" SCHOOL ADMINISTRATION PORTAL ") | bold | center | bgcolor(ftxui::Color::Blue) | color(ftxui::Color::White),
            hbox({ stats, separator(), content | flex }),
            separator(),
            c_tabs->Render() | center,
            separator(),
            btn_back->Render() | center,
            text(message) | bold | color(ftxui::Color::Red) | center
            }) | border | center;
        });

    screen.Loop(renderer);
}

inline void CityEditorViews::runEditHospitalView(Hospital* hospital) {
    if (!hospital) return;
    auto screen = ScreenInteractive::Fullscreen();
    string message = "";
    int tab_index = 0;
    std::vector<string> tabs = { "Info", "Doctors", "Patients" };

    Component btn_add_beds = Button("Add 10 Beds", [&] { hospital->totalBeds += 10; message = "Beds increased."; });
    Component btn_add_spec = Button("Add Specialization", [&] {
        runInputForm("New Specialization", { "Name (e.g., Cardiology)" }, [&](std::vector<string> res) {
            if (!res[0].empty()) {
                cityMgmt->addSpecializationToHospital(hospital->id, res[0]);
                message = "Added " + res[0];
            }
            });
        });

    Component btn_hire_doc = Button("Hire Doctor", [&] {
        Citizen* c = runPopulationSelector("Select Doctor to Hire");
        if (c) {
            runInputForm("Doctor Contract", { "Specialization" }, [&](std::vector<string> res) {
                if (!res[0].empty()) {
                    Doctor d(c, res[0]);
                    hospital->addDoctor(d);
                    message = "Hired Dr. " + c->name;
                }
                });
        }
        });

    Component btn_admit = Button("Admit Patient", [&] {
        Citizen* c = runPopulationSelector("Select Patient");
        if (c) {
            runInputForm("Admission", { "Condition", "Severity (1-10)" }, [&](std::vector<string> res) {
                int sev = 5; try { sev = std::stoi(res[1]); }
                catch (...) {}
                if (cityMgmt->admitPatient(c->cnic, hospital->id, sev, res[0])) message = "Admitted " + c->name;
                else message = "Admission Failed (No beds?)";
                });
        }
        });

    Component btn_back = Button("Back", screen.ExitLoopClosure());

    auto c_info = Container::Vertical({ btn_add_beds, btn_add_spec });
    auto c_docs = Container::Vertical({ btn_hire_doc });
    auto c_pats = Container::Vertical({ btn_admit });
    auto c_tabs = Toggle(&tabs, &tab_index);

    auto layout = Container::Vertical({ c_tabs, Container::Tab({c_info, c_docs, c_pats}, &tab_index), btn_back });

    auto renderer = Renderer(layout, [&] {
        Element content;
        if (tab_index == 0) {
            Elements specs;
            for (int i = 0; i < hospital->specializations.getSize(); ++i) specs.push_back(text("- " + hospital->specializations[i]));
            content = vbox({ text("Specializations:") | bold, vbox(specs) | border, btn_add_spec->Render(), btn_add_beds->Render() });
        }
        else if (tab_index == 1) {
            Elements docs;
            for (int i = 0; i < hospital->doctors.getSize(); ++i)
                docs.push_back(text("Dr. " + hospital->doctors[i].getCitizen()->name + " (" + hospital->doctors[i].specialization + ")"));
            content = vbox({ vbox(docs) | flex | border, btn_hire_doc->Render() | center });
        }
        else {
            Elements pats;
            for (int i = 0; i < hospital->admittedPatients.getSize(); ++i)
                pats.push_back(text(hospital->admittedPatients[i].getName() + " - " + hospital->admittedPatients[i].getDisease()));
            content = vbox({ vbox(pats) | flex | border, btn_admit->Render() | center });
        }

        return vbox({
            text(" HOSPITAL ADMIN: " + hospital->name) | bold | center | bgcolor(ftxui::Color::Red) | color(ftxui::Color::White),
            hbox({
                vbox({
                    hbox({text("Beds: "), text(std::to_string(hospital->getOccupiedBeds()) + "/" + std::to_string(hospital->totalBeds))})
                }) | border | size(WIDTH, EQUAL, 30),
                content | flex
            }) | flex,
            separator(),
            c_tabs->Render() | center,
            btn_back->Render() | center,
            text(message) | color(ftxui::Color::Yellow) | center
            }) | border;
        });

    screen.Loop(renderer);
}

inline void CityEditorViews::runEditShopView(Shop* shop, Mall* mall) {
    if (!shop) return;
    auto screen = ScreenInteractive::Fullscreen();
    string message = "";

    Component btn_add_prod = Button("Add Product", [&] {
        runInputForm("New Product", { "Product Name", "Price" }, [&](std::vector<string> res) {
            if (!res[0].empty() && !res[1].empty()) {
                int p = 0; try { p = std::stoi(res[1]); }
                catch (...) {}
                Product prod(res[0], shop->category, p);
                shop->addProduct(prod);
                message = "Added " + res[0];
            }
            });
        });

    Component btn_rem_prod = Button("Remove Product", [&] {
        runInputForm("Remove Product", { "Product Name" }, [&](std::vector<string> res) {
            if (shop->removeProduct(res[0])) message = "Removed " + res[0];
            else message = "Product not found.";
            });
        });

    Component btn_back = Button("Back", screen.ExitLoopClosure());

    auto container = Container::Vertical({ btn_add_prod, btn_rem_prod, btn_back });

    auto renderer = Renderer(container, [&] {
        Elements inv;
        for (int i = 0; i < shop->inventory.getSize(); ++i) {
            const Product* p = shop->getProduct(i);
            inv.push_back(hbox({ text(p->name), filler(), text("Rs " + std::to_string(p->price)) | color(ftxui::Color::Green) }));
        }

        return vbox({
            text(" SHOP INVENTORY: " + shop->name) | bold | center | bgcolor(ftxui::Color::Yellow) | color(ftxui::Color::Black),
            separator(),
            hbox({
                vbox(inv) | flex | border,
                vbox({
                    text("Actions") | bold | center,
                    separator(),
                    btn_add_prod->Render(),
                    text(" "),
                    btn_rem_prod->Render(),
                    filler(),
                    btn_back->Render()
                }) | size(WIDTH, EQUAL, 25)
            }) | flex,
            text(message) | color(ftxui::Color::Red) | center
            }) | border;
        });

    screen.Loop(renderer);
}

inline void CityEditorViews::runEditPharmacyView(Pharmacy* pharmacy) {
    if (!pharmacy) return;
    auto screen = ScreenInteractive::Fullscreen();
    string message = "";

    Component btn_add_med = Button("Add Medicine", [&] {
        runInputForm("New Medicine", { "Name", "Formula", "Price" }, [&](std::vector<string> res) {
            if (!res[0].empty() && !res[2].empty()) {
                float p = 0.0f; try { p = std::stof(res[2]); }
                catch (...) {}
                Medicine med(res[0], res[1], p);
                pharmacy->addMedicine(med);
                message = "Added " + res[0];
            }
            });
        });

    Component btn_rem_med = Button("Remove Medicine", [&] {
        runInputForm("Remove Medicine", { "Name" }, [&](std::vector<string> res) {
            if (pharmacy->removeMedicine(res[0])) message = "Removed " + res[0];
            else message = "Medicine not found.";
            });
        });

    Component btn_back = Button("Back", screen.ExitLoopClosure());

    auto container = Container::Vertical({ btn_add_med, btn_rem_med, btn_back });

    auto renderer = Renderer(container, [&] {
        Elements inv;
        for (int i = 0; i < pharmacy->inventory.getSize(); ++i) {
            const Medicine* m = pharmacy->getMedicine(i);
            inv.push_back(hbox({ text(m->name), filler(), text(m->formula) | dim, filler(), text("Rs " + std::to_string((int)m->price)) | color(ftxui::Color::Green) }));
        }

        return vbox({
            text(" PHARMACY INVENTORY: " + pharmacy->name) | bold | center | bgcolor(ftxui::Color::Magenta) | color(ftxui::Color::White),
            separator(),
            hbox({
                vbox({
                    hbox({text("ID: ") | bold, text(pharmacy->id)}),
                    hbox({text("Sector: ") | bold, text(pharmacy->sector)}),
                    hbox({text("Items: ") | bold, text(std::to_string(pharmacy->getMedicineCount()))})
                }) | border | size(WIDTH, EQUAL, 25),
                vbox(inv) | flex | border,
                vbox({
                    text("Actions") | bold | center,
                    separator(),
                    btn_add_med->Render(),
                    text(" "),
                    btn_rem_med->Render(),
                    filler(),
                    btn_back->Render()
                }) | size(WIDTH, EQUAL, 25)
            }) | flex,
            text(message) | color(ftxui::Color::Red) | center
            }) | border;
        });

    screen.Loop(renderer);
}

inline void CityEditorViews::runEditMallView(Mall* mall) {
    if (!mall) return;
    auto screen = ScreenInteractive::Fullscreen();
    string message = "";
    int selected_shop_idx = 0;

    Component btn_add_shop = Button("Add Shop", [&] {
        runInputForm("New Shop", { "Shop Name", "Category" }, [&](std::vector<string> res) {
            if (!res[0].empty()) {
                string newID = "SHOP-" + std::to_string(mall->getShopCount() + 100);
                Shop* s = new Shop(newID, res[0], res[1]);
                mall->addShop(s);
                message = "Added Shop: " + res[0];
            }
            });
        });

    Component btn_rem_shop = Button("Remove Shop", [&] {
        runInputForm("Remove Shop", { "Shop ID" }, [&](std::vector<string> res) {
            bool found = false;
            for (int i = 0; i < mall->shops.getSize(); ++i) {
                if (mall->shops[i]->id == res[0]) {
                    delete mall->shops[i];
                    mall->shops.erase(i);
                    found = true;
                    message = "Removed Shop ID: " + res[0];
                    break;
                }
            }
            if (!found) message = "Shop ID not found.";
            });
        });

    Component btn_edit_shop = Button("Edit Selected Shop", [&] {
        if (selected_shop_idx >= 0 && selected_shop_idx < mall->shops.getSize()) {
            runEditShopView(mall->shops[selected_shop_idx], mall);
        }
        else {
            message = "No shop selected.";
        }
        });

    Component btn_back = Button("Back", screen.ExitLoopClosure());

    std::vector<string> shop_names;

    auto menu_shops = Menu(&shop_names, &selected_shop_idx);

    auto container = Container::Vertical({
        menu_shops | vscroll_indicator | frame | flex,
        btn_add_shop, btn_rem_shop, btn_edit_shop, btn_back
        });

    auto renderer = Renderer(container, [&] {
        shop_names.clear();
        for (int i = 0; i < mall->shops.getSize(); ++i) {
            shop_names.push_back(mall->shops[i]->name + " (" + mall->shops[i]->category + ")");
        }

        return vbox({
            text(" MALL MANAGEMENT: " + mall->name) | bold | center | bgcolor(ftxui::Color::Yellow) | color(ftxui::Color::Black),
            separator(),
            hbox({
                vbox({
                    hbox({text("ID: ") | bold, text(mall->id)}),
                    hbox({text("Sector: ") | bold, text(mall->getSector())}),
                    hbox({text("Shops: ") | bold, text(std::to_string(mall->getShopCount()))})
                }) | border | size(WIDTH, EQUAL, 30),
                vbox({
                    text("Shops Directory") | bold | center,
                    separator(),
                    menu_shops->Render() | flex
                }) | flex | border,
                vbox({
                    text("Actions") | bold | center,
                    separator(),
                    btn_add_shop->Render(),
                    text(" "),
                    btn_rem_shop->Render(),
                    text(" "),
                    btn_edit_shop->Render(),
                    filler(),
                    btn_back->Render()
                }) | size(WIDTH, EQUAL, 25)
            }) | flex,
            text(message) | color(ftxui::Color::Red) | center
            }) | border;
        });

    screen.Loop(renderer);
}

inline void CityEditorViews::runEditObjectView(const string& objectID, const string& objectType) {
    if (objectType == "SCHOOL") {
        if (islamabad && islamabad->getSchoolManager()) {
            School* s = islamabad->getSchoolManager()->findSchoolByID(objectID);
            if (s) { runEditSchoolView(s); return; }
        }
    }
    else if (objectType == "HOSPITAL") {
        if (islamabad && islamabad->getMedicalManager()) {
            Hospital* h = islamabad->getMedicalManager()->findHospitalByID(objectID);
            if (h) { runEditHospitalView(h); return; }
        }
    }
    else if (objectType == "SHOP") {
        if (islamabad && islamabad->getCommercialManager()) {
            CommercialManager* cm = islamabad->getCommercialManager();
            for (int i = 0; i < cm->malls.getSize(); i++) {
                Shop* s = cm->malls[i]->findShopByID(objectID);
                if (s) { runEditShopView(s, cm->malls[i]); return; }
            }
        }
    }
    else if (objectType == "PHARMACY") {
        if (islamabad && islamabad->getMedicalManager()) {
            MedicalManager* mm = islamabad->getMedicalManager();
            for (int i = 0; i < mm->pharmacies.getSize(); ++i) {
                if (mm->pharmacies[i]->id == objectID) {
                    runEditPharmacyView(mm->pharmacies[i]);
                    return;
                }
            }
        }
    }
    else if (objectType == "MALL") {
        if (islamabad && islamabad->getCommercialManager()) {
            CommercialManager* cm = islamabad->getCommercialManager();
            if (cm->mallLookup.contains(objectID)) {
                Mall** m = cm->mallLookup.get(objectID);
                if (m && *m) {
                    runEditMallView(*m);
                    return;
                }
            }
            for (int i = 0; i < cm->malls.getSize(); ++i) {
                if (cm->malls[i]->id == objectID) {
                    runEditMallView(cm->malls[i]);
                    return;
                }
            }
        }
    }

    // Fallback generic editor
    auto screen = ScreenInteractive::Fullscreen();
    auto renderer = Renderer([&] {
        return vbox({
            text("GENERIC EDITOR: " + objectID) | bold | center,
            separator(),
            text("Type: " + objectType) | center,
            text("Specific editor not implemented yet.") | dim | center,
            text("Press Esc to return") | dim | center
            }) | border | center;
        });
    auto component = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Escape) { screen.Exit(); return true; }
        return false;
        });
    screen.Loop(component);
}

#endif // CITY_EDITOR_VIEWS_H
