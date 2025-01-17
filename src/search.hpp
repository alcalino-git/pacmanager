
#pragma once

#include <gtkmm.h>
#include <string>
#include <thread>
#include <vector>
#include <format>
#include <iostream>
#include <bits/stdc++.h>
using namespace std;

class Package {
    public:
    string name;
    string description;

    Package() {
        this->name = "";
        this->description = "";
    }

    Package(string name, string description) {
        this->name = name;
        this->description = description;
    }
};

///Blocks the main thead for `s` seconds
void sleep(int s) {
    std::this_thread::sleep_for( std::chrono::seconds(s) );
}



/// @brief Dummy implementation for testing purposes
/// @param s string pacman -Ss will use
/// @return a list of found packages
vector<Package> get_packages(std::string search) {
    //if (search == "xo") {search = "x";} 

    char command[1024];
    sprintf(command, "pacman -Ss \"%s\"", search.c_str());
    //std::cout << "WILL ATTEMPT TO RUN: " << command << "\n";

    FILE *result;
    result = popen(command, "r");
    
    char buffer[1024];
    int line_number = 0;

    std::vector<std::string> names{};
    std::vector<std::string> descriptions{}; 
    while(fgets(buffer, sizeof(buffer), result)) {
        if (line_number % 2 == 0) {names.push_back(std::string(buffer));} else {descriptions.push_back(std::string(buffer));}
        line_number++;
    }

    vector<Package> packages{};
    for (int i = 0; i < names.size(); i++) {
        auto package = Package(names[i], descriptions[i]);
        packages.push_back(package);
    }

    pclose(result);
    return packages;
}

#define PACKAGES_PER_PAGE 50


class SearchComponent : public Gtk::Box {

    public:
    Gtk::Entry textInput;
    Gtk::Label label;
    Gtk::ScrolledWindow scroll;
    Gtk::Box packages_components;

    vector<Package> packages;
    std::mutex mutex;
    std::jthread worker;

    /*
    Maybe an an std::atomic(0) int to count how many threads are currently running
    And only rerun Glib::signal_idle if it is 0?
    */

   void handle_input_submit() {
        auto query = this->textInput.get_text();
        this->worker.request_stop(); 

        this->worker = std::jthread([this, query](std::stop_token stopToken) {
            auto packages = get_packages(query);

            if (stopToken.stop_requested()) {
                return; //Do nothing
            }
            this->mutex.lock();
            //std::cout << "Query for " << query << " recieved " << packages.size() << " packages\n"; 
            this->packages = packages;
            this->scroll.get_vadjustment()->set_value(0);
            this->mutex.unlock();
        });
        this->worker.detach();
   }



    SearchComponent() {
        Glib::signal_idle().connect([this]() {
            this->render();
            return true;
        });


        set_orientation(Gtk::Orientation::VERTICAL);
        set_vexpand(true);

        textInput = Gtk::Entry();
        textInput.signal_activate().connect([this]() {this->handle_input_submit();});
        this->append(textInput);

        this->append(label);

        scroll.set_vexpand(true);
        this->append(scroll);

        
    }

    void render() {
        packages_components.set_orientation(Gtk::Orientation::VERTICAL);
        packages_components.set_vexpand(true);
        label.set_text("Found " + std::to_string( packages.size() ) + " package(s)");


       for (auto c: packages_components.get_children()) {packages_components.remove(*c);};

       for (auto p: this->packages ) {
            auto label = Gtk::manage( new Gtk::Label(p.name) );
            packages_components.append(*label);
       }

       this->scroll.set_child(this->packages_components);
  
    }

};

