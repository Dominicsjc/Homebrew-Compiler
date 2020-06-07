//
// Created by dominic on 3/1/20.
//

#ifndef LB_SCOPETRANSLATOR_H
#define LB_SCOPETRANSLATOR_H

#include "LB.h"
#include <deque>

void translateMultiDeclare(LB::Scope *currentS);

void renameDeclare(LB::Scope *currentS, std::unordered_map<std::string, std::string> &renameMap, int newVarId);

void renameIns(LB::Scope *currentS);

void renameVar(LB::Item *i, std::deque<std::unordered_map<std::string, std::string>> &myMaps);

void removeNestedScopes(LB::Scope *currentS, std::vector<LB::Instruction *> &instructions);

void flatScopes(LB::Function *f);

#endif //LB_SCOPETRANSLATOR_H
