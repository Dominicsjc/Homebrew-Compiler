//
// Created by dominic on 2/18/20.
//

#include "analysis.h"
#include "L3.h"

analysis::analysis(IR::Function *f) {
    generateBb(f);
    linkBbs();
    tracing();
    removeDbr();
}

void analysis::generateBb(IR::Function *f) {
    bb *curBb = nullptr;
    int index = 0;
    int bbId = 0;
    for (auto ins : f->instructions) {
        int id = ins->getTypeId();
        switch (id) {
            case IR::RET:
            case IR::VRET: {
                curBb->exit = true;
                curBb->te = ins;
                cfg.push_back(curBb);
                //bbList.insert(bbId);
                returnList.push_back(bbId);
                bbId++;

                break;
            }
            case IR::DBR:
            case IR::CBR: {
                curBb->te = ins;
                cfg.push_back(curBb);
                bbList.insert(bbId);
                bbId++;

                break;
            }
            case IR::LABEL: {
                curBb = new bb;
                curBb->id = bbId;
                if (index == 0) {
                    curBb->entry = true;
                } else {
                    auto label_ins = static_cast<IR::Instruction_label *>(ins);
                    jumpTable.emplace(make_pair(static_cast<IR::Label *>(label_ins->label)->name, curBb));
                }
                curBb->entry_label = ins;

                break;
            }
            case IR::DECLARE: {
                auto declare_ins = static_cast<IR::Instruction_declare *>(ins);

                f->typeOfLocalVars.emplace(make_pair(static_cast<IR::Variable *>(declare_ins->var)->name,
                                                     static_cast<IR::T *>(declare_ins->type)));

                curBb->insVec.push_back(ins);

                break;
            }
            default: {
                curBb->insVec.push_back(ins);
            }
        }
        index++;
    }
}

void analysis::linkBbs() {
    for (auto normalBb : cfg) {
        if (!normalBb->exit) {
            int insId = normalBb->te->getTypeId();
            switch (insId) {
                case IR::DBR: {
                    auto dbr_ins = static_cast<IR::Instruction_dbr *>(normalBb->te);

                    normalBb->successors.push_back(jumpTable[static_cast<IR::Label *>(dbr_ins->label)->name]->id);

                    break;
                }
                case IR::CBR: {
                    auto cbr_ins = static_cast<IR::Instruction_cbr *>(normalBb->te);

                    normalBb->successors.push_back(jumpTable[static_cast<IR::Label *>(cbr_ins->labelT)->name]->id);
                    normalBb->successors.push_back(jumpTable[static_cast<IR::Label *>(cbr_ins->labelF)->name]->id);

                    break;
                }
                default:;
            }
        }
    }
}

analysis::~analysis() {
    for (const auto &tr : traces) {
        for (auto block : tr) {
            delete block;
        }
    }
}

void analysis::generateL3Instructions(analysis::bb *block, L3::Function *targetF, IR::Function *f) {
    auto entryLabel = static_cast<IR::Instruction_label *>(block->entry_label);
    auto targetEntryLabel = new L3::Instruction_label;
    targetEntryLabel->label = IR::copyIRItemToL3(entryLabel->label);
    targetF->instructions.emplace_back(targetEntryLabel); // :entry

    for (auto instruction: block->insVec) {
        generateL3Instruction(instruction, targetF, f);
    }

    generateL3Instruction(block->te, targetF, f); // return | return value | br :tag | br %a :tag1 :tag2
}

void analysis::generateL3Instruction(IR::Instruction *ins, L3::Function *targetF, IR::Function *f) {
    if (!ins) {
        return;
    }
    switch (ins->getTypeId()) {
        case IR::InsId::RET: {
            auto targetIns = new L3::Instruction_ret;
            targetF->instructions.emplace_back(targetIns);
            break;
        }
        case IR::InsId::VRET: {
            auto value_ret_ins = static_cast<IR::Instruction_value_ret *>(ins);
            auto targetIns = new L3::Instruction_value_ret;
            targetIns->t = IR::copyIRItemToL3(value_ret_ins->t);
            targetF->instructions.emplace_back(targetIns);
            break;
        }
        case IR::InsId::ASS: {
            // %a <- %b
            auto assign_ins = static_cast<IR::Instruction_assignment *>(ins);
            auto targetIns = new L3::Instruction_assignment;
            targetIns->src = IR::copyIRItemToL3(assign_ins->src);
            targetIns->dst = IR::copyIRItemToL3(assign_ins->dst);
            targetF->instructions.emplace_back(targetIns);
            break;
        }
        case IR::InsId::LOAD: {
            auto load_ins = static_cast<IR::Instruction_load *>(ins);

            IR::DataType arrayType;
            if (f->typeOfLocalVars.find(static_cast<IR::Variable *>(load_ins->src)->name) != f->typeOfLocalVars.end()) {
                arrayType = f->typeOfLocalVars[static_cast<IR::Variable *>(load_ins->src)->name]->type;
            } else {
                arrayType = f->typeOfVars[static_cast<IR::Variable *>(load_ins->src)->name]->type;
            }

            if (arrayType == IR::TUPLE) {
                auto targetOffset = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src), "LoadOffset");
                auto targetIndex = IR::copyIRItemToL3(load_ins->indices[0]);
                auto targetConst = L3::generateConst(8);
                auto multi_assign_ins = L3::generateOpAssIns(targetOffset, targetIndex, targetConst, L3::MULTI);
                targetF->instructions.emplace_back(multi_assign_ins);

                auto targetVar = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src), "LoadAddr");
                auto targetL = IR::copyIRItemToL3(load_ins->src);
                auto targetR = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src), "LoadOffset");
                auto add_assign_ins = L3::generateOpAssIns(targetVar, targetL, targetR, L3::ADD);
                targetF->instructions.emplace_back(add_assign_ins);

                auto targetDst = IR::copyIRItemToL3(load_ins->dst);
                auto targetSrc = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src), "LoadAddr");
                auto targetLoadIns = L3::generateLoadIns(targetDst, targetSrc);
                targetF->instructions.emplace_back(targetLoadIns);
            } else {
                for (int i = 1; i < load_ins->indices.size(); i++) {
                    auto targetVar = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                               "LenAddr" + std::to_string(i));
                    auto targetL = IR::copyIRItemToL3(load_ins->src);
                    auto targetR = L3::generateConst(16 + i * 8);
                    auto op_assign_ins = L3::generateOpAssIns(targetVar, targetL, targetR, L3::ADD);
                    targetF->instructions.emplace_back(op_assign_ins);

                    auto targetDst = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                               "Len_" + std::to_string(i));
                    auto targetSrc = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                               "LenAddr" + std::to_string(i));
                    auto targetLoadIns = L3::generateLoadIns(targetDst, targetSrc);
                    targetF->instructions.emplace_back(targetLoadIns);

                    auto trueLen = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                             "Len" + std::to_string(i));
                    auto rawLen = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                            "Len_" + std::to_string(i));
                    auto decodeOffset = L3::generateConst(1);
                    auto targetRightIns = L3::generateOpAssIns(trueLen, rawLen, decodeOffset, L3::RIGHT);
                    targetF->instructions.emplace_back(targetRightIns);
                }

                for (int j = load_ins->indices.size() - 1; j >= 0; j--) {
                    if (j == load_ins->indices.size() - 1) {
                        auto targetDst = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                                   "TmpVar" +
                                                                   std::to_string(j));
                        auto targetSrc = IR::copyIRItemToL3(load_ins->indices[j]);
                        auto ass_ins = new L3::Instruction_assignment;
                        ass_ins->dst = targetDst;
                        ass_ins->src = targetSrc;
                        targetF->instructions.emplace_back(ass_ins);
                    } else if (j == load_ins->indices.size() - 2) {
                        auto targetVar = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                                   "Augend");
                        auto targetL = IR::copyIRItemToL3(load_ins->indices[j]);
                        auto targetR = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                                 "Len" + std::to_string(j + 1));
                        auto multi_ass_ins = L3::generateOpAssIns(targetVar, targetL, targetR, L3::MULTI);
                        targetF->instructions.emplace_back(multi_ass_ins);

                        auto newVar = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                                "TmpVar" +
                                                                std::to_string(j));
                        auto oldVar = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                                "TmpVar" +
                                                                std::to_string(j + 1));
                        auto augend = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                                "Augend");
                        auto add_ass_ins = L3::generateOpAssIns(newVar, oldVar, augend, L3::ADD);
                        targetF->instructions.emplace_back(add_ass_ins);
                    } else if (j == load_ins->indices.size() - 3) {
                        auto targetVar = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                                   "Size" + std::to_string(j));
                        auto targetL = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                                 "Len" + std::to_string(load_ins->indices.size() - 2));
                        auto targetR = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                                 "Len" + std::to_string(load_ins->indices.size() - 1));
                        auto getSizeIns = L3::generateOpAssIns(targetVar, targetL, targetR, L3::MULTI);
                        targetF->instructions.emplace_back(getSizeIns);

                        auto targetVar2 = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                                    "Augend");
                        auto targetL2 = IR::copyIRItemToL3(load_ins->indices[j]);
                        auto targetR2 = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                                  "Size" + std::to_string(j));
                        auto getAugend = L3::generateOpAssIns(targetVar2, targetL2, targetR2, L3::MULTI);
                        targetF->instructions.emplace_back(getAugend);

                        auto newVar = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                                "TmpVar" +
                                                                std::to_string(j));
                        auto oldVar = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                                "TmpVar" +
                                                                std::to_string(j + 1));
                        auto augend = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                                "Augend");
                        auto getNewVar = L3::generateOpAssIns(newVar, oldVar, augend, L3::ADD);
                        targetF->instructions.emplace_back(getNewVar);
                    } else {
                        auto targetVar = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                                   "Size" + std::to_string(j));
                        auto targetL = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                                 "Len" + std::to_string(j + 1));
                        auto targetR = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                                 "Size" + std::to_string(j + 1));

                        auto getSizeIns = L3::generateOpAssIns(targetVar, targetL, targetR, L3::MULTI);
                        targetF->instructions.emplace_back(getSizeIns);

                        auto targetVar2 = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                                    "Augend");
                        auto targetL2 = IR::copyIRItemToL3(load_ins->indices[j]);
                        auto targetR2 = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                                  "Size" + std::to_string(j));
                        auto getAugend = L3::generateOpAssIns(targetVar2, targetL2, targetR2, L3::MULTI);
                        targetF->instructions.emplace_back(getAugend);

                        auto newVar = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                                "TmpVar" +
                                                                std::to_string(j));
                        auto oldVar = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                                "TmpVar" +
                                                                std::to_string(j + 1));
                        auto augend = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                                "Augend");
                        auto getNewVar = L3::generateOpAssIns(newVar, oldVar, augend, L3::ADD);
                        targetF->instructions.emplace_back(getNewVar);
                    }
                }

                auto targetVar = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src), "LoadOffset");
                auto targetL = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src),
                                                         "TmpVar" + std::to_string(0));
                auto targetR = L3::generateConst(8);
                auto targetIns = L3::generateOpAssIns(targetVar, targetL, targetR, L3::MULTI);
                targetF->instructions.emplace_back(targetIns);

                auto targetVar2 = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src), "LoadOffset");
                auto targetL2 = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src), "LoadOffset");
                auto targetR2 = L3::generateConst(16 + load_ins->indices.size() * 8);
                auto targetIns2 = L3::generateOpAssIns(targetVar2, targetL2, targetR2, L3::ADD);
                targetF->instructions.emplace_back(targetIns2);

                auto targetVar3 = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src), "LoadAddr");
                auto targetL3 = IR::copyIRItemToL3(load_ins->src);
                auto targetR3 = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src), "LoadOffset");
                auto targetIns3 = L3::generateOpAssIns(targetVar3, targetL3, targetR3, L3::ADD);
                targetF->instructions.emplace_back(targetIns3);

                auto targetDst = IR::copyIRItemToL3(load_ins->dst);
                auto targetSrc = IR::generateVarWithSuffix(static_cast<IR::Variable *>(load_ins->src), "LoadAddr");
                auto targetLoadIns = L3::generateLoadIns(targetDst, targetSrc);
                targetF->instructions.emplace_back(targetLoadIns);
            }

            break;
        }
        case IR::InsId::STORE: {
            auto store_ins = static_cast<IR::Instruction_store *>(ins);

            IR::DataType arrayType;
            if (f->typeOfLocalVars.find(static_cast<IR::Variable *>(store_ins->dst)->name) !=
                f->typeOfLocalVars.end()) {
                arrayType = f->typeOfLocalVars[static_cast<IR::Variable *>(store_ins->dst)->name]->type;
            } else {
                arrayType = f->typeOfVars[static_cast<IR::Variable *>(store_ins->dst)->name]->type;
            }

            if (arrayType == IR::TUPLE) {
                auto targetOffset = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                              "StoreOffset");
                auto targetIndex = IR::copyIRItemToL3(store_ins->indices[0]);
                auto targetConst = L3::generateConst(8);
                auto multi_assign_ins = L3::generateOpAssIns(targetOffset, targetIndex, targetConst, L3::MULTI);
                targetF->instructions.emplace_back(multi_assign_ins);

                auto targetVar = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst), "StoreAddr");
                auto targetL = IR::copyIRItemToL3(store_ins->dst);
                auto targetR = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst), "StoreOffset");
                auto add_assign_ins = L3::generateOpAssIns(targetVar, targetL, targetR, L3::ADD);
                targetF->instructions.emplace_back(add_assign_ins);

                auto targetDst = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst), "StoreAddr");
                auto targetSrc = IR::copyIRItemToL3(store_ins->src);
                auto targetStoreIns = L3::generateStoreIns(targetDst, targetSrc);
                targetF->instructions.emplace_back(targetStoreIns);
            } else {
                for (int i = 1; i < store_ins->indices.size(); i++) {
                    auto targetVar = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                               "LenAddr" + std::to_string(i));
                    auto targetL = IR::copyIRItemToL3(store_ins->dst);
                    auto targetR = L3::generateConst(16 + i * 8);
                    auto op_assign_ins = L3::generateOpAssIns(targetVar, targetL, targetR, L3::ADD);
                    targetF->instructions.emplace_back(op_assign_ins);

                    auto targetDst = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                               "Len_" + std::to_string(i));
                    auto targetSrc = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                               "LenAddr" + std::to_string(i));
                    auto targetLoadIns = L3::generateLoadIns(targetDst, targetSrc);
                    targetF->instructions.emplace_back(targetLoadIns);

                    auto trueLen = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                             "Len" + std::to_string(i));
                    auto rawLen = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                            "Len_" + std::to_string(i));
                    auto decodeOffset = L3::generateConst(1);
                    auto targetRightIns = L3::generateOpAssIns(trueLen, rawLen, decodeOffset, L3::RIGHT);
                    targetF->instructions.emplace_back(targetRightIns);
                }

                for (int j = store_ins->indices.size() - 1; j >= 0; j--) {
                    if (j == store_ins->indices.size() - 1) {
                        auto targetDst = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                                   "TmpVar" +
                                                                   std::to_string(j));
                        auto targetSrc = IR::copyIRItemToL3(store_ins->indices[j]);
                        auto ass_ins = new L3::Instruction_assignment;
                        ass_ins->dst = targetDst;
                        ass_ins->src = targetSrc;
                        targetF->instructions.emplace_back(ass_ins);
                    } else if (j == store_ins->indices.size() - 2) {
                        auto targetVar = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                                   "Augend");
                        auto targetL = IR::copyIRItemToL3(store_ins->indices[j]);
                        auto targetR = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                                 "Len" + std::to_string(j + 1));
                        auto multi_ass_ins = L3::generateOpAssIns(targetVar, targetL, targetR, L3::MULTI);
                        targetF->instructions.emplace_back(multi_ass_ins);

                        auto newVar = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                                "TmpVar" +
                                                                std::to_string(j));
                        auto oldVar = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                                "TmpVar" +
                                                                std::to_string(j + 1));
                        auto augend = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                                "Augend");
                        auto add_ass_ins = L3::generateOpAssIns(newVar, oldVar, augend, L3::ADD);
                        targetF->instructions.emplace_back(add_ass_ins);
                    } else if (j == store_ins->indices.size() - 3) {
                        auto targetVar = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                                   "Size" + std::to_string(j));
                        auto targetL = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                                 "Len" + std::to_string(store_ins->indices.size() - 2));
                        auto targetR = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                                 "Len" + std::to_string(store_ins->indices.size() - 1));
                        auto getSizeIns = L3::generateOpAssIns(targetVar, targetL, targetR, L3::MULTI);
                        targetF->instructions.emplace_back(getSizeIns);

                        auto targetVar2 = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                                    "Augend");
                        auto targetL2 = IR::copyIRItemToL3(store_ins->indices[j]);
                        auto targetR2 = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                                  "Size" + std::to_string(j));
                        auto getAugend = L3::generateOpAssIns(targetVar2, targetL2, targetR2, L3::MULTI);
                        targetF->instructions.emplace_back(getAugend);

                        auto newVar = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                                "TmpVar" +
                                                                std::to_string(j));
                        auto oldVar = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                                "TmpVar" +
                                                                std::to_string(j + 1));
                        auto augend = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                                "Augend");
                        auto getNewVar = L3::generateOpAssIns(newVar, oldVar, augend, L3::ADD);
                        targetF->instructions.emplace_back(getNewVar);
                    } else {
                        auto targetVar = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                                   "Size" + std::to_string(j));
                        auto targetL = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                                 "Len" + std::to_string(j + 1));
                        auto targetR = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                                 "Size" + std::to_string(j + 1));

                        auto getSizeIns = L3::generateOpAssIns(targetVar, targetL, targetR, L3::MULTI);
                        targetF->instructions.emplace_back(getSizeIns);

                        auto targetVar2 = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                                    "Augend");
                        auto targetL2 = IR::copyIRItemToL3(store_ins->indices[j]);
                        auto targetR2 = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                                  "Size" + std::to_string(j));
                        auto getAugend = L3::generateOpAssIns(targetVar2, targetL2, targetR2, L3::MULTI);
                        targetF->instructions.emplace_back(getAugend);

                        auto newVar = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                                "TmpVar" +
                                                                std::to_string(j));
                        auto oldVar = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                                "TmpVar" +
                                                                std::to_string(j + 1));
                        auto augend = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                                "Augend");
                        auto getNewVar = L3::generateOpAssIns(newVar, oldVar, augend, L3::ADD);
                        targetF->instructions.emplace_back(getNewVar);
                    }
                }

                auto targetVar = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst), "StoreOffset");
                auto targetL = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst),
                                                         "TmpVar" + std::to_string(0));
                auto targetR = L3::generateConst(8);
                auto targetIns = L3::generateOpAssIns(targetVar, targetL, targetR, L3::MULTI);
                targetF->instructions.emplace_back(targetIns);

                auto targetVar2 = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst), "StoreOffset");
                auto targetL2 = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst), "StoreOffset");
                auto targetR2 = L3::generateConst(16 + store_ins->indices.size() * 8);
                auto targetIns2 = L3::generateOpAssIns(targetVar2, targetL2, targetR2, L3::ADD);
                targetF->instructions.emplace_back(targetIns2);

                auto targetVar3 = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst), "StoreAddr");
                auto targetL3 = IR::copyIRItemToL3(store_ins->dst);
                auto targetR3 = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst), "StoreOffset");
                auto targetIns3 = L3::generateOpAssIns(targetVar3, targetL3, targetR3, L3::ADD);
                targetF->instructions.emplace_back(targetIns3);

                auto targetDst = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_ins->dst), "StoreAddr");
                auto targetSrc = IR::copyIRItemToL3(store_ins->src);
                auto targetStoreIns = L3::generateStoreIns(targetDst, targetSrc);
                targetF->instructions.emplace_back(targetStoreIns);
            }

            break;
        }
        case IR::InsId::LENGTH: {
            auto store_length_ins = static_cast<IR::Instruction_store_length *>(ins);

            auto targetVar0 = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_length_ins->src),
                                                        "LenOffset0");
            auto targetL0 = IR::copyIRItemToL3(store_length_ins->t);
            auto targetR0 = L3::generateConst(8);
            auto op_assign_ins0 = L3::generateOpAssIns(targetVar0, targetL0, targetR0, L3::MULTI);
            targetF->instructions.emplace_back(op_assign_ins0);

            auto targetVar1 = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_length_ins->src),
                                                        "LenOffset1");
            auto targetL1 = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_length_ins->src), "LenOffset0");
            auto targetR1 = L3::generateConst(16);
            auto op_assign_ins1 = L3::generateOpAssIns(targetVar1, targetL1, targetR1, L3::ADD);
            targetF->instructions.emplace_back(op_assign_ins1);

            auto targetVar2 = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_length_ins->src),
                                                        "LenOffset2");
            auto targetL2 = IR::copyIRItemToL3(store_length_ins->src);
            auto targetR2 = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_length_ins->src), "LenOffset1");
            auto op_assign_ins2 = L3::generateOpAssIns(targetVar2, targetL2, targetR2, L3::ADD);
            targetF->instructions.emplace_back(op_assign_ins2);

            auto targetDst = IR::copyIRItemToL3(store_length_ins->dst);
            auto targetSrc = IR::generateVarWithSuffix(static_cast<IR::Variable *>(store_length_ins->src),
                                                       "LenOffset2");
            auto load_ins = L3::generateLoadIns(targetDst, targetSrc);
            targetF->instructions.emplace_back(load_ins);

            break;
        }
        case IR::InsId::OP_ASS: {
            auto op_assign_ins = static_cast<IR::Instruction_op_assignment *>(ins);

            auto targetVar = IR::copyIRItemToL3(op_assign_ins->var);
            auto targetL = IR::copyIRItemToL3(op_assign_ins->l);
            auto targetR = IR::copyIRItemToL3(op_assign_ins->r);
            auto targetOp = static_cast<L3::Op>(op_assign_ins->opId);

            targetF->instructions.emplace_back(
                    L3::generateOpAssIns(targetVar, targetL, targetR, targetOp));

            break;
        }
        case IR::InsId::CMP_ASS: {
            auto cmp_assign_ins = static_cast<IR::Instruction_cmp_assignment *>(ins);

            auto targetVar = IR::copyIRItemToL3(cmp_assign_ins->var);
            auto targetL = IR::copyIRItemToL3(cmp_assign_ins->l);
            auto targetR = IR::copyIRItemToL3(cmp_assign_ins->r);
            auto targetOp = static_cast<L3::Cmp>(cmp_assign_ins->cmpId);

            targetF->instructions.emplace_back(
                    L3::generateCmpAssIns(targetVar, targetL, targetR, targetOp));

            break;
        }
        case IR::InsId::LABEL: {
            auto label_ins = static_cast<IR::Instruction_label *>(ins);

            auto targetIns = new L3::Instruction_label;
            targetIns->label = IR::copyIRItemToL3(label_ins->label);

            targetF->instructions.emplace_back(targetIns);

            break;
        }
        case IR::InsId::DBR: {
            auto dbr_ins = static_cast<IR::Instruction_dbr *>(ins);

            auto targetIns = new L3::Instruction_dbr;
            targetIns->dst = IR::copyIRItemToL3(dbr_ins->label);

            targetF->instructions.emplace_back(targetIns);

            break;
        }
        case IR::InsId::CBR: {
            auto cbr_ins = static_cast<IR::Instruction_cbr *>(ins);

            if (cbr_ins->t->getTypeId() == IR::VAR) {
                auto targetVbr = new L3::Instruction_vbr;
                targetVbr->dst = IR::copyIRItemToL3(cbr_ins->labelT);
                targetVbr->var = IR::copyIRItemToL3(cbr_ins->t);

                auto targetDbr = new L3::Instruction_dbr;
                targetDbr->dst = IR::copyIRItemToL3(cbr_ins->labelF);

                targetF->instructions.emplace_back(targetVbr);
                targetF->instructions.emplace_back(targetDbr);
            } else {
                auto number = static_cast<IR::Constant *>(cbr_ins->t);
                auto targetDbr = new L3::Instruction_dbr;
                if (number->num != 0) {
                    targetDbr->dst = IR::copyIRItemToL3(cbr_ins->labelT);
                } else {
                    targetDbr->dst = IR::copyIRItemToL3(cbr_ins->labelF);
                }

                targetF->instructions.emplace_back(targetDbr);
            }

            break;
        }
        case IR::InsId::CALL: {
            auto call_ins = static_cast<IR::Instruction_call *>(ins);

            auto targetIns = new L3::Instruction_call;

            if (call_ins->isRuntime) {
                targetIns->isRuntime = true;
                if (call_ins->runtimeId == IR::RuntimeType::PRINT) {
                    targetIns->runtimeId = L3::RuntimeType::PRINT;
                } else if (call_ins->runtimeId == IR::RuntimeType::ARRAY_ERROR) {
                    targetIns->runtimeId = L3::RuntimeType::ARRAY;
                }
            } else {
                targetIns->u = IR::copyIRItemToL3(call_ins->u);
            }

            for (auto v: call_ins->args) {
                targetIns->args.emplace_back(IR::copyIRItemToL3(v));
            }

            targetF->instructions.emplace_back(targetIns);

            break;
        }
        case IR::InsId::CALL_ASS: {
            auto call_assign_ins = static_cast<IR::Instruction_call_assignment *>(ins);

            auto targetIns = new L3::Instruction_call_assignment;

            if (call_assign_ins->isRuntime) {
                targetIns->isRuntime = true;
                if (call_assign_ins->runtimeId == IR::RuntimeType::PRINT) {
                    targetIns->runtimeId = L3::RuntimeType::PRINT;
                } else if (call_assign_ins->runtimeId == IR::RuntimeType::ARRAY_ERROR) {
                    targetIns->runtimeId = L3::RuntimeType::ARRAY;
                }
            } else {
                targetIns->u = IR::copyIRItemToL3(call_assign_ins->u);
            }

            for (auto v: call_assign_ins->args) {
                targetIns->args.emplace_back(IR::copyIRItemToL3(v));
            }

            targetIns->var = IR::copyIRItemToL3(call_assign_ins->var);

            targetF->instructions.emplace_back(targetIns);

            break;
        }
        case IR::InsId::NEW_ARRAY: {
            auto new_array_ins = static_cast<IR::Instruction_new_array *>(ins);

            analysis::transferNewArrayIns(new_array_ins, targetF);

            break;
        }
        case IR::InsId::NEW_TUPLE: {
            auto new_tuple_ins = static_cast<IR::Instruction_new_tuple *>(ins);

            auto targetIns = new L3::Instruction_call_assignment;
            targetIns->isRuntime = true;
            targetIns->runtimeId = L3::ALLOCATE;
            targetIns->var = IR::copyIRItemToL3(new_tuple_ins->var);
            targetIns->args.emplace_back(IR::copyIRItemToL3(new_tuple_ins->t));
            targetIns->args.emplace_back(L3::generateConst(1));

            targetF->instructions.emplace_back(targetIns);

            break;
        }
        case IR::InsId::DECLARE: {
            break;
        }
        default:;
    }
}

void analysis::transferToL3Function(L3::Function *targetF, IR::Function *f) {
    for (const auto &tr : traces) {
        for (auto block : tr) {
            generateL3Instructions(block, targetF, f);
        }
    }
}

void analysis::tracing() {
    int count = 0;
    do {
        trace curTr;
        int curBb;
        if (count == 0) {
            curBb = 0;
        } else {
            curBb = *bbList.begin();
        }
        while (bbList.find(curBb) != bbList.end()) {
            bbList.erase(curBb);
            curTr.push_back(cfg[curBb]);

            if (cfg[curBb]->successors.size() == 1) {
                curBb = cfg[curBb]->successors[0];
            } else if (cfg[curBb]->successors.size() == 2){
                curBb = (countPath(cfg[curBb]->successors[0], bbList) >= countPath(cfg[curBb]->successors[1], bbList) ? cfg[curBb]->successors[0] : cfg[curBb]->successors[1]);
            }

        }
        traces.emplace_back(curTr);
        count++;
    } while (!bbList.empty());
    for (auto id : returnList) {
        trace curTr;
        curTr.push_back(cfg[id]);
        traces.emplace_back(curTr);
    }
    cfg.clear();
}

void analysis::removeDbr() {
    for (auto tr : traces) {
        for (int i = 0; i < tr.size(); i++) {
            if (i != tr.size() - 1) {
                if (tr[i]->te->getTypeId() == IR::DBR) {
                    tr[i]->te = nullptr;
                    tr[i]->successors.clear();
                }
            }
        }
    }
}

void analysis::transferNewArrayIns(IR::Instruction_new_array *ins, L3::Function *targetF) {
    int dim = ins->args.size();
    auto var = static_cast<IR::Variable *>(ins->var);

    int dimIndex = 1;
    for (auto arg : ins->args) {
        auto varD = new L3::Item;
        varD->type = L3::VAR;
        varD->name = var->name + "D" + std::to_string(dimIndex);
        auto L3arg = IR::copyIRItemToL3(arg);
        auto number = L3::generateConst(1);

        auto op_ass_ins = L3::generateOpAssIns(varD, L3arg, number, L3::RIGHT);
        targetF->instructions.push_back(op_ass_ins);

        dimIndex++;
    }

    dimIndex = 1;
    if (dim == 1) {
        auto dst = IR::generateVarWithSuffix(var, "Len");
        auto src = new L3::Item;
        src->type = L3::VAR;
        src->name = var->name + "D" + std::to_string(1);

        auto ass_ins = L3::generateAssIns(dst, src);
        targetF->instructions.push_back(ass_ins);
    } else {
        auto lenVar = IR::generateVarWithSuffix(var, "Len");
        auto l = new L3::Item;
        l->type = L3::VAR;
        l->name = var->name + "D" + std::to_string(dimIndex++);
        auto r = new L3::Item;
        r->type = L3::VAR;
        r->name = var->name + "D" + std::to_string(dimIndex++);

        auto first_op_ass_ins = L3::generateOpAssIns(lenVar, l, r, L3::MULTI);
        targetF->instructions.push_back(first_op_ass_ins);

        while (dim > 2) {
            auto selfVar = IR::generateVarWithSuffix(var, "Len");
            auto selfL = IR::generateVarWithSuffix(var, "Len");
            auto selfR = new L3::Item;
            selfR->type = L3::VAR;
            selfR->name = var->name + "D" + std::to_string(dimIndex++);

            auto op_ass_ins = L3::generateOpAssIns(selfVar, selfL, selfR, L3::MULTI);
            targetF->instructions.push_back(op_ass_ins);
            dim--;
        }
    }

    auto tmpVar = IR::generateVarWithSuffix(var, "Len");
    auto tmpL = IR::generateVarWithSuffix(var, "Len");
    auto tmpR = L3::generateConst(1);
    auto tmp_ins = L3::generateOpAssIns(tmpVar, tmpL, tmpR, L3::LEFT);
    targetF->instructions.push_back(tmp_ins);

    tmpVar = IR::generateVarWithSuffix(var, "Len");
    tmpL = IR::generateVarWithSuffix(var, "Len");
    tmpR = L3::generateConst(1);
    tmp_ins = L3::generateOpAssIns(tmpVar, tmpL, tmpR, L3::ADD);
    targetF->instructions.push_back(tmp_ins);

    tmpVar = IR::generateVarWithSuffix(var, "Len");
    tmpL = IR::generateVarWithSuffix(var, "Len");
    tmpR = L3::generateConst((1 + ins->args.size()) * 2);
    tmp_ins = L3::generateOpAssIns(tmpVar, tmpL, tmpR, L3::ADD);
    targetF->instructions.push_back(tmp_ins);

    auto call_assign_ins = new L3::Instruction_call_assignment;
    call_assign_ins->isRuntime = true;
    call_assign_ins->runtimeId = L3::ALLOCATE;
    tmpVar = IR::copyIRItemToL3(var);
    call_assign_ins->var = tmpVar;
    auto lenVar = IR::generateVarWithSuffix(var, "Len");
    auto number = L3::generateConst(1);
    call_assign_ins->args.emplace_back(lenVar);
    call_assign_ins->args.emplace_back(number);
    targetF->instructions.push_back(call_assign_ins);

    tmpVar = IR::generateVarWithSuffix(var, "Dim");
    tmpL = IR::copyIRItemToL3(var);
    tmpR = L3::generateConst(8);
    tmp_ins = L3::generateOpAssIns(tmpVar, tmpL, tmpR, L3::ADD);
    targetF->instructions.push_back(tmp_ins);

    tmpVar = IR::generateVarWithSuffix(var, "Dim");
    tmpR = L3::generateConst((ins->args.size() * 2) + 1);
    auto store_ins = L3::generateStoreIns(tmpVar, tmpR);
    targetF->instructions.push_back(store_ins);

    for (int i = 0; i < ins->args.size(); i++) {
        tmpVar = IR::generateVarWithSuffix(var, "Dim" + std::to_string(i + 1));
        tmpL = IR::copyIRItemToL3(var);
        tmpR = L3::generateConst(16 + i * 8);
        tmp_ins = L3::generateOpAssIns(tmpVar, tmpL, tmpR, L3::ADD);
        targetF->instructions.push_back(tmp_ins);

        tmpVar = IR::generateVarWithSuffix(var, "Dim" + std::to_string(i + 1));
        auto L3arg = IR::copyIRItemToL3(ins->args[i]);
        store_ins = L3::generateStoreIns(tmpVar, L3arg);
        targetF->instructions.push_back(store_ins);
    }

}

int analysis::countPath(int bbIndex, std::unordered_set<int> tmpList) {
    if (tmpList.find(bbIndex) == tmpList.end()) {
        return 0;
    } else if (cfg[bbIndex]->exit) {
        return 1;
    } else {
        tmpList.erase(bbIndex);
        if (cfg[bbIndex]->successors.size() == 1) {
            return countPath(cfg[bbIndex]->successors[0], tmpList) + 1;
        } else {
            int len1 = countPath(cfg[bbIndex]->successors[0], tmpList);
            int len2 = countPath(cfg[bbIndex]->successors[1], tmpList);
            return (len1 >= len2 ? len1 + 1 : len2 + 1);
        }
    }
}
