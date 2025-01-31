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


enum PackageOperations {
    INSTALL,
    UNINSTALL,
    SYS_UPDATE
};

class PackageDisplay : public Gtk::Box {
    using type_signal_package_update = sigc::signal<void(Package)>;

    Package package;
    Gtk::Label name;
    Gtk::Label description;
    Gtk::Label status;
    Gtk::Box controls_box;
    Gtk::Button install; //Doubles as update button since installing an already installed package updates it
    Gtk::Button uninstall;
    Gtk::Button system_update;
    Gtk::Spinner spinner;
    Gtk::MessageDialog* dialog;

    bool initialized; //set to `false` until a valid package is set via `set_package`
    bool installing; //set to `true` when a package install/update/delete operation is currently in progress

    public:

    type_signal_package_update m_signal_update;
    type_signal_package_update signal_update() {
        return m_signal_update;
    };

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

    void package_operation(PackageOperations operation) {
        //TODO: UPDATE PACKAGE DATABASE ONCE THIS IS DONE RUNNING
        this->installing = true;
        this->render();

        std::jthread([this, operation]() {
            string command;

            if (operation == PackageOperations::INSTALL) {command = ("pkexec pacman --noconfirm  -Syy " + this->package.get_property("Name") );} 
            if (operation == PackageOperations::UNINSTALL) {command = ("pkexec pacman --noconfirm  -R " + this->package.get_property("Name")); } 
            if (operation == PackageOperations::SYS_UPDATE) {command = ("pkexec pacman --noconfirm  -Syu "); } 

            std::cout << "WILL RUN: " << command << "\n";
            auto command_out = popen(command.c_str(), "r");

            char buffer[128];
            while (fgets(buffer, sizeof(buffer), command_out) !=  NULL) {
                std::cout << buffer << "\n"; //TODO: DRAW THIS TO THE INTERFACE
                this->status.set_text(buffer);
                Glib::signal_idle().connect_once([this](){
                    this->render();
                });
            }

            this->installing = false;
            auto status = pclose(command_out);
            if (status != 0 ) {
                string action_string;
                if (operation == PackageOperations::INSTALL) {action_string = "Failed to install/update"; } 
                if (operation == PackageOperations::UNINSTALL) {action_string = "Failed to uninstall"; } 
                if (operation == PackageOperations::SYS_UPDATE) {action_string = "Failed to perform system update";} 
                this->error_dialog(action_string, status);
            }
            this->package.refetch_data();
            this->signal_update().emit(this->package);
            Glib::signal_idle().connect_once([this](){
                this->render();
            });
        }).detach();
    }

    void error_dialog(string message, int err_code) {
        Glib::signal_idle().connect_once([this,message, err_code](){
            this->dialog = new Gtk::MessageDialog(message + "\nError code: " + std::to_string(err_code));
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
            this->package_operation(PackageOperations::INSTALL);
        });

        uninstall.set_margin(10);
        uninstall.set_tooltip_text("Removes package from the system");
        uninstall.signal_clicked().connect([this](){
            this->package_operation(PackageOperations::UNINSTALL);
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
            this->package_operation(PackageOperations::SYS_UPDATE);
        });

        append(name);
        append(description);
        append(controls_box);
        append(spinner);
        append(status);

        this->render();
    }

    void set_package(Package package) {
        this->package = package;
        this->package.refetch_data();
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

        if (!installing) {this->status.set_text("No operation is currently happening");}



    }

};