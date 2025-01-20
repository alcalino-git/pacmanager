
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sqlite3.h> 
#include <boost/algorithm/string.hpp> 

using namespace std;
using namespace boost::algorithm;

#define DATABASE_LOCATION "./package_database.sqlite"


/// Creates a database and its tables if it doesn't already exis
/// Database is always located at `DATABASE_LOCATION`
void prepare_database() {

}

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


/*
Event better idea: On launch, check for a file `cache.sqlite` and if it doesnt exist create it
Then throw the entire package repository in there
And allow the user to manually refetch it
*/

/*
Create a function `get_package_data` that takes in a package name and returns a HashMap<string, string> with all of its properties
as extracted from pacman -Si [package_name]
*/

enum Filter {
    EVERYTHING,
    INSTALLED,
    NOT_INSTALLED
};

class Package {
    
    private:
    unordered_map<string, string> properties;

    public:
    /// @brief Uses `pacman -Ss to search for packages`
    /// @param s string pacman -Ss will use
    /// @return a list of found packages
    static vector<Package> search_packages(std::string search, Filter f) {

        string command;
        command = "pacman -Ss \"" + search + "\"";

        std::cout << "SEARCHING WITH COMMAND: " << command << "\n";

        auto lines = get_command_line_output(command);

        std::cout << "GOT " << lines.size() << " packages\n";

        vector<Package> packages{};
        for (int i = 0; i < lines.size(); i++) {
            if (i % 2 != 0) {continue;}
            if (f == Filter::EVERYTHING || (lines[i].contains("installed") == (f == Filter::INSTALLED) )) {
                auto package = Package(lines[i]);
                packages.push_back(package);
            }
        }
        return packages;
    }

    static unordered_map<string, string> get_package_properties(string name) {
        unordered_map<string, string> result;

        auto lines = get_command_line_output("pacman -Si " + split_by_char(name, ' ')[0]);
        string last_key = "";
        string property_value = "";
        
        for (auto line: lines) {
            if (line.contains(":")) {
                auto parts = split_by_char(line, ':');
                trim(parts[0]);
                trim(parts[1]);
                last_key = (parts[0]);
                property_value = (parts[1]);

                result[last_key] += property_value;
            } else {
                trim(line);
                result[last_key] += line;
            }
        }

        return result;
    }


    Package() {
        this->properties["Name"] = "no-package";
        this->properties["Description"] = "no-description";
    }

    Package(string denominator) {
        this->properties["Name"] = Package::extract_name(denominator);
    }

    string get_property(string key) {
        if (!this->properties.count(key)) {
            this->refetch_data();
        } 
        return this->properties[key];
    }
    
    //TODO: This doesnt work. Needs to use `pacman -Si`
    void refetch_data() {
        this->properties = Package::get_package_properties( this->properties["Name"] );
    }

    /// @brief Extracts the raw name from the package (no repo or [installed])
    /// @param denominator a denominator string of shape `[database]/[package-name]`
    /// @return 
    static string extract_name(string denominator) {
        return  split_by_char(split_by_char(denominator, ' ')[0], '/')[1];
    }

    bool is_installed() {
        return system(("pacman -Q " + this->properties["Name"] + " > /dev/null 2>&1" ).c_str()) == 0;
    }

    void install() {
        auto output = get_command_line_output( "pkexec pacman --noconfirm  -Syy " + this->properties["Name"] );
        for (auto l: output) {
            std::cout << l << "\n";
        }
        this->refetch_data();
    }

    void uninstall() {
        auto output = get_command_line_output( "pkexec pacman --noconfirm  -R " + this->properties["Name"] );
        for (auto l: output) {
            std::cout << l << "\n";
        }
        this->refetch_data();
    }
};