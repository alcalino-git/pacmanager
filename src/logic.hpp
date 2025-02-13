
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cmath>

#include <sqlite3.h>
#include <boost/algorithm/string.hpp>



using namespace std;
using namespace boost::algorithm;

time_t string_to_date(string date_str) {
    std::tm tm = {};
    std::istringstream ss(date_str);

    // Parsing format: "%a %d %b %Y %I:%M:%S %p %Z"
    ss >> std::get_time(&tm, "%a %d %b %Y %I:%M:%S %p");

    if (ss.fail()) {
        return 1;
    }

    // Convert to time_t (seconds since epoch)
    time_t time_obj = std::mktime(&tm);

    return time_obj;
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
    if (buffer.length() != 0 || result.size() == 0) {result.push_back(buffer);}

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

enum Sorter {
    INSTALLED_SIZE,
    INSTALLED_DATE,
    NONE
};

class Package {

    private:
    unordered_map<string, string> properties;

    public:
    // /// @brief Uses `pacman -Ss to search for packages`
    // /// @param s string pacman -Ss will use
    // /// @return a list of found packages
    // static vector<Package> search_packages(std::string search, Filter f, Sorter sort) {

    //     string command;
    //     command = "pacman -Ss \"" + search + "\"";

    //     std::cout << "SEARCHING WITH COMMAND: " << command << "\n";

    //     auto lines = get_command_line_output(command);

    //     std::cout << "GOT " << lines.size() << " packages\n";

    //     vector<Package> packages{};
    //     for (int i = 0; i < lines.size(); i++) {
    //         if (i % 2 != 0) {continue;}
    //         if (f == Filter::EVERYTHING || (lines[i].contains("installed") == (f == Filter::INSTALLED) )) {
    //             auto package = Package(lines[i]);
    //             packages.push_back(package);
    //         }
    //     }

    //     if (sort == Sorter::NONE) {return packages;}

    //     //Preemptively fetch the required properties in parallaler for better performance
    //     vector<jthread> threads;
    //     int batch_size = std::ceil((float)packages.size() / (float)std::thread::hardware_concurrency()) ;
    //     for (int i = 0; i < std::thread::hardware_concurrency(); i++) {
    //         threads.emplace_back([&packages, i, batch_size]() {

    //             int start = batch_size * i;
    //             int end = (batch_size) * (i+1);
    //             std::cout << start <<  "..." << end << "\n";
    //             for (int k = start; k < end; k++) {
    //                 if (k >= packages.size()) {break;}
    //                 packages[k].get_property("Installed Size");
    //                 packages[k].get_property("Install Date");
    //             }
    //         });

    //     }

    //     if (sort == Sorter::INSTALLED_SIZE) {
    //         std::sort(packages.begin(), packages.end(), [](auto a, auto b) {
    //             string a_size = a.get_property("Installed Size");;
    //             string b_size = b.get_property("Installed Size");;

    //             float a_float = std::stof(a_size);
    //             float b_float = std::stof(b_size);
    //             if (split_by_char(a_size, ' ')[1] == "MiB") {a_float *= 1024;}
    //             if (split_by_char(b_size, ' ')[1] == "MiB") {b_float *= 1024;}

    //             return a_float > b_float;
    //         });
    //     }

    //     if (sort == Sorter::INSTALLED_DATE) {
    //         std::sort(packages.begin(), packages.end(), [](auto a, auto b) {
    //             string a_date_str = a.get_property("Install Date");
    //             string b_date_str = b.get_property("Install Date");

    //             if (a_date_str == "") {return false;}
    //             if (b_date_str == "") {return true;}

    //             return string_to_date(a_date_str) > string_to_date(b_date_str);
    //         });
    //     }

    //     return packages;
    // }

    static unordered_map<string, string> get_package_properties(string name) {
        unordered_map<string, string> result;

        bool is_installed = system(("pacman -Q " + split_by_char(name, ' ')[0] + " > /dev/null 2>&1" ).c_str()) == 0;

        string command;
        if (is_installed) {command = "pacman -Qi " + split_by_char(name, ' ')[0];}
        if (!is_installed) {command = "pacman -Si " + split_by_char(name, ' ')[0];}

        auto lines = get_command_line_output(command);
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

    void static system_update() {
        system("pkexec pacman -Syu --noconfirm");
    }


    Package() {
        this->properties["Name"] = "no-package";
        this->properties["Description"] = "no-description";
    }

    Package(string denominator) {
        this->properties["Name"] = Package::extract_name(denominator);
    }

    void set_property(string key, string value) {
        this->properties[key] = value;
    }


    /**
     * Gets a property of the package
     * @param key The property to fetch
     * @returns A string with the selected property, or "" if it doesn't exist
     */
    string get_property(string key) {
        // if (!this->properties.count(key)) {
        //     this->refetch_data();
        // }
        // if (!this->properties.count(key)) {
        //     this->properties[key] = "";
        // }
        return this->properties[key];
    }


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

    /**
     * @deprecated Prefer using `package_operation` on the UI component directly
     */
    int install() {
        auto output = system( ("pkexec pacman --noconfirm  -Syy " + this->properties["Name"] ).c_str());
        this->refetch_data();
        return output;
    }

    /**
     * @deprecated Prefer using `package_operation` on the UI component directly
     */
    int uninstall() {
        auto output = system( ("pkexec pacman --noconfirm  -R " + this->properties["Name"]).c_str() );
        this->refetch_data();
        return output;
    }
};
