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

#include <parser.h>
#include <analysis.h>
//#include <transformer.h>
#include <code_generator.h>
#include <spiller.h>
//#include <register_allocation.h>
//#include <utils.h>

using namespace std;

void print_help(char *progName) {
    std::cerr << "Usage: " << progName << " [-v] [-g 0|1] [-O 0|1|2] [-s] [-l 1|2] [-i] SOURCE" << std::endl;
}



int main(
        int argc,
        char **argv
) {
    auto enable_code_generator = true;
    auto spill_only = false;
    auto interference_only = false;
    int32_t liveness_only = 0;
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

            case 'l':
                liveness_only = strtoul(optarg, NULL, 0);
                break;

            case 'i':
                interference_only = true;
                break;

            case 's':
                spill_only = true;
                break;

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
    L2::Program p;
    if (spill_only) {

        /*
         * Parse an L2 function and the spiller arguments.
         */
        p = L2::parse_spill_file(argv[optind]);

    } else if (liveness_only) {

        /*
         * Parse an L2 function.
         */
        p = L2::parse_function_file(argv[optind]);

    } else if (interference_only) {

        /*
         * Parse an L2 function.
         */
        p = L2::parse_function_file(argv[optind]);

    } else {

        /*
         * Parse the L2 program.
         */
        p = L2::parse_file(argv[optind]);

        /*
         * Print the source program.
         */
        if (verbose) {
            printL2Program(cout, p);
        }
    }

    /*
     * Special cases.
     */
    if (spill_only) {
        //naiveSpiller(cout, p.functions[0]);
        spill(p.functions[0]);
        L2::printL2Function(cout, p.functions[0]);
        freeProgram(p);
        return 0;
    }

    /*
     * Liveness test.
     */
    if (liveness_only) {
        Analysis analysor(p.functions[0]);
        analysor.printInOut(cout);
        freeProgram(p);
        return 0;
    }

    /*
     * Interference graph test.
     */
    if (interference_only) {
        Analysis analysor(p.functions[0]);
        analysor.computeInterferenceGraph();
        analysor.printInterferenceGraph(cout);
        freeProgram(p);
        return 0;
    }

    for (auto f : p.functions) {
        bool allocated = false;
        int prefixIndex = 0;
        do
        {
            Analysis analyzer(f);
            analyzer.computeInterferenceGraph();

            /*
             * DEBUG
             */
            //analyzer.printInterferenceGraph(cout);
            //cout << endl;

            allocated = analyzer.allocator(f);
            if (!allocated) {
                analyzer.batchSpill(f, prefixIndex);
            } else {
                analyzer.replaceAllWithColor(f);
            }
        } while(!allocated);
    }
    /*
     * Generate the target code.
     */
    if (enable_code_generator) {
        L2::generate_code(p);
    }

    freeProgram(p);

    return 0;
}


