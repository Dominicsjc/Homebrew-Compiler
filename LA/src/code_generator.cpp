#include <string>
#include <iostream>
#include <fstream>

#include "code_generator.h"

void generate_code(IR::Program &targetP) {
    std::ofstream outputFile;
    outputFile.open("prog.IR");

    IR::printProgram(outputFile, targetP);

    outputFile.close();
}