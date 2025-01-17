#include <string>
#include <vector>
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


class Package {
    public:
    string name;
    string description;

    /// @brief Dummy implementation for testing purposes
    /// @param s string pacman -Ss will use
    /// @return a list of found packages
    static vector<Package> get_packages(std::string search) {
        //if (search == "xo") {search = "x";} 

        char command[1024];
        sprintf(command, "pacman -Ss \"%s\"", search.c_str());
        //std::cout << "WILL ATTEMPT TO RUN: " << command << "\n";

        FILE *result;
        result = popen(command, "r");
        
        char buffer[1024];
        int line_number = 0;

        std::vector<std::string> names{};
        std::vector<std::string> descriptions{}; 
        while(fgets(buffer, sizeof(buffer), result)) {
            if (line_number % 2 == 0) {names.push_back(std::string(buffer));} else {descriptions.push_back(std::string(buffer));}
            line_number++;
        }

        vector<Package> packages{};
        for (int i = 0; i < names.size(); i++) {
            auto package = Package(names[i], descriptions[i]);
            packages.push_back(package);
        }

        pclose(result);
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
};

