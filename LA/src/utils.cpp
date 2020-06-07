//
// Created by dominic on 2/26/20.
//

#include "utils.h"

IR::Item *transferLAItemToIR(LA::Item *i) {
    switch (i->getTypeId()) {
        case LA::TYPE: {
            auto src = static_cast<LA::T *>(i);
            auto dst = new IR::T;
            dst->type = static_cast<IR::DataType>(src->type);
            dst->arrayDim = src->arrayDim;
            return dst;
        }
        case LA::CONST: {
            auto src = static_cast<LA::Constant *>(i);
            auto dst = new IR::Constant;
            dst->num = src->num;
            return dst;
        }
        case LA::VAR: {
            auto src = static_cast<LA::Variable *>(i);
            auto dst = new IR::Variable;
            dst->name = src->name;
            return dst;
        }
        case LA::LAB: {
            auto src = static_cast<LA::Label *>(i);
            auto dst = new IR::Label;
            dst->name = src->name;
            return dst;
        }
        default:
            return nullptr;
    }
}

IR::Item *copyIRVariable(IR::Variable *var) {
    auto res = new IR::Variable;
    res->name = var->name;
    return res;
}

IR::Item *generateNewIRVar(int id) {
    auto res = new IR::Variable;
    res->name = "myNewVar" + std::to_string(id);
    return res;
}

IR::Item *generateNewIRLabel(int id) {
    auto res = new IR::Label;
    res->name = ":my_new_label" + std::to_string(id);
    return res;
}

IR::Item *generateIRConstant(long long num) {
    auto number = new IR::Constant;
    number->num = num;
    return number;
}

IR::Instruction_declare *generateInt64DeclareIns(IR::Item *var) {
    auto declare_ins = new IR::Instruction_declare;
    auto type = new IR::T;
    type->type = IR::INT64;
    type->arrayDim = 0;
    declare_ins->type = type;
    declare_ins->var = var;
    return declare_ins;
}

IR::Instruction_op_assignment *generateVarShiftOpIns(IR::Item *dst, IR::Item *src, IR::Op dir) {
    auto op_assign_ins = new IR::Instruction_op_assignment;
    auto number = new IR::Constant;
    number->num = 1;

    op_assign_ins->var = dst;
    op_assign_ins->l = src;
    op_assign_ins->r = number;
    op_assign_ins->opId = dir;

    return op_assign_ins;
}

IR::Instruction_op_assignment *generateOpIns(IR::Item *var, IR::Item *l, IR::Item *r, IR::Op opId) {
    auto op_assign_ins = new IR::Instruction_op_assignment;

    op_assign_ins->var = var;
    op_assign_ins->l = l;
    op_assign_ins->r = r;
    op_assign_ins->opId = opId;

    return op_assign_ins;
}

IR::Instruction_cmp_assignment *generateCmpIns(IR::Item *var, IR::Item *l, IR::Item *r, IR::Cmp cmpId) {
    auto cmp_assign_ins = new IR::Instruction_cmp_assignment;

    cmp_assign_ins->var = var;
    cmp_assign_ins->l = l;
    cmp_assign_ins->r = r;
    cmp_assign_ins->cmpId = cmpId;

    return cmp_assign_ins;
}

IR::Instruction_label *generateLabelIns(IR::Item *label) {
    auto label_ins = new IR::Instruction_label;
    label_ins->label = label;
    return label_ins;
}