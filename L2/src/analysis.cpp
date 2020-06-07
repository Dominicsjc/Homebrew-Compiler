#include "analysis.h"
#include "spiller.h"

Analysis::Analysis(L2::Function *f) {
    for (auto ins : f->instructions) {
        auto info = new InsLiveness();
        if (ins->getTypeId() == L2::LABEL) {
            auto label_ins = static_cast<L2::Instruction_label *>(ins);
            labels.emplace(make_pair(label_ins->label->name, info));
        }
        getGen(info, ins);
        getKill(info, ins);
        infos.push_back(info);
    }
    for (const auto &p : varNames) {
        varIndex.insert({p.second, p.first});
    }
    analyzeSuccessor(f);
    // printGenKill(cerr);
    computeInOut();
}

void Analysis::getGen(InsLiveness *info, L2::Instruction *ins) {
    switch (ins->getTypeId()) {
        case L2::InsId::RET: {
            info->gen.insert(L2::rax);

            for (auto reg: calleeSavedRegisters) {
                info->gen.insert(reg);
            }

            break;
        }
        case L2::InsId::ASS: {
            auto ass_ins = static_cast<L2::Instruction_assignment *>(ins);
            if (!ass_ins->src->isConstant()) {
                switch (ass_ins->src->type) {
                    case L2::ItemType::REG: {
                        info->gen.insert(ass_ins->src->r);
                        break;
                    }
                    case L2::ItemType::VAR: {
                        if (varNames.find(ass_ins->src->name) == varNames.end()) {
                            int varId = varNames.size() + 16;
                            varNames.emplace(make_pair(ass_ins->src->name, varId));
                            info->gen.insert(varId);
                        } else
                            info->gen.insert(varNames.find(ass_ins->src->name)->second);
                        break;
                    }
                    default:;
                }
            }
            break;
        }
        case L2::InsId::LOAD: {
            auto load_ins = static_cast<L2::Instruction_load *>(ins);
            if (!load_ins->src->isConstant()) {
                switch (load_ins->src->type) {
                    case L2::ItemType::REG: {
                        info->gen.insert(load_ins->src->r);
                        break;
                    }
                    case L2::ItemType::VAR: {
                        if (varNames.find(load_ins->src->name) == varNames.end()) {
                            int varId = varNames.size() + 16;
                            varNames.emplace(make_pair(load_ins->src->name, varId));
                            info->gen.insert(varId);
                        } else
                            info->gen.insert(varNames.find(load_ins->src->name)->second);
                        break;
                    }
                    default:;
                }
            }
            break;
        }
        case L2::InsId::STORE: {
            auto store_ins = static_cast<L2::Instruction_store *>(ins);
            if (!store_ins->src->isConstant()) {
                switch (store_ins->src->type) {
                    case L2::ItemType::REG: {
                        info->gen.insert(store_ins->src->r);
                        break;
                    }
                    case L2::ItemType::VAR: {
                        if (varNames.find(store_ins->src->name) == varNames.end()) {
                            int varId = varNames.size() + 16;
                            varNames.emplace(make_pair(store_ins->src->name, varId));
                            info->gen.insert(varId);
                        } else
                            info->gen.insert(varNames.find(store_ins->src->name)->second);
                        break;
                    }
                    default:;
                }
            }
            if (!store_ins->dst->isConstant()) {
                switch (store_ins->dst->type) {
                    case L2::ItemType::REG: {
                        info->gen.insert(store_ins->dst->r);
                        break;
                    }
                    case L2::ItemType::VAR: {
                        if (varNames.find(store_ins->dst->name) == varNames.end()) {
                            int varId = varNames.size() + 16;
                            varNames.emplace(make_pair(store_ins->dst->name, varId));
                            info->gen.insert(varId);
                        } else
                            info->gen.insert(varNames.find(store_ins->dst->name)->second);
                        break;
                    }
                    default:;
                }
            }
            break;
        }
        case L2::InsId::AOP_ASS: {
            auto aop_assign_ins = static_cast<L2::Instruction_aop_assignment *>(ins);
            if (!aop_assign_ins->src->isConstant()) {
                switch (aop_assign_ins->src->type) {
                    case L2::ItemType::REG: {
                        info->gen.insert(aop_assign_ins->src->r);
                        break;
                    }
                    case L2::ItemType::VAR: {
                        if (varNames.find(aop_assign_ins->src->name) == varNames.end()) {
                            int varId = varNames.size() + 16;
                            varNames.emplace(make_pair(aop_assign_ins->src->name, varId));
                            info->gen.insert(varId);
                        } else
                            info->gen.insert(varNames.find(aop_assign_ins->src->name)->second);
                        break;
                    }
                    default:;
                }
            }
            if (!aop_assign_ins->dst->isConstant()) {
                switch (aop_assign_ins->dst->type) {
                    case L2::ItemType::REG: {
                        info->gen.insert(aop_assign_ins->dst->r);
                        break;
                    }
                    case L2::ItemType::VAR: {
                        if (varNames.find(aop_assign_ins->dst->name) == varNames.end()) {
                            int varId = varNames.size() + 16;
                            varNames.emplace(make_pair(aop_assign_ins->dst->name, varId));
                            info->gen.insert(varId);
                        } else
                            info->gen.insert(varNames.find(aop_assign_ins->dst->name)->second);
                        break;
                    }
                    default:;
                }
            }
            break;
        }
        case L2::InsId::SELFAOP: {
            auto self_aop_ins = static_cast<L2::Instruction_selfAop *>(ins);
            if (!self_aop_ins->reg->isConstant()) {
                switch (self_aop_ins->reg->type) {
                    case L2::ItemType::REG: {
                        info->gen.insert(self_aop_ins->reg->r);
                        break;
                    }
                    case L2::ItemType::VAR: {
                        if (varNames.find(self_aop_ins->reg->name) == varNames.end()) {
                            int varId = varNames.size() + 16;
                            varNames.emplace(make_pair(self_aop_ins->reg->name, varId));
                            info->gen.insert(varId);
                        } else
                            info->gen.insert(varNames.find(self_aop_ins->reg->name)->second);
                        break;
                    }
                    default:;
                }
            }
            break;
        }
        case L2::InsId::AOP_LOAD: {
            auto aop_load_ins = static_cast<L2::Instruction_aop_load *>(ins);
            if (!aop_load_ins->src->isConstant()) {
                switch (aop_load_ins->src->type) {
                    case L2::ItemType::REG: {
                        info->gen.insert(aop_load_ins->src->r);
                        break;
                    }
                    case L2::ItemType::VAR: {
                        if (varNames.find(aop_load_ins->src->name) == varNames.end()) {
                            int varId = varNames.size() + 16;
                            varNames.emplace(make_pair(aop_load_ins->src->name, varId));
                            info->gen.insert(varId);
                        } else
                            info->gen.insert(varNames.find(aop_load_ins->src->name)->second);
                        break;
                    }
                    default:;
                }
            }
            if (!aop_load_ins->dst->isConstant()) {
                switch (aop_load_ins->dst->type) {
                    case L2::ItemType::REG: {
                        info->gen.insert(aop_load_ins->dst->r);
                        break;
                    }
                    case L2::ItemType::VAR: {
                        if (varNames.find(aop_load_ins->dst->name) == varNames.end()) {
                            int varId = varNames.size() + 16;
                            varNames.emplace(make_pair(aop_load_ins->dst->name, varId));
                            info->gen.insert(varId);
                        } else
                            info->gen.insert(varNames.find(aop_load_ins->dst->name)->second);
                        break;
                    }
                    default:;
                }
            }
            break;
        }
        case L2::InsId::AOP_STORE: {
            auto aop_store_ins = static_cast<L2::Instruction_aop_store *>(ins);
            if (!aop_store_ins->src->isConstant()) {
                switch (aop_store_ins->src->type) {
                    case L2::ItemType::REG: {
                        info->gen.insert(aop_store_ins->src->r);
                        break;
                    }
                    case L2::ItemType::VAR: {
                        if (varNames.find(aop_store_ins->src->name) == varNames.end()) {
                            int varId = varNames.size() + 16;
                            varNames.emplace(make_pair(aop_store_ins->src->name, varId));
                            info->gen.insert(varId);
                        } else
                            info->gen.insert(varNames.find(aop_store_ins->src->name)->second);
                        break;
                    }
                    default:;
                }
            }
            if (!aop_store_ins->dst->isConstant()) {
                switch (aop_store_ins->dst->type) {
                    case L2::ItemType::REG: {
                        info->gen.insert(aop_store_ins->dst->r);
                        break;
                    }
                    case L2::ItemType::VAR: {
                        if (varNames.find(aop_store_ins->dst->name) == varNames.end()) {
                            int varId = varNames.size() + 16;
                            varNames.emplace(make_pair(aop_store_ins->dst->name, varId));
                            info->gen.insert(varId);
                        } else
                            info->gen.insert(varNames.find(aop_store_ins->dst->name)->second);
                        break;
                    }
                    default:;
                }
            }
            break;
        }
        case L2::InsId::SOP: {
            auto sop_ins = static_cast<L2::Instruction_sop *>(ins);
            if (!sop_ins->offset->isConstant()) {
                switch (sop_ins->offset->type) {
                    case L2::ItemType::REG: {
                        info->gen.insert(sop_ins->offset->r);
                        break;
                    }
                    case L2::ItemType::VAR: {
                        if (varNames.find(sop_ins->offset->name) == varNames.end()) {
                            int varId = varNames.size() + 16;
                            varNames.emplace(make_pair(sop_ins->offset->name, varId));
                            info->gen.insert(varId);
                            sopSrcVar.insert(varId);
                        } else {
                            info->gen.insert(varNames.find(sop_ins->offset->name)->second);
                            sopSrcVar.insert(varNames.find(sop_ins->offset->name)->second);
                        }

                        break;
                    }
                    default:;
                }
            }
            if (!sop_ins->reg->isConstant()) {
                switch (sop_ins->reg->type) {
                    case L2::ItemType::REG: {
                        info->gen.insert(sop_ins->reg->r);
                        break;
                    }
                    case L2::ItemType::VAR: {
                        if (varNames.find(sop_ins->reg->name) == varNames.end()) {
                            int varId = varNames.size() + 16;
                            varNames.emplace(make_pair(sop_ins->reg->name, varId));
                            info->gen.insert(varId);
                        } else
                            info->gen.insert(varNames.find(sop_ins->reg->name)->second);
                        break;
                    }
                    default:;
                }
            }
            break;
        }
        case L2::InsId::CMP: {
            auto cmp_ins = static_cast<L2::Instruction_cmp *>(ins);
            if (!cmp_ins->l->isConstant()) {
                switch (cmp_ins->l->type) {
                    case L2::ItemType::REG: {
                        info->gen.insert(cmp_ins->l->r);
                        break;
                    }
                    case L2::ItemType::VAR: {
                        if (varNames.find(cmp_ins->l->name) == varNames.end()) {
                            int varId = varNames.size() + 16;
                            varNames.emplace(make_pair(cmp_ins->l->name, varId));
                            info->gen.insert(varId);
                        } else
                            info->gen.insert(varNames.find(cmp_ins->l->name)->second);
                        break;
                    }
                    default:;
                }
            }
            if (!cmp_ins->r->isConstant()) {
                switch (cmp_ins->r->type) {
                    case L2::ItemType::REG: {
                        info->gen.insert(cmp_ins->r->r);
                        break;
                    }
                    case L2::ItemType::VAR: {
                        if (varNames.find(cmp_ins->r->name) == varNames.end()) {
                            int varId = varNames.size() + 16;
                            varNames.emplace(make_pair(cmp_ins->r->name, varId));
                            info->gen.insert(varId);
                        } else
                            info->gen.insert(varNames.find(cmp_ins->r->name)->second);
                        break;
                    }
                    default:;
                }
            }
            break;
        }
        case L2::InsId::LABEL: {
            break;
        }
        case L2::InsId::DJUMP: {
            break;
        }
        case L2::InsId::CJUMP: {
            auto cjump_ins = static_cast<L2::Instruction_cjump *>(ins);
            if (!cjump_ins->l->isConstant()) {
                switch (cjump_ins->l->type) {
                    case L2::ItemType::REG: {
                        info->gen.insert(cjump_ins->l->r);
                        break;
                    }
                    case L2::ItemType::VAR: {
                        if (varNames.find(cjump_ins->l->name) == varNames.end()) {
                            int varId = varNames.size() + 16;
                            varNames.emplace(make_pair(cjump_ins->l->name, varId));
                            info->gen.insert(varId);
                        } else
                            info->gen.insert(varNames.find(cjump_ins->l->name)->second);
                        break;
                    }
                    default:;
                }
            }
            if (!cjump_ins->r->isConstant()) {
                switch (cjump_ins->r->type) {
                    case L2::ItemType::REG: {
                        info->gen.insert(cjump_ins->r->r);
                        break;
                    }
                    case L2::ItemType::VAR: {
                        if (varNames.find(cjump_ins->r->name) == varNames.end()) {
                            int varId = varNames.size() + 16;
                            varNames.emplace(make_pair(cjump_ins->r->name, varId));
                            info->gen.insert(varId);
                        } else
                            info->gen.insert(varNames.find(cjump_ins->r->name)->second);
                        break;
                    }
                    default:;
                }
            }
            break;
        }
        case L2::InsId::CISC: {
            auto cisc_in = static_cast<L2::Instruction_CISC *>(ins);
            if (!cisc_in->base->isConstant()) {
                switch (cisc_in->base->type) {
                    case L2::ItemType::REG: {
                        info->gen.insert(cisc_in->base->r);
                        break;
                    }
                    case L2::ItemType::VAR: {
                        if (varNames.find(cisc_in->base->name) == varNames.end()) {
                            int varId = varNames.size() + 16;
                            varNames.emplace(make_pair(cisc_in->base->name, varId));
                            info->gen.insert(varId);
                        } else
                            info->gen.insert(varNames.find(cisc_in->base->name)->second);
                        break;
                    }
                    default:
                        break;
                }
            }
            if (!cisc_in->index->isConstant()) {
                switch (cisc_in->index->type) {
                    case L2::ItemType::REG: {
                        info->gen.insert(cisc_in->index->r);
                        break;
                    }
                    case L2::ItemType::VAR: {
                        if (varNames.find(cisc_in->index->name) == varNames.end()) {
                            int varId = varNames.size() + 16;
                            varNames.emplace(make_pair(cisc_in->index->name, varId));
                            info->gen.insert(varId);
                        } else
                            info->gen.insert(varNames.find(cisc_in->index->name)->second);
                        break;
                    }
                    default:;
                }
            }
            break;
        }
        case L2::InsId::CALL: {
            auto call_ins = static_cast<L2::Instruction_call *>(ins);

            uint argumentCount = static_cast<L2::Constant *>(call_ins->num)->num;

            for (uint i = 0; i < min((uint) 6, argumentCount); i++) {
                info->gen.insert(argumentRegisters[i]);
            }

            if (!call_ins->isRuntime) {
                if (!call_ins->u->isConstant()) {
                    switch (call_ins->u->type) {
                        case L2::ItemType::REG: {
                            info->gen.insert(call_ins->u->r);
                            break;
                        }
                        case L2::ItemType::VAR: {
                            if (varNames.find(call_ins->u->name) == varNames.end()) {
                                int varId = varNames.size() + 16;
                                varNames.emplace(make_pair(call_ins->u->name, varId));
                                info->gen.insert(varId);
                            } else
                                info->gen.insert(varNames.find(call_ins->u->name)->second);
                            break;
                        }
                        default:
                            break;
                    }
                }
            }
            break;
        }
        case L2::InsId::STACK: {
            break;
        }
        default:
            break;
    }

    info->gen.erase(L2::rsp);
}

void Analysis::getKill(InsLiveness *info, L2::Instruction *ins) {
    switch (ins->getTypeId()) {
        case L2::InsId::RET: {
            break;
        }
        case L2::InsId::ASS: {
            auto ass_ins = static_cast<L2::Instruction_assignment *>(ins);
            if (!ass_ins->dst->isConstant()) {
                switch (ass_ins->dst->type) {
                    case L2::ItemType::REG: {
                        info->kill.insert(ass_ins->dst->r);
                        break;
                    }
                    case L2::ItemType::VAR: {
                        if (varNames.find(ass_ins->dst->name) == varNames.end()) {
                            int varId = varNames.size() + 16;
                            varNames.emplace(make_pair(ass_ins->dst->name, varId));
                            info->kill.insert(varId);
                        } else
                            info->kill.insert(varNames.find(ass_ins->dst->name)->second);
                        break;
                    }
                    default:
                        break;
                }
            }
            break;
        }
        case L2::InsId::LOAD: {
            auto load_ins = static_cast<L2::Instruction_load *>(ins);
            if (!load_ins->dst->isConstant()) {
                switch (load_ins->dst->type) {
                    case L2::ItemType::REG: {
                        info->kill.insert(load_ins->dst->r);
                        break;
                    }
                    case L2::ItemType::VAR: {
                        if (varNames.find(load_ins->dst->name) == varNames.end()) {
                            int varId = varNames.size() + 16;
                            varNames.emplace(make_pair(load_ins->dst->name, varId));
                            info->kill.insert(varId);
                        } else
                            info->kill.insert(varNames.find(load_ins->dst->name)->second);
                        break;
                    }
                    default:
                        break;
                }
            }
            break;
        }
        case L2::InsId::STORE: {
            break;
        }
        case L2::InsId::AOP_ASS: {
            auto aop_assign_ins = static_cast<L2::Instruction_aop_assignment *>(ins);
            if (!aop_assign_ins->dst->isConstant()) {
                switch (aop_assign_ins->dst->type) {
                    case L2::ItemType::REG: {
                        info->kill.insert(aop_assign_ins->dst->r);
                        break;
                    }
                    case L2::ItemType::VAR: {
                        if (varNames.find(aop_assign_ins->dst->name) == varNames.end()) {
                            int varId = varNames.size() + 16;
                            varNames.emplace(make_pair(aop_assign_ins->dst->name, varId));
                            info->kill.insert(varId);
                        } else
                            info->kill.insert(varNames.find(aop_assign_ins->dst->name)->second);
                        break;
                    }
                    default:
                        break;
                }
            }
            break;
        }
        case L2::InsId::SELFAOP: {
            auto self_aop_ins = static_cast<L2::Instruction_selfAop *>(ins);
            if (!self_aop_ins->reg->isConstant()) {
                switch (self_aop_ins->reg->type) {
                    case L2::ItemType::REG: {
                        info->kill.insert(self_aop_ins->reg->r);
                        break;
                    }
                    case L2::ItemType::VAR: {
                        if (varNames.find(self_aop_ins->reg->name) == varNames.end()) {
                            int varId = varNames.size() + 16;
                            varNames.emplace(make_pair(self_aop_ins->reg->name, varId));
                            info->kill.insert(varId);
                        } else
                            info->kill.insert(varNames.find(self_aop_ins->reg->name)->second);
                        break;
                    }
                    default:
                        break;
                }
            }
            break;
        }
        case L2::InsId::AOP_LOAD: {
            auto aop_load_ins = static_cast<L2::Instruction_aop_load *>(ins);
            switch (aop_load_ins->dst->type) {
                case L2::ItemType::REG: {
                    info->kill.insert(aop_load_ins->dst->r);
                    break;
                }
                case L2::ItemType::VAR: {
                    if (varNames.find(aop_load_ins->dst->name) == varNames.end()) {
                        int varId = varNames.size() + 16;
                        varNames.emplace(make_pair(aop_load_ins->dst->name, varId));
                        info->kill.insert(varId);
                    } else
                        info->kill.insert(varNames.find(aop_load_ins->dst->name)->second);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case L2::InsId::AOP_STORE: {
            break;
        }
        case L2::InsId::SOP: {
            auto sop_ins = static_cast<L2::Instruction_sop *>(ins);
            switch (sop_ins->reg->type) {
                case L2::ItemType::REG: {
                    info->kill.insert(sop_ins->reg->r);
                    break;
                }
                case L2::ItemType::VAR: {
                    if (varNames.find(sop_ins->reg->name) == varNames.end()) {
                        int varId = varNames.size() + 16;
                        varNames.emplace(make_pair(sop_ins->reg->name, varId));
                        info->kill.insert(varId);
                    } else
                        info->kill.insert(varNames.find(sop_ins->reg->name)->second);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case L2::InsId::CMP: {
            auto cmp_ins = static_cast<L2::Instruction_cmp *>(ins);
            switch (cmp_ins->reg->type) {
                case L2::ItemType::REG: {
                    info->kill.insert(cmp_ins->reg->r);
                    break;
                }
                case L2::ItemType::VAR: {
                    if (varNames.find(cmp_ins->reg->name) == varNames.end()) {
                        int varId = varNames.size() + 16;
                        varNames.emplace(make_pair(cmp_ins->reg->name, varId));
                        info->kill.insert(varId);
                    } else
                        info->kill.insert(varNames.find(cmp_ins->reg->name)->second);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case L2::InsId::LABEL: {
            break;
        }
        case L2::InsId::DJUMP: {
            break;
        }
        case L2::InsId::CJUMP: {
            break;
        }
        case L2::InsId::CISC: {
            auto cisc_ins = static_cast<L2::Instruction_CISC *>(ins);
            switch (cisc_ins->reg->type) {
                case L2::ItemType::REG: {
                    info->kill.insert(cisc_ins->reg->r);
                    break;
                }
                case L2::ItemType::VAR: {
                    if (varNames.find(cisc_ins->reg->name) == varNames.end()) {
                        int varId = varNames.size() + 16;
                        varNames.emplace(make_pair(cisc_ins->reg->name, varId));
                        info->kill.insert(varId);
                    } else
                        info->kill.insert(varNames.find(cisc_ins->reg->name)->second);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case L2::InsId::CALL: {

            for (const auto &v: callerSavedRegisters) {
                info->kill.insert(v);
            }

            break;
        }
        case L2::InsId::STACK: {
            auto stack_ins = static_cast<L2::Instruction_stackArg *>(ins);
            switch (stack_ins->reg->type) {
                case L2::ItemType::REG: {
                    info->kill.insert(stack_ins->reg->r);
                    break;
                }
                case L2::ItemType::VAR: {
                    if (varNames.find(stack_ins->reg->name) == varNames.end()) {
                        int varId = varNames.size() + 16;
                        varNames.emplace(make_pair(stack_ins->reg->name, varId));
                        info->kill.insert(varId);
                    } else
                        info->kill.insert(varNames.find(stack_ins->reg->name)->second);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }

    info->kill.erase(L2::rsp);
}

void Analysis::analyzeSuccessor(L2::Function *f) {
    for (int i = 0; i < infos.size(); i++) {
        switch (f->instructions[i]->getTypeId()) {
            case L2::DJUMP: {
                auto djump_ins = static_cast<L2::Instruction_djump *>(f->instructions[i]);
                infos[i]->successor.push_back(labels.find(djump_ins->dst->name)->second);
                break;
            }
            case L2::RET: {
                break;
            }
            case L2::CJUMP: {
                auto cjump_ins = static_cast<L2::Instruction_cjump *>(f->instructions[i]);
                infos[i]->successor.push_back(labels.find(cjump_ins->dst->name)->second);
                if (i != f->instructions.size() - 1) {
                    infos[i]->successor.push_back(infos[i + 1]);
                }
                break;
            }
            case L2::CALL: {
                auto call_ins = static_cast<L2::Instruction_call *>(f->instructions[i]);
                if (call_ins->isRuntime && call_ins->runtimeId == L2::ARRAY) {

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

void Analysis::computeInOut() {
    bool needNextIteration = true;

    while (needNextIteration) {
        needNextIteration = false;

        for (auto info: infos) {
            unordered_set<int> newIn;

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

            unordered_set<int> newOut;

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

void Analysis::printInOut(ostream &out) {
    out << "(" << endl;

    out << "(in" << endl;

    for (auto info: infos) {
        Analysis::printHashSet(out, info->in);
        out << endl;
    }

    out << ")" << endl;
    out << endl;
    out << "(out" << endl;

    for (auto info: infos) {
        Analysis::printHashSet(out, info->out);
        out << endl;
    }

    out << ")" << endl;
    out << endl;

    out << ")";
}

void Analysis::computeInterferenceGraph() {

    for (const auto &v: generalPurposeRegisters) { // connect a general purpose register to all other registers

        for (const auto &w: generalPurposeRegisters) {
            if (v != w) {
                if (interferenceGraph.find(v) == interferenceGraph.end()) { // v not in this.interferenceGraph
                    unordered_set<int> set = {w};
                    interferenceGraph.emplace(v, set);
                } else {
                    interferenceGraph[v].insert(w);
                }

                if (interferenceGraph.find(w) == interferenceGraph.end()) { // w not in this.interferenceGraph
                    unordered_set<int> set = {v};
                    interferenceGraph.emplace(w, set);
                } else {
                    interferenceGraph[w].insert(v);
                }
            }
        }

    }

    for (auto info: infos) { // connect each pair of variables that belong to the same IN set

        for (const auto &v: info->in) {

            for (const auto &w: info->in) {
                if (v != w) {
                    if (interferenceGraph.find(v) == interferenceGraph.end()) { // v not in this.interferenceGraph
                        unordered_set<int> set = {w};
                        interferenceGraph.emplace(v, set);
                    } else {
                        interferenceGraph[v].insert(w);
                    }

                    if (interferenceGraph.find(w) == interferenceGraph.end()) { // w not in this.interferenceGraph
                        unordered_set<int> set = {v};
                        interferenceGraph.emplace(w, set);
                    } else {
                        interferenceGraph[w].insert(v);
                    }
                }
            }

        }

        for (const auto &v: info->out) { // connect each pair of variables that belong to the same OUT set

            for (const auto &w: info->out) {
                if (v != w) {
                    if (interferenceGraph.find(v) == interferenceGraph.end()) { // v not in this.interferenceGraph
                        unordered_set<int> set = {w};
                        interferenceGraph.emplace(v, set);
                    } else {
                        interferenceGraph[v].insert(w);
                    }

                    if (interferenceGraph.find(w) == interferenceGraph.end()) { // w not in this.interferenceGraph
                        unordered_set<int> set = {v};
                        interferenceGraph.emplace(w, set);
                    } else {
                        interferenceGraph[w].insert(v);
                    }
                }
            }

        }

        for (const auto &v: info->kill) {

            for (const auto &w: info->out) {
                if (v != w) {
                    if (interferenceGraph.find(v) == interferenceGraph.end()) { // v not in this.interferenceGraph
                        unordered_set<int> set = {w};
                        interferenceGraph.emplace(v, set);
                    } else {
                        interferenceGraph[v].insert(w);
                    }

                    if (interferenceGraph.find(w) == interferenceGraph.end()) { // w not in this.interferenceGraph
                        unordered_set<int> set = {v};
                        interferenceGraph.emplace(w, set);
                    } else {
                        interferenceGraph[w].insert(v);
                    }
                }
            }

        }


    }

    unordered_set<int> noRcxSet;

    for (auto w: generalPurposeRegisters) {
        if (w != L2::Register::rcx) {
            noRcxSet.emplace(w);
        }
    }

    for (auto v: this->sopSrcVar) {
        if (interferenceGraph.find(v) == interferenceGraph.end()) {
            interferenceGraph.emplace(v, noRcxSet);
        } else {

            for (auto w: noRcxSet) {
                interferenceGraph[v].insert(w);
                interferenceGraph[w].insert(v);
            }

        }
    }
    generateNodes();
}

void Analysis::coloring() {
    for (int i = 0; i < colorOfNodes.size(); i++) {
        removeAndPutNode();
    }
    while (!interferenceStack.empty()) {
        rebuildGraph();
    }
}

void Analysis::removeAndPutNode() {
    int max = 0;
    int max_index = -1;
    int backup_index = -1;
    for (const auto &node : interferenceGraph) {
        if (node.first > 15) {
            int degree = node.second.size();
            if (degree < 15 && degree >= max) {
                max = degree;
                max_index = node.first;
            } else if (degree >= 15) {
                backup_index = node.first;
            }
        }
    }
    if (max_index == -1) {
        max_index = backup_index;
    }
    auto max_node = interferenceGraph.find(max_index);
    for (auto neighbor : max_node->second) {
        interferenceGraph.find(neighbor)->second.erase(max_index);
    }
    interferenceStack.push(*max_node);
    interferenceGraph.erase(max_index);
}

void Analysis::rebuildGraph() {
    bool needSpill = true;
    auto tmp_node = interferenceStack.top();
    interferenceStack.pop();
    for (int color = 0; color < 15; color++) {
        if (tryColoring(color, tmp_node)) {
            needSpill = false;
            break;
        }
    }
    if (needSpill) {
        spillList.push_back(tmp_node.first);
    }
    for (auto neighbor : tmp_node.second) {
        interferenceGraph[neighbor].insert(tmp_node.first);
    }
    interferenceGraph.emplace(tmp_node);
}

bool Analysis::tryColoring(int color, node &cur) {
    for (auto neighbor : cur.second) {
        if (interferenceGraph.find(neighbor) != interferenceGraph.end()) {
            if (neighbor < 15) {
                if (neighbor == color)
                    return false;
            } else {
                if (colorOfNodes[neighbor] == color)
                    return false;
            }
        }
    }
    colorOfNodes[cur.first] = color;
    return true;
}

bool Analysis::allocator(L2::Function *f) {
    coloring();
    if (!spillList.empty()) {
        return false;
    } else {
        replaceAllWithColor(f);
        return true;
    }
}

void Analysis::replaceAllWithColor(L2::Function *f) {
    for (auto ins : f->instructions) {
        int id = ins->getTypeId();
        switch (id) {
            case L2::RET: {
                break;
            }
            case L2::ASS: {
                auto ass_ins = static_cast<L2::Instruction_assignment *>(ins);
                if (ass_ins->src->type == L2::VAR) {
                    replaceVarWithColor(ass_ins->src);
                }
                if (ass_ins->dst->type == L2::VAR) {
                    replaceVarWithColor(ass_ins->dst);
                }
                break;
            }
            case L2::LOAD: {
                auto load_ins = static_cast<L2::Instruction_load *>(ins);
                if (load_ins->src->type == L2::VAR) {
                    replaceVarWithColor(load_ins->src);
                }
                if (load_ins->dst->type == L2::VAR) {
                    replaceVarWithColor(load_ins->dst);
                }
                break;
            }
            case L2::STORE: {
                auto store_ins = static_cast<L2::Instruction_store *>(ins);
                if (store_ins->src->type == L2::VAR) {
                    replaceVarWithColor(store_ins->src);
                }
                if (store_ins->dst->type == L2::VAR) {
                    replaceVarWithColor(store_ins->dst);
                }
                break;
            }
            case L2::AOP_ASS: {
                auto aop_assign_ins = static_cast<L2::Instruction_aop_assignment *>(ins);
                if (aop_assign_ins->src->type == L2::VAR) {
                    replaceVarWithColor(aop_assign_ins->src);
                }
                if (aop_assign_ins->dst->type == L2::VAR) {
                    replaceVarWithColor(aop_assign_ins->dst);
                }
                break;
            }
            case L2::SELFAOP: {
                auto selfAop_ins = static_cast<L2::Instruction_selfAop *>(ins);
                if (selfAop_ins->reg->type == L2::VAR) {
                    replaceVarWithColor(selfAop_ins->reg);
                }
                break;
            }
            case L2::AOP_LOAD: {
                auto aop_load_ins = static_cast<L2::Instruction_aop_load *>(ins);
                if (aop_load_ins->src->type == L2::VAR) {
                    replaceVarWithColor(aop_load_ins->src);
                }
                if (aop_load_ins->dst->type == L2::VAR) {
                    replaceVarWithColor(aop_load_ins->dst);
                }
                break;
            }
            case L2::AOP_STORE: {
                auto aop_store_ins = static_cast<L2::Instruction_aop_store *>(ins);
                if (aop_store_ins->src->type == L2::VAR) {
                    replaceVarWithColor(aop_store_ins->src);
                }
                if (aop_store_ins->dst->type == L2::VAR) {
                    replaceVarWithColor(aop_store_ins->dst);
                }
                break;
            }
            case L2::SOP: {
                auto sop_ins = static_cast<L2::Instruction_sop *>(ins);
                if (sop_ins->reg->type == L2::VAR) {
                    replaceVarWithColor(sop_ins->reg);
                }
                if (sop_ins->offset->type == L2::VAR) {
                    replaceVarWithColor(sop_ins->offset);
                }
                break;
            }
            case L2::CMP: {
                auto cmp_ins = static_cast<L2::Instruction_cmp *>(ins);
                if (cmp_ins->l->type == L2::VAR) {
                    replaceVarWithColor(cmp_ins->l);
                }
                if (cmp_ins->r->type == L2::VAR) {
                    replaceVarWithColor(cmp_ins->r);
                }
                if (cmp_ins->reg->type == L2::VAR) {
                    replaceVarWithColor(cmp_ins->reg);
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
                if (cjump_ins->l->type == L2::VAR) {
                    replaceVarWithColor(cjump_ins->l);
                }
                if (cjump_ins->r->type == L2::VAR) {
                    replaceVarWithColor(cjump_ins->r);
                }
                break;
            }
            case L2::CISC: {
                auto CISC_ins = static_cast<L2::Instruction_CISC *>(ins);
                if (CISC_ins->reg->type == L2::VAR) {
                    replaceVarWithColor(CISC_ins->reg);
                }
                if (CISC_ins->base->type == L2::VAR) {
                    replaceVarWithColor(CISC_ins->base);
                }
                if (CISC_ins->index->type == L2::VAR) {
                    replaceVarWithColor(CISC_ins->index);
                }
                break;
            }
            case L2::CALL: {
                auto call_ins = static_cast<L2::Instruction_call *>(ins);
                if (!call_ins->isRuntime) {
                    if (call_ins->u->type == L2::VAR) {
                        replaceVarWithColor(call_ins->u);
                    }
                }
                break;
            }
            case L2::STACK: {
                auto stack_ins = static_cast<L2::Instruction_stackArg *>(ins);
                if (stack_ins->reg->type == L2::VAR) {
                    replaceVarWithColor(stack_ins->reg);
                }
                break;
            }
            default:;
        }
    }
}

void Analysis::replaceVarWithColor(L2::Item *var) {
    var->type = L2::REG;
    var->r = static_cast<L2::Register>(colorOfNodes[varNames[var->name]]);
    var->name = "";
}

void Analysis::printInterferenceGraph(ostream &out) {
    for (const auto &entry: interferenceGraph) {
        printNameWithId(out, entry.first);

        for (const auto &neighbor: entry.second) {
            out << " ";
            printNameWithId(out, neighbor);
        }

        out << endl;
    }
}

void Analysis::printGenKill(ostream &out) {
    for (auto info : infos) {
        out << "GEN: ";
        printHashSet(out, info->gen);
        out << "; KILL: ";
        printHashSet(out, info->kill);
        out << endl;
    }
}

void Analysis::printHashSet(ostream &out, unordered_set<int> &hashSet) {
    out << "(";
    auto it = hashSet.begin();
    for (int i = 0; i < hashSet.size(); i++) {
        printNameWithId(out, *it);
        if (i != hashSet.size() - 1)
            out << " ";
        ++it;
    }
    out << ")";
}

void Analysis::generateNodes() {
    for (const auto &vertex : interferenceGraph) {
        if (vertex.first >= 16) {
            colorOfNodes.insert({vertex.first, -1});
        }
    }
}

void Analysis::printNameWithId(ostream &out, int id) {
    if (id < 16)
        out << L2::register_name[id];
    else
        out << varIndex[id];
}

void Analysis::batchSpill(L2::Function *f, int &prefixIndex) {
    for (auto var : spillList) {
        f->targetVar->name = varIndex[var];
        f->spillerVar->name = "S" + to_string(prefixIndex);
        spill(f);
        prefixIndex++;
    }
}

Analysis::~Analysis() {
    for (auto info : infos) {
        delete info;
    }
}
