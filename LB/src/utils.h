//
// Created by dominic on 3/1/20.
//

#ifndef LB_UTILS_H
#define LB_UTILS_H

#include "LB.h"
#include "LA.h"

LB::Item *copyLBItem(LB::Item *src);

LB::Item *createNewVarWithId(const int &newVarId);

LB::Item *createNewVarWithSuffix(const std::string &suffix);

LA::Item *createLAVarWithName(const std::string &name);

LB::Item *createNewLabelWithId(const int &newLabelId);

LA::Item *copyLBItemToLA(LB::Item *src);

LA::Instruction* transferLBConditionToLA(LB::Item* l, LB::Item* r, LB::Op op);

LA::Instruction* transferLBOpAssToLA(LB::Item *var, LB::Item* l, LB::Item* r, LB::Op op);

#endif //LB_UTILS_H
