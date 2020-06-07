#ifndef L3_ANALYSIS_H
#define L3_ANALYSIS_H

#include "L3.h"
#include "L2.h"
#include <stack>
#include <unordered_set>
#include <unordered_map>

using namespace std;

class analysis {
private:
    enum nodeOp {
        RET, VRET, ASS, ADD, SUB, MULTI, AND, LEFT, RIGHT, LE, LEQ, EQL, GR, GEQ, LOAD, STORE, LABEL, BR, CALL, CALL_ASS, ITEM
    };

    typedef struct treeNode {
        L3::Item *node = nullptr;
        treeNode *lchild = nullptr;
        treeNode *rchild = nullptr;
        nodeOp op = nodeOp::ITEM;
        L3::Instruction *ins = nullptr;
        int contextId = -1;
        int insId = -1;
    } tree;

    typedef struct InstructionLivenessInfo {
        vector<InstructionLivenessInfo *> successor;
        unordered_set<string> gen;
        unordered_set<string> kill;
        unordered_set<string> in;
        unordered_set<string> out;
    } InsLiveness;

    vector<InsLiveness *> infos;

    unordered_map<string, InsLiveness *> labels;

    vector<tree *> trees;

    int calltimes = 0;

    string function_name;

    void generateTrees(L3::Function *f);

    bool tryMerge(int index);

    static bool relateVar(tree *a, tree *b, bool allow);

    void tileOneTree(L2::Function *f, tree *cur);

    void tiling(tree *cur, std::stack<L2::Instruction *> &insStack);

    static L2::Item * copyL3ItemtoL2(L3::Item * l3);

    void getGen(InsLiveness *insLiveness, L3::Instruction *ins);

    void getKill(InsLiveness *insLiveness, L3::Instruction *ins);

    void analyzeSuccessor(L3::Function *f);

    bool whetherUse(tree * i, tree *ptr);

public:

    analysis() = delete;

    explicit analysis(L3::Function *f);

    void computeInOut();

    void mergeTrees();

    void tileTrees(L2::Function *f);

    static void argsAssign(L3::Function *f, L2::Function *newF);

    ~analysis() = default;
};


#endif //L3_ANALYSIS_H
