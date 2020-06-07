#ifndef L2_SPILLER_H
#define L2_SPILLER_H

#include <iostream>
#include "L2.h"

using namespace std;

enum spillType {NONE, GEN, KILL, BOTH};

void spill(L2::Function *f);

L2::Item * createReplacedVar(L2::Item *spillerVar, int index);

bool checkVar(L2::Item *i, L2::Item *targetVar);

bool checkIns(L2::Instruction *ins, L2::Item *targetVar);

int checkAndReplaceIns(L2::Instruction *ins, L2::Item *targetVar, L2::Item *spillerVar, int index);

L2::Instruction_load *createTmpLoad(L2::Function *f, int index);

L2::Instruction_store *createTmpStore(L2::Function *f, int index);

#endif //L2_SPILLER_H
