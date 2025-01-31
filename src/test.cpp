#include "logic.hpp"
#include "database.hpp"



bool test_get_package_properties(bool logging) {
    auto result = Package::get_package_properties("extra/cuda 12.6.3-1");
    if (result["Name"] != "cuda") {
        std::cout << "Expected result[Name] to be cuda, got " << result["Name"] << "\n";
        return false;
    }
    if (logging) {
        //std::cout << "BEGIN LOGGING FOR GET_PACKAGE_PROPERTIES WITH INPUT " << "extra/cuda 12.6.3-1" << "\n";   
        for (auto i = result.begin(); i != result.end(); i++) {
            std::cout << i->first << " = " << i->second << "\n";
        }
    }
    
    return true;
}

int main() {

    auto result = Package::get_package_properties("extra/cuda 12.6.3-1");


    auto date = string_to_date("Tue 21 Jan 2025 03:51:59 PM CST");
    std::cout << date << "\n";

    if (!test_get_package_properties(true)) {
        return 1;
    }

    auto database = new PackageDatabase();
    auto packages = database->query_database("pacman", Filter::EVERYTHING, Sorter::NONE);
    if (packages.size() < 10) {
        cout << "didn't recieve enough packages\n";
        return 1;
    }
    cout << "\n\nQUERY FOR \'pacman\'\n";
    for (auto p: packages) {
        std::cout << p.get_property("Name") << "\n";
    }

    packages = database->query_database("pacman", Filter::INSTALLED, Sorter::NONE);
    cout << "\n\nQUERY FOR \'pacman\' (INSTALLED ONLY)\n";
    for (auto p: packages) {
        std::cout << p.get_property("Name") << "\n";
    }

    packages = database->query_database("pacman", Filter::NOT_INSTALLED, Sorter::NONE);
    cout << "\n\nQUERY FOR \'pacman\' (NOT INSTALLED ONLY)\n";
    for (auto p: packages) {
        std::cout << p.get_property("Name") << "\n";
    }

    packages = database->query_database("pacman", Filter::EVERYTHING, Sorter::INSTALLED_SIZE);
    cout << "\n\nQUERY FOR \'pacman\' (SORT BY INSTALLED_SIZE)\n";
    for (auto p: packages) {
        std::cout << p.get_property("Name") << "\n";
        std::cout << p.get_property("Installed Size") << "\n";
    }

    packages = database->query_database("pacman", Filter::EVERYTHING, Sorter::INSTALLED_DATE);
    cout << "\n\nQUERY FOR \'pacman\' (SORT BY INSTALLED_DATE)\n";
    for (auto p: packages) {
        std::cout << p.get_property("Name") << "\n";
        std::cout << "date: " << p.get_property("Install Date") << "\n";
    }

    packages = database->query_database("", Filter::EVERYTHING, Sorter::INSTALLED_DATE);
    cout << "\n\nQUERY FOR \'\' (SORT BY INSTALLED_DATE)\n";
    for (auto p: packages) {
        std::cout << p.get_property("Name") << "\n";
        std::cout << "date: " << p.get_property("Install Date") << "\n";
    }


    return 0;
}