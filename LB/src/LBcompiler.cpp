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
#include "scopeTranslator.h"
#include "whileAnalyzer.h"
#include "code_generator.h"
#include "utils.h"
#include "LB.h"
#include "LA.h"
#include "transformation.h"

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
    LB::Program p = LB::parse_file(argv[optind]);
    LA::Program targetP;

    if (verbose) {

    }

    for (auto f : p.functions) {
        translateMultiDeclare(f->root);
        flatScopes(f);
        whileAnalyzer analyzer(f);
        auto targetF = new LA::Function;
        transferLBFunctionToLA(f, targetF);
        targetP.functions.emplace_back(targetF);
    }
    /*
     * Generate the target code.
     */
    if (enable_code_generator) {
        generate_code(targetP);
    }

    LB::freeProgram(p);
    LA::freeProgram(targetP);

    return 0;
}