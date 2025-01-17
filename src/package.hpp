#pragma once

#include <string>
#include <vector>
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


class Package {
    public:
    string name;
    string description;

    /// @brief Dummy implementation for testing purposes
    /// @param s string pacman -Ss will use
    /// @return a list of found packages
    static vector<Package> get_packages(std::string search) {

        auto lines = get_command_line_output("pacman -Ss " + search);

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
    }

    void uninstall() {
        auto output = get_command_line_output( "pkexec pacman --noconfirm  -R " + this->extract_name() );
        for (auto l: output) {
            std::cout << l << "\n";
        }
    }
};


class PackageButton : public Gtk::Button {

    Gtk::Box content;
    Gtk::Label name;


    Package package;

    public:
    PackageButton(Package package) {
        this->package = package;
        this->set_hexpand(true);
        this->set_margin(10);
        this->set_can_focus(true);   // Allows the button to receive focus
        this->set_sensitive(true);   // Ensures the button is sensitive (not disabled)

        

        content.set_hexpand(true);
        content.set_vexpand(true);
        content.set_orientation(Gtk::Orientation::HORIZONTAL);

        name.set_text(package.name);
        name.set_halign(Gtk::Align::START);
        content.append(name);

        this->set_child(content);
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
            this->package.install();
            this->render();
        });

        uninstall.signal_clicked().connect([this](){
            this->package.uninstall();
            this->render();
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

        install.set_label( package.is_installed() ? "Update" : "Install" );

        uninstall.set_label( "Uninstall" );


        controls_box.set_orientation(Gtk::Orientation::HORIZONTAL);
        controls_box.append(install);
        controls_box.append(uninstall);
        controls_box.set_halign(Gtk::Align::CENTER);

        append(name);
        append(controls_box);
    }

};