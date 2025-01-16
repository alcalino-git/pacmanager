
#include <gtkmm.h>
#include <string>
#include <thread>
#include <vector>
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
vector<Package> get_packages(string s) {

    vector<Package> result{};

    srand( time(0) );

    for (int i = 0; i < rand() % 10000; i++) {
        result.push_back( Package(s, s) );
    }
    sleep(2);

    return result;
}


class SearchComponent : public Gtk::Box {

    public:
    Gtk::Entry textInput;
    Gtk::Label label;
    Gtk::ScrolledWindow scroll;
    Gtk::Box packages_components;

    vector<Package> packages;
    std::mutex mutex;

    SearchComponent() {
        set_orientation(Gtk::Orientation::VERTICAL);

        textInput = Gtk::Entry();
        textInput.signal_activate().connect([this]() {
            std::jthread([this]() {
                auto packages = get_packages(this->textInput.get_text());
                std::cout << "Query for " << this->textInput.get_text() << " recieved " << packages.size() << " packages\n"; 
                this->packages = packages;
                this->render();
            }).detach();

        });
        this->append(textInput);

        this->append(label);

        this->append(scroll);

        packages_components.set_orientation(Gtk::Orientation::VERTICAL);
        
    }

    void render() {
        label.set_text("Found " + std::to_string( packages.size() ) + " package(s)");

        /*
        IDEA:
        Protect `packages` with a mutex
        check if mutex is currently locked
        if so, do a Glib idle and try again
        otherwise, acquite the lock and render
        */
       for (auto c: packages_components.get_children()) {packages_components.remove(*c);};

       for (auto p: this->packages ) {
            auto label = Gtk::manage( new Gtk::Label(p.name) );
            packages_components.append(*label);
       }

       this->scroll.set_child(this->packages_components);


       
    }

};

