#include <string>
#include <iostream>
#include <fstream>

#include "code_generator.h"

using namespace std;

namespace L1 {
    static std::string tab = "   ";

    void generate_code(Program p) {

        /*
         * Open the output file.
         */
        std::ofstream outputFile;
        outputFile.open("prog.S");

        /*
         * Generate target code
         */
        //TODO

        // .text
        outputFile << tab;
        outputFile << ".text" << endl;

        // .globl entryPointLabel
        outputFile << tab;
        outputFile << ".globl " << "go" << endl;
        outputFile << endl;

        // entryPointLabel:
        outputFile << "go" << ":";
        outputFile << endl;

        // save callee-saved registers
        outputFile << tab << "# save callee-saved registers" << endl;
        outputFile << tab << "pushq %rbx" << endl;
        outputFile << tab << "pushq %rbp" << endl;
        outputFile << tab << "pushq %r12" << endl;
        outputFile << tab << "pushq %r13" << endl;
        outputFile << tab << "pushq %r14" << endl;
        outputFile << tab << "pushq %r15" << endl;
        outputFile << endl;

        // call first function
        outputFile << tab << "call _" << p.entryPointLabel.substr(1) << endl;
        outputFile << endl;

        // restore callee-saved registers and return
        outputFile << tab << "# restore callee-saved registers and return" << endl;
        outputFile << tab << "popq %r15" << endl;
        outputFile << tab << "popq %r14" << endl;
        outputFile << tab << "popq %r13" << endl;
        outputFile << tab << "popq %r12" << endl;
        outputFile << tab << "popq %rbp" << endl;
        outputFile << tab << "popq %rbx" << endl;
        outputFile << tab << "retq" << endl;
        outputFile << endl;

        for (auto f: p.functions) {
            outputFile << endl;
            assemblyFunction(outputFile, f);
            outputFile << endl;
        }
    
        /*
         * Close the output file.
         */
        outputFile.close();
    }
}
