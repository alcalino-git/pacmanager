#pragma once

#include <string>
#include <vector>
#include <thread>
#include <gtkmm.h>
#include "logic.hpp"


class PackageButton : public Gtk::Button {

    Gtk::Box content;
    Gtk::Label name;
    Gtk::Image icon;

    public:
    Package package;

    PackageButton(Package package) {
        this->package = package;
        this->set_hexpand(true);
        this->set_margin(10);
        this->set_can_focus(true);   // Allows the button to receive focus
        this->set_sensitive(true);   // Ensures the button is sensitive (not disabled)

        

        content.set_hexpand(true);
        content.set_vexpand(true);
        content.set_orientation(Gtk::Orientation::HORIZONTAL);


        icon.set_halign(Gtk::Align::END);

        content.append(name);
        content.append(icon);
        this->set_child(content);

        this->signal_clicked().connect([this]() {
            std::jthread([this]() {
                this->package.refetch_data();
                Glib::signal_idle().connect_once([this](){this->update();});
            }).detach();
        });

        this->update();
    }

    ///Re-renders the entire button
    void update() {
        name.set_text(package.get_property("Name"));
        name.set_halign(Gtk::Align::START);
        name.set_hexpand(true);

        std::jthread([this]() {
            //TODO: The `is_installed` operation is stupidly expensive for some reason
            icon.set_from_icon_name( /*this->package.is_installed()*/ false ? "emblem-default" : "system-software-install" );
        }).detach();
    }


};

class PackageDisplay : public Gtk::Box {
    Package package;
    Gtk::Label name;
    Gtk::Label description;
    Gtk::Box controls_box;
    Gtk::Button install; //Doubles as update button since installing an already installed package updates it
    Gtk::Button uninstall;
    Gtk::Spinner spinner;

    bool initialized; //set to `false` until a valid package is set via `set_package`
    bool installing; //set to `true` when a package install/update/delete operation is currently in progress

    public:

    string get_package_data() {
        return initialized 
        ?
        "Description: " + package.get_property("Description") + "\n\n" +
        "Packager: " + package.get_property("Packager") + "\n\n" +
        "Installed size: " + package.get_property("Installed Size")+ "\n\n" +
        "Version: " + package.get_property("Version")
        :
        ""
        ;
    }

    PackageDisplay() {
        this->initialized = false;
        this->package = Package("none/no-package noversion", "NO DESCRIPTION");
        this->set_margin(50);
        this->set_size_request(500);

        install.set_margin(10);
        install.signal_clicked().connect([this](){
            // this->install.set_sensitive(false);
            // this->uninstall.set_sensitive(false);
            this->installing = true;
            this->render();

            std::jthread([this]() {
                this->package.install();
                this->installing = false;
                Glib::signal_idle().connect_once([this](){
                    this->render();
                });
            }).detach();
        });

        uninstall.set_margin(10);
        uninstall.signal_clicked().connect([this](){
            // this->install.set_sensitive(false);
            // this->uninstall.set_sensitive(false);
            this->installing = true;
            this->render();

            std::jthread([this]() {
                this->package.uninstall();
                this->installing = false;
                Glib::signal_idle().connect_once([this](){
                    this->render();
                });

            }).detach();

        });

        controls_box.append(install);
        controls_box.append(uninstall);

        name.set_margin(10);
        description.set_margin(10);
        description.set_wrap(true);
        description.set_size_request(400);
        
        spinner.start();
        spinner.set_visible(false);

        append(name);
        append(description);
        append(controls_box);
        append(spinner);

        this->render();
    }

    void set_package(Package package) {
        this->package = package;
        this->initialized = true;
        this->render();
    } 

    void render() {
        //for (auto c: this->get_children()) {this->remove(*c);}

        this->set_orientation(Gtk::Orientation::VERTICAL);
        this->set_hexpand(true);
        this->set_vexpand(true);
        this->set_valign(Gtk::Align::CENTER);

        name.set_text(package.get_property("Name"));
        description.set_text(this->get_package_data());

        this->install.set_sensitive(!installing && initialized);
        install.set_label( package.is_installed() ? "Update" : "Install" );

        this->uninstall.set_sensitive(!installing && initialized);
        uninstall.set_label( "Uninstall" );

        spinner.set_visible(installing);


        controls_box.set_orientation(Gtk::Orientation::HORIZONTAL);
        controls_box.set_halign(Gtk::Align::CENTER);

    }

};