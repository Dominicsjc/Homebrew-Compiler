#include "LA.h"
#include <iostream>

using namespace std;

namespace LA {
    void parseTypeOfFunctionVars(Function *f) {
        for (int i = 0; i < f->typedVars.size();) {
            f->typeOfAllVars.insert({static_cast<Variable *>(f->typedVars[i+1])->name, static_cast<T *>(f->typedVars[i])});
            i += 2;
        }

        for(auto ins : f->instructions) {
            if (ins->getTypeId() == DECLARE) {
                auto declare_ins = static_cast<Instruction_declare *>(ins);
                f->typeOfAllVars.insert({static_cast<Variable *>(declare_ins->var)->name, static_cast<T *>(declare_ins->type)});
            }
        }
    }

    void freeProgram(Program &p) {
        for (auto f : p.functions) {
            for (auto ins : f->instructions) {
                freeInstruction(ins);
            }
            for (auto item : f->typedVars) {
                delete item;
            }
            delete f->res;
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
                for (auto t : load_ins->indices) {
                    delete t;
                }
                delete load_ins;
                break;
            }
            case STORE: {
                auto store_ins = static_cast<Instruction_store *>(ins);
                delete store_ins->src;
                delete store_ins->dst;
                for (auto t : store_ins->indices) {
                    delete t;
                }
                delete store_ins;
                break;
            }
            case LENGTH: {
                auto store_length_ins = static_cast<Instruction_store_length *>(ins);
                delete store_length_ins->src;
                delete store_length_ins->dst;
                delete store_length_ins->t;
                delete store_length_ins;
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
                delete dbr_ins->label;
                delete dbr_ins;
                break;
            }
            case CBR: {
                auto cbr_ins = static_cast<Instruction_cbr *>(ins);
                delete cbr_ins->t;
                delete cbr_ins->labelT;
                delete cbr_ins->labelF;
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
                break;
            }
            case NEW_ARRAY: {
                auto new_array_ins = static_cast<Instruction_new_array *>(ins);
                delete new_array_ins->var;
                for (auto arg : new_array_ins->args) {
                    delete arg;
                }
                delete new_array_ins;
                break;
            }
            case NEW_TUPLE: {
                auto new_tuple_ins = static_cast<Instruction_new_tuple *>(ins);
                delete new_tuple_ins->var;
                delete new_tuple_ins->t;
                delete new_tuple_ins;
                break;
            }
            default:;
        }
    }

    void printItem(std::ostream& out, LA::Item* item)
    {
        out << toString(item);
    }

    std::string toString(LA::Item* item)
    {
        switch (item->getTypeId()) {
        case LA::ItemId::CONST: {
            auto constant = (Constant*)item;
            return to_string(constant->num);
        }
        case LA::ItemId::LAB: {
            auto label = (Label*)item;
            return label->name;
        }
        case LA::ItemId::TYPE: {
            auto type = (T*)item;
            std::string res;
            res.append(toString(type));

            for (uint i = 0; i < type->arrayDim; i++) {
                res.append("[]");
            }

            return res;
        }
        case LA::ItemId::VAR: {
            auto variable = (Variable*)item;
            return variable->name;
        }
        default:
            return "";
        }
    }

    std::string toString(DataType type)
    {
        switch (type) {
        case LA::DataType::CODE: {
            return "code";
        }
        case LA::DataType::INT64: {
            return "int64";
        }
        case LA::DataType::TUPLE: {
            return "tuple";
        }
        case LA::DataType::VOID: {
            return "void";
        }
        default:
            return "";
        }
    }

    void printProgram(std::ostream& out, Program& p)
    {
        for (auto f : p.functions) {
            printFunction(out, f);
        }
    }

    void printFunction(std::ostream& out, Function* f)
    {
        out << toString(f->res) << "(";

        for (uint i = 0; i < f->typedVars.size(); i++) {
            auto v = f->typedVars[i];
            if (i % 2 == 0) { // type
                if (i != 0) {
                    out << ", ";
                };
                out << toString(v);
            } else {
                out << " " << toString(v);
            }
        }

        out << ") {" << endl;

        for (auto instruction : f->instructions) {
            out << "   ";
            printInstruction(out, instruction);
            out << endl;
        }
    }

    void printInstruction(std::ostream& out, Instruction* ins)
    {
        switch (ins->getTypeId()) {
        case LA::InsId::RET: {
            out << "return";
            break;
        }
        case LA::InsId::VRET: {
            auto v = (Instruction_value_ret*)ins;
            out << "return " << toString(v->t);
        }
        case LA::InsId::ASS: {
            auto v = (Instruction_assignment*)ins;
            out << toString(v->dst) << " <- " << toString(v->src);
        }
        default:
            break;
        }
    }
}