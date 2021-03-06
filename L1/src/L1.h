#ifndef L1_H
#define L1_H
#pragma once

#include <vector>
#include <string>

namespace L1 {

    enum Register {
        rdi, rax, rbx, rcx, rdx, rsi, rbp, rsp, r8, r9, r10, r11, r12, r13, r14, r15
    };

    enum InsId {
        RET, ASS, LOAD, STORE, AOP_ASS, SELFAOP, AOP_LOAD, AOP_STORE, SOP, CMP, LABEL, DJUMP, CJUMP, CISC, CALL
    };

    enum Aop {
        ADD, SUB, MULTI, AND
    };

    enum Sop {
        LEFT, RIGHT
    };

    enum Cmp {
        LE, LEQ, EQL
    };

    enum RuntimeType {
        PRINT, ALLOCATE, ARRAY
    };

    static std::string register_name[16]{"rdi", "rax", "rbx", "rcx", "rdx", "rsi", "rbp", "rsp", "r8", "r9", "r10", "r11", "r12",
                                  "r13", "r14", "r15"};

    static std::string register8b_name[16]{"dil", "al", "bl", "cl", "dl", "sil", "bpl", "rsp", "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b"};

    struct Item {
        virtual bool isConstant() {
            return false;
        }

        std::string labelName;
        Register r;
        bool isARegister;
    };

    struct Constant : Item {
        long long num = 0;

        bool isConstant() override {
            return true;
        }
    };

    /*
     * Instruction interface.
     */
    struct Instruction {
        virtual int getTypeId() {
            return -1;
        }
    };

    /*
     * Instructions.
     */
    struct Instruction_ret : Instruction {
        int getTypeId() override {
            return RET;
        }
    };

    struct Instruction_assignment : Instruction {
        Item *src, *dst;

        int getTypeId() override {
            return ASS;
        }
    };

    struct Instruction_load : Instruction {
        Item *src, *dst, *offset;

        int getTypeId() override {
            return LOAD;
        }
    };

    struct Instruction_store : Instruction {
        Item *src, *dst, *offset;

        int getTypeId() override {
            return STORE;
        }
    };

    struct Instruction_aop_assignment : Instruction {
        Item *src,*dst;
        Aop opId;

        int getTypeId() override {
            return AOP_ASS;
        }
    };

    struct Instruction_selfAop : Instruction {
        Item *reg;
        Aop opId;

        int getTypeId() override {
            return SELFAOP;
        }
    };

    struct Instruction_aop_load : Instruction {
        Item *src,*dst, *offset;
        Aop opId;

        int getTypeId() override {
            return AOP_LOAD;
        }
    };

    struct Instruction_aop_store : Instruction {
        Item *src,*dst, *offset;
        Aop opId;

        int getTypeId() override {
            return AOP_STORE;
        }
    };

    struct Instruction_sop : Instruction {
        Item *reg, *offset;
        Sop opId;

        int getTypeId() override {
            return SOP;
        }
    };

    struct Instruction_cmp : Instruction {
        Item *reg, *l, *r;
        Cmp cmpId;

        int getTypeId() override {
            return CMP;
        }
    };

    struct Instruction_label : Instruction {
        Item *label;

        int getTypeId() override {
            return LABEL;
        }
    };

    struct Instruction_djump : Instruction {
        Item *dst;

        int getTypeId() override {
            return DJUMP;
        }
    };

    struct Instruction_cjump : Instruction {
        Item *dst, *l, *r;
        Cmp cmpId;

        int getTypeId() override {
            return CJUMP;
        }
    };

    struct Instruction_CISC : Instruction {
        Item *reg, *base, *index, *scale;

        int getTypeId() override {
            return CISC;
        }
    };

    struct Instruction_call : Instruction {
        bool isRuntime;
        Item *u, *num;
        RuntimeType runtimeId;

        int getTypeId() override {
            return CALL;
        }
    };

    /*
     * Function.
     */
    struct Function {
        std::string name;
        int64_t arguments;
        int64_t locals;
        std::vector<Instruction *> instructions;
    };

    /*
     * Program.
     */
    struct Program {
        std::string entryPointLabel;
        std::vector<Function *> functions;
    };

    void printItem(std::ostream &out, Item *i);

    void assemblyItem(std::ostream &out, Item *i);

    void assembly8bItem(std::ostream &out, Item *i);

    void assemblyInstruction(std::ostream &out, Instruction* ins, Function* f);

    void assemblyFunction(std::ostream &out, Function* f);

    void printAop(std::ostream &out, Aop opId);

    void assemblyAop(std::ostream &out, Aop opId);

    void printSelfAop(std::ostream &out, Aop opId);

    void assemblySelfAop(std::ostream &out, Aop opId);

    void printSop(std::ostream &out, Sop opId);

    void assemblySop(std::ostream &out, Sop opId);

    void printCmp(std::ostream &out, Cmp cmpId);

    void assemblyCmp(std::ostream &out, Cmp cmpId, bool reverse);

    void assemblyCjump(std::ostream &out, Cmp cmpId, bool reverse);

    void assemblyPrologue(std::ostream &out, Function* f);

    void assemblyReturn(std::ostream &out, Function* f);

    void printRuntime(std::ostream &out, RuntimeType runtimeId);

    void assemblyRuntime(std::ostream &out, RuntimeType runtimeId);

    bool simpleCompute(Constant *l, Constant *r, Cmp cmpId);
}

#endif //L1_H
