#ifndef L2_ANALYSIS_H
#define L2_ANALYSIS_H

#include "L2.h"
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <stack>
#include <vector>
#include <queue>
#include "L2.h"

using namespace std;

static const int calleeSavedRegisters[] = {
        L2::rax,
        L2::r12,
        L2::r13,
        L2::r14,
        L2::r15,
        L2::rbp,
        L2::rbx
};

static const int callerSavedRegisters[] = {
        L2::r10,
        L2::r11,
        L2::r8,
        L2::r9,
        L2::rax,
        L2::rcx,
        L2::rdi,
        L2::rdx,
        L2::rsi
};

static const int argumentRegisters[] = {
        L2::rdi,
        L2::rsi,
        L2::rdx,
        L2::rcx,
        L2::r8,
        L2::r9,
};

static const int generalPurposeRegisters[] = {
        L2::rax,
        L2::rdi,
        L2::rsi,
        L2::rdx,
        L2::rcx,
        L2::rbp,
        L2::rbx,
        L2::r8,
        L2::r9,
        L2::r10,
        L2::r11,
        L2::r12,
        L2::r13,
        L2::r14,
        L2::r15
};

class Analysis {
private:
    typedef struct InstructionLivenessInfo {
        vector<InstructionLivenessInfo *> successor;
        unordered_set<int> gen;
        unordered_set<int> kill;
        unordered_set<int> in;
        unordered_set<int> out;
    } InsLiveness;

    typedef pair<int, unordered_set<int>> node;

    vector<InsLiveness *> infos;

    unordered_set<int> sopSrcVar;

    unordered_map<string, InsLiveness *> labels;

    unordered_map<string, int> varNames;

    unordered_map<int, string> varIndex;

    unordered_map<int, unordered_set<int>> interferenceGraph;

    unordered_map<int, int> colorOfNodes; //<index, color>

    stack<node, vector<node>> interferenceStack;

    vector<int> spillList;

    void getGen(InsLiveness *insLiveness, L2::Instruction *ins);

    void getKill(InsLiveness *insLiveness, L2::Instruction *ins);

    void analyzeSuccessor(L2::Function *f);

    void printGenKill(ostream &out);

    void printHashSet(ostream &out, unordered_set<int> &hashSet);

    void generateNodes();

    void coloring();

    void removeAndPutNode();

    void rebuildGraph();

    bool tryColoring(int color, node &cur);

    void replaceVarWithColor(L2::Item * var);

public:
    Analysis() = delete;

    explicit Analysis(L2::Function *f);

    void computeInOut();

    void printInOut(ostream &out);

    void computeInterferenceGraph();

    void printInterferenceGraph(ostream &out);

    void printNameWithId(ostream &out, int id);

    bool allocator(L2::Function *f);

    void replaceAllWithColor(L2::Function *f);

    void batchSpill(L2::Function *f, int &prefixIndex);

    ~Analysis();
};

#endif //L2_ANALYSIS_H
