#include <algorithm>
#include "analysis.h"

analysis::analysis(L3::Function *f) {
    for (auto ins : f->instructions) {
        auto info = new InsLiveness();
        if (ins->getTypeId() == L3::LABEL) {
            auto label_ins = static_cast<L3::Instruction_label *>(ins);
            labels.emplace(make_pair(label_ins->label->name, info));
        }
        getGen(info, ins);
        getKill(info, ins);
        infos.push_back(info);
    }
    analyzeSuccessor(f);
    // printGenKill(cerr);
    computeInOut();
    function_name = f->name;
    generateTrees(f);
}

void analysis::getGen(InsLiveness *info, L3::Instruction *ins) {
    switch (ins->getTypeId()) {
        case L3::InsId::RET: {
            break;
        }
        case L3::InsId::VRET: {
            auto vret_ins = static_cast<L3::Instruction_value_ret *>(ins);
            info->gen.insert(vret_ins->t->name);
            break;
        }
        case L3::InsId::ASS: {
            auto ass_ins = static_cast<L3::Instruction_assignment *>(ins);
            if (!ass_ins->src->isConstant() && ass_ins->src->type == L3::VAR) {
                info->gen.insert(ass_ins->src->name);
            }
            break;
        }
        case L3::InsId::LOAD: {
            auto load_ins = static_cast<L3::Instruction_load *>(ins);

            info->gen.insert(load_ins->src->name);

            break;
        }
        case L3::InsId::STORE: {
            auto store_ins = static_cast<L3::Instruction_store *>(ins);
            if (!store_ins->src->isConstant() && store_ins->src->type == L3::VAR) {
                info->gen.insert(store_ins->src->name);
            }
            info->gen.insert(store_ins->dst->name);
            break;
        }
        case L3::InsId::OP_ASS: {
            auto op_assign_ins = static_cast<L3::Instruction_op_assignment *>(ins);
            if (!op_assign_ins->l->isConstant()) {
                info->gen.insert(op_assign_ins->l->name);
            }
            if (!op_assign_ins->r->isConstant()) {
                info->gen.insert(op_assign_ins->r->name);
            }
            break;
        }
        case L3::InsId::CMP_ASS: {
            auto cmp_ass_ins = static_cast<L3::Instruction_cmp_assignment *>(ins);
            if (!cmp_ass_ins->l->isConstant()) {
                info->gen.insert(cmp_ass_ins->l->name);
            }
            if (!cmp_ass_ins->r->isConstant()) {
                info->gen.insert(cmp_ass_ins->r->name);
            }
            break;
        }
        case L3::InsId::LABEL: {
            break;
        }
        case L3::InsId::DBR: {
            break;
        }
        case L3::InsId::VBR: {
            auto vbr_ins = static_cast<L3::Instruction_vbr *>(ins);

            info->gen.insert(vbr_ins->var->name);

            break;
        }
        case L3::InsId::CALL: {
            auto call_ins = static_cast<L3::Instruction_call *>(ins);

            for (auto arg : call_ins->args) {
                if (!arg->isConstant()) {
                    info->gen.insert(arg->name);
                }
            }

            if (!call_ins->isRuntime) {
                if (call_ins->u->type == L3::VAR) {
                    info->gen.insert(call_ins->u->name);
                }
            }
            break;
        }
        case L3::InsId::CALL_ASS: {
            auto call_ass_ins = static_cast<L3::Instruction_call_assignment *>(ins);

            for (auto arg : call_ass_ins->args) {
                if (!arg->isConstant()) {
                    info->gen.insert(arg->name);
                }
            }

            if (!call_ass_ins->isRuntime) {
                if (call_ass_ins->u->type == L3::VAR) {
                    info->gen.insert(call_ass_ins->u->name);
                }
            }
            break;
        }
        default:
            break;
    }

}

void analysis::getKill(InsLiveness *info, L3::Instruction *ins) {
    switch (ins->getTypeId()) {
        case L3::InsId::RET: {
            break;
        }
        case L3::InsId::VRET: {
            break;
        }
        case L3::InsId::ASS: {
            auto ass_ins = static_cast<L3::Instruction_assignment *>(ins);

            info->kill.insert(ass_ins->dst->name);

            break;
        }
        case L3::InsId::LOAD: {
            auto load_ins = static_cast<L3::Instruction_load *>(ins);

            info->kill.insert(load_ins->dst->name);

            break;
        }
        case L3::InsId::STORE: {
            break;
        }
        case L3::InsId::OP_ASS: {
            auto op_assign_ins = static_cast<L3::Instruction_op_assignment *>(ins);

            info->kill.insert(op_assign_ins->var->name);

            break;
        }
        case L3::InsId::CMP_ASS: {
            auto cmp_ass_ins = static_cast<L3::Instruction_cmp_assignment *>(ins);

            info->kill.insert(cmp_ass_ins->var->name);

            break;
        }
        case L3::InsId::LABEL: {
            break;
        }
        case L3::InsId::DBR: {
            break;
        }
        case L3::InsId::VBR: {
            break;
        }
        case L3::InsId::CALL: {
            break;
        }
        case L3::InsId::CALL_ASS: {
            auto call_ass_ins = static_cast<L3::Instruction_call_assignment *>(ins);

            info->kill.insert(call_ass_ins->var->name);

            break;
        }
        default:
            break;
    }

}

void analysis::analyzeSuccessor(L3::Function *f) {
    for (int i = 0; i < infos.size(); i++) {
        switch (f->instructions[i]->getTypeId()) {
            case L3::DBR: {
                auto dbr_ins = static_cast<L3::Instruction_dbr *>(f->instructions[i]);
                infos[i]->successor.push_back(labels.find(dbr_ins->dst->name)->second);
                break;
            }
            case L3::RET: {
                break;
            }
            case L3::VRET: {
                break;
            }
            case L3::VBR: {
                auto vbr_ins = static_cast<L3::Instruction_vbr *>(f->instructions[i]);
                infos[i]->successor.push_back(labels.find(vbr_ins->dst->name)->second);
                if (i != f->instructions.size() - 1) {
                    infos[i]->successor.push_back(infos[i + 1]);
                }
                break;
            }
            case L3::CALL: {
                auto call_ins = static_cast<L3::Instruction_call *>(f->instructions[i]);
                if (call_ins->isRuntime && call_ins->runtimeId == L3::ARRAY) {

                } else if (i != f->instructions.size() - 1) {
                    infos[i]->successor.push_back(infos[i + 1]);
                }
                break;
            }
            default: {
                if (i != f->instructions.size() - 1) {
                    infos[i]->successor.push_back(infos[i + 1]);
                }
            }
        }
    }
}

void analysis::computeInOut() {
    bool needNextIteration = true;

    while (needNextIteration) {
        needNextIteration = false;

        for (auto info: infos) {
            unordered_set<string> newIn;

            for (const auto &v: info->out) { // out[i] - kill[i]
                if (info->kill.find(v) == info->kill.end()) { // if not in kill[i]
                    newIn.insert(v);
                }
            }

            for (const auto &v: info->gen) { // union with gen[i]
                newIn.insert(v);
            }

            if (newIn != info->in) {
                needNextIteration = true;

                info->in.clear();
                for (const auto &v: newIn) {
                    info->in.insert(v);
                }
                // info->in = newIn
            }

            unordered_set<string> newOut;

            for (auto successorInfo: info->successor) {

                for (const auto &v: successorInfo->in) {
                    newOut.insert(v);
                }

            }

            if (newOut != info->out) {
                needNextIteration = true;

                info->out.clear();
                for (const auto &v: newOut) {
                    info->out.insert(v);
                }
                // info->out = newOut
            }
        }
    }
}

void analysis::generateTrees(L3::Function *f) {
    int contextId = 0;
    int id = 0;
    for (auto ins : f->instructions) {
        int type = ins->getTypeId();
        switch (type) {
            case L3::RET: {
                tree *newT = new tree;
                newT->op = nodeOp::RET;
                newT->contextId = contextId;
                newT->insId = id;
                trees.emplace_back(newT);

                break;
            }
            case L3::VRET: {
                auto vret_ins = static_cast<L3::Instruction_value_ret *>(ins);

                tree *newT = new tree;
                newT->op = nodeOp::VRET;
                newT->insId = id;

                tree *newL = new tree;
                newL->node = vret_ins->t;
                newL->op = nodeOp::ITEM;
                newL->contextId = contextId;

                newT->lchild = newL;
                newT->contextId = contextId;
                trees.emplace_back(newT);

                break;
            }
            case L3::ASS: {
                auto ass_ins = static_cast<L3::Instruction_assignment *>(ins);

                tree *newT = new tree;
                newT->op = nodeOp::ASS;
                newT->insId = id;

                tree *newL = new tree;
                newL->node = ass_ins->src;
                newL->op = nodeOp::ITEM;
                newL->contextId = contextId;

                newT->node = ass_ins->dst;
                newT->lchild = newL;
                newT->contextId = contextId;
                trees.emplace_back(newT);

                break;
            }
            case L3::LOAD: {
                auto load_ins = static_cast<L3::Instruction_load *>(ins);

                tree *newT = new tree;
                newT->op = nodeOp::LOAD;
                newT->insId = id;

                tree *newL = new tree;
                newL->node = load_ins->src;
                newL->op = nodeOp::ITEM;
                newL->contextId = contextId;

                newT->node = load_ins->dst;
                newT->lchild = newL;
                newT->contextId = contextId;
                trees.emplace_back(newT);

                break;
            }
            case L3::STORE: {
                auto store_ins = static_cast<L3::Instruction_store *>(ins);

                tree *newT = new tree;
                newT->op = nodeOp::STORE;
                newT->insId = id;

                tree *newL = new tree;
                newL->node = store_ins->src;
                newL->op = nodeOp::ITEM;
                newL->contextId = contextId;

                tree *newR = new tree;
                newR->node = store_ins->dst;
                newR->op = nodeOp::ITEM;
                newR->contextId = contextId;

                newT->lchild = newL;
                newT->rchild = newR;
                newT->contextId = contextId;
                trees.emplace_back(newT);

                break;
            }
            case L3::OP_ASS: {
                auto op_ass_ins = static_cast<L3::Instruction_op_assignment *>(ins);

                tree *newT = new tree;
                newT->op = static_cast<nodeOp>(op_ass_ins->opId + 3);
                newT->insId = id;

                tree *newL = new tree;
                newL->node = op_ass_ins->l;
                newL->op = nodeOp::ITEM;
                newL->contextId = contextId;

                tree *newR = new tree;
                newR->node = op_ass_ins->r;
                newR->op = nodeOp::ITEM;
                newR->contextId = contextId;

                newT->node = op_ass_ins->var;
                newT->lchild = newL;
                newT->rchild = newR;
                newT->contextId = contextId;
                trees.emplace_back(newT);

                break;
            }
            case L3::CMP_ASS: {
                auto cmp_ass_ins = static_cast<L3::Instruction_cmp_assignment *>(ins);

                tree *newT = new tree;
                newT->op = static_cast<nodeOp>(cmp_ass_ins->cmpId + 9);
                newT->insId = id;

                tree *newL = new tree;
                newL->node = cmp_ass_ins->l;
                newL->op = nodeOp::ITEM;
                newL->contextId = contextId;

                tree *newR = new tree;
                newR->node = cmp_ass_ins->r;
                newR->op = nodeOp::ITEM;
                newR->contextId = contextId;

                newT->node = cmp_ass_ins->var;
                newT->lchild = newL;
                newT->rchild = newR;
                newT->contextId = contextId;
                trees.emplace_back(newT);

                break;
            }
            case L3::LABEL: {
                auto label_ins = static_cast<L3::Instruction_label *>(ins);

                tree *newT = new tree;
                newT->op = nodeOp::LABEL;
                newT->node = label_ins->label;
                newT->contextId = -1;
                newT->insId = id;
                contextId++;
                trees.emplace_back(newT);

                break;
            }
            case L3::DBR: {
                auto dbr_ins = static_cast<L3::Instruction_dbr *>(ins);

                tree *newT = new tree;
                newT->op = nodeOp::BR;
                newT->node = dbr_ins->dst;
                newT->insId = id;
                newT->contextId = contextId;
                contextId++;
                trees.emplace_back(newT);

                break;
            }
            case L3::VBR: {
                auto vbr_ins = static_cast<L3::Instruction_vbr *>(ins);

                tree *newT = new tree;
                newT->op = nodeOp::BR;
                newT->node = vbr_ins->dst;
                newT->insId = id;

                tree *newL = new tree;
                newL->node = vbr_ins->var;
                newL->op = nodeOp::ITEM;
                newL->contextId = contextId;

                newT->lchild = newL;
                newT->contextId = contextId;
                contextId++;
                trees.emplace_back(newT);

                break;
            }
            case L3::CALL: {
                tree *newT = new tree;
                newT->op = nodeOp::CALL;
                newT->ins = ins;
                newT->insId = id;
                newT->contextId = contextId;
                trees.emplace_back(newT);

                break;
            }
            case L3::CALL_ASS: {
                tree *newT = new tree;
                newT->op = nodeOp::CALL_ASS;
                newT->ins = ins;
                newT->insId = id;
                newT->contextId = contextId;
                trees.emplace_back(newT);

                break;
            }
            default:;
        }
        id++;
    }
}

void analysis::mergeTrees() {
    int i = 0;
    while (i < trees.size()) {
        if (tryMerge(i)) {
        } else {
            i++;
        }
    }
}

bool analysis::tryMerge(int index) {
    if (!trees[index]->node || trees[index]->node->type != L3::VAR) {
        return false;
    }
    int ptr = index + 1;
    while (ptr < trees.size()) {
        if (trees[ptr]->contextId != trees[index]->contextId) {
            return false;
        }
        if (trees[ptr]->op == nodeOp::CALL_ASS) {
            auto call_ass_ins = static_cast<L3::Instruction_call_assignment *>(trees[ptr]->ins);

            if (call_ass_ins->var->name == trees[index]->node->name || (!call_ass_ins->isRuntime && call_ass_ins->u->type == L3::VAR && call_ass_ins->u->name == trees[index]->node->name) ) {
                return false;
            }

            for (auto args : call_ass_ins->args) {
                if (!args->isConstant() && args->name == trees[index]->node->name) {
                    return false;
                }
            }
        } else if (trees[ptr]->op == nodeOp::CALL) {
            auto call_ins = static_cast<L3::Instruction_call *>(trees[ptr]->ins);

            if (!call_ins->isRuntime && call_ins->u->type == L3::VAR && call_ins->u->name == trees[index]->node->name) {
                return false;
            }

            for (auto args : call_ins->args) {
                if (!args->isConstant() && args->name == trees[index]->node->name) {
                    return false;
                }
            }
        }
        if (infos[trees[ptr]->insId]->kill.find(trees[index]->node->name) != infos[trees[ptr]->insId]->kill.end()) {
            if (relateVar(trees[index], trees[ptr], true)) {
                trees.erase(trees.begin() + index);
                return true;
            } else {
                return false;
            }
        }
        if (infos[trees[ptr]->insId]->out.find(trees[index]->node->name) !=
            infos[trees[ptr]->insId]->out.end()) {
            if (relateVar(trees[index], trees[ptr], false)) {
                return false;
            } else {
                if (whetherUse(trees[index]->lchild, trees[ptr]) || whetherUse(trees[index]->rchild, trees[ptr])) {
                    return false;
                } else {
                    ptr++;
                }
            }
        } else if (whetherUse(trees[index]->lchild, trees[ptr]) || whetherUse(trees[index]->rchild, trees[ptr])) {
            return false;
        } else if (relateVar(trees[index], trees[ptr], true)) {
            trees.erase(trees.begin() + index);
            return true;
        } else {
            ptr++;
        }
    }
    return false;
}

bool analysis::relateVar(tree *a, tree *b, bool allow) {
    if (!b) {
        return false;
    } else if (b->op == nodeOp::ITEM) {
        if (b->node && !b->node->isConstant() && b->node->type == L3::VAR && b->node->name == a->node->name) {
            if (allow) {
                b->op = a->op;
                b->node = a->node;
                b->lchild = a->lchild;
                b->rchild = a->rchild;
            }
            return true;
        } else {
            return false;
        }
    } else {
        return relateVar(a, b->lchild, allow) || relateVar(a, b->rchild, allow);
    }
}

void analysis::tileTrees(L2::Function *f) {
    for (auto tree : trees) {
        tileOneTree(f, tree);
    }
}

void analysis::tileOneTree(L2::Function *f, tree *cur) {
    std::stack<L2::Instruction *> insStack;

    tiling(cur, insStack);

    while (!insStack.empty()) {
        f->instructions.push_back(insStack.top());
        insStack.pop();
    }
}

void analysis::tiling(analysis::tree *cur, std::stack<L2::Instruction *> &insStack) {
    if (!cur)
        return;
    switch (cur->op) {
        case ITEM: {
            break;
        }
        case RET: {
            auto ret_ins = new L2::Instruction_ret;
            insStack.push(ret_ins);
            break;
        }
        case VRET: {
            auto ass_ins = new L2::Instruction_assignment;
            auto dst = new L2::Item;
            dst->type = L2::REG;
            dst->r = L2::rax;
            auto src = copyL3ItemtoL2(cur->lchild->node);
            ass_ins->dst = dst;
            ass_ins->src = src;
            auto ret_ins = new L2::Instruction_ret;

            insStack.push(ret_ins);
            insStack.push(ass_ins);

            tiling(cur->lchild, insStack);

            break;
        }
        case ASS: {
            if (!cur->lchild->node->isConstant() && cur->node->name == cur->lchild->node->name) {
                tiling(cur->lchild, insStack);
            } else {
                auto ass_ins = new L2::Instruction_assignment;
                auto dst = new L2::Item;
                dst->type = L2::VAR;
                dst->name = cur->node->name;
                ass_ins->dst = dst;
                if (cur->lchild->node->isConstant()) {
                    auto number = static_cast<L3::Constant *>(cur->lchild->node);
                    auto src = new L2::Constant;
                    src->num = number->num;
                    ass_ins->src = src;
                } else {
                    auto src = copyL3ItemtoL2(cur->lchild->node);
                    ass_ins->src = src;
                }

                insStack.push(ass_ins);
                tiling(cur->lchild, insStack);
            }

            break;
        }
        case ADD: {
            bool finish = false;
            if (!cur->lchild->node->isConstant() && !cur->rchild->node->isConstant()) {
                if (cur->lchild->op == nodeOp::MULTI) {
                    if (cur->lchild->lchild->node->isConstant() && !cur->lchild->rchild->node->isConstant()) {
                        auto number = static_cast<L3::Constant *>(cur->lchild->lchild->node);
                        if (number->num == 1 || number->num == 2 || number->num == 4 || number->num == 8) {
                            auto CISC_ins = new L2::Instruction_CISC;
                            auto reg = copyL3ItemtoL2(cur->node);
                            auto base = copyL3ItemtoL2(cur->rchild->node);
                            auto index = copyL3ItemtoL2(cur->lchild->rchild->node);
                            auto scale = new L2::Constant;
                            scale->num = number->num;
                            CISC_ins->reg = reg;
                            CISC_ins->base = base;
                            CISC_ins->index = index;
                            CISC_ins->scale = scale;

                            finish = true;

                            insStack.push(CISC_ins);

                            tiling(cur->rchild, insStack);
                            tiling(cur->lchild->lchild, insStack);
                            tiling(cur->lchild->rchild, insStack);
                        }
                    } else if (!cur->lchild->lchild->node->isConstant() && cur->lchild->rchild->node->isConstant()) {
                        auto number = static_cast<L3::Constant *>(cur->lchild->rchild->node);
                        if (number->num == 1 || number->num == 2 || number->num == 4 || number->num == 8) {
                            auto CISC_ins = new L2::Instruction_CISC;
                            auto reg = copyL3ItemtoL2(cur->node);
                            auto base = copyL3ItemtoL2(cur->rchild->node);
                            auto index = copyL3ItemtoL2(cur->lchild->lchild->node);
                            auto scale = new L2::Constant;
                            scale->num = number->num;
                            CISC_ins->reg = reg;
                            CISC_ins->base = base;
                            CISC_ins->index = index;
                            CISC_ins->scale = scale;

                            finish = true;

                            insStack.push(CISC_ins);

                            tiling(cur->rchild, insStack);
                            tiling(cur->lchild->lchild, insStack);
                            tiling(cur->lchild->rchild, insStack);
                        }
                    }
                } else if (cur->rchild->op == nodeOp::MULTI) {
                    if (cur->rchild->lchild->node->isConstant() && !cur->rchild->rchild->node->isConstant()) {
                        auto number = static_cast<L3::Constant *>(cur->rchild->lchild->node);
                        if (number->num == 1 || number->num == 2 || number->num == 4 || number->num == 8) {
                            auto CISC_ins = new L2::Instruction_CISC;
                            auto reg = copyL3ItemtoL2(cur->node);
                            auto base = copyL3ItemtoL2(cur->lchild->node);
                            auto index = copyL3ItemtoL2(cur->rchild->rchild->node);
                            auto scale = new L2::Constant;
                            scale->num = number->num;
                            CISC_ins->reg = reg;
                            CISC_ins->base = base;
                            CISC_ins->index = index;
                            CISC_ins->scale = scale;

                            finish = true;

                            insStack.push(CISC_ins);

                            tiling(cur->lchild, insStack);
                            tiling(cur->rchild->lchild, insStack);
                            tiling(cur->rchild->rchild, insStack);
                        }
                    } else if (!cur->rchild->lchild->node->isConstant() && cur->rchild->rchild->node->isConstant()) {
                        auto number = static_cast<L3::Constant *>(cur->rchild->rchild->node);
                        if (number->num == 1 || number->num == 2 || number->num == 4 || number->num == 8) {
                            auto CISC_ins = new L2::Instruction_CISC;
                            auto reg = copyL3ItemtoL2(cur->node);
                            auto base = copyL3ItemtoL2(cur->lchild->node);
                            auto index = copyL3ItemtoL2(cur->rchild->lchild->node);
                            auto scale = new L2::Constant;
                            scale->num = number->num;
                            CISC_ins->reg = reg;
                            CISC_ins->base = base;
                            CISC_ins->index = index;
                            CISC_ins->scale = scale;

                            finish = true;

                            insStack.push(CISC_ins);

                            tiling(cur->lchild, insStack);
                            tiling(cur->rchild->lchild, insStack);
                            tiling(cur->rchild->rchild, insStack);
                        }
                    }
                }
            }

            if (!finish) {
                auto aop_ass_ins = new L2::Instruction_aop_assignment;
                aop_ass_ins->opId = static_cast<L2::Aop>(cur->op - 3);
                auto dst = new L2::Item;
                dst->type = L2::VAR;
                dst->name = cur->node->name;
                aop_ass_ins->dst = dst;
                if (!cur->lchild->node->isConstant() && cur->lchild->node->name == cur->node->name) {
                    if (cur->rchild->node->isConstant()) {
                        auto src = new L2::Constant;
                        src->num = static_cast<L3::Constant *>(cur->rchild->node)->num;
                        aop_ass_ins->src = src;
                    } else {
                        auto src = new L2::Item;
                        src->type = L2::VAR;
                        src->name = cur->rchild->node->name;
                        aop_ass_ins->src = src;
                    }
                    insStack.push(aop_ass_ins);
                } else if (!cur->rchild->node->isConstant() && cur->rchild->node->name == cur->node->name) {
                    if (cur->lchild->node->isConstant()) {
                        auto src = new L2::Constant;
                        src->num = static_cast<L3::Constant *>(cur->lchild->node)->num;
                        aop_ass_ins->src = src;
                    } else {
                        auto src = new L2::Item;
                        src->type = L2::VAR;
                        src->name = cur->lchild->node->name;
                        aop_ass_ins->src = src;
                    }
                    insStack.push(aop_ass_ins);
                } else {
                    if (cur->rchild->node->isConstant()) {
                        auto src = new L2::Constant;
                        src->num = static_cast<L3::Constant *>(cur->rchild->node)->num;
                        aop_ass_ins->src = src;
                    } else {
                        auto src = new L2::Item;
                        src->type = L2::VAR;
                        src->name = cur->rchild->node->name;
                        aop_ass_ins->src = src;
                    }
                    insStack.push(aop_ass_ins);
                    auto ass_ins = new L2::Instruction_assignment;
                    auto var3 = new L2::Item;
                    var3->type = L2::VAR;
                    var3->name = cur->node->name;
                    ass_ins->dst = var3;
                    if (cur->lchild->node->isConstant()) {
                        auto var1 = new L2::Constant;
                        var1->num = static_cast<L3::Constant *>(cur->lchild->node)->num;
                        ass_ins->src = var1;
                    } else {
                        auto var1 = new L2::Item;
                        var1->type = L2::VAR;
                        var1->name = cur->lchild->node->name;
                        ass_ins->src = var1;
                    }
                    insStack.push(ass_ins);
                }

                tiling(cur->lchild, insStack);
                tiling(cur->rchild, insStack);
            }

            break;
            /*
            if (cur->lchild->node->isConstant() && cur->rchild->node->isConstant()) {
                auto ass_ins = new L2::Instruction_assignment;
                auto dst = new L2::Item;
                dst->type = L2::VAR;
                dst->name = cur->node->name;
                ass_ins->dst = dst;

                auto numberL = static_cast<L3::Constant *>(cur->lchild->node);
                auto numberR = static_cast<L3::Constant *>(cur->rchild->node);
                auto src = new L2::Constant;
                src->num = numberL->num + numberR->num;
                ass_ins->src = src;

                insStack.push(ass_ins);
            } else if (!cur->lchild->node->isConstant() && !cur->rchild->node->isConstant()) {
                if (cur->lchild->op == nodeOp::MULTI) {
                    if (cur->lchild->lchild->node->isConstant()) {
                        auto number = static_cast<L3::Constant *>(cur->lchild->lchild->node);
                        if (number->num == 1 || number->num == 2 || number->num == 4 || number->num == 8) {
                            auto CISC_ins = new L2::Instruction_CISC;
                            auto reg = new L2::Item;
                            reg->type = L2::VAR;
                            reg->name = cur->node->name;
                            CISC_ins->reg = reg;
                            auto base = new L2::Item;
                            base->type = L2::VAR;
                            base->name = cur->rchild->node->name;
                            CISC_ins->base = base;
                            auto index = new L2::Item;
                            index->type = L2::VAR;
                            index->name = cur->lchild->rchild->node->name;
                            CISC_ins->index = index;
                            auto scale = new L2::Constant;
                            scale->num = number->num;
                            CISC_ins->scale = scale;

                            insStack.push(CISC_ins);

                            tiling(cur->rchild, insStack);
                            tiling(cur->lchild->rchild, insStack);
                        }
                    } else if (cur->lchild->rchild->node->isConstant()) {
                        auto number = static_cast<L3::Constant *>(cur->lchild->rchild->node);
                        if (number->num == 1 || number->num == 2 || number->num == 4 || number->num == 8) {
                            auto CISC_ins = new L2::Instruction_CISC;
                            auto reg = new L2::Item;
                            reg->type = L2::VAR;
                            reg->name = cur->node->name;
                            CISC_ins->reg = reg;
                            auto base = new L2::Item;
                            base->type = L2::VAR;
                            base->name = cur->rchild->node->name;
                            CISC_ins->base = base;
                            auto index = new L2::Item;
                            index->type = L2::VAR;
                            index->name = cur->lchild->lchild->node->name;
                            CISC_ins->index = index;
                            auto scale = new L2::Constant;
                            scale->num = number->num;
                            CISC_ins->scale = scale;

                            insStack.push(CISC_ins);

                            tiling(cur->rchild, insStack);
                            tiling(cur->lchild->lchild, insStack);
                        }
                    }
                } else if (cur->rchild->op == nodeOp::MULTI) {
                    if (cur->rchild->lchild->node->isConstant()) {
                        auto number = static_cast<L3::Constant *>(cur->rchild->lchild->node);
                        if (number->num == 1 || number->num == 2 || number->num == 4 || number->num == 8) {
                            auto CISC_ins = new L2::Instruction_CISC;
                            auto reg = new L2::Item;
                            reg->type = L2::VAR;
                            reg->name = cur->node->name;
                            CISC_ins->reg = reg;
                            auto base = new L2::Item;
                            base->type = L2::VAR;
                            base->name = cur->lchild->node->name;
                            CISC_ins->base = base;
                            auto index = new L2::Item;
                            index->type = L2::VAR;
                            index->name = cur->rchild->rchild->node->name;
                            CISC_ins->index = index;
                            auto scale = new L2::Constant;
                            scale->num = number->num;
                            CISC_ins->scale = scale;

                            insStack.push(CISC_ins);

                            tiling(cur->lchild, insStack);
                            tiling(cur->rchild->rchild, insStack);
                        }
                    } else if (cur->rchild->rchild->node->isConstant()) {
                        auto number = static_cast<L3::Constant *>(cur->rchild->rchild->node);
                        if (number->num == 1 || number->num == 2 || number->num == 4 || number->num == 8) {
                            auto CISC_ins = new L2::Instruction_CISC;
                            auto reg = new L2::Item;
                            reg->type = L2::VAR;
                            reg->name = cur->node->name;
                            CISC_ins->reg = reg;
                            auto base = new L2::Item;
                            base->type = L2::VAR;
                            base->name = cur->lchild->node->name;
                            CISC_ins->base = base;
                            auto index = new L2::Item;
                            index->type = L2::VAR;
                            index->name = cur->rchild->lchild->node->name;
                            CISC_ins->index = index;
                            auto scale = new L2::Constant;
                            scale->num = number->num;
                            CISC_ins->scale = scale;

                            insStack.push(CISC_ins);

                            tiling(cur->lchild, insStack);
                            tiling(cur->rchild->lchild, insStack);
                        }
                    }
                } else if (cur->lchild->node->name == cur->rchild->node->name) {
                    string tmp = cur->lchild->node->name;
                    if (cur->lchild->op == nodeOp::ITEM && cur->rchild->op == nodeOp::ADD &&
                        !cur->rchild->lchild->node->isConstant() && !cur->rchild->rchild->node->isConstant() &&
                        cur->rchild->lchild->node->name == tmp && cur->rchild->rchild->node->name == tmp) {

                        if (cur->node->name != tmp) {
                            auto ass_ins = new L2::Instruction_assignment;
                            auto dst = new L2::Item;
                            dst->type = L2::VAR;
                            dst->name = cur->node->name;
                            ass_ins->dst = dst;
                            auto src = new L2::Item;
                            src->type = L2::VAR;
                            src->name = cur->lchild->node->name;
                            ass_ins->src = src;

                            insStack.push(ass_ins);
                        }

                        auto sop_ins = new L2::Instruction_sop;
                        sop_ins->opId = L2::LEFT;
                        auto reg = new L2::Item;
                        reg->type = L2::VAR;
                        reg->name = tmp;
                        sop_ins->reg = reg;
                        auto offset = new L2::Constant;
                        offset->num = 2;
                        sop_ins->offset = offset;

                        insStack.push(sop_ins);

                        tiling(cur->rchild->lchild, insStack);
                        tiling(cur->rchild->rchild, insStack);
                    } else if (cur->rchild->op == nodeOp::ITEM && cur->lchild->op == nodeOp::ADD &&
                               !cur->lchild->lchild->node->isConstant() && !cur->lchild->rchild->node->isConstant() &&
                               cur->lchild->lchild->node->name == tmp && cur->lchild->rchild->node->name == tmp) {
                        if (cur->node->name != tmp) {
                            auto ass_ins = new L2::Instruction_assignment;
                            auto dst = new L2::Item;
                            dst->type = L2::VAR;
                            dst->name = cur->node->name;
                            ass_ins->dst = dst;
                            auto src = new L2::Item;
                            src->type = L2::VAR;
                            src->name = cur->lchild->node->name;
                            ass_ins->src = src;

                            insStack.push(ass_ins);
                        }

                        auto sop_ins = new L2::Instruction_sop;
                        sop_ins->opId = L2::LEFT;
                        auto reg = new L2::Item;
                        reg->type = L2::VAR;
                        reg->name = tmp;
                        sop_ins->reg = reg;
                        auto offset = new L2::Constant;
                        offset->num = 2;
                        sop_ins->offset = offset;

                        insStack.push(sop_ins);

                        tiling(cur->lchild->lchild, insStack);
                        tiling(cur->lchild->rchild, insStack);
                    } else {
                        if (cur->node->name != tmp) {
                            auto ass_ins = new L2::Instruction_assignment;
                            auto dst = new L2::Item;
                            dst->type = L2::VAR;
                            dst->name = cur->node->name;
                            ass_ins->dst = dst;
                            auto src = new L2::Item;
                            src->type = L2::VAR;
                            src->name = cur->lchild->node->name;
                            ass_ins->src = src;

                            insStack.push(ass_ins);
                        }

                        auto sop_ins = new L2::Instruction_sop;
                        sop_ins->opId = L2::LEFT;
                        auto reg = new L2::Item;
                        reg->type = L2::VAR;
                        reg->name = tmp;
                        sop_ins->reg = reg;
                        auto offset = new L2::Constant;
                        offset->num = 1;
                        sop_ins->offset = offset;

                        insStack.push(sop_ins);

                        tiling(cur->lchild, insStack);
                        tiling(cur->rchild, insStack);
                    }
                } else if (cur->lchild->node->name == cur->node->name) {
                    auto aop_ass_ins = new L2::Instruction_aop_assignment;
                    aop_ass_ins->opId = L2::ADD;
                    auto dst = new L2::Item;
                    dst->type = L2::VAR;
                    dst->name = cur->node->name;
                    aop_ass_ins->dst = dst;
                    auto src = new L2::Item;
                    src->type = L2::VAR;
                    src->name = cur->rchild->node->name;
                    aop_ass_ins->src = src;

                    insStack.push(aop_ass_ins);

                    tiling(cur->lchild, insStack);
                    tiling(cur->rchild, insStack);
                } else if (cur->rchild->node->name == cur->node->name) {
                    auto aop_ass_ins = new L2::Instruction_aop_assignment;
                    aop_ass_ins->opId = L2::ADD;
                    auto dst = new L2::Item;
                    dst->type = L2::VAR;
                    dst->name = cur->node->name;
                    aop_ass_ins->dst = dst;
                    auto src = new L2::Item;
                    src->type = L2::VAR;
                    src->name = cur->lchild->node->name;
                    aop_ass_ins->src = src;

                    insStack.push(aop_ass_ins);

                    tiling(cur->lchild, insStack);
                    tiling(cur->rchild, insStack);
                } else {
                    auto ass_ins = new L2::Instruction_assignment;
                    auto var3 = new L2::Item;
                    var3->type = L2::VAR;
                    var3->name = cur->node->name;
                    ass_ins->dst = var3;
                    auto var1 = new L2::Item;
                    var1->type = L2::VAR;
                    var1->name = cur->lchild->node->name;
                    ass_ins->src = var1;

                    auto aop_ass_ins = new L2::Instruction_aop_assignment;
                    aop_ass_ins->opId = L2::ADD;
                    auto dst = new L2::Item;
                    dst->type = L2::VAR;
                    dst->name = cur->node->name;
                    aop_ass_ins->dst = dst;
                    auto src = new L2::Item;
                    src->type = L2::VAR;
                    src->name = cur->rchild->node->name;
                    aop_ass_ins->src = src;

                    insStack.push(aop_ass_ins);
                    insStack.push(ass_ins);

                    tiling(cur->lchild, insStack);
                    tiling(cur->rchild, insStack);
                }
            } else if (cur->lchild->node->isConstant()) {
                auto aop_ass_ins = new L2::Instruction_aop_assignment;
                aop_ass_ins->opId = L2::ADD;
                auto var = new L2::Item;
                var->type = L2::VAR;
                var->name = cur->node->name;
                aop_ass_ins->dst = var;
                auto number = new L2::Constant;
                number->num = static_cast<L3::Constant *>(cur->lchild->node)->num;
                aop_ass_ins->src = number;

                insStack.push(aop_ass_ins);

                if (cur->node->name != cur->rchild->node->name) {
                    auto ass_ins = new L2::Instruction_assignment;
                    auto dst = new L2::Item;
                    dst->type = L2::VAR;
                    dst->name = cur->node->name;
                    ass_ins->dst = dst;
                    auto src = new L2::Item;
                    src->type = L2::VAR;
                    src->name = cur->rchild->node->name;
                    ass_ins->src = src;

                    insStack.push(ass_ins);
                }

                tiling(cur->lchild, insStack);
                tiling(cur->rchild, insStack);
            } else if (cur->rchild->node->isConstant()) {
                auto aop_ass_ins = new L2::Instruction_aop_assignment;
                aop_ass_ins->opId = L2::ADD;
                auto var = new L2::Item;
                var->type = L2::VAR;
                var->name = cur->node->name;
                aop_ass_ins->dst = var;
                auto number = new L2::Constant;
                number->num = static_cast<L3::Constant *>(cur->rchild->node)->num;
                aop_ass_ins->src = number;

                insStack.push(aop_ass_ins);

                if (cur->node->name != cur->lchild->node->name) {
                    auto ass_ins = new L2::Instruction_assignment;
                    auto dst = new L2::Item;
                    dst->type = L2::VAR;
                    dst->name = cur->node->name;
                    ass_ins->dst = dst;
                    auto src = new L2::Item;
                    src->type = L2::VAR;
                    src->name = cur->lchild->node->name;
                    ass_ins->src = src;

                    insStack.push(ass_ins);
                }

                tiling(cur->lchild, insStack);
                tiling(cur->rchild, insStack);
            }

            break;
             */
        }
        case SUB:
        case MULTI:
        case AND: {
            auto aop_ass_ins = new L2::Instruction_aop_assignment;
            aop_ass_ins->opId = static_cast<L2::Aop>(cur->op - 3);
            auto dst = new L2::Item;
            dst->type = L2::VAR;
            dst->name = cur->node->name;
            aop_ass_ins->dst = dst;
            if (!cur->lchild->node->isConstant() && cur->lchild->node->name == cur->node->name) {
                if (cur->rchild->node->isConstant()) {
                    auto src = new L2::Constant;
                    src->num = static_cast<L3::Constant *>(cur->rchild->node)->num;
                    aop_ass_ins->src = src;
                } else {
                    auto src = new L2::Item;
                    src->type = L2::VAR;
                    src->name = cur->rchild->node->name;
                    aop_ass_ins->src = src;
                }
                insStack.push(aop_ass_ins);
            } else if (!cur->rchild->node->isConstant() && cur->rchild->node->name == cur->node->name) {
                if (cur->lchild->node->isConstant()) {
                    auto src = new L2::Constant;
                    src->num = static_cast<L3::Constant *>(cur->lchild->node)->num;
                    aop_ass_ins->src = src;
                } else {
                    auto src = new L2::Item;
                    src->type = L2::VAR;
                    src->name = cur->lchild->node->name;
                    aop_ass_ins->src = src;
                }
                insStack.push(aop_ass_ins);
            } else {
                if (cur->rchild->node->isConstant()) {
                    auto src = new L2::Constant;
                    src->num = static_cast<L3::Constant *>(cur->rchild->node)->num;
                    aop_ass_ins->src = src;
                } else {
                    auto src = new L2::Item;
                    src->type = L2::VAR;
                    src->name = cur->rchild->node->name;
                    aop_ass_ins->src = src;
                }
                insStack.push(aop_ass_ins);
                auto ass_ins = new L2::Instruction_assignment;
                auto var3 = new L2::Item;
                var3->type = L2::VAR;
                var3->name = cur->node->name;
                ass_ins->dst = var3;
                if (cur->lchild->node->isConstant()) {
                    auto var1 = new L2::Constant;
                    var1->num = static_cast<L3::Constant *>(cur->lchild->node)->num;
                    ass_ins->src = var1;
                } else {
                    auto var1 = new L2::Item;
                    var1->type = L2::VAR;
                    var1->name = cur->lchild->node->name;
                    ass_ins->src = var1;
                }
                insStack.push(ass_ins);
            }

            tiling(cur->lchild, insStack);
            tiling(cur->rchild, insStack);

            break;
        }
        case LEFT:
        case RIGHT: {
            auto sop_ins = new L2::Instruction_sop;
            sop_ins->opId = static_cast<L2::Sop>(cur->op - 7);
            auto reg = new L2::Item;
            reg->type = L2::VAR;
            reg->name = cur->node->name;
            sop_ins->reg = reg;
            if (!cur->lchild->node->isConstant() && cur->lchild->node->name == cur->node->name) {
                if (cur->rchild->node->isConstant()) {
                    auto offset = new L2::Constant;
                    offset->num = static_cast<L3::Constant *>(cur->rchild->node)->num;
                    sop_ins->offset = offset;
                } else {
                    auto offset = new L2::Item;
                    offset->type = L2::VAR;
                    offset->name = cur->rchild->node->name;
                    sop_ins->offset = offset;
                }
                insStack.push(sop_ins);
            } else if (!cur->rchild->node->isConstant() && cur->rchild->node->name == cur->node->name) {
                if (cur->lchild->node->isConstant()) {
                    auto offset = new L2::Constant;
                    offset->num = static_cast<L3::Constant *>(cur->lchild->node)->num;
                    sop_ins->offset = offset;
                } else {
                    auto offset = new L2::Item;
                    offset->type = L2::VAR;
                    offset->name = cur->lchild->node->name;
                    sop_ins->offset = offset;
                }
                insStack.push(sop_ins);
            } else {
                if (cur->rchild->node->isConstant()) {
                    auto offset = new L2::Constant;
                    offset->num = static_cast<L3::Constant *>(cur->rchild->node)->num;
                    sop_ins->offset = offset;
                } else {
                    auto offset = new L2::Item;
                    offset->type = L2::VAR;
                    offset->name = cur->rchild->node->name;
                    sop_ins->offset = offset;
                }
                insStack.push(sop_ins);
                auto ass_ins = new L2::Instruction_assignment;
                auto var3 = new L2::Item;
                var3->type = L2::VAR;
                var3->name = cur->node->name;
                ass_ins->dst = var3;
                if (cur->lchild->node->isConstant()) {
                    auto var1 = new L2::Constant;
                    var1->num = static_cast<L3::Constant *>(cur->lchild->node)->num;
                    ass_ins->src = var1;
                } else {
                    auto var1 = new L2::Item;
                    var1->type = L2::VAR;
                    var1->name = cur->lchild->node->name;
                    ass_ins->src = var1;
                }
                insStack.push(ass_ins);
            }

            tiling(cur->lchild, insStack);
            tiling(cur->rchild, insStack);

            break;
        }
        case LE:
        case LEQ:
        case EQL: {
            auto cmp_ins = new L2::Instruction_cmp;
            auto reg = copyL3ItemtoL2(cur->node);
            auto l = copyL3ItemtoL2(cur->lchild->node);
            auto r = copyL3ItemtoL2(cur->rchild->node);
            cmp_ins->reg = reg;
            cmp_ins->l = l;
            cmp_ins->r = r;
            cmp_ins->cmpId = static_cast<L2::Cmp>(cur->op - 9);
            insStack.push(cmp_ins);

            tiling(cur->lchild, insStack);
            tiling(cur->rchild, insStack);

            break;
        }
        case GR:
        case GEQ: {
            auto cmp_ins = new L2::Instruction_cmp;
            auto reg = copyL3ItemtoL2(cur->node);
            auto l = copyL3ItemtoL2(cur->lchild->node);
            auto r = copyL3ItemtoL2(cur->rchild->node);
            cmp_ins->reg = reg;
            cmp_ins->l = r;
            cmp_ins->r = l;
            cmp_ins->cmpId = static_cast<L2::Cmp>(cur->op - 12);
            insStack.push(cmp_ins);

            tiling(cur->lchild, insStack);
            tiling(cur->rchild, insStack);

            break;
        }
        case LOAD: {
            auto load_ins = new L2::Instruction_load;
            bool merge = false;
            auto dst = copyL3ItemtoL2(cur->node);
            load_ins->dst = dst;

            if (cur->lchild->op == nodeOp::ADD || cur->lchild->op == nodeOp::SUB) {
                if (cur->lchild->lchild->node->isConstant() &&
                    static_cast<L3::Constant *>(cur->lchild->lchild->node)->num % 8 == 0 &&
                    !cur->lchild->rchild->node->isConstant()) {
                    auto merged_src = copyL3ItemtoL2(cur->lchild->rchild->node);
                    auto merged_offset = new L2::Constant;
                    if (cur->lchild->op == nodeOp::ADD) {
                        merged_offset->num = static_cast<L3::Constant *>(cur->lchild->lchild->node)->num;
                    } else {
                        merged_offset->num = -static_cast<L3::Constant *>(cur->lchild->lchild->node)->num;
                    }
                    load_ins->src = merged_src;
                    load_ins->offset = merged_offset;
                    insStack.push(load_ins);
                    merge = true;

                    tiling(cur->lchild->rchild, insStack);
                } else if (cur->lchild->rchild->node->isConstant() &&
                           static_cast<L3::Constant *>(cur->lchild->rchild->node)->num % 8 == 0 &&
                           !cur->lchild->lchild->node->isConstant()) {
                    auto merged_src = copyL3ItemtoL2(cur->lchild->lchild->node);
                    auto merged_offset = new L2::Constant;
                    if (cur->lchild->op == nodeOp::ADD) {
                        merged_offset->num = static_cast<L3::Constant *>(cur->lchild->rchild->node)->num;
                    } else {
                        merged_offset->num = -static_cast<L3::Constant *>(cur->lchild->rchild->node)->num;
                    }
                    load_ins->src = merged_src;
                    load_ins->offset = merged_offset;
                    insStack.push(load_ins);
                    merge = true;

                    tiling(cur->lchild->lchild, insStack);
                }
            }

            if (!merge) {
                auto src = copyL3ItemtoL2(cur->lchild->node);
                auto offset = new L2::Constant;
                offset->num = 0;
                load_ins->src = src;
                load_ins->offset = offset;
                insStack.push(load_ins);

                tiling(cur->lchild, insStack);
            }

            break;
        }
        case STORE: {
            auto store_ins = new L2::Instruction_store;
            bool merge = false;
            auto src = copyL3ItemtoL2(cur->lchild->node);
            store_ins->src = src;

            if (cur->rchild->op == nodeOp::ADD || cur->rchild->op == nodeOp::SUB) {
                if (cur->rchild->lchild->node->isConstant() &&
                    static_cast<L3::Constant *>(cur->rchild->lchild->node)->num % 8 == 0 &&
                    !cur->rchild->rchild->node->isConstant()) {
                    auto merged_dst = copyL3ItemtoL2(cur->rchild->rchild->node);
                    auto merged_offset = new L2::Constant;
                    if (cur->rchild->op == nodeOp::ADD) {
                        merged_offset->num = static_cast<L3::Constant *>(cur->rchild->lchild->node)->num;
                    } else {
                        merged_offset->num = -static_cast<L3::Constant *>(cur->rchild->lchild->node)->num;
                    }
                    store_ins->dst = merged_dst;
                    store_ins->offset = merged_offset;
                    insStack.push(store_ins);
                    merge = true;

                    tiling(cur->lchild, insStack);
                    tiling(cur->rchild->rchild, insStack);
                } else if (cur->rchild->rchild->node->isConstant() &&
                           static_cast<L3::Constant *>(cur->rchild->rchild->node)->num % 8 == 0 &&
                           !cur->rchild->lchild->node->isConstant()) {
                    auto merged_dst = copyL3ItemtoL2(cur->rchild->lchild->node);
                    auto merged_offset = new L2::Constant;
                    if (cur->rchild->op == nodeOp::ADD) {
                        merged_offset->num = static_cast<L3::Constant *>(cur->rchild->rchild->node)->num;
                    } else {
                        merged_offset->num = -static_cast<L3::Constant *>(cur->rchild->rchild->node)->num;
                    }
                    store_ins->dst = merged_dst;
                    store_ins->offset = merged_offset;
                    insStack.push(store_ins);
                    merge = true;

                    tiling(cur->lchild, insStack);
                    tiling(cur->rchild->lchild, insStack);
                }
            }

            if (!merge) {
                auto dst = copyL3ItemtoL2(cur->rchild->node);
                auto offset = new L2::Constant;
                offset->num = 0;
                store_ins->dst = dst;
                store_ins->offset = offset;
                insStack.push(store_ins);

                tiling(cur->lchild, insStack);
                tiling(cur->rchild, insStack);
            }

            break;
        }
        case LABEL: {
            auto label_ins = new L2::Instruction_label;
            auto label = copyL3ItemtoL2(cur->node);
            label_ins->label = label;
            insStack.push(label_ins);
            break;
        }
        case BR: {
            if (cur->lchild) {
                if (cur->lchild->op == LE || cur->lchild->op == LEQ || cur->lchild->op == EQL ||
                    cur->lchild->op == GR || cur->lchild->op == GEQ) {
                    auto cjump_ins = new L2::Instruction_cjump;
                    auto dst = copyL3ItemtoL2(cur->node);
                    auto l = copyL3ItemtoL2(cur->lchild->lchild->node);;
                    auto r = copyL3ItemtoL2(cur->lchild->rchild->node);
                    if (cur->lchild->op == GR || cur->lchild->op == GEQ) {
                        cjump_ins->cmpId = static_cast<L2::Cmp>(cur->lchild->op - 12);
                        cjump_ins->l = r;
                        cjump_ins->r = l;
                    } else {
                        cjump_ins->cmpId = static_cast<L2::Cmp>(cur->lchild->op - 9);
                        cjump_ins->l = l;
                        cjump_ins->r = r;
                    }
                    cjump_ins->dst = dst;
                    insStack.push(cjump_ins);

                    tiling(cur->lchild->lchild, insStack);
                    tiling(cur->lchild->rchild, insStack);
                } else {
                    auto cjump_ins = new L2::Instruction_cjump;
                    auto dst = copyL3ItemtoL2(cur->node);
                    auto l = copyL3ItemtoL2(cur->lchild->node);;
                    auto r = new L2::Constant;
                    r->num = 1;
                    cjump_ins->cmpId = L2::EQL;
                    cjump_ins->dst = dst;
                    cjump_ins->l = l;
                    cjump_ins->r = r;
                    insStack.push(cjump_ins);

                    tiling(cur->lchild, insStack);
                }
            } else {
                auto djump_ins = new L2::Instruction_djump;
                auto dst = copyL3ItemtoL2(cur->node);
                djump_ins->dst = dst;
                insStack.push(djump_ins);
            }
            break;
        }
        case CALL: {
            auto call_ins = static_cast<L3::Instruction_call *>(cur->ins);
            auto l2_call_ins = new L2::Instruction_call;

            if (!call_ins->isRuntime) {
                auto label_ins = new L2::Instruction_label;
                auto label = new L2::Item;
                label->type = L2::LAB;
                if (call_ins->u->type == L3::VAR) {
                    label->name = ":" + call_ins->u->name + "_ret_" + function_name.substr(1) + to_string(calltimes);
                } else {
                    label->name = call_ins->u->name + "_ret_" + function_name.substr(1) + to_string(calltimes);
                }
                label_ins->label = label;
                insStack.push(label_ins);

                auto u = copyL3ItemtoL2(call_ins->u);
                l2_call_ins->isRuntime = false;
                l2_call_ins->u = u;
                auto number = new L2::Constant;
                number->num = call_ins->args.size();
                l2_call_ins->num = number;
            } else {
                l2_call_ins->isRuntime = true;
                l2_call_ins->runtimeId = static_cast<L2::RuntimeType>(call_ins->runtimeId);
                auto number = new L2::Constant;
                if (call_ins->runtimeId == L3::PRINT) {
                    number->num = 1;
                } else {
                    number->num = 2;
                }
                l2_call_ins->num = number;
            }
            insStack.push(l2_call_ins);

            if (!call_ins->isRuntime) {
                auto ret_store_ins = new L2::Instruction_store;
                auto dst = new L2::Item;
                dst->type = L2::REG;
                dst->r = L2::rsp;
                auto label = new L2::Item;
                label->type = L2::LAB;
                if (call_ins->u->type == L3::VAR) {
                    label->name = ":" + call_ins->u->name + "_ret_" + function_name.substr(1) + to_string(calltimes);
                } else {
                    label->name = call_ins->u->name + "_ret_" + function_name.substr(1) + to_string(calltimes);
                }
                calltimes++;
                auto offset = new L2::Constant;
                offset->num = -8;
                ret_store_ins->dst = dst;
                ret_store_ins->src = label;
                ret_store_ins->offset = offset;
                insStack.push(ret_store_ins);
            }

            int count = call_ins->args.size() - 1;
            reverse(call_ins->args.begin(), call_ins->args.end());
            for (auto arg : call_ins->args) {
                if (count > 5) {
                    auto store_ins = new L2::Instruction_store;
                    auto dst = new L2::Item;
                    dst->type = L2::REG;
                    dst->r = L2::rsp;
                    auto src = copyL3ItemtoL2(arg);
                    auto offset = new L2::Constant;
                    offset->num = -8 - 8 * (call_ins->args.size() - count);
                    store_ins->dst = dst;
                    store_ins->src = src;
                    store_ins->offset = offset;
                    insStack.push(store_ins);
                } else {
                    auto ass_ins = new L2::Instruction_assignment;
                    auto dst = new L2::Item;
                    dst->type = L2::REG;
                    dst->r = L2::argRegisters[count];
                    auto src = copyL3ItemtoL2(arg);
                    ass_ins->dst = dst;
                    ass_ins->src = src;
                    insStack.push(ass_ins);
                }
                count--;
            }

            break;
        }
        case CALL_ASS: {
            auto call_ass_ins = static_cast<L3::Instruction_call_assignment *>(cur->ins);

            auto var_ass_ins = new L2::Instruction_assignment;
            auto var_dst = copyL3ItemtoL2(call_ass_ins->var);
            auto var_src = new L2::Item;
            var_src->type = L2::REG;
            var_src->r = L2::rax;
            var_ass_ins->dst = var_dst;
            var_ass_ins->src = var_src;
            insStack.push(var_ass_ins);

            auto l2_call_ins = new L2::Instruction_call;

            if (!call_ass_ins->isRuntime) {
                auto label_ins = new L2::Instruction_label;
                auto label = new L2::Item;
                label->type = L2::LAB;
                if (call_ass_ins->u->type == L3::VAR) {
                    label->name =
                            ":" + call_ass_ins->u->name + "_ret_" + function_name.substr(1) + to_string(calltimes);
                } else {
                    label->name = call_ass_ins->u->name + "_ret_" + function_name.substr(1) + to_string(calltimes);
                }
                label_ins->label = label;
                insStack.push(label_ins);

                auto u = copyL3ItemtoL2(call_ass_ins->u);
                l2_call_ins->isRuntime = false;
                l2_call_ins->u = u;
                auto number = new L2::Constant;
                number->num = call_ass_ins->args.size();
                l2_call_ins->num = number;
            } else {
                l2_call_ins->isRuntime = true;
                l2_call_ins->runtimeId = static_cast<L2::RuntimeType>(call_ass_ins->runtimeId);
                auto number = new L2::Constant;
                if (call_ass_ins->runtimeId == L3::PRINT) {
                    number->num = 1;
                } else {
                    number->num = 2;
                }
                l2_call_ins->num = number;
            }
            insStack.push(l2_call_ins);

            if (!call_ass_ins->isRuntime) {
                auto ret_store_ins = new L2::Instruction_store;
                auto dst = new L2::Item;
                dst->type = L2::REG;
                dst->r = L2::rsp;
                auto label = new L2::Item;
                label->type = L2::LAB;
                if (call_ass_ins->u->type == L3::VAR) {
                    label->name =
                            ":" + call_ass_ins->u->name + "_ret_" + function_name.substr(1) + to_string(calltimes);
                } else {
                    label->name = call_ass_ins->u->name + "_ret_" + function_name.substr(1) + to_string(calltimes);
                }
                calltimes++;
                auto offset = new L2::Constant;
                offset->num = -8;
                ret_store_ins->dst = dst;
                ret_store_ins->src = label;
                ret_store_ins->offset = offset;
                insStack.push(ret_store_ins);
            }

            int count = call_ass_ins->args.size() - 1;
            reverse(call_ass_ins->args.begin(), call_ass_ins->args.end());
            for (auto arg : call_ass_ins->args) {
                if (count > 5) {
                    auto store_ins = new L2::Instruction_store;
                    auto dst = new L2::Item;
                    dst->type = L2::REG;
                    dst->r = L2::rsp;
                    auto src = copyL3ItemtoL2(arg);
                    auto offset = new L2::Constant;
                    offset->num = -8 - 8 * (call_ass_ins->args.size() - count);
                    store_ins->dst = dst;
                    store_ins->src = src;
                    store_ins->offset = offset;
                    insStack.push(store_ins);
                } else {
                    auto ass_ins = new L2::Instruction_assignment;
                    auto dst = new L2::Item;
                    dst->type = L2::REG;
                    dst->r = L2::argRegisters[count];
                    auto src = copyL3ItemtoL2(arg);
                    ass_ins->dst = dst;
                    ass_ins->src = src;
                    insStack.push(ass_ins);
                }
                count--;
            }

            break;
        }
        default:;
    }
}

L2::Item *analysis::copyL3ItemtoL2(L3::Item *l3) {
    if (l3->isConstant()) {
        auto l2 = new L2::Constant;
        l2->num = static_cast<L3::Constant *>(l3)->num;
        return l2;
    } else {
        auto l2 = new L2::Item;
        l2->type = static_cast<L2::ItemType>(l3->type + 1);
        l2->name = l3->name;
        return l2;
    }
}

void analysis::argsAssign(L3::Function *f, L2::Function *newF) {
    int count = 0;
    for (auto var : f->vars) {
        if (count < 6) {
            auto ass_ins = new L2::Instruction_assignment;
            auto dst = copyL3ItemtoL2(var);
            auto src = new L2::Item;
            src->type = L2::REG;
            src->r = L2::argRegisters[count];
            ass_ins->dst = dst;
            ass_ins->src = src;
            newF->instructions.push_back(ass_ins);
        } else {
            auto stack_ins = new L2::Instruction_stackArg;
            auto reg = copyL3ItemtoL2(var);
            auto offset = new L2::Constant;
            offset->num = 8 * (count - 6);
            stack_ins->reg = reg;
            stack_ins->offset = offset;
            newF->instructions.push_back(stack_ins);
        }
        count++;
    }
}

bool analysis::whetherUse(analysis::tree *i, analysis::tree *ptr) {
    if (!i) {
        return false;
    }

    if(i->node && infos[ptr->insId]->kill.find(i->node->name) != infos[ptr->insId]->kill.end()) {
        return true;
    }

    return whetherUse(i->lchild, ptr) || whetherUse(i->rchild, ptr);
}

