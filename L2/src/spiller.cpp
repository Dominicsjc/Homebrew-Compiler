#include "spiller.h"

void spill(L2::Function *f) {
    int index = 0;
    bool spilled = false;
    for (auto it = f->instructions.begin(); it != f->instructions.end();) {
        switch (checkAndReplaceIns(*it, f->targetVar, f->spillerVar, index)) {
            case NONE: {
                ++it;
                break;
            }
            case BOTH: {
                auto load_ins = createTmpLoad(f, index);
                auto store_ins = createTmpStore(f, index);
                it = f->instructions.insert(it, load_ins);
                it += 2;
                it = f->instructions.insert(it, store_ins);
                ++it;
                index++;
                spilled = true;
                break;
            }
            case GEN: {
                auto load_ins = createTmpLoad(f, index);
                it = f->instructions.insert(it, load_ins);
                it += 2;
                index++;
                spilled = true;
                break;
            }
            case KILL: {
                auto store_ins = createTmpStore(f, index);
                ++it;
                it = f->instructions.insert(it, store_ins);
                ++it;
                index++;
                spilled = true;
                break;
            }
            default:;
        }
    }
    if (spilled) {
        f->locals++;

        /*
         * DEBUG
         */
        //cout << "Spilled Function:" << endl;
        //L2::printL2Function(cout, f);
        //cout << endl;
    }
}

L2::Item * createReplacedVar(L2::Item *spillerVar, int index) {
    auto tmp_var = new L2::Item;
    tmp_var->type = L2::VAR;
    tmp_var->name = spillerVar->name + to_string(index);

    return tmp_var;
}

L2::Instruction_load *createTmpLoad(L2::Function *f, int index) {
    auto tmp_src = new L2::Item;
    tmp_src->type = L2::REG;
    tmp_src->r = L2::rsp;
    auto tmp_dst = new L2::Item;
    tmp_dst->type = L2::VAR;
    tmp_dst->name = f->spillerVar->name + to_string(index);
    auto tmp_offset = new L2::Constant;
    tmp_offset->num = f->locals*8;

    auto tmp_load = new L2::Instruction_load;
    tmp_load->dst = tmp_dst;
    tmp_load->src = tmp_src;
    tmp_load->offset = tmp_offset;

    return tmp_load;
}

L2::Instruction_store *createTmpStore(L2::Function *f, int index) {
    auto tmp_dst = new L2::Item;
    tmp_dst->type = L2::REG;
    tmp_dst->r = L2::rsp;
    auto tmp_src = new L2::Item;
    tmp_src->type = L2::VAR;
    tmp_src->name = f->spillerVar->name + to_string(index);
    auto tmp_offset = new L2::Constant;
    tmp_offset->num = f->locals*8;

    auto tmp_store = new L2::Instruction_store;
    tmp_store->dst = tmp_dst;
    tmp_store->src = tmp_src;
    tmp_store->offset = tmp_offset;

    return tmp_store;
}

bool checkVar(L2::Item *i, L2::Item *targetVar) {
    return !i->isConstant() && i->type == L2::VAR && i->name == targetVar->name;
}

int checkAndReplaceIns(L2::Instruction *ins, L2::Item *targetVar, L2::Item *spillerVar, int index) {
    int id = ins->getTypeId();
    int res = NONE;
    switch (id) {
        case L2::RET: {
            break;
        }
        case L2::ASS: {
            auto ass_ins = static_cast<L2::Instruction_assignment *>(ins);
            if (checkVar(ass_ins->dst, targetVar)) {
                auto dst_deleter = ass_ins->dst;
                ass_ins->dst = createReplacedVar(spillerVar, index);
                delete dst_deleter;
                if (checkVar(ass_ins->src, targetVar)) {
                    auto src_deleter = ass_ins->src;
                    ass_ins->src = createReplacedVar(spillerVar, index);
                    delete src_deleter;
                    res = BOTH;
                }
                else
                    res = KILL;
            } else {
                if (checkVar(ass_ins->src, targetVar)) {
                    auto src_deleter = ass_ins->src;
                    ass_ins->src = createReplacedVar(spillerVar, index);
                    delete src_deleter;
                    res = GEN;
                }
                else
                    res = NONE;
            }
            break;
        }
        case L2::LOAD: {
            auto load_ins = static_cast<L2::Instruction_load *>(ins);
            if (checkVar(load_ins->dst, targetVar)) {
                auto dst_deleter = load_ins->dst;
                load_ins->dst = createReplacedVar(spillerVar, index);
                delete dst_deleter;
                if (checkVar(load_ins->src, targetVar)) {
                    auto src_deleter = load_ins->src;
                    load_ins->src = createReplacedVar(spillerVar, index);
                    delete src_deleter;
                    res = BOTH;
                }
                else
                    res = KILL;
            } else {
                if (checkVar(load_ins->src, targetVar)) {
                    auto src_deleter = load_ins->src;
                    load_ins->src = createReplacedVar(spillerVar, index);
                    delete src_deleter;
                    res = GEN;
                }
                else
                    res = NONE;
            }
            break;
        }
        case L2::STORE: {
            auto store_ins = static_cast<L2::Instruction_store *>(ins);
            bool src = false, dst = false;
            if (checkVar(store_ins->src, targetVar)) {
                auto src_deleter = store_ins->src;
                store_ins->src = createReplacedVar(spillerVar, index);
                delete src_deleter;
                src = true;
            }
            if (checkVar(store_ins->dst, targetVar)){
                auto dst_deleter = store_ins->dst;
                store_ins->dst = createReplacedVar(spillerVar, index);
                delete dst_deleter;
                dst = true;
            }
            if (src || dst)
                res = GEN;
            else
                res = NONE;
            break;
        }
        case L2::AOP_ASS: {
            auto aop_assign_ins = static_cast<L2::Instruction_aop_assignment *>(ins);
            if (checkVar(aop_assign_ins->dst, targetVar)) {
                auto dst_deleter = aop_assign_ins->dst;
                aop_assign_ins->dst = createReplacedVar(spillerVar, index);
                delete dst_deleter;
                res = BOTH;
                if (checkVar(aop_assign_ins->src, targetVar)) {
                    auto src_deleter = aop_assign_ins->src;
                    aop_assign_ins->src = createReplacedVar(spillerVar, index);
                    delete src_deleter;
                }
            } else {
                if (checkVar(aop_assign_ins->src, targetVar)) {
                    auto src_deleter = aop_assign_ins->src;
                    aop_assign_ins->src = createReplacedVar(spillerVar, index);
                    delete src_deleter;
                    res = GEN;
                }
                else
                    res = NONE;
            }
            break;
        }
        case L2::SELFAOP: {
            auto selfAop_ins = static_cast<L2::Instruction_selfAop *>(ins);
            if (checkVar(selfAop_ins->reg, targetVar)) {
                auto reg_deleter = selfAop_ins->reg;
                selfAop_ins->reg = createReplacedVar(spillerVar, index);
                delete reg_deleter;
                res = BOTH;
            }
            break;
        }
        case L2::AOP_LOAD: {
            auto aop_load_ins = static_cast<L2::Instruction_aop_load *>(ins);
            if (checkVar(aop_load_ins->dst, targetVar)) {
                auto dst_deleter = aop_load_ins->dst;
                aop_load_ins->dst = createReplacedVar(spillerVar, index);
                delete dst_deleter;
                res = BOTH;
                if (checkVar(aop_load_ins->src, targetVar)) {
                    auto src_deleter = aop_load_ins->src;
                    aop_load_ins->src = createReplacedVar(spillerVar, index);
                    delete src_deleter;
                }
            } else {
                if (checkVar(aop_load_ins->src, targetVar)) {
                    auto src_deleter = aop_load_ins->src;
                    aop_load_ins->src = createReplacedVar(spillerVar, index);
                    delete src_deleter;
                    res = GEN;
                }
                else
                    res = NONE;
            }
            break;
        }
        case L2::AOP_STORE: {
            auto aop_store_ins = static_cast<L2::Instruction_aop_store *>(ins);
            if (checkVar(aop_store_ins->dst, targetVar)) {
                auto dst_deleter = aop_store_ins->dst;
                aop_store_ins->dst = createReplacedVar(spillerVar, index);
                delete dst_deleter;
                res = BOTH;
                if (checkVar(aop_store_ins->src, targetVar)) {
                    auto src_deleter = aop_store_ins->src;
                    aop_store_ins->src = createReplacedVar(spillerVar, index);
                    delete src_deleter;
                }
            } else {
                if (checkVar(aop_store_ins->src, targetVar)) {
                    auto src_deleter = aop_store_ins->src;
                    aop_store_ins->src = createReplacedVar(spillerVar, index);
                    delete src_deleter;
                    res = GEN;
                }
                else
                    res = NONE;
            }
            break;
        }
        case L2::SOP: {
            auto sop_ins = static_cast<L2::Instruction_sop *>(ins);
            if (checkVar(sop_ins->reg, targetVar)) {
                auto reg_deleter = sop_ins->reg;
                sop_ins->reg = createReplacedVar(spillerVar, index);
                delete reg_deleter;
                res = BOTH;
                if (checkVar(sop_ins->offset, targetVar)) {
                    auto offset_deleter = sop_ins->offset;
                    sop_ins->offset = createReplacedVar(spillerVar, index);
                    delete offset_deleter;
                }
            } else {
                if (checkVar(sop_ins->offset, targetVar)) {
                    auto offset_deleter = sop_ins->offset;
                    sop_ins->offset = createReplacedVar(spillerVar, index);
                    delete offset_deleter;
                    res = GEN;
                }
                else
                    res = NONE;
            }
            break;
        }
        case L2::CMP: {
            auto cmp_ins = static_cast<L2::Instruction_cmp *>(ins);
            if (checkVar(cmp_ins->reg, targetVar)) {
                auto reg_deleter = cmp_ins->reg;
                cmp_ins->reg = createReplacedVar(spillerVar, index);
                delete reg_deleter;

                bool l = false, r = false;
                if (checkVar(cmp_ins->l, targetVar)) {
                    auto l_deleter = cmp_ins->l;
                    cmp_ins->l = createReplacedVar(spillerVar, index);
                    delete l_deleter;
                    l = true;
                }
                if (checkVar(cmp_ins->r, targetVar)){
                    auto r_deleter = cmp_ins->r;
                    cmp_ins->r = createReplacedVar(spillerVar, index);
                    delete r_deleter;
                    r = true;
                }
                if (l || r)
                    res = BOTH;
                else
                    res = KILL;
            } else {
                bool l = false, r = false;
                if (checkVar(cmp_ins->l, targetVar)) {
                    auto l_deleter = cmp_ins->l;
                    cmp_ins->l = createReplacedVar(spillerVar, index);
                    delete l_deleter;
                    l = true;
                }
                if (checkVar(cmp_ins->r, targetVar)){
                    auto r_deleter = cmp_ins->r;
                    cmp_ins->r = createReplacedVar(spillerVar, index);
                    delete r_deleter;
                    r = true;
                }
                if (l || r)
                    res = GEN;
                else
                    res = NONE;
            }
            break;
        }
        case L2::LABEL: {
            break;
        }
        case L2::DJUMP: {
            break;
        }
        case L2::CJUMP: {
            auto cjump_ins = static_cast<L2::Instruction_cjump *>(ins);
            bool l = false, r = false;
            if (checkVar(cjump_ins->l, targetVar)) {
                auto l_deleter = cjump_ins->l;
                cjump_ins->l = createReplacedVar(spillerVar, index);
                delete l_deleter;
                l = true;
            }
            if (checkVar(cjump_ins->r, targetVar)){
                auto r_deleter = cjump_ins->r;
                cjump_ins->r = createReplacedVar(spillerVar, index);
                delete r_deleter;
                r = true;
            }
            if (l || r)
                res = GEN;
            else
                res = NONE;
            break;
        }
        case L2::CISC: {
            auto CISC_ins = static_cast<L2::Instruction_CISC *>(ins);
            if (checkVar(CISC_ins->reg, targetVar)) {
                auto reg_deleter = CISC_ins->reg;
                CISC_ins->reg = createReplacedVar(spillerVar, index);
                delete reg_deleter;

                bool base = false, CISC_index = false;
                if (checkVar(CISC_ins->base, targetVar)) {
                    auto base_deleter = CISC_ins->base;
                    CISC_ins->base = createReplacedVar(spillerVar, index);
                    delete base_deleter;
                    base = true;
                }
                if (checkVar(CISC_ins->index, targetVar)){
                    auto r_deleter = CISC_ins->index;
                    CISC_ins->index = createReplacedVar(spillerVar, index);
                    delete r_deleter;
                    CISC_index = true;
                }
                if (base || CISC_index)
                    res = BOTH;
                else
                    res = KILL;
            } else {
                bool base = false, CISC_index = false;
                if (checkVar(CISC_ins->base, targetVar)) {
                    auto base_deleter = CISC_ins->base;
                    CISC_ins->base = createReplacedVar(spillerVar, index);
                    delete base_deleter;
                    base = true;
                }
                if (checkVar(CISC_ins->index, targetVar)){
                    auto r_deleter = CISC_ins->index;
                    CISC_ins->index = createReplacedVar(spillerVar, index);
                    delete r_deleter;
                    CISC_index = true;
                }
                if (base || CISC_index)
                    res = GEN;
                else
                    res = NONE;
            }
            break;
        }
        case L2::CALL: {
            auto call_ins = static_cast<L2::Instruction_call *>(ins);
            if (!call_ins->isRuntime) {
                if (checkVar(call_ins->u, targetVar)) {
                    auto u_deleter = call_ins->u;
                    call_ins->u = createReplacedVar(spillerVar, index);
                    delete u_deleter;
                    res = GEN;
                }
            }
            break;
        }
        case L2::STACK: {
            auto stack_ins = static_cast<L2::Instruction_stackArg *>(ins);
            if (checkVar(stack_ins->reg, targetVar)) {
                auto reg_deleter = stack_ins->reg;
                stack_ins->reg = createReplacedVar(spillerVar, index);
                delete reg_deleter;
                res = KILL;
            }
            break;
        }
        default:;
    }
    return res;
}
