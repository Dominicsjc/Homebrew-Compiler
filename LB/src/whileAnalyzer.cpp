//
// Created by dominic on 3/1/20.
//

#include "whileAnalyzer.h"
#include "utils.h"

whileAnalyzer::whileAnalyzer(LB::Function *f) {
    generateMaps(f);
    insMapping(f);
}

void whileAnalyzer::generateMaps(LB::Function *f) {
    int id = 0;
    for (auto ins : f->instructions) {
        if (ins->getTypeId() == LB::WHILE) {
            auto while_ins = static_cast<LB::Instruction_while *>(ins);
            while_ins->id = id;
            beginWhile.insert({static_cast<LB::Label *>(while_ins->labelT)->name, while_ins});
            endWhile.insert({static_cast<LB::Label *>(while_ins->labelF)->name, while_ins});
            while_ins->condLabel = createNewLabelWithId(f->newLabelId++);
            id++;
        }
    }
}

void whileAnalyzer::insMapping(LB::Function *f) {
    for (auto ins : f->instructions) {
        if (ins->getTypeId() == LB::CONTINUE) {
            auto continue_ins = static_cast<LB::Instruction_continue *>(ins);
            auto while_ins = loopStack.top();
            continue_ins->cond = copyLBItem(while_ins->condLabel);
        } else if (ins->getTypeId() == LB::BREAK) {
            auto break_ins = static_cast<LB::Instruction_break *>(ins);
            auto while_ins = loopStack.top();
            break_ins->exit = copyLBItem(while_ins->labelF);
        } else if (ins->getTypeId() == LB::WHILE) {
            auto while_ins = static_cast<LB::Instruction_while *>(ins);
            if (whileSeen.find(while_ins->id) == whileSeen.end()) {
                whileSeen.insert(while_ins->id);
                loopStack.push(while_ins);
            }
        } else if (ins->getTypeId() == LB::LABEL) {
            auto label_ins = static_cast<LB::Instruction_label *>(ins);
            auto label_name = static_cast<LB::Label *>(label_ins->label)->name;
            if (beginWhile.find(label_name) != beginWhile.end()) {
                if (whileSeen.find(beginWhile[label_name]->id) == whileSeen.end()) {
                    whileSeen.insert(beginWhile[label_name]->id);
                    loopStack.push(beginWhile[label_name]);
                }
            } else if (endWhile.find(label_name) != endWhile.end()) {
                loopStack.pop();
            }
        }

    }
}
