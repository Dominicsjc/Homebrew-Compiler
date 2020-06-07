//
// Created by dominic on 3/1/20.
//

#include "utils.h"

LB::Item *copyLBItem(LB::Item *src) {
    switch (src->getTypeId()) {
        case LB::LAB: {
            auto label = static_cast<LB::Label *>(src);
            auto dst = new LB::Label;
            dst->name = label->name;
            return dst;
        }
        case LB::VAR: {
            auto var = static_cast<LB::Variable *>(src);
            auto dst = new LB::Variable;
            dst->name = var->name;
            return dst;
        }
        case LB::CONST: {
            auto number = static_cast<LB::Constant *>(src);
            auto dst = new LB::Constant;
            dst->num = number->num;
            return dst;
        }
        case LB::TYPE: {
            auto type = static_cast<LB::T *>(src);
            auto dst = new LB::T;
            dst->type = type->type;
            dst->arrayDim = type->arrayDim;
            return dst;
        }
        default:
            return nullptr;
    }
}

LB::Item *createNewVarWithId(const int &newVarId) {
    auto var = new LB::Variable;
    var->name = "myNewVar" + std::to_string(newVarId);
    return var;
}

LB::Item *createNewVarWithSuffix(const std::string &suffix) {
    auto var = new LB::Variable;
    var->name = "LBNewVar" + suffix;
    return var;
}

LA::Item *createLAVarWithName(const std::string &name) {
    auto var = new LA::Variable;
    var->name = name;
    return var;
}

LB::Item *createNewLabelWithId(const int &newLabelId) {
    auto label = new LB::Label;
    label->name = ":LB_new_label" + std::to_string(newLabelId);
    return label;
}

LA::Item *copyLBItemToLA(LB::Item *src) {
    switch (src->getTypeId()) {
        case LB::LAB: {
            auto label = static_cast<LB::Label *>(src);
            auto dst = new LA::Label;
            dst->name = label->name;
            return dst;
        }
        case LB::VAR: {
            auto var = static_cast<LB::Variable *>(src);
            auto dst = new LA::Variable;
            dst->name = var->name;
            return dst;
        }
        case LB::CONST: {
            auto number = static_cast<LB::Constant *>(src);
            auto dst = new LA::Constant;
            dst->num = number->num;
            return dst;
        }
        case LB::TYPE: {
            auto type = static_cast<LB::T *>(src);
            auto dst = new LA::T;
            dst->type = static_cast<LA::DataType>(type->type);
            dst->arrayDim = type->arrayDim;
            return dst;
        }
        default:
            return nullptr;
    }
}

LA::Instruction *transferLBConditionToLA(LB::Item *l, LB::Item *r, LB::Op op) {
    switch (op) {
        case LB::Op::ADD:
        case LB::Op::SUB:
        case LB::Op::MULTI:
        case LB::Op::AND:
        case LB::Op::LEFT:
        case LB::Op::RIGHT: {
            auto targetInstruction = new LA::Instruction_op_assignment();
            targetInstruction->opId = static_cast<LA::Op>(op);
            targetInstruction->l = copyLBItemToLA(l);
            targetInstruction->r = copyLBItemToLA(r);
            targetInstruction->var = createLAVarWithName("condVar");
            return targetInstruction;
        }
        case LB::Op::LE:
        case LB::Op::LEQ:
        case LB::Op::EQL:
        case LB::Op::GR:
        case LB::Op::GEQ: {
            auto targetInstruction = new LA::Instruction_cmp_assignment();
            targetInstruction->cmpId = static_cast<LA::Cmp>(op - 6);
            targetInstruction->l = copyLBItemToLA(l);
            targetInstruction->r = copyLBItemToLA(r);
            targetInstruction->var = createLAVarWithName("condVar");
            return targetInstruction;
        }
        default: {
            exit(-1);
        }
    }
}

LA::Instruction *transferLBOpAssToLA(LB::Item *var, LB::Item *l, LB::Item *r, LB::Op op) {
    switch (op) {
        case LB::Op::ADD:
        case LB::Op::SUB:
        case LB::Op::MULTI:
        case LB::Op::AND:
        case LB::Op::LEFT:
        case LB::Op::RIGHT: {
            auto targetInstruction = new LA::Instruction_op_assignment();
            targetInstruction->opId = static_cast<LA::Op>(op);
            targetInstruction->l = copyLBItemToLA(l);
            targetInstruction->r = copyLBItemToLA(r);
            targetInstruction->var = copyLBItemToLA(var);
            return targetInstruction;
        }
        case LB::Op::LE:
        case LB::Op::LEQ:
        case LB::Op::EQL:
        case LB::Op::GR:
        case LB::Op::GEQ: {
            auto targetInstruction = new LA::Instruction_cmp_assignment();
            targetInstruction->cmpId = static_cast<LA::Cmp>(op - 6);
            targetInstruction->l = copyLBItemToLA(l);
            targetInstruction->r = copyLBItemToLA(r);
            targetInstruction->var = copyLBItemToLA(var);
            return targetInstruction;
        }
        default: {
            exit(-1);
        }
    }
}