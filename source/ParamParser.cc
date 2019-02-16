#include "headers/ParamParser.hh"

Switch ParamParser::get(const char* a_name) const {
    for(std::size_t i=0; i<this->parameters.size(); i++) {
        const Switch* current = &this->parameters.at(i);
        if(current->name == a_name) return *current;
    }

    Switch empty_switch;
    empty_switch.name = "NULL";
    empty_switch.value = "NULL";
    return empty_switch;
}

ParamParser::ParamParser(int argc, char* argv[]) {
    for(std::size_t i=1; i<argc; i++) {
        Switch parameter = Switch();
        std::string istr = argv[i];

        std::string name, value;
        bool value_mode;

        for(std::size_t i=0; i<istr.size(); i++) {
            char* ichar = &istr.at(i);
            
            if(*ichar == '=' && !value_mode) {
                value_mode = true;
                continue;
            } else if (value_mode) {
                value.push_back(*ichar);
            } else {
                name.push_back(*ichar);
            }
        }
        
        parameter.name = name;
        parameter.value = value;
        
        parameters.push_back(parameter); 
    }
}