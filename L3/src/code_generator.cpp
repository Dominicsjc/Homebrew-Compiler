#include <string>
#include <iostream>
#include <fstream>

#include "code_generator.h"

void generate_code(L2::Program &targetP) {
    std::ofstream outputFile;
    outputFile.open("prog.L2");

    L2::printL2Program(outputFile, targetP);

    outputFile.close();
}