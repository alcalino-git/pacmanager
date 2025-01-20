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
    Gtk::Button system_update;
    Gtk::Spinner spinner;
    Gtk::MessageDialog* dialog;

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

    void package_operation(bool install) {
        this->installing = true;
        this->render();

        std::jthread([this, install]() {
            auto result = install ? this->package.install() : this->package.uninstall();
            if (result != 0) {this->error_dialog(install ? "installed/updated" : "removed", result);}
            this->installing = false;
            Glib::signal_idle().connect_once([this](){
                this->render();
            });
        }).detach();
    }

    void error_dialog(string s, int err_code) {
        Glib::signal_idle().connect_once([this,s, err_code](){
            this->dialog = new Gtk::MessageDialog("Package could not be " + s + "\nError code: " + std::to_string(err_code));
            this->dialog->set_title("An error has ocurred");
            this->dialog->set_size_request(500);
            this->dialog->signal_response().connect([this](auto r){
                this->dialog->set_visible(false);
            });
            this->dialog->set_visible(true);
        });
    }

    PackageDisplay() {
        this->initialized = false;
        this->installing = false;
        this->package = Package("none/no-package noversion");
        this->set_margin(50);
        this->set_size_request(500);

        this->set_orientation(Gtk::Orientation::VERTICAL);
        this->set_hexpand(true);
        this->set_vexpand(true);
        this->set_valign(Gtk::Align::CENTER);

        install.set_margin(10);
        install.signal_clicked().connect([this](){
            this->package_operation(true);
        });

        uninstall.set_margin(10);
        uninstall.set_tooltip_text("Removes package from the system");
        uninstall.signal_clicked().connect([this](){
            this->package_operation(false);
        });

        system_update.set_label("full update");
        system_update.set_tooltip_text("Updates every package in the system all at the same time");
        system_update.set_margin(10);


        controls_box.append(install);
        controls_box.append(uninstall);
        controls_box.append(system_update);
        controls_box.set_orientation(Gtk::Orientation::HORIZONTAL);
        controls_box.set_halign(Gtk::Align::CENTER);

        name.set_margin(10);
        description.set_margin(10);
        description.set_wrap(true);
        description.set_size_request(400);
        
        spinner.start();
        spinner.set_visible(false);


        system_update.signal_clicked().connect([this](){
        
            this->installing = true;
            this->render();

            std::jthread([this]() {
                Package::system_update();
                this->installing = false;
                Glib::signal_idle().connect_once([this](){
                    this->render();
                });

            }).detach();

        });

        append(name);
        append(description);
        append(controls_box);
        append(spinner);

        this->render();
    }

    void set_package(Package package) {
        this->package = package;
        //this->installing = false;
        this->initialized = true;
        this->render();
    } 

    void render() {
        //for (auto c: this->get_children()) {this->remove(*c);}


        name.set_text(package.get_property("Name"));
        description.set_text(this->get_package_data());

        this->install.set_sensitive(!installing && initialized);
        install.set_label( package.is_installed() ? "Update" : "Install" );
        install.set_tooltip_text( 
            package.is_installed() 
            ? "Installs updates for this package if any are avalible" 
            : "Installs this package in the system"
        );

        this->uninstall.set_sensitive(!installing && initialized && this->package.is_installed());
        uninstall.set_label( "Uninstall" );

        system_update.set_sensitive(!installing);

        spinner.set_visible(installing);



    }

};