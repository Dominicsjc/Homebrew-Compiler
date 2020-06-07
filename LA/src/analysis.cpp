//
// Created by dominic on 2/26/20.
//
#include <iostream>
#include "analysis.h"
#include "utils.h"

void encodeAllConstants(LA::Function *f, std::unordered_set<std::string> &functionNames) {
    for (auto ins : f->instructions) {
        switch (ins->getTypeId()) {
            case LA::ASS: {
                auto ass_ins = static_cast<LA::Instruction_assignment *>(ins);
                tryEncodeConstant(ass_ins->src);
                break;
            }
                /*
                case LA::OP_ASS: {
                    auto op_assign_ins = static_cast<LA::Instruction_op_assignment *>(ins);
                    tryEncodeConstant(op_assign_ins->l);
                    tryEncodeConstant(op_assign_ins->r);
                    break;
                }

                case LA::CBR: {
                    auto cbr_ins = static_cast<LA::Instruction_cbr *>(ins);
                    tryEncodeConstant(cbr_ins->t);
                    break;
                }
                 */
            case LA::STORE: {
                auto store_ins = static_cast<LA::Instruction_store *>(ins);
                tryEncodeConstant(store_ins->src);
                break;
            }
            case LA::VRET: {
                auto vret_ins = static_cast<LA::Instruction_value_ret *>(ins);
                tryEncodeConstant(vret_ins->t);
                break;
            }
            case LA::CALL: {
                auto call_ins = static_cast<LA::Instruction_call *>(ins);
                if (!call_ins->isPrint) {
                    if (functionNames.find(static_cast<LA::Variable *>(call_ins->u)->name) != functionNames.end()) {
                        auto label = new LA::Label;
                        label->name = ":" + static_cast<LA::Variable *>(call_ins->u)->name;
                        delete call_ins->u;
                        call_ins->u = label;
                    }
                }
                for (auto arg : call_ins->args) {
                    tryEncodeConstant(arg);
                }
                break;
            }
            case LA::CALL_ASS: {
                auto call_assign_ins = static_cast<LA::Instruction_call_assignment *>(ins);
                if (functionNames.find(static_cast<LA::Variable *>(call_assign_ins->u)->name) != functionNames.end()) {
                    auto label = new LA::Label;
                    label->name = ":" + static_cast<LA::Variable *>(call_assign_ins->u)->name;
                    delete call_assign_ins->u;
                    call_assign_ins->u = label;
                }
                for (auto arg : call_assign_ins->args) {
                    tryEncodeConstant(arg);
                }
                break;
            }
            case LA::NEW_ARRAY: {
                auto new_array_ins = static_cast<LA::Instruction_new_array *>(ins);
                for (auto arg : new_array_ins->args) {
                    tryEncodeConstant(arg);
                }
                break;
            }
            case LA::NEW_TUPLE: {
                auto new_tuple_ins = static_cast<LA::Instruction_new_tuple *>(ins);
                tryEncodeConstant(new_tuple_ins->t);
                break;
            }
            default:;
        }
    }
}

void tryEncodeConstant(LA::Item *i) {
    if (i->getTypeId() != LA::CONST) {
        return;
    }

    auto number = static_cast<LA::Constant *>(i);
    if (number->num < 0) {
        number->num = - ((-number->num << 1) - 1);
    }
    number->num = (number->num << 1) + 1;
}

void transferLAInsToIR(LA::Instruction *ins, IR::Function *targetF, int &newVarId, int &newLabelId, LA::Function *f) {
    switch (ins->getTypeId()) {
        case LA::RET: {
            auto targetIns = new IR::Instruction_ret;

            targetF->instructions.emplace_back(targetIns);
            break;
        }
        case LA::VRET: {
            auto vret_ins = static_cast<LA::Instruction_value_ret *>(ins);
            auto targetIns = new IR::Instruction_value_ret;

            targetIns->t = transferLAItemToIR(vret_ins->t);

            targetF->instructions.emplace_back(targetIns);
            break;
        }
        case LA::ASS: {
            auto assign_ins = static_cast<LA::Instruction_assignment *>(ins);
            auto targetIns = new IR::Instruction_assignment;

            targetIns->dst = transferLAItemToIR(assign_ins->dst);
            if (f->typeOfAllVars[static_cast<LA::Variable *>(assign_ins->dst)->name]->type != LA::CODE) {
                targetIns->src = transferLAItemToIR(assign_ins->src);
            } else {
                if (assign_ins->src->getTypeId() == LA::VAR) {
                    auto label = new IR::Label;
                    label->name = ":" + static_cast<LA::Variable *>(assign_ins->src)->name;
                    targetIns->src = label;
                } else {
                    targetIns->src = transferLAItemToIR(assign_ins->src);
                }
            }
            targetF->instructions.emplace_back(targetIns);
            break;
        }
        case LA::LOAD: {
            auto load_ins = static_cast<LA::Instruction_load *>(ins);
            auto targetIns = new IR::Instruction_load;

            checkArrayAccess(load_ins, targetF, targetIns, newVarId, newLabelId, f);
            /*
            for (auto index : load_ins->indices) {
                if (index->getTypeId() == LA::CONST) {
                    targetIns->indices.emplace_back(transferLAItemToIR(index));
                } else {
                    decodeVar(index, targetF, newVarId);
                    targetIns->indices.emplace_back(generateNewIRVar(newVarId));
                    newVarId++;
                }
            }
             */

            targetIns->dst = transferLAItemToIR(load_ins->dst);
            targetIns->src = transferLAItemToIR(load_ins->src);

            targetF->instructions.emplace_back(targetIns);
            break;
        }
        case LA::STORE: {
            auto store_ins = static_cast<LA::Instruction_store *>(ins);
            auto targetIns = new IR::Instruction_store;

            checkArrayAccess(store_ins, targetF, targetIns, newVarId, newLabelId, f);
            /*
            for (auto index : store_ins->indices) {
                if (index->getTypeId() == LA::CONST) {
                    targetIns->indices.emplace_back(transferLAItemToIR(index));
                } else {
                    decodeVar(index, targetF, newVarId);
                    targetIns->indices.emplace_back(generateNewIRVar(newVarId));
                    newVarId++;
                }
            }
             */

            targetIns->dst = transferLAItemToIR(store_ins->dst);
            targetIns->src = transferLAItemToIR(store_ins->src);

            targetF->instructions.emplace_back(targetIns);
            break;
        }
        case LA::LENGTH: {
            auto length_ins = static_cast<LA::Instruction_store_length *>(ins);
            auto targetIns = new IR::Instruction_store_length;

            if (length_ins->t->getTypeId() != LA::CONST) {
                decodeVar(length_ins->t, targetF, newVarId);
                targetIns->t = generateNewIRVar(newVarId);
                newVarId++;
            } else {
                targetIns->t = transferLAItemToIR(length_ins->t);
            }
            targetIns->dst = transferLAItemToIR(length_ins->dst);
            targetIns->src = transferLAItemToIR(length_ins->src);

            targetF->instructions.emplace_back(targetIns);
            break;
        }
        case LA::OP_ASS: {
            auto op_assign_ins = static_cast<LA::Instruction_op_assignment *>(ins);
            auto targetIns = new IR::Instruction_op_assignment;

            if (op_assign_ins->l->getTypeId() != LA::CONST) {
                decodeVarSimple(op_assign_ins->l, targetF, 0);
                targetIns->l = generateNewIRVar(0);
            } else {
                targetIns->l = transferLAItemToIR(op_assign_ins->l);
            }
            if (op_assign_ins->r->getTypeId() != LA::CONST) {
                decodeVarSimple(op_assign_ins->r, targetF, 1);
                targetIns->r = generateNewIRVar(1);
            } else {
                targetIns->r = transferLAItemToIR(op_assign_ins->r);
            }
            targetIns->var = transferLAItemToIR(op_assign_ins->var);
            targetIns->opId = static_cast<IR::Op>(op_assign_ins->opId);

            targetF->instructions.emplace_back(targetIns);
            encodeVar(op_assign_ins->var, targetF, newVarId);
            break;
        }
        case LA::CMP_ASS: {
            auto cmp_assign_ins = static_cast<LA::Instruction_cmp_assignment *>(ins);
            auto targetIns = new IR::Instruction_cmp_assignment;

            if (cmp_assign_ins->l->getTypeId() != LA::CONST) {
                decodeVar(cmp_assign_ins->l, targetF, newVarId);
                targetIns->l = generateNewIRVar(newVarId);
                newVarId++;
            } else {
                targetIns->l = transferLAItemToIR(cmp_assign_ins->l);
            }
            if (cmp_assign_ins->r->getTypeId() != LA::CONST) {
                decodeVar(cmp_assign_ins->r, targetF, newVarId);
                targetIns->r = generateNewIRVar(newVarId);
                newVarId++;
            } else {
                targetIns->r = transferLAItemToIR(cmp_assign_ins->r);
            }
            targetIns->var = transferLAItemToIR(cmp_assign_ins->var);
            targetIns->cmpId = static_cast<IR::Cmp>(cmp_assign_ins->cmpId);

            targetF->instructions.emplace_back(targetIns);
            encodeVar(cmp_assign_ins->var, targetF, newVarId);
            break;
        }
        case LA::LABEL: {
            auto label_ins = static_cast<LA::Instruction_label *>(ins);
            auto targetIns = new IR::Instruction_label;

            targetIns->label = transferLAItemToIR(label_ins->label);

            targetF->instructions.emplace_back(targetIns);
            break;
        }
        case LA::DBR: {
            auto dbr_ins = static_cast<LA::Instruction_dbr *>(ins);
            auto targetIns = new IR::Instruction_dbr;

            targetIns->label = transferLAItemToIR(dbr_ins->label);

            targetF->instructions.emplace_back(targetIns);
            break;
        }
        case LA::CBR: {
            auto cbr_ins = static_cast<LA::Instruction_cbr *>(ins);
            auto targetIns = new IR::Instruction_cbr;

            if (cbr_ins->t->getTypeId() != LA::CONST) {
                decodeVar(cbr_ins->t, targetF, newVarId);
                targetIns->t = generateNewIRVar(newVarId);
                newVarId++;
            } else {
                targetIns->t = transferLAItemToIR(cbr_ins->t);
            }
            targetIns->labelT = transferLAItemToIR(cbr_ins->labelT);
            targetIns->labelF = transferLAItemToIR(cbr_ins->labelF);

            targetF->instructions.emplace_back(targetIns);
            break;
        }
        case LA::CALL: {
            auto call_ins = static_cast<LA::Instruction_call *>(ins);
            auto targetIns = new IR::Instruction_call;

            for (auto arg : call_ins->args) {
                targetIns->args.emplace_back(transferLAItemToIR(arg));
            }
            if (call_ins->isPrint) {
                targetIns->isRuntime = true;
                targetIns->runtimeId = IR::PRINT;
            } else {
                targetIns->u = transferLAItemToIR(call_ins->u);
            }

            targetF->instructions.emplace_back(targetIns);
            break;
        }
        case LA::CALL_ASS: {
            auto call_assign_ins = static_cast<LA::Instruction_call_assignment *>(ins);
            auto targetIns = new IR::Instruction_call_assignment;

            for (auto arg : call_assign_ins->args) {
                targetIns->args.emplace_back(transferLAItemToIR(arg));
            }
            targetIns->u = transferLAItemToIR(call_assign_ins->u);
            targetIns->var = transferLAItemToIR(call_assign_ins->var);

            targetF->instructions.emplace_back(targetIns);
            break;
        }
        case LA::NEW_ARRAY: {
            auto new_array_ins = static_cast<LA::Instruction_new_array *>(ins);
            auto targetIns = new IR::Instruction_new_array;

            for (auto arg : new_array_ins->args) {
                targetIns->args.emplace_back(transferLAItemToIR(arg));
            }
            targetIns->var = transferLAItemToIR(new_array_ins->var);

            targetF->instructions.emplace_back(targetIns);
            break;
        }
        case LA::NEW_TUPLE: {
            auto new_tuple_ins = static_cast<LA::Instruction_new_tuple *>(ins);
            auto targetIns = new IR::Instruction_new_tuple;

            targetIns->t = transferLAItemToIR(new_tuple_ins->t);
            targetIns->var = transferLAItemToIR(new_tuple_ins->var);

            targetF->instructions.emplace_back(targetIns);
            break;
        }
        case LA::DECLARE: {
            auto declare_ins = static_cast<LA::Instruction_declare *>(ins);
            auto targetIns = new IR::Instruction_declare;
            auto type = static_cast<LA::T *>(declare_ins->type);

            targetIns->type = transferLAItemToIR(declare_ins->type);
            targetIns->var = transferLAItemToIR(declare_ins->var);

            targetF->instructions.emplace_back(targetIns);

            if (type->type == LA::TUPLE || (type->type == LA::INT64 && type->arrayDim > 0)) {
                auto initialize_ins = new IR::Instruction_assignment;
                initialize_ins->dst = transferLAItemToIR(declare_ins->var);
                initialize_ins->src = generateIRConstant(0);
                targetF->instructions.emplace_back(initialize_ins);
            }
            break;
        }
    }
}

void decodeVar(LA::Item *i, IR::Function *targetF, int &newVarId) {
    auto declare_ins = generateInt64DeclareIns(generateNewIRVar(newVarId));
    targetF->instructions.emplace_back(declare_ins);

    auto op_assign_ins = generateVarShiftOpIns(generateNewIRVar(newVarId), transferLAItemToIR(i), IR::RIGHT);
    targetF->instructions.emplace_back(op_assign_ins);
}

void decodeVarSimple(LA::Item *i, IR::Function *targetF, int reserved) {
    auto op_assign_ins = generateVarShiftOpIns(generateNewIRVar(reserved), transferLAItemToIR(i), IR::RIGHT);
    targetF->instructions.emplace_back(op_assign_ins);
}

void encodeVar(LA::Item *i, IR::Function *targetF, int &newVarId) {
    auto op_assign_ins = generateVarShiftOpIns(transferLAItemToIR(i), transferLAItemToIR(i), IR::LEFT);
    targetF->instructions.emplace_back(op_assign_ins);

    auto add_ins = generateOpIns(transferLAItemToIR(i), transferLAItemToIR(i), generateIRConstant(1), IR::ADD);
    targetF->instructions.emplace_back(add_ins);
}

void checkArrayAccess(LA::Instruction *ins, IR::Function *targetF, IR::Instruction *targetIns, int &newVarId,
                      int &newLabelId, LA::Function *f) {
    LA::Item *arr = nullptr;
    std::vector<LA::Item *> *indices = nullptr;
    std::vector<IR::Item *> *targetIndices = nullptr;
    if (ins->getTypeId() == LA::LOAD) {
        arr = static_cast<LA::Instruction_load *>(ins)->src;
        indices = &static_cast<LA::Instruction_load *>(ins)->indices;
        targetIndices = &static_cast<IR::Instruction_load *>(targetIns)->indices;
    } else {
        arr = static_cast<LA::Instruction_store *>(ins)->dst;
        indices = &static_cast<LA::Instruction_store *>(ins)->indices;
        targetIndices = &static_cast<IR::Instruction_store *>(targetIns)->indices;
    }
    for (auto index : *indices) {
        if (index->getTypeId() == LA::CONST) {
            targetIndices->emplace_back(transferLAItemToIR(index));
        } else {
            decodeVar(index, targetF, newVarId);
            targetIndices->emplace_back(generateNewIRVar(newVarId++));
        }
    }
    auto declare_ins1 = generateInt64DeclareIns(generateNewIRVar(newVarId));
    targetF->instructions.emplace_back(declare_ins1);
    auto initialize_ins = new IR::Instruction_assignment;
    initialize_ins->dst = generateNewIRVar(newVarId);
    initialize_ins->src = generateIRConstant(0);
    targetF->instructions.emplace_back(initialize_ins);
    int indexVarId = newVarId++;

    auto declare_ins2 = generateInt64DeclareIns(generateNewIRVar(newVarId));
    targetF->instructions.emplace_back(declare_ins2);
    auto var = generateNewIRVar(newVarId);
    auto cmp_assign_ins1 = generateCmpIns(var, transferLAItemToIR(arr), generateIRConstant(0), IR::EQL);
    targetF->instructions.emplace_back(cmp_assign_ins1);

    auto cbr_ins1 = new IR::Instruction_cbr;
    cbr_ins1->t = generateNewIRVar(newVarId++);
    cbr_ins1->labelT = generateNewIRLabel(newLabelId);
    int errorLabelIndex = newLabelId++;
    cbr_ins1->labelF = generateNewIRLabel(newLabelId);
    targetF->instructions.emplace_back(cbr_ins1);

    int dim = 0;
    for (auto index : *indices) {
        auto indexCheck = generateLabelIns(generateNewIRLabel(newLabelId++));
        targetF->instructions.emplace_back(indexCheck);

        if (static_cast<LA::T *>(f->typeOfAllVars[static_cast<LA::Variable *>(arr)->name])->type == LA::TUPLE) {
            auto dbr_ins1 = new IR::Instruction_dbr;
            dbr_ins1->label = generateNewIRLabel(newLabelId);
            targetF->instructions.emplace_back(dbr_ins1);
        } else {
            auto declare_ins3 = generateInt64DeclareIns(generateNewIRVar(newVarId));
            targetF->instructions.emplace_back(declare_ins3);
            auto store_length_ins = new IR::Instruction_store_length;
            store_length_ins->dst = generateNewIRVar(newVarId);
            store_length_ins->src = transferLAItemToIR(arr);
            store_length_ins->t = generateIRConstant(dim);
            targetF->instructions.emplace_back(store_length_ins);
            auto decode_ins = generateVarShiftOpIns(generateNewIRVar(newVarId), generateNewIRVar(newVarId), IR::RIGHT);
            targetF->instructions.emplace_back(decode_ins);
            int lengthVarIndex = newVarId;
            newVarId++;

            IR::Item *l = nullptr;
            if (index->getTypeId() == LA::CONST) {
                auto assign_ins = new IR::Instruction_assignment;
                assign_ins->dst = generateNewIRVar(indexVarId);
                assign_ins->src = transferLAItemToIR(index);
                targetF->instructions.emplace_back(assign_ins);
                l = transferLAItemToIR(index);
            } else {
                auto assign_ins = new IR::Instruction_assignment;
                assign_ins->dst = generateNewIRVar(indexVarId);
                assign_ins->src = copyIRVariable(static_cast<IR::Variable *>((*targetIndices)[dim]));
                targetF->instructions.emplace_back(assign_ins);
                l = copyIRVariable(static_cast<IR::Variable *>((*targetIndices)[dim]));
            }
            auto left_ins = generateVarShiftOpIns(generateNewIRVar(indexVarId), generateNewIRVar(indexVarId),
                                                  IR::LEFT);
            targetF->instructions.emplace_back(left_ins);
            auto add_ins = generateOpIns(generateNewIRVar(indexVarId), generateNewIRVar(indexVarId),
                                         generateIRConstant(1), IR::ADD);
            targetF->instructions.emplace_back(add_ins);


            auto declare_ins4 = generateInt64DeclareIns(generateNewIRVar(newVarId));
            targetF->instructions.emplace_back(declare_ins4);
            auto cmp_assign_ins2 = generateCmpIns(generateNewIRVar(newVarId), l, generateNewIRVar(lengthVarIndex),
                                                  IR::LE);
            targetF->instructions.emplace_back(cmp_assign_ins2);
            auto cbr_ins2 = new IR::Instruction_cbr;
            cbr_ins2->t = generateNewIRVar(newVarId++);
            cbr_ins2->labelT = generateNewIRLabel(newLabelId);
            cbr_ins2->labelF = generateNewIRLabel(errorLabelIndex);
            targetF->instructions.emplace_back(cbr_ins2);
        }

        dim++;
    }

    auto error = generateLabelIns(generateNewIRLabel(errorLabelIndex));
    targetF->instructions.emplace_back(error);
    auto call_error_ins = new IR::Instruction_call;
    call_error_ins->isRuntime = true;
    call_error_ins->runtimeId = IR::ARRAY_ERROR;
    call_error_ins->args.emplace_back(transferLAItemToIR(arr));
    call_error_ins->args.emplace_back(generateNewIRVar(indexVarId));
    targetF->instructions.emplace_back(call_error_ins);
    auto back_dbr_ins2 = new IR::Instruction_dbr;
    back_dbr_ins2->label = generateNewIRLabel(newLabelId);
    targetF->instructions.emplace_back(back_dbr_ins2);


    auto backPoint = generateLabelIns(generateNewIRLabel(newLabelId++));
    targetF->instructions.emplace_back(backPoint);
}

void createBB(IR::Function *targetF, int &newLabelId) {
    bool startBB = true;
    for (int i = 0; i < targetF->instructions.size(); i++) {
        if (startBB) {
            if (targetF->instructions[i]->getTypeId() != IR::LABEL) {
                targetF->instructions.insert(targetF->instructions.begin() + i,
                                             generateLabelIns(generateNewIRLabel(newLabelId)));
                i++;
            }
            startBB = false;
        } else if (targetF->instructions[i]->getTypeId() == IR::LABEL) {
            auto dbr_ins = new IR::Instruction_dbr;
            auto label = new IR::Label;
            label->name = static_cast<IR::Label *>(static_cast<IR::Instruction_label *>(targetF->instructions[i])->label)->name;
            dbr_ins->label = label;
            targetF->instructions.insert(targetF->instructions.begin() + i, dbr_ins);
            i++;
        }
        if (targetF->instructions[i]->getTypeId() == IR::DBR ||
            targetF->instructions[i]->getTypeId() == IR::CBR ||
            targetF->instructions[i]->getTypeId() == IR::RET ||
            targetF->instructions[i]->getTypeId() == IR::VRET) {
            startBB = true;
        }
    }
}
