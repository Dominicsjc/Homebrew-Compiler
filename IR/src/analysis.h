#ifndef IR_ANALYSIS_H
#define IR_ANALYSIS_H

#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>

#include "IR.h"
#include "L3.h"


class analysis {
private:
    typedef struct basicBlock {
        int id;
        bool entry = false;
        bool exit = false;
        //bool marked = false;
        IR::Instruction *entry_label = nullptr;
        std::vector<IR::Instruction *> insVec;
        IR::Instruction *te = nullptr;
        //std::vector<basicBlock *> successors;
        std::vector<int> successors;
    }bb;

    typedef std::vector<bb *> trace;

    std::vector<bb *> cfg;

    std::unordered_set<int> bbList;

    std::vector<int> returnList;

    std::unordered_map<std::string, bb *> jumpTable;

    std::vector<trace> traces;

    void generateBb(IR::Function *f);

    void linkBbs();

    void tracing();

    void removeDbr();

    int countPath(int bbIndex, std::unordered_set<int> tmpList);

public:

    analysis() = delete;

    explicit analysis(IR::Function *f);

    ~analysis();

    void generateL3Instructions(bb *block, L3::Function *targetF, IR::Function *f); //Include explicit data types and transformation of other ins

    static void generateL3Instruction(IR::Instruction* ins, L3::Function* targetF, IR::Function *f);

    void transferToL3Function(L3::Function *targetF, IR::Function *f);

    static void transferNewArrayIns(IR::Instruction_new_array *ins, L3::Function *targetF);
};


#endif //IR_ANALYSIS_H
