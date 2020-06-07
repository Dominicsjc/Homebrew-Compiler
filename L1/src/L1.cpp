#include <iostream>
#include "L1.h"

using namespace std;

namespace L1 {
    static std::string tab = "   ";

    void printItem(ostream &out, Item *i) {
        if (i->isConstant()) {
            auto n = static_cast<Constant *>(i);
            out << n->num;
            return;
        } else {
            if (!i->isARegister)
                out << i->labelName;
            else
                out << register_name[i->r];
        }
    }

    void assemblyItem(ostream &out, Item *i) {
        if (i->isConstant()) {
            auto n = static_cast<Constant *>(i);
            out << "$" << n->num;
            return;
        } else {
            if (!i->isARegister)
                out << "_" << i->labelName.substr(1);
            else
                out << "%" << register_name[i->r];
        }
    }

    void assembly8bItem(ostream &out, Item *i) {
        if (!i->isConstant() && i->isARegister)
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
                if (!ass_ins->src->isConstant() && !ass_ins->src->isARegister)
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
                if (!store_ins->src->isConstant() && !store_ins->src->isARegister)
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
                    if (!cmp_ins->l->isARegister && cmp_ins->r->isARegister) {
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
                    if (!cjump_ins->l->isARegister && cjump_ins->r->isARegister) {
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
                    if (call_ins->u->isARegister) {
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

        if (f->locals > 0) {
            out << tab;
            assemblyPrologue(out, f);
            out << endl;
        }

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
                out << "++ ";
                break;
            }
            case SUB: {
                out << "-- ";
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
        if (f->locals > 0) {
            out << "subq $" << f->locals * 8 << ", %rsp # Allocate spills";
        }
    }

    void assemblyReturn(std::ostream &out, Function* f) {
        if (f->locals + max(0l, f->arguments - 6) > 0) {
            out << "addq $" << (f->locals + max(0l, f->arguments - 6)) * 8 << ", %rsp # Free stack locations" << endl;
            out << tab << "retq";
            return;
        } else {
            out << "retq";
        }
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
}
