
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <boost/algorithm/string.hpp> 

using namespace std;
using namespace boost::algorithm;

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

class Package {
    public:
    string name;
    string description;

    /// @brief Uses `pacman -Ss to search for packages`
    /// @param s string pacman -Ss will use
    /// @return a list of found packages
    static vector<Package> search_packages(std::string search) {

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

    static unordered_map<string, string> get_package_properties(string denominator) {
        unordered_map<string, string> result;

        auto lines = get_command_line_output("pacman -Si " + split_by_char(denominator, ' ')[0]);
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
        this->name = "";
        this->description = "";
    }

    Package(string name, string description) {
        this->name = name;
        this->description = description;
    }
    
    //TODO: This doesnt work. Needs to use `pacman -Si`
    void refetch_data() {
        auto lines = get_command_line_output("pacman -Ss \"" + this->extract_name() + "\"");
        this->name = lines[0];
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