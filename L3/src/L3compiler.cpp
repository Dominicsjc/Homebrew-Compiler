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
#include "code_generator.h"
#include "globalization.h"
#include "analysis.h"

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
    L3::Program p = L3::parse_file(argv[optind]);;

    if (verbose) {

    }

    labelGlobalize(p);

    L2::Program targetP;
    targetP.entryPointLabel = ":main";
    bool mergeTree = true;

    for (auto f : p.functions) {
        auto newF = new L2::Function;
        newF->arguments = f->vars.size();
        newF->locals = 0;
        newF->name = f->name;
        analysis analyzer(f);
        analysis::argsAssign(f, newF);
        if (mergeTree)
            analyzer.mergeTrees();
        analyzer.tileTrees(newF);
        targetP.functions.push_back(newF);
    }
    /*
     * Generate the target code.
     */
    if (enable_code_generator) {
        generate_code(targetP);
    }

    L3::freeProgram(p);
    L2::freeProgram(targetP);

    return 0;
}

