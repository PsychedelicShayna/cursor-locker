#ifndef PARAMPARSER
#define PARAMPARSER

#include <vector>
#include <string>

/**
 * Struct that represents a parameter switch. The name of the switch, and the value can be found here.
 * If there is no value, the value (should) be nullptr.
 */

struct Switch {
    std::string name;
    std::string value;
};


/**
 * Class to make handling command line parameters simpler. It uses the above Switch struct to store each
 * parsed parameter, in a Switch vector.
 */

class ParamParser {
protected:
    std::vector<Switch> parameters; // This is every parameter parsed as a Switch vector.

public:
    // Method that finds and returns a switch in the parameters vector with the matching name
    Switch get(const char*) const;

    // Constructor is supposed to take in command line arguments from the entrypoint
    ParamParser(int argc, char* argv[]);
};

#endif