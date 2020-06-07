#include <string>
#include <iostream>
#include <fstream>

#include "code_generator.h"

void generate_code(L3::Program &targetP) {
    std::ofstream outputFile;
    outputFile.open("prog.L3");

    L3::printProgram(outputFile, targetP);

    outputFile.close();
}