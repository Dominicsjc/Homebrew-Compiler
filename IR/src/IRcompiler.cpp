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

#include "parser.h"
#include "analysis.h"
#include "code_generator.h"

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
    IR::Program p = IR::parse_file(argv[optind]);
    L3::Program targetP;


    if (verbose) {

    }

    for (auto f : p.functions) {
        auto targetF = new L3::Function;
        targetF->name = f->name;
        for (int i = 0; i < f->typedVars.size();) {
            targetF->vars.emplace_back(IR::copyIRItemToL3(f->typedVars[i+1]));
            f->typeOfVars.insert({static_cast<IR::Variable *>(f->typedVars[i+1])->name, static_cast<IR::T *>(f->typedVars[i])});
            i += 2;
        }

        analysis analyzer(f);
        analyzer.transferToL3Function(targetF, f);

        targetP.functions.emplace_back(targetF);
    }
    /*
     * Generate the target code.
     */
    if (enable_code_generator) {
        generate_code(targetP);
    }

    IR::freeProgram(p);
    L3::freeProgram(targetP);

    return 0;
}
