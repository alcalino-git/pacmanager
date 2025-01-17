
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

#define PACKAGES_PER_PAGE 100







class SearchComponent : public Gtk::Box {

    using type_signal_package_changed = sigc::signal<void(Package)>;



    public:

    type_signal_package_changed m_signal_changed;
    type_signal_package_changed signal_changed() {
        return m_signal_changed;
    };

    Gtk::SearchEntry text_input;
    Gtk::Label found_label;
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
            this->page = 1;

            Glib::signal_idle().connect_once([this]() {
                this->render();
            });

            this->mutex.unlock();

        });
        this->worker.detach();
   }


    ///Returns how many pages of `PACKAGES_PER_PAGE` size can exist given the current amount of packages
    int get_num_pages() {
        return std::ceil((float)this->packages.size() / PACKAGES_PER_PAGE);
    }

    SearchComponent() {
        this->handle_input_submit();
        this->page = 1;


        set_orientation(Gtk::Orientation::VERTICAL);
        set_vexpand(true);

        top_bar.set_orientation(Gtk::Orientation::HORIZONTAL);

        page_down = Gtk::Button("<");
        page_up = Gtk::Button(">");

        page_down.signal_clicked().connect([this](){
            this->page--;
            if (this->page < 1) {this->page = this->get_num_pages();}
            this->render();
        });
        page_up.signal_clicked().connect([this](){
            this->page++;
            if (this->page > this->get_num_pages()) {this->page = 1;}  
            this->render();          
        });

        text_input = Gtk::SearchEntry();
        text_input.set_hexpand(true);
        text_input.signal_activate().connect([this]() {this->handle_input_submit();});

        top_bar.append(text_input);
        top_bar.append(page_down);
        top_bar.append(page_label);
        top_bar.append(page_up);

        scroll.set_vexpand(true);

        this->append(top_bar);
        this->append(found_label);
        this->append(range_label);
        this->append(scroll);

        
    }

    void render() {
        
        page_label.set_text(std::to_string(page) + "/" + std::to_string( this->get_num_pages() ));

        this->scroll.get_vadjustment()->set_value(0);

        for (auto c: packages_components.get_children()) {packages_components.remove(*c);};
        packages_components.set_orientation(Gtk::Orientation::VERTICAL);
        packages_components.set_vexpand(true);
        

        found_label.set_text("Found " + std::to_string( packages.size() ) + " package(s)");

        auto start = ((this->page-1) * PACKAGES_PER_PAGE);
        auto end = ( this->page * PACKAGES_PER_PAGE );
        if (end >= this->packages.size()) {end = this->packages.size();}

        range_label.set_text("Rendering packages " + std::to_string(start) + "-" + std::to_string(end));

        for (int i = start; i < end; i++ ) {
            auto p = this->packages[i];
            auto label = Gtk::manage( new PackageButton(p) );

            label->signal_clicked().connect([this, p, label](){
                std::jthread([this, label](){
                    label->package.refetch_data();
                    this->signal_changed().emit(label->package);
                }).detach();
            });

            label->set_vexpand(false);
            packages_components.append(*label);
        }

        this->scroll.set_child(this->packages_components);
  
    }

};

