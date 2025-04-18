#pragma once

#include <iostream>
#include <unordered_map>
#include <string>
#include <mutex>
#include <unordered_set>
#include <boost/algorithm/string.hpp>

#include "./logic.hpp"

using namespace std;
using namespace boost::algorithm;

class PackageDatabase {
    std::mutex mtx; //Database mutex because duh
    unordered_map<string, Package> packages;
    unordered_set<string> installed_packages;

    public:
    PackageDatabase() {
        populate_database();
        populate_installed_packages();
    }

    /**
     * Creates an in-memory representation of the entire pacman database
     * This includes properties of any given package
     *
     * It is guaranteed that every package in `this->packages` will be valid and will have all of its properties populated
     * @warning: This method is extremely expensive
     */
    void populate_database() {
        vector<string> database_lines = get_command_line_output("pacman -Si");
        database_lines.push_back("");

        for (auto temp_line: get_command_line_output("pacman -Qi")) {
            database_lines.push_back(temp_line);
        }

        auto current_package = new Package();
        string current_property = "";
        string current_value = "";

        for (string line: database_lines) {

            if (line.length() <= 1) {
                current_package->set_property(current_property, current_value);
                packages[current_package->get_property("Name")] = *current_package;
                current_package = new Package();
            }
            if (line.contains(":")) {
                current_package->set_property(current_property, current_value);
                auto properties = split_by_char(line, ':');
                trim(properties[0]);
                trim(properties[1]);
                current_property = properties[0];
                current_value = properties[1];
            } else {
                trim(line);
                current_value += line;
            }
        }
    }

    void populate_installed_packages() {
        auto installed = get_command_line_output("pacman -Ss");

        for (int i = 0; i < installed.size(); i+=2) {
            if (installed[i].contains("[installed]")) {
                this->installed_packages.insert(  split_by_char(split_by_char(installed[i], ' ')[0], '/')[1]  );
            }
        }
    }

    void update_package_state(Package p) {
        this->packages[p.get_property("Name")].refetch_data();
        bool installed = p.is_installed();
        if (installed) {
            this->installed_packages.insert(p.get_property("Name"));
        } else {
            this->installed_packages.erase(p.get_property("Name"));
        }
    }

    static void sort_by_installed_size(vector<Package>* packages) {
        std::sort(packages->begin(), packages->end(), [](Package a, Package b){
        	float a_size = 0;
         	float b_size = 0;

          	try {
	            a_size = std::stof(a.get_property("Installed Size"));
	            b_size = std::stof(b.get_property("Installed Size"));
           	} catch (...) {

            }

            if (a.get_property("Installed Size").contains("MiB")) {a_size*=1024;}
            if (b.get_property("Installed Size").contains("MiB")) {b_size*=1024;}

            return a_size > b_size;
        });
    }

    static void sort_by_installed_date(vector<Package>* packages) {
        std::sort(packages->begin(), packages->end(), [](Package a, Package b){
            auto a_date = string_to_date(a.get_property("Install Date"));
            auto b_date = string_to_date(b.get_property("Install Date"));

            return a_date > b_date;
        });
    }

    vector<Package> query_database(string query, Filter filter, Sorter sorter) {
        //this->mtx.lock();
        std::cout << "Processing query for \'" << query << "\'\n";
        auto queried_packages = get_command_line_output("pacman -Ss " + query);
        vector<Package> result;

        cout << "obtained queried packages command line output with " << queried_packages.size() << " packages\n";

        for (int i = 0; i < queried_packages.size(); i+=2) {
            auto name = split_by_char(split_by_char(queried_packages[i], ' ')[0], '/')[1];

            if (filter == Filter::INSTALLED && !this->installed_packages.contains(name)) {continue;}
            if (filter == Filter::NOT_INSTALLED && this->installed_packages.contains(name)) {continue;}


            result.push_back(this->packages[name]);
        }

        if (sorter == Sorter::INSTALLED_SIZE) {PackageDatabase::sort_by_installed_size(&result);}
        if (sorter == Sorter::INSTALLED_DATE) {PackageDatabase::sort_by_installed_date(&result);}

        std::cout << "Finished query for \'" << query << "\'\n";
        //this->mtx.unlock();
        return result;
    }
};
