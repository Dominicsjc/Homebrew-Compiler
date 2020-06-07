#include <string>
#include <iostream>
#include <fstream>

#include "code_generator.h"

void generate_code(LA::Program &targetP) {
    std::ofstream outputFile;
    outputFile.open("prog.a");

    LA::printProgram(outputFile, targetP);

    outputFile.close();
}