#include "logic.hpp"



bool test_get_package_properties(bool logging) {
    auto result = Package::get_package_properties("extra/cuda 12.6.3-1");
    if (result["Name"] != "cuda") {
        //std::cout << "Expected result[Name] to be cuda, got " << result["Name"] << "\n";
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

    if (!test_get_package_properties(true)) {
        return 1;
    }

    auto packages = Package::search_packages("pacman");
    std::cout << "Recievec " << packages.size() << " packages\n";
    if (packages.size() < 10) {
        return 1;
    }

    return 0;
}