//
// Created by dominic on 3/1/20.
//

#ifndef LB_WHILEANALYZER_H
#define LB_WHILEANALYZER_H

#include <unordered_map>
#include <unordered_set>
#include <stack>
#include "LB.h"

class whileAnalyzer {
private:
    //std::unordered_map<int, LB::Instruction_while *> idToWhile;

    //std::unordered_map<std::string, int> labelToId;

    std::unordered_map<std::string, LB::Instruction_while *> beginWhile;

    std::unordered_map<std::string, LB::Instruction_while *> endWhile;

    std::unordered_set<int> whileSeen;

    std::stack<LB::Instruction_while *> loopStack;

    void generateMaps(LB::Function *f);

    void insMapping(LB::Function *f);

public:
    whileAnalyzer() = delete;

    explicit whileAnalyzer(LB::Function *f);

    ~whileAnalyzer() = default;
};


#endif //LB_WHILEANALYZER_H
