#include <iostream>
#include <string>
#include <conio.h>
#include <vector>
#include <functional>
#include <array>
#include <map>

#include "headers/MouseLocker.hh"
#include "headers/InputChecker.hh"
#include "headers/ProcessChecker.hh"
#include "headers/ParamParser.hh"
#include "headers/UiCore.hh"

int main(int argc, char* argv[]) {

    UiCore uiCore;
    uiCore.Dialogue_Main();
    
    return 0;
}
