#ifndef LB_LB_H
#define LB_LB_H

#include <string>
#include <unordered_map>
#include <vector>

namespace LB {

    enum InsId {
        RET,
        VRET,
        ASS,
        LOAD,
        STORE,
        LENGTH,
        OP_ASS,
        LABEL,
        DBR,
        IF,
        WHILE,
        CONTINUE,
        BREAK,
        CALL,
        CALL_ASS,
        NEW_ARRAY,
        NEW_TUPLE,
        DECLARE,
        SCOPE
    };

    enum Op {
        ADD,
        SUB,
        MULTI,
        AND,
        LEFT,
        RIGHT,
        LE,
        LEQ,
        EQL,
        GR,
        GEQ
    };

    enum ItemId {
        LAB,
        VAR,
        CONST,
        TYPE
    };

    enum DataType {
        VOID,
        INT64,
        TUPLE,
        CODE
    };

    struct Item {
        virtual int getTypeId() {
            return -1;
        }
    };

    struct Label : Item {
        int getTypeId() override {
            return LAB;
        }

        std::string name;
    };

    struct Variable : Item {
        int getTypeId() override {
            return VAR;
        }

        std::string name;
    };

    struct Constant : Item {
        int getTypeId() override {
            return CONST;
        }

        long long num = 0;
    };

    struct T : Item {
        int getTypeId() override {
            return TYPE;
        }

        DataType type;
        int arrayDim = 0;
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
        std::vector<Item *> indices;

        int getTypeId() override {
            return LOAD;
        }
    };

    struct Instruction_store : Instruction {
        Item *src, *dst;
        std::vector<Item *> indices;

        int getTypeId() override {
            return STORE;
        }
    };

    struct Instruction_store_length : Instruction {
        Item *src, *dst, *t;

        int getTypeId() override {
            return LENGTH;
        }
    };

    struct Instruction_op_assignment : Instruction {
        Item *var, *l, *r;
        Op opId;

        int getTypeId() override {
            return OP_ASS;
        }
    };

    struct Instruction_label : Instruction {
        Item *label;

        int getTypeId() override {
            return LABEL;
        }
    };

    struct Instruction_dbr : Instruction {
        Item *label;

        int getTypeId() override {
            return DBR;
        }
    };

    struct Instruction_if : Instruction {
        Item *l, *r, *labelT, *labelF;
        Op opId;

        int getTypeId() override {
            return IF;
        }
    };

    struct Instruction_while : Instruction {
        Item *l, *r, *labelT, *labelF;
        Op opId;

        Item *condLabel = nullptr;
        int id = -1;

        int getTypeId() override {
            return WHILE;
        }
    };

    struct Instruction_continue : Instruction {
        Item *cond = nullptr;

        int getTypeId() override {
            return CONTINUE;
        }
    };

    struct Instruction_break : Instruction {
        Item *exit = nullptr;

        int getTypeId() override {
            return BREAK;
        }
    };

    struct Instruction_call : Instruction {
        bool isPrint = false;
        Item *u;
        std::vector<Item *> args;

        int getTypeId() override {
            return CALL;
        }
    };

    struct Instruction_call_assignment : Instruction {
        Item *var, *u;
        std::vector<Item *> args;

        int getTypeId() override {
            return CALL_ASS;
        }
    };

    struct Instruction_new_array : Instruction {
        Item *var;
        std::vector<Item *> args;

        int getTypeId() override {
            return NEW_ARRAY;
        }
    };

    struct Instruction_new_tuple : Instruction {
        Item *var, *t;

        int getTypeId() override {
            return NEW_TUPLE;
        }
    };

    struct Instruction_declare : Instruction {
        Item *type;
        std::vector<Item *> vars;

        int getTypeId() override {
            return DECLARE;
        }
    };

    typedef std::vector<Instruction *> Scope;

    struct Instruction_scope : Instruction {
        Scope *content;
        std::unordered_map<std::string, std::string> renameMap;

        int getTypeId() override {
            return SCOPE;
        }
    };

/*
* Function.
*/
    struct Function {
        std::string name;
        std::vector<Instruction *> instructions;
        Scope *root;
        std::unordered_map<std::string, std::string> renameMap;
        std::vector<Item *> typedVars;

        Item *res;

        std::unordered_map<std::string, T *> typeOfAllVars;

        int newVarId = 0;
        int newLabelId = 0;
    };

/*
* Program.
*/
    struct Program {
        std::vector<Function *> functions;
    };

    void freeInstruction(Instruction *ins);

    void freeProgram(Program &p);

    void parseTypeOfFunctionVars(Function *f);
}

#endif
