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
        name.set_text(package.extract_name());
        name.set_halign(Gtk::Align::START);
        name.set_hexpand(true);

        icon.set_from_icon_name( this->package.is_installed() ? "emblem-default" : "system-software-install" );
    }


};

class PackageDisplay : public Gtk::Box {
    Package package;
    Gtk::Label name;
    Gtk::Box controls_box;
    Gtk::Button install; //Doubles as update button since installing an already installed package updates it
    Gtk::Button uninstall;

    public:
    PackageDisplay() {
        this->package = Package("none/no-package noversion", "NO DESCRIPTION");

        install.signal_clicked().connect([this](){
            this->install.set_sensitive(false);
            this->uninstall.set_sensitive(false);

            std::jthread([this]() {
                this->package.install();

                Glib::signal_idle().connect_once([this](){
                    this->render();
                });
            }).detach();
        });

        uninstall.signal_clicked().connect([this](){
            this->install.set_sensitive(false);
            this->uninstall.set_sensitive(false);

            std::jthread([this]() {
                this->package.uninstall();

                Glib::signal_idle().connect_once([this](){
                    this->render();
                });

            }).detach();

        });

        this->render();
    }

    void set_package(Package package) {
        this->package = package;
        this->render();
    } 

    void render() {
        for (auto c: this->get_children()) {this->remove(*c);}

        this->set_orientation(Gtk::Orientation::VERTICAL);
        this->set_hexpand(true);
        this->set_vexpand(true);
        this->set_valign(Gtk::Align::CENTER);

        name.set_text(package.extract_name());

        this->install.set_sensitive(true);
        install.set_label( package.is_installed() ? "Update" : "Install" );

        this->uninstall.set_sensitive(true);
        uninstall.set_label( "Uninstall" );


        controls_box.set_orientation(Gtk::Orientation::HORIZONTAL);
        controls_box.append(install);
        controls_box.append(uninstall);
        controls_box.set_halign(Gtk::Align::CENTER);

        append(name);
        append(controls_box);
    }

};