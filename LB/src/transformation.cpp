//
// Created by dominic on 3/1/20.
//

#include "transformation.h"
#include "utils.h"

void transferLBFunctionToLA(LB::Function *f, LA::Function *targetF) {
    targetF->name = f->name; // copy function name
    targetF->res = copyLBItemToLA(f->res); // copy return type

    for (auto var : f->typedVars) { // copy arguments
        targetF->typedVars.emplace_back(copyLBItemToLA(var));
    }

    auto declare_ins = new LA::Instruction_declare;
    auto t = new LA::T;
    t->type = LA::INT64;
    t->arrayDim = 0;
    declare_ins->type = t;
    declare_ins->var = createLAVarWithName("condVar");
    targetF->instructions.emplace_back(declare_ins);

    for (auto instruction : f->instructions) {
        switch (instruction->getTypeId()) {
            case LB::InsId::RET: {
                auto targetInstruction = new LA::Instruction_ret();

                targetF->instructions.emplace_back(targetInstruction);
                break;
            }
            case LB::InsId::VRET: {
                auto ins = static_cast<LB::Instruction_value_ret *>(instruction);
                auto targetInstruction = new LA::Instruction_value_ret();
                targetInstruction->t = copyLBItemToLA(ins->t);

                targetF->instructions.emplace_back(targetInstruction);
                break;
            }
            case LB::InsId::ASS: {
                auto ins = static_cast<LB::Instruction_assignment *>(instruction);
                auto targetInstruction = new LA::Instruction_assignment();
                targetInstruction->dst = copyLBItemToLA(ins->dst);
                targetInstruction->src = copyLBItemToLA(ins->src);

                targetF->instructions.emplace_back(targetInstruction);
                break;
            }
            case LB::InsId::LOAD: {
                auto ins = static_cast<LB::Instruction_load *>(instruction);
                auto targetInstruction = new LA::Instruction_load();
                targetInstruction->dst = copyLBItemToLA(ins->dst);
                targetInstruction->src = copyLBItemToLA(ins->src);

                for (auto index : ins->indices) {
                    targetInstruction->indices.emplace_back(copyLBItemToLA(index));
                }

                targetF->instructions.emplace_back(targetInstruction);
                break;
            }
            case LB::InsId::STORE: {
                auto ins = static_cast<LB::Instruction_store *>(instruction);
                auto targetInstruction = new LA::Instruction_store();
                targetInstruction->dst = copyLBItemToLA(ins->dst);
                targetInstruction->src = copyLBItemToLA(ins->src);

                for (auto index : ins->indices) {
                    targetInstruction->indices.emplace_back(copyLBItemToLA(index));
                }

                targetF->instructions.emplace_back(targetInstruction);
                break;
            }
            case LB::InsId::LENGTH: {
                auto ins = static_cast<LB::Instruction_store_length *>(instruction);
                auto targetInstruction = new LA::Instruction_store_length();
                targetInstruction->dst = copyLBItemToLA(ins->dst);
                targetInstruction->src = copyLBItemToLA(ins->src);
                targetInstruction->t = copyLBItemToLA(ins->t);

                targetF->instructions.emplace_back(targetInstruction);
                break;
            }
            case LB::InsId::OP_ASS: {
                auto ins = static_cast<LB::Instruction_op_assignment *>(instruction);
                auto targetInstruction = transferLBOpAssToLA(ins->var, ins->l, ins->r, ins->opId);
                targetF->instructions.emplace_back(targetInstruction);
                break;
            }
            case LB::InsId::LABEL: {
                auto ins = static_cast<LB::Instruction_label *>(instruction);
                auto targetInstruction = new LA::Instruction_label();
                targetInstruction->label = copyLBItemToLA(ins->label);

                targetF->instructions.emplace_back(targetInstruction);
                break;
            }
            case LB::InsId::DBR: {
                auto ins = static_cast<LB::Instruction_dbr *>(instruction);
                auto targetInstruction = new LA::Instruction_dbr();
                targetInstruction->label = copyLBItemToLA(ins->label);

                targetF->instructions.emplace_back(targetInstruction);
                break;
            }
            case LB::InsId::IF: {
                auto ins = static_cast<LB::Instruction_if *>(instruction);

                auto targetConditionInstruction = transferLBConditionToLA(ins->l, ins->r, ins->opId);
                targetF->instructions.emplace_back(targetConditionInstruction);

                auto targetJumpInstruction = new LA::Instruction_cbr();
                targetJumpInstruction->t = createLAVarWithName("condVar");
                targetJumpInstruction->labelT = copyLBItemToLA(ins->labelT);
                targetJumpInstruction->labelF = copyLBItemToLA(ins->labelF);
                targetF->instructions.emplace_back(targetJumpInstruction);
                break;
            }
            case LB::InsId::WHILE: {
                auto ins = static_cast<LB::Instruction_while *>(instruction);
                auto targetBackInstruction = new LA::Instruction_label();
                targetBackInstruction->label = copyLBItemToLA(ins->condLabel);
                targetF->instructions.emplace_back(targetBackInstruction);

                auto targetConditionInstruction = transferLBConditionToLA(ins->l,
                                                                          ins->r, ins->opId);
                targetF->instructions.emplace_back(targetConditionInstruction);

                auto targetJumpInstruction = new LA::Instruction_cbr();
                targetJumpInstruction->t = createLAVarWithName("condVar");
                targetJumpInstruction->labelT = copyLBItemToLA(ins->labelT);
                targetJumpInstruction->labelF = copyLBItemToLA(ins->labelF);
                targetF->instructions.emplace_back(targetJumpInstruction);
                break;
            }
            case LB::InsId::CONTINUE: {
                auto ins = static_cast<LB::Instruction_continue *>(instruction);
                auto targetInstruction = new LA::Instruction_dbr();
                targetInstruction->label = copyLBItemToLA(ins->cond);
                targetF->instructions.emplace_back(targetInstruction);
                break;
            }
            case LB::InsId::BREAK: {
                auto ins = static_cast<LB::Instruction_break *>(instruction);
                auto targetInstruction = new LA::Instruction_dbr();
                targetInstruction->label = copyLBItemToLA(ins->exit);
                targetF->instructions.emplace_back(targetInstruction);
                break;
            }
            case LB::InsId::CALL: {
                auto ins = static_cast<LB::Instruction_call *>(instruction);
                auto targetInstruction = new LA::Instruction_call();
                targetInstruction->isPrint = ins->isPrint;
                if (ins->u) {
                    targetInstruction->u = copyLBItemToLA(ins->u);
                }

                for (auto argument : ins->args) {
                    targetInstruction->args.emplace_back(copyLBItemToLA(argument));
                }

                targetF->instructions.emplace_back(targetInstruction);
                break;
            }
            case LB::InsId::CALL_ASS: {
                auto ins = static_cast<LB::Instruction_call_assignment *>(instruction);
                auto targetInstruction = new LA::Instruction_call_assignment();
                targetInstruction->u = copyLBItemToLA(ins->u);
                targetInstruction->var = copyLBItemToLA(ins->var);

                for (auto argument : ins->args) {
                    targetInstruction->args.emplace_back(copyLBItemToLA(argument));
                }

                targetF->instructions.emplace_back(targetInstruction);
                break;
            }
            case LB::InsId::NEW_ARRAY: {
                auto ins = static_cast<LB::Instruction_new_array *>(instruction);
                auto targetInstruction = new LA::Instruction_new_array();
                targetInstruction->var = copyLBItemToLA(ins->var);

                for (auto argument : ins->args) {
                    targetInstruction->args.emplace_back(copyLBItemToLA(argument));
                }

                targetF->instructions.emplace_back(targetInstruction);
                break;
            }
            case LB::InsId::NEW_TUPLE: {
                auto ins = static_cast<LB::Instruction_new_tuple *>(instruction);
                auto targetInstruction = new LA::Instruction_new_tuple();
                targetInstruction->var = copyLBItemToLA(ins->var);
                targetInstruction->t = copyLBItemToLA(ins->t);

                targetF->instructions.emplace_back(targetInstruction);
                break;
            }
            case LB::InsId::DECLARE: {
                auto ins = static_cast<LB::Instruction_declare *>(instruction);

                auto targetInstruction = new LA::Instruction_declare();
                targetInstruction->var = copyLBItemToLA(ins->vars[0]);
                targetInstruction->type = copyLBItemToLA(ins->type);
                targetF->instructions.emplace_back(targetInstruction);

                break;
            }
            default:;
        }
    }
}