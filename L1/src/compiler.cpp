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
#include <code_generator.h>

using namespace std;


void print_help(char *progName) {
    std::cerr << "Usage: " << progName << " [-v] [-g 0|1] [-O 0|1|2] SOURCE" << std::endl;
}

void freeInstruction(L1::Instruction *ins);

int main(
        int argc,
        char **argv
) {
    auto enable_code_generator = true;
    int32_t optLevel = 0;
    bool verbose = false;

    /*
     * Check the compiler arguments.
     */
    if (argc < 2) {
        print_help(argv[0]);
        return 1;
    }
    int32_t opt;
    while ((opt = getopt(argc, argv, "vg:O:")) != -1) {
        switch (opt) {
            case 'O':
                optLevel = strtoul(optarg, NULL, 0);
                break;

            case 'g':
                enable_code_generator = (strtoul(optarg, NULL, 0) == 0) ? false : true;
                break;

            case 'v':
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
    auto p = L1::parse_file(argv[optind]);

    /*
     * Print the source program.
     */
    if (verbose) {
        cout << "(" << p.entryPointLabel << endl;
        cout << endl;

        for (auto f : p.functions) {
            cout << "(" << f->name << endl;
            for (auto ins : f->instructions) {
                int id = ins->getTypeId();
                switch (id) {
                    case L1::RET: {
                        cout << "  return" << endl;
                        break;
                    }
                    case L1::ASS: {
                        auto ass_ins = static_cast<L1::Instruction_assignment *>(ins);
                        cout << "  ";
                        printItem(cout, ass_ins->dst);
                        cout << " <- ";
                        printItem(cout, ass_ins->src);
                        cout << endl;
                        break;
                    }
                    case L1::LOAD: {
                        auto load_ins = static_cast<L1::Instruction_load *>(ins);
                        cout << "  ";
                        printItem(cout, load_ins->dst);
                        cout << " <- mem ";
                        printItem(cout, load_ins->src);
                        cout << " ";
                        printItem(cout, load_ins->offset);
                        cout << endl;
                        break;
                    }
                    case L1::STORE: {
                        auto store_ins = static_cast<L1::Instruction_store *>(ins);
                        cout << "  mem ";
                        printItem(cout, store_ins->dst);
                        cout << " ";
                        printItem(cout, store_ins->offset);
                        cout << " <- ";
                        printItem(cout, store_ins->src);
                        cout << endl;
                        break;
                    }
                    case L1::AOP_ASS: {
                        auto aop_assign_ins = static_cast<L1::Instruction_aop_assignment *>(ins);
                        cout << "  ";
                        printItem(cout, aop_assign_ins->dst);
                        printAop(cout, aop_assign_ins->opId);
                        printItem(cout, aop_assign_ins->src);
                        cout << endl;
                        break;
                    }
                    case L1::SELFAOP: {
                        auto selfAop_ins = static_cast<L1::Instruction_selfAop *>(ins);
                        cout << "  ";
                        printSelfAop(cout, selfAop_ins->opId);
                        printItem(cout, selfAop_ins->reg);
                        cout << endl;
                        break;
                    }
                    case L1::AOP_LOAD: {
                        auto aop_load_ins = static_cast<L1::Instruction_aop_load *>(ins);
                        cout << "  ";
                        printItem(cout, aop_load_ins->dst);
                        printAop(cout, aop_load_ins->opId);
                        cout << "mem ";
                        printItem(cout, aop_load_ins->src);
                        cout << " ";
                        printItem(cout, aop_load_ins->offset);
                        cout << endl;
                        break;
                    }
                    case L1::AOP_STORE: {
                        auto aop_store_ins = static_cast<L1::Instruction_aop_store *>(ins);
                        cout << "  mem ";
                        printItem(cout, aop_store_ins->dst);
                        cout << " ";
                        printItem(cout, aop_store_ins->offset);
                        printAop(cout, aop_store_ins->opId);
                        printItem(cout, aop_store_ins->src);
                        cout << endl;
                        break;
                    }
                    case L1::SOP: {
                        auto sop_ins = static_cast<L1::Instruction_sop *>(ins);
                        cout << "  ";
                        printItem(cout, sop_ins->reg);
                        printSop(cout, sop_ins->opId);
                        printItem(cout, sop_ins->offset);
                        cout << endl;
                        break;
                    }
                    case L1::CMP: {
                        auto cmp_ins = static_cast<L1::Instruction_cmp *>(ins);
                        cout << "  ";
                        printItem(cout, cmp_ins->reg);
                        cout << " <- ";
                        printItem(cout, cmp_ins->l);
                        printCmp(cout, cmp_ins->cmpId);
                        printItem(cout, cmp_ins->r);
                        cout << endl;
                        break;
                    }
                    case L1::LABEL: {
                        auto label_ins = static_cast<L1::Instruction_label *>(ins);
                        cout << "  ";
                        printItem(cout, label_ins->label);
                        cout << endl;
                        break;
                    }
                    case L1::DJUMP: {
                        auto djump_ins = static_cast<L1::Instruction_djump *>(ins);
                        cout << "  ";
                        cout << "goto ";
                        printItem(cout, djump_ins->dst);
                        cout << endl;
                        break;
                    }
                    case L1::CJUMP: {
                        auto cjump_ins = static_cast<L1::Instruction_cjump *>(ins);
                        cout << "  ";
                        cout << "cjump ";
                        printItem(cout, cjump_ins->l);
                        printCmp(cout, cjump_ins->cmpId);
                        printItem(cout, cjump_ins->r);
                        cout << " ";
                        printItem(cout, cjump_ins->dst);
                        cout << endl;
                        break;
                    }
                    case L1::CISC: {
                        auto CISC_ins = static_cast<L1::Instruction_CISC *>(ins);
                        cout << "  ";
                        printItem(cout, CISC_ins->reg);
                        cout << " @ ";
                        printItem(cout, CISC_ins->base);
                        cout << " ";
                        printItem(cout, CISC_ins->index);
                        cout << " ";
                        printItem(cout, CISC_ins->scale);
                        cout << endl;
                        break;
                    }
                    case L1::CALL: {
                        auto call_ins = static_cast<L1::Instruction_call *>(ins);
                        cout << "  ";
                        cout << "call ";
                        if (call_ins->isRuntime) {
                            printRuntime(cout, call_ins->runtimeId);
                        } else {
                            printItem(cout, call_ins->u);
                        }
                        cout << " ";
                        printItem(cout, call_ins->num);
                        cout << endl;
                        break;
                    }
                    default:
                        cerr << "Cannot recognize the instruction!" << endl;
                }
            }
            cout << ")" << endl;
        }
        cout << endl;
        cout << ")" << endl;
    }

    /*
     * Generate x86_64 assembly.
     */
    if (enable_code_generator) {
        L1::generate_code(p);
    }

    for (auto f : p.functions) {
        for (auto ins : f->instructions) {
            freeInstruction(ins);
        }
        delete f;
    }

    return 0;
}

void freeInstruction(L1::Instruction *ins) {
    int id = ins->getTypeId();
    switch (id) {
        case L1::RET: {
            auto ret_ins = static_cast<L1::Instruction_ret *>(ins);
            delete ret_ins;
            break;
        }
        case L1::ASS: {
            auto ass_ins = static_cast<L1::Instruction_assignment *>(ins);
            delete ass_ins->src;
            delete ass_ins->dst;
            delete ass_ins;
            break;
        }
        case L1::LOAD: {
            auto load_ins = static_cast<L1::Instruction_load *>(ins);
            delete load_ins->src;
            delete load_ins->dst;
            delete load_ins->offset;
            delete load_ins;
            break;
        }
        case L1::STORE: {
            auto store_ins = static_cast<L1::Instruction_store *>(ins);
            delete store_ins->src;
            delete store_ins->dst;
            delete store_ins->offset;
            delete store_ins;
            break;
        }
        case L1::AOP_ASS: {
            auto aop_assign_ins = static_cast<L1::Instruction_aop_assignment *>(ins);
            delete aop_assign_ins->src;
            delete aop_assign_ins->dst;
            delete aop_assign_ins;
            break;
        }
        case L1::SELFAOP: {
            auto selfAop_ins = static_cast<L1::Instruction_selfAop *>(ins);
            delete selfAop_ins->reg;
            delete selfAop_ins;
            break;
        }
        case L1::AOP_LOAD: {
            auto aop_load_ins = static_cast<L1::Instruction_aop_load *>(ins);
            delete aop_load_ins->src;
            delete aop_load_ins->dst;
            delete aop_load_ins->offset;
            delete aop_load_ins;
            break;
        }
        case L1::AOP_STORE: {
            auto aop_store_ins = static_cast<L1::Instruction_aop_store *>(ins);
            delete aop_store_ins->src;
            delete aop_store_ins->dst;
            delete aop_store_ins->offset;
            delete aop_store_ins;
            break;
        }
        case L1::SOP: {
            auto sop_ins = static_cast<L1::Instruction_sop *>(ins);
            delete sop_ins->reg;
            delete sop_ins->offset;
            delete sop_ins;
            break;
        }
        case L1::CMP: {
            auto cmp_ins = static_cast<L1::Instruction_cmp *>(ins);
            delete cmp_ins->reg;
            delete cmp_ins->l;
            delete cmp_ins->r;
            delete cmp_ins;
            break;
        }
        case L1::LABEL: {
            auto label_ins = static_cast<L1::Instruction_label *>(ins);
            delete label_ins->label;
            delete label_ins;
            break;
        }
        case L1::DJUMP: {
            auto djump_ins = static_cast<L1::Instruction_djump *>(ins);
            delete djump_ins->dst;
            delete djump_ins;
            break;
        }
        case L1::CJUMP: {
            auto cjump_ins = static_cast<L1::Instruction_cjump *>(ins);
            delete cjump_ins->dst;
            delete cjump_ins->l;
            delete cjump_ins->r;
            delete cjump_ins;
            break;
        }
        case L1::CISC: {
            auto CISC_ins = static_cast<L1::Instruction_CISC *>(ins);
            delete CISC_ins->reg;
            delete CISC_ins->base;
            delete CISC_ins->index;
            delete CISC_ins->scale;
            delete CISC_ins;
            break;
        }
        case L1::CALL: {
            auto call_ins = static_cast<L1::Instruction_call *>(ins);
            delete call_ins->u;
            delete call_ins->num;
            delete call_ins;
            break;
        }
        default:;
    }
}
