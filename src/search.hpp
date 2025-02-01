
#pragma once

#include <gtkmm.h>
#include <string>
#include <thread>
#include <vector>
#include <format>
#include <iostream>
#include <bits/stdc++.h>
#include "package.hpp"
#include "database.hpp"
using namespace std;

#define PACKAGES_PER_PAGE 100





string filterToText(Filter f) {
    switch (f) {
        case EVERYTHING:
            return "Everything";
            break;
        case INSTALLED:
            return "Installed";
            break;
        case NOT_INSTALLED:
            return "Not installed";
            break;
    }
}

Filter textToFilter(string s) {
    if (s == "Everything") {return Filter::EVERYTHING;}
    if (s == "Installed") {return Filter::INSTALLED;}
    if (s == "Not installed") {return Filter::NOT_INSTALLED;}
}


//{"Default Sort", "Sort By Installed Size", "Sort By Install Date"}
string sorterToText(Sorter s) {
    switch (s) {
        case NONE:
            return "Default Sort";
            break;
        case INSTALLED_SIZE:
            return "Sort By Installed Size";
            break;
        case INSTALLED_DATE:
            return "Sort By Install Date";
            break;
    }
}

Sorter textToSorter(string text) {
    if (text == "Default Sort") {
        return NONE;
    } else if (text == "Sort By Installed Size") {
        return INSTALLED_SIZE;
    } else if (text == "Sort By Install Date") {
        return INSTALLED_DATE;
    } else {
        throw invalid_argument("Unknown sort type: " + text);
    }
}




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
    Gtk::Box search_options;
    Gtk::Button page_up;
    Gtk::Button page_down;
    Gtk::Label page_label;
    Gtk::Label range_label;
    Gtk::Spinner spinner;
    Gtk::DropDown filter_selector;
    Gtk::DropDown sorter_selector;
    Gtk::Button order_button;
    
    PackageDatabase* database;
    bool is_loading;
    bool inverse_order;
    Filter filter_state;
    Sorter sorter_state;
    vector<Package> packages;
    std::mutex mutex;
    std::jthread worker;
    int page;

    void connect_database(PackageDatabase* database) {
        this->database = database;
        this->handle_input_submit();
    }

   void handle_input_submit() {
        if (this->database == NULL) {return;} //Database is not ready yet 
        auto query = this->text_input.get_text();
        this->worker.request_stop(); 
        this->is_loading = true;
        this->render();

        this->worker = std::jthread([this, query](std::stop_token stopToken) {
            auto filter_state = this->filter_state;
            auto sorter_state = this->sorter_state;


            auto packages = this->database->query_database(query, filter_state, sorter_state);
            if (this->inverse_order) {std::reverse(packages.begin(), packages.end());}
            //std::cout << "Query for " << query << " recieved " << packages.size() << " packages\n"; 


            if (stopToken.stop_requested()) {
                //std::cout << "THREAD STOPPED\n";
                Glib::signal_idle().connect_once([this]() {
                    this->is_loading = false;
                    this->render();
                });

                return; //Do nothing
            }

            this->mutex.lock();

            this->packages = packages;
            this->page = 1;

            Glib::signal_idle().connect_once([this]() {
                this->is_loading = false;
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
        this->page = 1;
        this->filter_state = Filter::EVERYTHING;
        this->sorter_state = Sorter::NONE;
        this->inverse_order = false;
        //this->database = PackageDatabase();

        this->set_margin(10);

        spinner.start();
        spinner.set_size_request(100,100);

        set_orientation(Gtk::Orientation::VERTICAL);
        set_vexpand(true);

        auto filter_store = Gtk::StringList::create({"Everything", "Installed", "Not installed"});
        filter_selector.set_model(filter_store); // Gtk::DropDown

        Glib::signal_idle().connect([this]() {
            auto string_selection = GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(filter_selector.gobj()));
            if (this->filter_state != textToFilter(gtk_string_object_get_string(string_selection)) ) {
                this->filter_state = textToFilter(gtk_string_object_get_string(string_selection));
                this->handle_input_submit();
            }
            return true;
        });

        auto sorter_store = Gtk::StringList::create({"Default Sort", "Sort By Installed Size", "Sort By Install Date"});
        sorter_selector.set_model(sorter_store);

        Glib::signal_idle().connect([this](){
            auto string_selection = GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(sorter_selector.gobj()));
            if (this->sorter_state != textToSorter(gtk_string_object_get_string(string_selection)) ) {
                this->sorter_state = textToSorter(gtk_string_object_get_string(string_selection));
                this->handle_input_submit();
            }
            return true;
        });

        order_button.signal_clicked().connect([this]() {
            this->inverse_order = !this->inverse_order;
            this->handle_input_submit();
        });

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
        //text_input.signal_changed().connect([this]() {this->handle_input_submit();});
        text_input.signal_activate().connect([this]() {this->handle_input_submit();});

        top_bar.set_orientation(Gtk::Orientation::HORIZONTAL);
        top_bar.append(text_input);
        top_bar.append(page_down);
        top_bar.append(page_label);
        top_bar.append(page_up);

        search_options.append(filter_selector);
        search_options.append(sorter_selector);
        search_options.append(order_button);

        scroll.set_vexpand(true);

        this->append(top_bar);
        this->append(search_options);
        this->append(found_label);
        this->append(range_label);
        this->append(scroll);

        //this->handle_input_submit();
        this->render();
    }

    void render() {
        
        page_label.set_text(std::to_string(page) + "/" + std::to_string( this->get_num_pages() ));
        order_button.set_label(inverse_order ? "Ascending" : "Descending");

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
            //`this->packages` might be overwritten mid-rendering
            if (i >= packages.size()) {break;}
            auto p = this->packages[i];

            auto label = Gtk::manage( new PackageButton(p) );

            label->signal_clicked().connect([this, p, label](){
                std::jthread([this, label](){
                    this->signal_changed().emit(label->package);
                }).detach();
            });
            label->set_vexpand(false);
            packages_components.append(*label);
        }

        if (!is_loading) {
            this->scroll.set_child(this->packages_components) ;
        } else {
            auto spinner_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
            spinner_box->set_halign(Gtk::Align::CENTER);  // Horizontal centering
            spinner_box->set_valign(Gtk::Align::CENTER);  // Vertical centering
            spinner_box->append(spinner);
            this->scroll.set_child(*spinner_box);
        }
  
    }

};

