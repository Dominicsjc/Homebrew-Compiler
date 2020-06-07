//
// Created by dominic on 2/26/20.
//

#ifndef LA_UTILS_H
#define LA_UTILS_H

#include "IR.h"
#include "LA.h"

IR::Item *transferLAItemToIR(LA::Item *i);

IR::Item *copyIRVariable(IR::Variable *var);

IR::Item *generateNewIRVar(int id);

IR::Item *generateNewIRLabel(int id);

IR::Item *generateIRConstant(long long num);

IR::Instruction_declare *generateInt64DeclareIns(IR::Item *var);

IR::Instruction_op_assignment *generateVarShiftOpIns(IR::Item *dst, IR::Item *src, IR::Op dir);

IR::Instruction_op_assignment *generateOpIns(IR::Item *var, IR::Item *l, IR::Item *r, IR::Op opId);

IR::Instruction_cmp_assignment *generateCmpIns(IR::Item *var, IR::Item *l, IR::Item *r, IR::Cmp cmpId);

IR::Instruction_label *generateLabelIns(IR::Item *label);

#endif //LA_UTILS_H
