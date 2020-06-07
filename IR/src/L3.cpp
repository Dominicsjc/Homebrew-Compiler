#include "L3.h"
#include <iostream>

using namespace std;

namespace L3 {
    void freeProgram(Program &p) {
        for (auto f : p.functions) {
            for (auto ins : f->instructions) {
                freeInstruction(ins);
            }
            for (auto var : f->vars) {
                delete var;
            }
            delete f;
        }

    }

    void freeInstruction(Instruction *ins) {
        int id = ins->getTypeId();
        switch (id) {
            case RET: {
                auto ret_ins = static_cast<Instruction_ret *>(ins);
                delete ret_ins;
                break;
            }
            case VRET: {
                auto vret_ins = static_cast<Instruction_value_ret *>(ins);
                delete vret_ins->t;
                delete vret_ins;
                break;
            }
            case ASS: {
                auto ass_ins = static_cast<Instruction_assignment *>(ins);
                delete ass_ins->src;
                delete ass_ins->dst;
                delete ass_ins;
                break;
            }
            case LOAD: {
                auto load_ins = static_cast<Instruction_load *>(ins);
                delete load_ins->src;
                delete load_ins->dst;
                delete load_ins;
                break;
            }
            case STORE: {
                auto store_ins = static_cast<Instruction_store *>(ins);
                delete store_ins->src;
                delete store_ins->dst;
                delete store_ins;
                break;
            }
            case OP_ASS: {
                auto op_assign_ins = static_cast<Instruction_op_assignment *>(ins);
                delete op_assign_ins->var;
                delete op_assign_ins->l;
                delete op_assign_ins->r;
                delete op_assign_ins;
                break;
            }
            case CMP_ASS: {
                auto cmp_ass_ins = static_cast<Instruction_cmp_assignment *>(ins);
                delete cmp_ass_ins->var;
                delete cmp_ass_ins->l;
                delete cmp_ass_ins->r;
                delete cmp_ass_ins;
                break;
            }
            case LABEL: {
                auto label_ins = static_cast<Instruction_label *>(ins);
                delete label_ins->label;
                delete label_ins;
                break;
            }
            case DBR: {
                auto dbr_ins = static_cast<Instruction_dbr *>(ins);
                delete dbr_ins->dst;
                delete dbr_ins;
                break;
            }
            case VBR: {
                auto vbr_ins = static_cast<Instruction_vbr *>(ins);
                delete vbr_ins->dst;
                delete vbr_ins->var;
                delete vbr_ins;
                break;
            }
            case CALL: {
                auto call_ins = static_cast<Instruction_call *>(ins);
                delete call_ins->u;
                for (auto arg : call_ins->args) {
                    delete arg;
                }
                delete call_ins;
                break;
            }
            case CALL_ASS: {
                auto call_ass_ins = static_cast<Instruction_call_assignment *>(ins);
                delete call_ass_ins->var;
                delete call_ass_ins->u;
                for (auto arg : call_ass_ins->args) {
                    delete arg;
                }
                delete call_ass_ins;
            }
            default:;
        }
    }

    Instruction *generateOpAssIns(Item *var, Item *l, Item *r, Op opId) {
        auto res = new Instruction_op_assignment;

        res->var = var;
        res->l = l;
        res->r = r;
        res->opId = opId;

        return res;
    }

    Instruction *generateCmpAssIns(Item *var, Item *l, Item *r, Cmp cmp) {
        auto res = new Instruction_cmp_assignment();

        res->var = var;
        res->l = l;
        res->r = r;
        res->cmpId = cmp;

        return res;
    }

    Instruction *generateAssIns(Item *dst, Item *src) {
        auto res = new Instruction_assignment;

        res->dst = dst;
        res->src = src;

        return res;
    }

    Instruction *generateStoreIns(Item *dst, Item *src) {
        auto res = new Instruction_store;

        res->dst = dst;
        res->src = src;

        return res;
    }

    Instruction *generateLoadIns(Item *dst, Item *src) {
        auto res = new Instruction_load;

        res->dst = dst;
        res->src = src;

        return res;
    }

    Item *generateConst(long long num) {
        auto res = new Constant;

        res->num = num;

        return res;
    }

    void printProgram(std::ostream& out, L3::Program& p) {
        for (auto v: p.functions) {
            printFunction(out, v);
        }
    }

    void printFunction(std::ostream& out, L3::Function* f) {
        out << "define " << f->name << "(";

        for (uint i = 0; i < f->vars.size(); i ++) {
            if (i != 0) {
                out << ", ";
            }
            out << toString(f->vars[i]);
        }

        out << ") {" << endl;

        for (auto v: f->instructions) {
            out << "\t";
            printInstruction(out, v);
            out << endl;
        }

        out << "}" << endl;
        out << endl;
    }

    void printInstruction(ostream& out, L3::Instruction* instruction) {
        auto type = instruction->getTypeId();

        switch (type) {
            case L3::InsId::RET: {
                out << "return" << endl;
                break;
            }
            case L3::InsId::VRET: {
                auto returnInstruction = (L3::Instruction_value_ret*)instruction;
                out << "return " << toString(returnInstruction->t);
                break;
            }
            case L3::InsId::ASS: {
                auto assignmentInstruction = (L3::Instruction_assignment*)instruction;
                out << toString(assignmentInstruction->dst) << " <- " << toString(assignmentInstruction->src);
                break;
            }
            case L3::InsId::LOAD: {
                auto loadInstruction = (L3::Instruction_load*)instruction;
                out << toString(loadInstruction->dst) << " <- load " << toString(loadInstruction->src);
                break;
            }
            case L3::InsId::STORE: {
                auto storeInstruction = (L3::Instruction_store*)instruction;
                out << "store " << toString(storeInstruction->dst) << " <- " << toString(storeInstruction->src);
                break;
            }
            case L3::InsId::OP_ASS: {
                auto operationInstruction = (L3::Instruction_op_assignment*)instruction;
                out << toString(operationInstruction->var) << " <- " << toString(operationInstruction->l) << " " << toString(operationInstruction->opId) << " " << toString(operationInstruction->r);
                break;
            }
            case L3::InsId::CMP_ASS: {
                auto comparisonInstruction = (L3::Instruction_cmp_assignment*)instruction;
                out << toString(comparisonInstruction->var) << " <- " << toString(comparisonInstruction->l) << " " << toString(comparisonInstruction->cmpId) << " " << toString(comparisonInstruction->r);
                break;
            }
            case L3::InsId::LABEL: {
                auto labelInstruction = (L3::Instruction_label*)instruction;
                out << toString(labelInstruction->label);
                break;
            }
            case L3::InsId::DBR: {
                auto jumpInstruction = (L3::Instruction_dbr*)instruction;
                out << "br " << toString(jumpInstruction->dst);
                break;
            }
            case L3::InsId::VBR: {
                auto jumpInstruction = (L3::Instruction_vbr*)instruction;
                out << "br " << toString(jumpInstruction->var) << " " << toString(jumpInstruction->dst);
                break;
            }
            case L3::InsId::CALL: {
                auto callInstruction = (L3::Instruction_call*)instruction;
                if (callInstruction->isRuntime) {
                    switch (callInstruction->runtimeId) {
                        case L3::RuntimeType::PRINT: {
                            out << "call print(";
                            break;
                        }
                        case L3::RuntimeType::ALLOCATE: {
                            out << "call allocate(";
                            break;
                        }
                        case L3::RuntimeType::ARRAY: {
                            out << "call array-error(";
                            break;
                        }
                    }
                } else {
                    out << "call " << toString(callInstruction->u) << "(";
                }

                for (uint i = 0; i < callInstruction->args.size(); i ++) {
                    if (i != 0) {
                        out << ", ";
                    }
                    out << toString(callInstruction->args[i]);
                }

                out << ")";
                break;
            }
            case L3::InsId::CALL_ASS: {
                auto callInstruction = (L3::Instruction_call_assignment*)instruction;
                out << toString(callInstruction->var) << " <- ";

                if (callInstruction->isRuntime) {
                    switch (callInstruction->runtimeId) {
                        case L3::RuntimeType::PRINT: {
                            out << "call print(";
                            break;
                        }
                        case L3::RuntimeType::ALLOCATE: {
                            out << "call allocate(";
                            break;
                        }
                        case L3::RuntimeType::ARRAY: {
                            out << "call array-error(";
                            break;
                        }
                    }
                } else {
                    out << "call " << toString(callInstruction->u) << "(";
                }

                for (uint i = 0; i < callInstruction->args.size(); i ++) {
                    if (i != 0) {
                        out << ", ";
                    }
                    out << toString(callInstruction->args[i]);
                }

                out << ")";
                break;
            }
            default:
                break;
        }
    }

    std::string toString(L3::Item* item) {
        if (item->isConstant()) {
            auto constant = (L3::Constant*)item;
            return to_string(constant->num);
        } else {
            if (item->type == L3::ItemType::LAB) {
                return item->name;
            } else {
                return "%" + item->name;
            }
        }
    }

    std::string toString(L3::Op op) {
        switch (op) {
            case L3::Op::ADD: {
                return "+";
            }
            case L3::Op::SUB: {
                return "-";
            }
            case L3::Op::MULTI: {
                return "*";
            }
            case L3::Op::AND: {
                return "&";
            }
            case L3::Op::LEFT: {
                return "<<";
            }
            case L3::Op::RIGHT: {
                return ">>";
            }
            default: {
                return "";
            }
        }
    }

    std::string toString(L3::Cmp comparison) {
        switch (comparison) {
            case L3::Cmp::LE: {
                return "<";
            }
            case L3::Cmp::LEQ: {
                return "<=";
            }
            case L3::Cmp::EQL: {
                return "=";
            }
            case L3::Cmp::GR: {
                return ">";
            }
            case L3::Cmp::GEQ: {
                return ">=";
            }
            default: {
                return "";
            }
        }
    }

    void printItem(ostream& out, L3::Item* item) {
        if (item->isConstant()) {
            auto constant = (L3::Constant*)item;
            out << constant->num;
        } else {
            if (item->type == L3::ItemType::LAB) {
                out << item->name;
            } else {
                out << "%" + item->name;
            }
        }
    }
}