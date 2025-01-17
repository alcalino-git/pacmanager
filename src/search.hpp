
#pragma once

#include <gtkmm.h>
#include <string>
#include <thread>
#include <vector>
#include <format>
#include <iostream>
#include <bits/stdc++.h>
#include "package.hpp"
using namespace std;

#define PACKAGES_PER_PAGE 50







class SearchComponent : public Gtk::Box {

    public:
    Gtk::Entry text_input;
    Gtk::Label label;
    Gtk::ScrolledWindow scroll;
    Gtk::Box packages_components;
    Gtk::Box top_bar;
    Gtk::Button page_up;
    Gtk::Button page_down;
    Gtk::Label page_label;
    Gtk::Label range_label;

    vector<Package> packages;
    std::mutex mutex;
    std::jthread worker;
    int page;

    /*
    Maybe an an std::atomic(0) int to count how many threads are currently running
    And only rerun Glib::signal_idle if it is 0?
    */

   void handle_input_submit() {
        auto query = this->text_input.get_text();
        this->worker.request_stop(); 

        this->worker = std::jthread([this, query](std::stop_token stopToken) {
            auto packages = Package::get_packages(query);

            if (stopToken.stop_requested()) {
                return; //Do nothing
            }
            this->mutex.lock();
            //std::cout << "Query for " << query << " recieved " << packages.size() << " packages\n"; 
            this->packages = packages;
            this->scroll.get_vadjustment()->set_value(0);
            this->page = 1;
            this->mutex.unlock();
        });
        this->worker.detach();
   }


    ///Returns how many pages of `PACKAGES_PER_PAGE` size can exist given the current amount of packages
    int get_num_pages() {
        return std::ceil((float)this->packages.size() / PACKAGES_PER_PAGE);
    }

    SearchComponent() {
        Glib::signal_idle().connect([this]() {
            this->render();
            return true;
        });
        this->page = 1;


        set_orientation(Gtk::Orientation::VERTICAL);
        set_vexpand(true);

        top_bar.set_orientation(Gtk::Orientation::HORIZONTAL);

        page_down = Gtk::Button("<");
        page_up = Gtk::Button(">");

        page_down.signal_clicked().connect([this](){
            this->page--;
            if (this->page < 1) {this->page = this->get_num_pages();}
        });
        page_up.signal_clicked().connect([this](){
            this->page++;
            if (this->page > this->get_num_pages()) {this->page = 1;}            
        });

        text_input = Gtk::Entry();
        text_input.set_hexpand(true);
        text_input.signal_activate().connect([this]() {this->handle_input_submit();});

        top_bar.append(text_input);
        top_bar.append(page_down);
        top_bar.append(page_label);
        top_bar.append(page_up);

        scroll.set_vexpand(true);

        this->append(top_bar);
        this->append(label);
        this->append(range_label);
        this->append(scroll);

        
    }

    void render() {
        page_label.set_text(std::to_string(page) + "/" + std::to_string( this->get_num_pages() ));

        packages_components.set_orientation(Gtk::Orientation::VERTICAL);
        packages_components.set_vexpand(true);

        label.set_text("Found " + std::to_string( packages.size() ) + " package(s)");


        for (auto c: packages_components.get_children()) {packages_components.remove(*c);};

        auto start = ((this->page-1) * 50);
        auto end = ( this->page * 50 );
        if (end >= this->packages.size()) {end = this->packages.size();}

        range_label.set_text("Rendering packages " + std::to_string(start) + "-" + std::to_string(end));

        for (int i = start; i < end; i++ ) {
            auto p = this->packages[i];
            auto label = Gtk::manage( new Gtk::Label(p.extract_name()) );
            packages_components.append(*label);
        }

        this->scroll.set_child(this->packages_components);
  
    }

};

