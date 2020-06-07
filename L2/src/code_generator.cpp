#include <string>
#include <iostream>
#include <fstream>

#include "code_generator.h"

using namespace std;

namespace L2 {

    void generate_code(Program &p) {
        std::ofstream outputFile;
        outputFile.open("prog.L1");

        targetProgram(outputFile, p);

        outputFile.close();
    }
}