#pragma once

#include <string>
#include <vector>
#include <thread>
#include <gtkmm.h>
using namespace std;

vector<string> split_by_char(string s, char c) {
    vector<string> result{};

    string buffer = "";

    for (int i = 0; i < s.length(); i++) {
        if (s[i] == c) {
            result.push_back(buffer);
            buffer = "";
        } else {
            buffer += s[i];
        }
    }
    if (buffer.length() != 0) {result.push_back(buffer);}

    return result;
}


/// @brief Runs `command` and returns its full output. Highly blocking
/// @param command 
/// @return 
vector<string> get_command_line_output(string command) {
    FILE *result;
    result = popen(command.c_str(), "r");

    char buffer[1024];
    int line_number = 0;

    std::vector<std::string> lines{};
    while(fgets(buffer, sizeof(buffer), result)) {
        lines.push_back(buffer);
    }

    pclose(result);
    return lines;
}



//TODO: Actually re implement this entire thing, it sucks
class Package {
    public:
    string name;
    string description;

    /// @brief Dummy implementation for testing purposes
    /// @param s string pacman -Ss will use
    /// @return a list of found packages
    static vector<Package> get_packages(std::string search) {

        auto lines = get_command_line_output("pacman -Ss \"" + search + "\"");

        std::vector<std::string> names{};
        std::vector<std::string> descriptions{}; 
        for (int i = 0; i < lines.size(); i++) {
            if (i % 2 == 0) {names.push_back(lines[i]);} else {descriptions.push_back(lines[i]);}
        }

        vector<Package> packages{};
        for (int i = 0; i < names.size(); i++) {
            auto package = Package(names[i], descriptions[i]);
            packages.push_back(package);
        }
        return packages;
    }

    Package() {
        this->name = "";
        this->description = "";
    }

    Package(string name, string description) {
        this->name = name;
        this->description = description;
    }
    
    //TODO: This doesnt work. Needs to use `pacman -Si` instead
    void refetch_data() {
        auto lines = get_command_line_output("pacman -Si \"" + this->extract_name() + "\"");
        this->name = split_by_char(lines[1], ':')[1];
        this->description = lines[1];
    }

    /// @brief Extracts the raw name from the package (no repo or [installed])
    /// @return 
    string extract_name() {
        return  split_by_char(split_by_char(this->name, ' ')[0], '/')[1];
    }

    bool is_installed() {
        return this->name.contains("[installed]");
    }

    void install() {
        auto output = get_command_line_output( "pkexec pacman --noconfirm  -Syy " + this->extract_name() );
        for (auto l: output) {
            std::cout << l << "\n";
        }
        this->refetch_data();
    }

    void uninstall() {
        auto output = get_command_line_output( "pkexec pacman --noconfirm  -R " + this->extract_name() );
        for (auto l: output) {
            std::cout << l << "\n";
        }
        this->refetch_data();
    }
};


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