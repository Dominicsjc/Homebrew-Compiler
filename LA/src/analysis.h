//
// Created by dominic on 2/26/20.
//

#ifndef LA_ANALYSIS_H
#define LA_ANALYSIS_H

#include <unordered_set>
#include "LA.h"
#include "IR.h"

void encodeAllConstants(LA::Function *f, std::unordered_set<std::string> &functionNames);

void tryEncodeConstant(LA::Item *i);

void transferLAInsToIR(LA::Instruction *ins, IR::Function *targetF, int &newVarId, int &newLabelId, LA::Function *f);

void decodeVar(LA::Item *i, IR::Function *targetF, int &newVarId);

void decodeVarSimple(LA::Item *i, IR::Function *targetF, int reserved);

void encodeVar(LA::Item *i, IR::Function *targetF, int &newVarId);

void checkArrayAccess(LA::Instruction *ins, IR::Function *targetF, IR::Instruction *targetIns, int &newVarId, int &newLabelId, LA::Function *f);

void createBB(IR::Function *targetF, int &newLabelId);

#endif //LA_ANALYSIS_H
