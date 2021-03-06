#ifndef L3_L3_H
#define L3_L3_H

#include <string>
#include <vector>

namespace L3 {

    enum InsId {
        RET, VRET, ASS, LOAD, STORE, OP_ASS, CMP_ASS, LABEL, DBR, VBR, CALL, CALL_ASS
    };

    enum Op {
        ADD, SUB, MULTI, AND, LEFT, RIGHT
    };

    enum Cmp {
        LE, LEQ, EQL, GR, GEQ
    };

    enum RuntimeType {
        PRINT, ALLOCATE, ARRAY
    };

    enum ItemType {
        LAB, VAR
    };

    struct Item {
        virtual bool isConstant() {
            return false;
        }

        std::string name;
        ItemType type;
    };

    struct Constant : Item {
        bool isConstant() override {
            return true;
        }

        long long num = 0;
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

    struct Instruction_value_ret : Instruction {
        Item *t;

        int getTypeId() override {
            return VRET;
        }
    };

    struct Instruction_assignment : Instruction {
        Item *src, *dst;

        int getTypeId() override {
            return ASS;
        }
    };

    struct Instruction_load : Instruction {
        Item *src, *dst;

        int getTypeId() override {
            return LOAD;
        }
    };

    struct Instruction_store : Instruction {
        Item *src, *dst;

        int getTypeId() override {
            return STORE;
        }
    };

    struct Instruction_op_assignment : Instruction {
        Item *var, *l, *r;
        Op opId;

        int getTypeId() override {
            return OP_ASS;
        }
    };

    struct Instruction_cmp_assignment : Instruction {
        Item *var, *l, *r;
        Cmp cmpId;

        int getTypeId() override {
            return CMP_ASS;
        }
    };

    struct Instruction_label : Instruction {
        Item *label;

        int getTypeId() override {
            return LABEL;
        }
    };

    struct Instruction_dbr : Instruction {
        Item *dst;

        int getTypeId() override {
            return DBR;
        }
    };

    struct Instruction_vbr : Instruction {
        Item *dst, *var;

        int getTypeId() override {
            return VBR;
        }
    };

    struct Instruction_call : Instruction {
        bool isRuntime;
        Item *u;
        std::vector<Item *> args;
        RuntimeType runtimeId;

        int getTypeId() override {
            return CALL;
        }
    };

    struct Instruction_call_assignment : Instruction {
        bool isRuntime;
        Item *var, *u;
        std::vector<Item *> args;
        RuntimeType runtimeId;

        int getTypeId() override {
            return CALL_ASS;
        }
    };

    /*
     * Function.
     */
    struct Function {
        std::string name;
        std::vector<Instruction *> instructions;
        std::vector<Item *> vars;
    };

    /*
     * Program.
     */
    struct Program {
        std::vector<Function *> functions;
    };

    void freeInstruction(Instruction *ins);

    void freeProgram(Program &p);
}


#endif
