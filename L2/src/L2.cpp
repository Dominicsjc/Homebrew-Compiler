#include <iostream>
#include "L2.h"

using namespace std;

namespace L2 {
    static const std::string tab = "   ";

    void printItem(ostream &out, Item *i) {
        if (i->isConstant()) {
            auto n = static_cast<Constant *>(i);
            out << n->num;
            return;
        } else {
            if (i->type == LAB)
                out << i->name;
            else if (i->type == REG)
                out << register_name[i->r];
            else
                out << "%" << i->name;
        }
    }

    void printSpilledItem(std::ostream &out, Item *i, Item *targetVar, Item *spillerVar, int index) {
        if (i->isConstant()) {
            auto n = static_cast<Constant *>(i);
            out << n->num;
            return;
        } else {
            if (i->type == LAB)
                out << i->name;
            else if (i->type == REG)
                out << register_name[i->r];
            else {
                if (i->name == targetVar->name) {
                    out << "%" << spillerVar->name << index;
                }
                else
                    out << "%" << i->name;
            }
        }
    }

    void assemblyItem(ostream &out, Item *i) {
        if (i->isConstant()) {
            auto n = static_cast<Constant *>(i);
            out << "$" << n->num;
            return;
        } else {
            if (i->type == LAB)
                out << "_" << i->name.substr(1);
            else
                out << "%" << register_name[i->r];
        }
    }

    void assembly8bItem(ostream &out, Item *i) {
        if (i->type == REG)
            out << "%" << register8b_name[i->r];
        else
            assemblyItem(out, i);
    }

    void assemblyInstruction(ostream &out, Instruction *ins, Function* f) {
        int type = ins->getTypeId();
        switch (type) {
            case RET: {
                assemblyReturn(out, f);

                break;
            }
            case ASS: {
                auto ass_ins = static_cast<Instruction_assignment *>(ins);

                out << "movq ";
                if (ass_ins->src->type == LAB)
                    out << "$";
                assemblyItem(out, ass_ins->src);
                out << ", ";
                assemblyItem(out, ass_ins->dst);

                break;
            }
            case LOAD: {
                auto load_ins = static_cast<Instruction_load *>(ins);

                out << "movq ";
                printItem(out, load_ins->offset);
                out << "(";
                assemblyItem(out, load_ins->src);
                out << ")";
                out << ", ";
                assemblyItem(out, load_ins->dst);

                break;
            }
            case STORE: {
                auto store_ins = static_cast<Instruction_store *>(ins);

                out << "movq ";
                if (store_ins->src->type == LAB)
                    out << "$";
                assemblyItem(out, store_ins->src);
                out << ", ";
                printItem(out, store_ins->offset);
                out << "(";
                assemblyItem(out, store_ins->dst);
                out << ")";

                break;
            }
            case AOP_ASS: {
                auto aop_assign_ins = static_cast<Instruction_aop_assignment *>(ins);

                assemblyAop(out, aop_assign_ins->opId);
                assemblyItem(out, aop_assign_ins->src);
                out << ", ";
                assemblyItem(out, aop_assign_ins->dst);

                break;
            }
            case SELFAOP: {
                auto selfAop_ins = static_cast<Instruction_selfAop *>(ins);

                assemblySelfAop(out, selfAop_ins->opId);
                assemblyItem(out, selfAop_ins->reg);

                break;
            }
            case AOP_LOAD: {
                auto aop_load_ins = static_cast<Instruction_aop_load *>(ins);

                assemblyAop(out, aop_load_ins->opId);
                printItem(out, aop_load_ins->offset);
                out << "(";
                assemblyItem(out, aop_load_ins->src);
                out << ")";
                out << ", ";
                assemblyItem(out, aop_load_ins->dst);

                break;
            }
            case AOP_STORE: {
                auto aop_store_ins = static_cast<Instruction_aop_store *>(ins);

                assemblyAop(out, aop_store_ins->opId);
                assemblyItem(out, aop_store_ins->src);
                out << ", ";
                printItem(out, aop_store_ins->offset);
                out << "(";
                assemblyItem(out, aop_store_ins->dst);
                out << ")";

                break;
            }
            case SOP: {
                auto sop_ins = static_cast<Instruction_sop *>(ins);

                assemblySop(out, sop_ins->opId);
                assembly8bItem(out, sop_ins->offset);
                out << ", ";
                assemblyItem(out, sop_ins->reg);

                break;
            }
            case CMP: {
                auto cmp_ins = static_cast<Instruction_cmp *>(ins);

                if (cmp_ins->l->isConstant() && cmp_ins->r->isConstant()) {
                    bool res = simpleCompute(static_cast<Constant *>(cmp_ins->l), static_cast<Constant *>(cmp_ins->r),
                                             cmp_ins->cmpId);
                    out << "movq $" << res << ", ";
                    assemblyItem(out, cmp_ins->reg);
                } else {
                    bool reverse = false;

                    out << "cmpq ";
                    if (cmp_ins->l->isConstant() && cmp_ins->r->type == REG) {
                        assemblyItem(out, cmp_ins->l);
                        out << ", ";
                        assemblyItem(out, cmp_ins->r);
                        reverse = true;
                    } else {
                        assemblyItem(out, cmp_ins->r);
                        out << ", ";
                        assemblyItem(out, cmp_ins->l);
                    }
                    out << endl;

                    out << tab;
                    assemblyCmp(out, cmp_ins->cmpId, reverse);
                    assembly8bItem(out, cmp_ins->reg);
                    out << endl;

                    out << tab << "movzbq ";
                    assembly8bItem(out, cmp_ins->reg);
                    out << ", ";
                    assemblyItem(out, cmp_ins->reg);
                }

                break;
            }
            case LABEL: {
                auto label_ins = static_cast<Instruction_label*>(ins);

                assemblyItem(out, label_ins->label);
                out << ":";

                break;
            }
            case DJUMP: {
                auto djump_ins = static_cast<Instruction_djump*>(ins);

                out << "jmp ";
                assemblyItem(out, djump_ins->dst);

                break;
            }
            case CJUMP: {
                auto cjump_ins = static_cast<Instruction_cjump*>(ins);

                if (cjump_ins->l->isConstant() && cjump_ins->r->isConstant()) {
                    bool res = simpleCompute(static_cast<Constant *>(cjump_ins->l), static_cast<Constant *>(cjump_ins->r), cjump_ins->cmpId);
                    if (res) {
                        out << "jmp ";
                        assemblyItem(out, cjump_ins->dst);
                    }
                } else {
                    bool reverse = false;

                    out << "cmpq ";
                    if (cjump_ins->l->isConstant() && cjump_ins->r->type == REG) {
                        assemblyItem(out, cjump_ins->l);
                        out << ", ";
                        assemblyItem(out, cjump_ins->r);
                        reverse = true;
                    } else {
                        assemblyItem(out, cjump_ins->r);
                        out << ", ";
                        assemblyItem(out, cjump_ins->l);
                    }
                    out << endl;

                    out << tab;
                    assemblyCjump(out, cjump_ins->cmpId, reverse);
                    assemblyItem(out, cjump_ins->dst);
                }

                break;
            }
            case CISC: {
                auto cisc_ins = static_cast<Instruction_CISC*>(ins);

                out << "lea (";
                assemblyItem(out, cisc_ins->base);
                out << ", ";
                assemblyItem(out, cisc_ins->index);
                out << ", ";
                printItem(out, cisc_ins->scale);
                out << "), ";
                assemblyItem(out, cisc_ins->reg);

                break;
            }
            case CALL: {
                auto call_ins = static_cast<Instruction_call*>(ins);

                if (call_ins->isRuntime) {
                    out << "call ";
                    assemblyRuntime(out, call_ins->runtimeId);
                    out << "  # runtime system call";
                } else {
                    auto argumentCount = static_cast<Constant*>(call_ins->num);
                    out << "subq $" << max(0ll, argumentCount->num - 6) * 8 + 8 << ", %rsp      # Allocate extra arguments on the stack as well as the return address " << endl;

                    out << tab;
                    out << "jmp ";
                    if (call_ins->u->type == REG) {
                        out << "*";
                        assemblyItem(out, call_ins->u);
                        out << "        # Indirect call";
                    } else {
                        assemblyItem(out, call_ins->u);
                        out << "        # Call to ";
                        printItem(out, call_ins->u);
                    }
                }
            }
            default:;
        }
    }

    void assemblyFunction(ostream &out, Function *f) {
        out << "_" << f->name.substr(1) << ":" << endl;


        for (auto ins: f->instructions) {
            if (ins->getTypeId() != LABEL) {
                out << tab;
            }
            assemblyInstruction(out, ins, f);
            out << endl;
        }
    }

    void printAop(std::ostream &out, Aop opId) {
        switch (opId) {
            case ADD: {
                out << " += ";
                break;
            }
            case SUB: {
                out << " -= ";
                break;
            }
            case MULTI: {
                out << " *= ";
                break;
            }
            case AND: {
                out << " &= ";
                break;
            }
            default:;
        }
    }

    void assemblyAop(std::ostream &out, Aop opId) {
        switch (opId) {
            case ADD: {
                out << "addq ";
                break;
            }
            case SUB: {
                out << "subq ";
                break;
            }
            case MULTI: {
                out << "imulq ";
                break;
            }
            case AND: {
                out << "andq ";
                break;
            }
            default:;
        }
    }

    void printSelfAop(std::ostream &out, Aop opId) {
        switch (opId) {
            case ADD: {
                out << "++";
                break;
            }
            case SUB: {
                out << "--";
                break;
            }
            default:;
        }
    }

    void assemblySelfAop(std::ostream &out, Aop opId) {
        switch (opId) {
            case ADD: {
                out << "inc ";
                break;
            }
            case SUB: {
                out << "dec ";
                break;
            }
            default:;
        }
    }

    void printSop(std::ostream &out, Sop opId) {
        switch (opId) {
            case LEFT: {
                out << " <<= ";
                break;
            }
            case RIGHT: {
                out << " >>= ";
                break;
            }
            default:;
        }
    }

    void assemblySop(std::ostream &out, Sop opId) {
        switch (opId) {
            case LEFT: {
                out << "salq ";
                break;
            }
            case RIGHT: {
                out << "sarq ";
                break;
            }
            default:;
        }
    }

    void printCmp(std::ostream &out, Cmp cmpId) {
        switch (cmpId) {
            case LE: {
                out << " < ";
                break;
            }
            case LEQ: {
                out << " <= ";
                break;
            }
            case EQL: {
                out << " = ";
                break;
            }
            default:;
        }
    }

    void assemblyCmp(std::ostream &out, Cmp cmpId, bool reverse) {
        switch (cmpId) {
            case LE: {
                if (reverse)
                    out << "setg ";
                else
                    out << "setl ";
                break;
            }
            case LEQ: {
                if (reverse)
                    out << "setge ";
                else
                    out << "setle ";
                break;
            }
            case EQL: {
                out << "sete ";
                break;
            }
            default:;
        }
    }

    void assemblyCjump(std::ostream &out, Cmp cmpId, bool reverse) {
        switch (cmpId) {
            case LE: {
                if (reverse)
                    out << "jg ";
                else
                    out << "jl ";
                break;
            }
            case LEQ: {
                if (reverse)
                    out << "jge ";
                else
                    out << "jle ";
                break;
            }
            case EQL: {
                out << "je ";
                break;
            }
            default:;
        }
    }

    void assemblyPrologue(std::ostream &out, Function* f) {

    }

    void assemblyReturn(std::ostream &out, Function* f) {

    }

    void printRuntime(std::ostream &out, RuntimeType runtimeId) {
        switch (runtimeId) {
            case PRINT: {
                out << "print";
                break;
            }
            case ALLOCATE: {
                out << "allocate";
                break;
            }
            case ARRAY: {
                out << "array-error";
                break;
            }
            default:;
        }
    }

    void assemblyRuntime(std::ostream &out, RuntimeType runtimeId) {
        switch (runtimeId) {
            case PRINT: {
                out << "print";
                break;
            }
            case ALLOCATE: {
                out << "allocate";
                break;
            }
            case ARRAY: {
                out << "array_error";
                break;
            }
            default:;
        }
    }

    bool simpleCompute(Constant *l, Constant *r, Cmp cmpId) {
        switch (cmpId) {
            case LE: {
                return l->num < r->num;
            }
            case LEQ: {
                return l->num <= r->num;
            }
            case EQL: {
                return l->num == r->num;
            }
            default:
                return true;
        }
    }

    void printL2Instruction(std::ostream& out, Instruction *ins){
        int id = ins->getTypeId();
        switch (id) {
            case RET: {
                out << "\treturn" << endl;
                break;
            }
            case ASS: {
                auto ass_ins = static_cast<Instruction_assignment *>(ins);
                out << "\t";
                printItem(out, ass_ins->dst);
                out << " <- ";
                printItem(out, ass_ins->src);
                out << endl;
                break;
            }
            case LOAD: {
                auto load_ins = static_cast<Instruction_load *>(ins);
                out << "\t";
                printItem(out, load_ins->dst);
                out << " <- mem ";
                printItem(out, load_ins->src);
                out << " ";
                printItem(out, load_ins->offset);
                out << endl;
                break;
            }
            case STORE: {
                auto store_ins = static_cast<Instruction_store *>(ins);
                out << "\tmem ";
                printItem(out, store_ins->dst);
                out << " ";
                printItem(out, store_ins->offset);
                out << " <- ";
                printItem(out, store_ins->src);
                out << endl;
                break;
            }
            case AOP_ASS: {
                auto aop_assign_ins = static_cast<Instruction_aop_assignment *>(ins);
                out << "\t";
                printItem(out, aop_assign_ins->dst);
                printAop(out, aop_assign_ins->opId);
                printItem(out, aop_assign_ins->src);
                out << endl;
                break;
            }
            case SELFAOP: {
                auto selfAop_ins = static_cast<Instruction_selfAop *>(ins);
                out << "\t";
                printItem(out, selfAop_ins->reg);
                printSelfAop(out, selfAop_ins->opId);
                out << endl;
                break;
            }
            case AOP_LOAD: {
                auto aop_load_ins = static_cast<Instruction_aop_load *>(ins);
                out << "\t";
                printItem(out, aop_load_ins->dst);
                printAop(out, aop_load_ins->opId);
                out << "mem ";
                printItem(out, aop_load_ins->src);
                out << " ";
                printItem(out, aop_load_ins->offset);
                out << endl;
                break;
            }
            case AOP_STORE: {
                auto aop_store_ins = static_cast<Instruction_aop_store *>(ins);
                out << "\tmem ";
                printItem(out, aop_store_ins->dst);
                out << " ";
                printItem(out, aop_store_ins->offset);
                printAop(out, aop_store_ins->opId);
                printItem(out, aop_store_ins->src);
                out << endl;
                break;
            }
            case SOP: {
                auto sop_ins = static_cast<Instruction_sop *>(ins);
                out << "\t";
                printItem(out, sop_ins->reg);
                printSop(out, sop_ins->opId);
                printItem(out, sop_ins->offset);
                out << endl;
                break;
            }
            case CMP: {
                auto cmp_ins = static_cast<Instruction_cmp *>(ins);
                out << "\t";
                printItem(out, cmp_ins->reg);
                out << " <- ";
                printItem(out, cmp_ins->l);
                printCmp(out, cmp_ins->cmpId);
                printItem(out, cmp_ins->r);
                out << endl;
                break;
            }
            case LABEL: {
                auto label_ins = static_cast<Instruction_label *>(ins);
                out << "\t";
                printItem(out, label_ins->label);
                out << endl;
                break;
            }
            case DJUMP: {
                auto djump_ins = static_cast<Instruction_djump *>(ins);
                out << "\t";
                out << "goto ";
                printItem(out, djump_ins->dst);
                out << endl;
                break;
            }
            case CJUMP: {
                auto cjump_ins = static_cast<Instruction_cjump *>(ins);
                out << "\t";
                out << "cjump ";
                printItem(out, cjump_ins->l);
                printCmp(out, cjump_ins->cmpId);
                printItem(out, cjump_ins->r);
                out << " ";
                printItem(out, cjump_ins->dst);
                out << endl;
                break;
            }
            case CISC: {
                auto CISC_ins = static_cast<Instruction_CISC *>(ins);
                out << "\t";
                printItem(out, CISC_ins->reg);
                out << " @ ";
                printItem(out, CISC_ins->base);
                out << " ";
                printItem(out, CISC_ins->index);
                out << " ";
                printItem(out, CISC_ins->scale);
                out << endl;
                break;
            }
            case CALL: {
                auto call_ins = static_cast<Instruction_call *>(ins);
                out << "\t";
                out << "call ";
                if (call_ins->isRuntime) {
                    printRuntime(out, call_ins->runtimeId);
                } else {
                    printItem(out, call_ins->u);
                }
                out << " ";
                printItem(out, call_ins->num);
                out << endl;
                break;
            }
            case STACK: {
                auto stack_ins = static_cast<Instruction_stackArg *>(ins);
                out << "\t";
                printItem(out, stack_ins->reg);
                out << " <- stack-arg ";
                printItem(out, stack_ins->offset);
                out << endl;
                break;
            }
            default:
                out << "Cannot recognize the instruction!" << endl;
        }
    }

    void printL2Function(std::ostream& out, Function *f){
        out << "(" << f->name << endl;
        out << "\t" << f->arguments << " " << f->locals << endl;
        for (auto ins : f->instructions) {
            printL2Instruction(out, ins);
        }
        out << ")" << endl;
    }

    void printL2Program(ostream& out, L2::Program &p){
        out << "(" << p.entryPointLabel << endl;
        out << endl;

        for (auto f : p.functions) {
            printL2Function(out, f);
        }
        out << endl;
        out << ")" << endl;
    }

    void targetProgram(ostream& out, L2::Program& p) {
        out << "(" << p.entryPointLabel << endl;

        for (auto f: p.functions) {
            targetFunction(out, f);
        }

        out << endl;
        out << ")" << endl;
    }

    void targetFunction(ostream& out, L2::Function* f) {
        out << "(" << f->name << endl;
        out << "\t" << f->arguments << " " << f->locals << endl;
        for (auto ins: f->instructions) {
            targetInstruction(out, ins, f);
        }

        out << ")" << endl;
    }

    void targetInstruction(ostream& out, L2::Instruction* ins, L2::Function* f) {
        int id = ins->getTypeId();
        switch (id) {
            case RET: {
                out << "\treturn" << endl;
                break;
            }
            case ASS: {
                auto ass_ins = static_cast<Instruction_assignment *>(ins);
                out << "\t";
                printItem(out, ass_ins->dst);
                out << " <- ";
                printItem(out, ass_ins->src);
                out << endl;
                break;
            }
            case LOAD: {
                auto load_ins = static_cast<Instruction_load *>(ins);
                out << "\t";
                printItem(out, load_ins->dst);
                out << " <- mem ";
                printItem(out, load_ins->src);
                out << " ";
                printItem(out, load_ins->offset);
                out << endl;
                break;
            }
            case STORE: {
                auto store_ins = static_cast<Instruction_store *>(ins);
                out << "\tmem ";
                printItem(out, store_ins->dst);
                out << " ";
                printItem(out, store_ins->offset);
                out << " <- ";
                printItem(out, store_ins->src);
                out << endl;
                break;
            }
            case AOP_ASS: {
                auto aop_assign_ins = static_cast<Instruction_aop_assignment *>(ins);
                out << "\t";
                printItem(out, aop_assign_ins->dst);
                printAop(out, aop_assign_ins->opId);
                printItem(out, aop_assign_ins->src);
                out << endl;
                break;
            }
            case SELFAOP: {
                auto selfAop_ins = static_cast<Instruction_selfAop *>(ins);
                out << "\t";
                printItem(out, selfAop_ins->reg);
                printSelfAop(out, selfAop_ins->opId);
                out << endl;
                break;
            }
            case AOP_LOAD: {
                auto aop_load_ins = static_cast<Instruction_aop_load *>(ins);
                out << "\t";
                printItem(out, aop_load_ins->dst);
                printAop(out, aop_load_ins->opId);
                out << "mem ";
                printItem(out, aop_load_ins->src);
                out << " ";
                printItem(out, aop_load_ins->offset);
                out << endl;
                break;
            }
            case AOP_STORE: {
                auto aop_store_ins = static_cast<Instruction_aop_store *>(ins);
                out << "\tmem ";
                printItem(out, aop_store_ins->dst);
                out << " ";
                printItem(out, aop_store_ins->offset);
                printAop(out, aop_store_ins->opId);
                printItem(out, aop_store_ins->src);
                out << endl;
                break;
            }
            case SOP: {
                auto sop_ins = static_cast<Instruction_sop *>(ins);
                out << "\t";
                printItem(out, sop_ins->reg);
                printSop(out, sop_ins->opId);
                printItem(out, sop_ins->offset);
                out << endl;
                break;
            }
            case CMP: {
                auto cmp_ins = static_cast<Instruction_cmp *>(ins);
                out << "\t";
                printItem(out, cmp_ins->reg);
                out << " <- ";
                printItem(out, cmp_ins->l);
                printCmp(out, cmp_ins->cmpId);
                printItem(out, cmp_ins->r);
                out << endl;
                break;
            }
            case LABEL: {
                auto label_ins = static_cast<Instruction_label *>(ins);
                out << "\t";
                printItem(out, label_ins->label);
                out << endl;
                break;
            }
            case DJUMP: {
                auto djump_ins = static_cast<Instruction_djump *>(ins);
                out << "\t";
                out << "goto ";
                printItem(out, djump_ins->dst);
                out << endl;
                break;
            }
            case CJUMP: {
                auto cjump_ins = static_cast<Instruction_cjump *>(ins);
                out << "\t";
                out << "cjump ";
                printItem(out, cjump_ins->l);
                printCmp(out, cjump_ins->cmpId);
                printItem(out, cjump_ins->r);
                out << " ";
                printItem(out, cjump_ins->dst);
                out << endl;
                break;
            }
            case CISC: {
                auto CISC_ins = static_cast<Instruction_CISC *>(ins);
                out << "\t";
                printItem(out, CISC_ins->reg);
                out << " @ ";
                printItem(out, CISC_ins->base);
                out << " ";
                printItem(out, CISC_ins->index);
                out << " ";
                printItem(out, CISC_ins->scale);
                out << endl;
                break;
            }
            case CALL: {
                auto call_ins = static_cast<Instruction_call *>(ins);
                out << "\t";
                out << "call ";
                if (call_ins->isRuntime) {
                    printRuntime(out, call_ins->runtimeId);
                } else {
                    printItem(out, call_ins->u);
                }
                out << " ";
                printItem(out, call_ins->num);
                out << endl;
                break;
            }
            case STACK: {
                auto stack_ins = static_cast<Instruction_stackArg *>(ins);
                out << "\t";
                printItem(out, stack_ins->reg);
                out << " <- mem rsp ";
                auto offsetItem = static_cast<Constant*>(stack_ins->offset);
                auto offset = offsetItem->num + f->locals * 8;
                out << offset;
                out << endl;
                break;
            }
            default:
                out << "Cannot recognize the instruction!" << endl;
        }
    }

    void freeProgram(L2::Program &p) {
        for (auto f : p.functions) {
            for (auto ins : f->instructions) {
                freeInstruction(ins);
            }
            delete f->targetVar;
            delete f->spillerVar;
            delete f;
        }

    }

    void freeInstruction(L2::Instruction *ins) {
        int id = ins->getTypeId();
        switch (id) {
            case L2::RET: {
                auto ret_ins = static_cast<L2::Instruction_ret *>(ins);
                delete ret_ins;
                break;
            }
            case L2::ASS: {
                auto ass_ins = static_cast<L2::Instruction_assignment *>(ins);
                delete ass_ins->src;
                delete ass_ins->dst;
                delete ass_ins;
                break;
            }
            case L2::LOAD: {
                auto load_ins = static_cast<L2::Instruction_load *>(ins);
                delete load_ins->src;
                delete load_ins->dst;
                delete load_ins->offset;
                delete load_ins;
                break;
            }
            case L2::STORE: {
                auto store_ins = static_cast<L2::Instruction_store *>(ins);
                delete store_ins->src;
                delete store_ins->dst;
                delete store_ins->offset;
                delete store_ins;
                break;
            }
            case L2::AOP_ASS: {
                auto aop_assign_ins = static_cast<L2::Instruction_aop_assignment *>(ins);
                delete aop_assign_ins->src;
                delete aop_assign_ins->dst;
                delete aop_assign_ins;
                break;
            }
            case L2::SELFAOP: {
                auto selfAop_ins = static_cast<L2::Instruction_selfAop *>(ins);
                delete selfAop_ins->reg;
                delete selfAop_ins;
                break;
            }
            case L2::AOP_LOAD: {
                auto aop_load_ins = static_cast<L2::Instruction_aop_load *>(ins);
                delete aop_load_ins->src;
                delete aop_load_ins->dst;
                delete aop_load_ins->offset;
                delete aop_load_ins;
                break;
            }
            case L2::AOP_STORE: {
                auto aop_store_ins = static_cast<L2::Instruction_aop_store *>(ins);
                delete aop_store_ins->src;
                delete aop_store_ins->dst;
                delete aop_store_ins->offset;
                delete aop_store_ins;
                break;
            }
            case L2::SOP: {
                auto sop_ins = static_cast<L2::Instruction_sop *>(ins);
                delete sop_ins->reg;
                delete sop_ins->offset;
                delete sop_ins;
                break;
            }
            case L2::CMP: {
                auto cmp_ins = static_cast<L2::Instruction_cmp *>(ins);
                delete cmp_ins->reg;
                delete cmp_ins->l;
                delete cmp_ins->r;
                delete cmp_ins;
                break;
            }
            case L2::LABEL: {
                auto label_ins = static_cast<L2::Instruction_label *>(ins);
                delete label_ins->label;
                delete label_ins;
                break;
            }
            case L2::DJUMP: {
                auto djump_ins = static_cast<L2::Instruction_djump *>(ins);
                delete djump_ins->dst;
                delete djump_ins;
                break;
            }
            case L2::CJUMP: {
                auto cjump_ins = static_cast<L2::Instruction_cjump *>(ins);
                delete cjump_ins->dst;
                delete cjump_ins->l;
                delete cjump_ins->r;
                delete cjump_ins;
                break;
            }
            case L2::CISC: {
                auto CISC_ins = static_cast<L2::Instruction_CISC *>(ins);
                delete CISC_ins->reg;
                delete CISC_ins->base;
                delete CISC_ins->index;
                delete CISC_ins->scale;
                delete CISC_ins;
                break;
            }
            case L2::CALL: {
                auto call_ins = static_cast<L2::Instruction_call *>(ins);
                delete call_ins->u;
                delete call_ins->num;
                delete call_ins;
                break;
            }
            case L2::STACK: {
                auto stack_ins = static_cast<L2::Instruction_stackArg *>(ins);
                delete stack_ins->reg;
                delete stack_ins->offset;
                delete stack_ins;
            }
            default:;
        }
    }
}