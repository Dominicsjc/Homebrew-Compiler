#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <iostream>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <cstdint>
#include <unistd.h>
#include <iostream>
#include <unordered_set>

#include "parser.h"
#include "analysis.h"
#include "code_generator.h"
#include "IR.h"
#include "utils.h"
#include "LA.h"
// #include "analysis.h"
// #include "code_generator.h"

using namespace std;

void print_help(char *progName) {
    std::cerr << "Usage: " << progName << " [-v] [-g 0|1] [-O 0|1|2] SOURCE" << std::endl;
}


int main(
        int argc,
        char **argv
) {
    auto enable_code_generator = true;
    int32_t optLevel = 3;

    /*
     * Check the compiler arguments.
     */
    //Utils::verbose = false;
    bool verbose = false;
    if (argc < 2) {
        print_help(argv[0]);
        return 1;
    }
    int32_t opt;
    while ((opt = getopt(argc, argv, "vg:O:sl:i")) != -1) {
        switch (opt) {

            case 'O':
                optLevel = strtoul(optarg, NULL, 0);
                break;

            case 'g':
                enable_code_generator = (strtoul(optarg, NULL, 0) == 0) ? false : true;
                break;

            case 'v':
                //Utils::verbose = true;
                verbose = true;
                break;

            default:
                print_help(argv[0]);
                return 1;
        }
    }

    /*
     * Parse the input file.
     */
    LA::Program p = LA::parse_file(argv[optind]);
    IR::Program targetP;

    if (verbose) {
        LA::printProgram(cout, p);
    }

    unordered_set<string> functionNames;
    for (auto f : p.functions) {
        functionNames.insert(f->name);
        //LA::parseTypeOfFunctionVars(f, arraySizes);
        //encodeAllConstants(f, functionNames, arraySizes);
    }
    for (auto f : p.functions) {
        LA::parseTypeOfFunctionVars(f);
        auto targetF = new IR::Function;
        targetF->name = f->name;
        targetF->res = transferLAItemToIR(f->res);
        for (auto i : f->typedVars) {
            targetF->typedVars.emplace_back(transferLAItemToIR(i));
        }
        encodeAllConstants(f, functionNames);
        auto declare_ins1 = generateInt64DeclareIns(generateNewIRVar(f->newVarId++));
        targetF->instructions.emplace_back(declare_ins1);
        auto declare_ins2 = generateInt64DeclareIns(generateNewIRVar(f->newVarId++));
        targetF->instructions.emplace_back(declare_ins2);
        for (auto ins : f->instructions) {
            transferLAInsToIR(ins, targetF, f->newVarId, f->newLabelId, f);
        }
        createBB(targetF, f->newLabelId);
        targetP.functions.emplace_back(targetF);
    }

    /*
     * Generate the target code.
     */
    if (enable_code_generator) {
        generate_code(targetP);
    }

    LA::freeProgram(p);
    IR::freeProgram(targetP);

    return 0;
}