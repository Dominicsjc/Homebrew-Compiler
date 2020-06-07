//
// Created by dominic on 3/1/20.
//
#include "scopeTranslator.h"
#include "utils.h"

std::deque<std::unordered_map<std::string, std::string>> maps;

void translateMultiDeclare(LB::Scope *currentS) {
    for (int i = 0; i < currentS->size(); i++) {
        auto ins = (*currentS)[i];
        if (ins->getTypeId() == LB::SCOPE) {
            auto scope_ins = static_cast<LB::Instruction_scope *>(ins);
            translateMultiDeclare(scope_ins->content);
        } else if (ins->getTypeId() == LB::DECLARE) {
            auto declare_ins = static_cast<LB::Instruction_declare *>(ins);
            while (declare_ins->vars.size() > 1) {
                auto new_declare_ins = new LB::Instruction_declare;
                new_declare_ins->type = copyLBItem(declare_ins->type);
                new_declare_ins->vars.emplace_back(copyLBItem(declare_ins->vars.back()));
                declare_ins->vars.pop_back();
                currentS->insert(currentS->begin() + i, new_declare_ins);
                i++;
            }
        }
    }
}

void renameDeclare(LB::Scope *currentS, std::unordered_map<std::string, std::string> &renameMap, int newVarId) {
    for (auto ins : *currentS) {
        if (ins->getTypeId() == LB::DECLARE) {
            auto declare_ins = static_cast<LB::Instruction_declare *>(ins);
            auto suffix = std::to_string(newVarId) + static_cast<LB::Variable *>(declare_ins->vars[0])->name;
            renameMap.insert({static_cast<LB::Variable *>(declare_ins->vars[0])->name, "LBNewVar" + suffix});
            delete declare_ins->vars[0];
            declare_ins->vars.clear();
            declare_ins->vars.emplace_back(createNewVarWithSuffix(suffix));
        } else if (ins->getTypeId() == LB::SCOPE) {
            auto scope_ins = static_cast<LB::Instruction_scope *>(ins);
            renameDeclare(scope_ins->content, scope_ins->renameMap, newVarId + 1);
        }
    }
}

void renameIns(LB::Scope *currentS) {
    for (auto ins : *currentS) {
        switch (ins->getTypeId()) {
            case LB::VRET: {
                auto vret_ins = static_cast<LB::Instruction_value_ret *>(ins);
                renameVar(vret_ins->t, maps);
                break;
            }
            case LB::ASS: {
                auto ass_ins = static_cast<LB::Instruction_assignment *>(ins);
                renameVar(ass_ins->dst, maps);
                renameVar(ass_ins->src, maps);
                break;
            }
            case LB::LOAD: {
                auto load_ins = static_cast<LB::Instruction_load *>(ins);
                renameVar(load_ins->dst, maps);
                renameVar(load_ins->src, maps);
                for (auto index : load_ins->indices) {
                    renameVar(index, maps);
                }
                break;
            }
            case LB::STORE: {
                auto store_ins = static_cast<LB::Instruction_store *>(ins);
                renameVar(store_ins->dst, maps);
                renameVar(store_ins->src, maps);
                for (auto index : store_ins->indices) {
                    renameVar(index, maps);
                }
                break;
            }
            case LB::LENGTH: {
                auto store_length_ins = static_cast<LB::Instruction_store_length *>(ins);
                renameVar(store_length_ins->dst, maps);
                renameVar(store_length_ins->src, maps);
                renameVar(store_length_ins->t, maps);
                break;
            }
            case LB::OP_ASS: {
                auto op_assign_ins = static_cast<LB::Instruction_op_assignment *>(ins);
                renameVar(op_assign_ins->var, maps);
                renameVar(op_assign_ins->l, maps);
                renameVar(op_assign_ins->r, maps);
                break;
            }
            case LB::IF: {
                auto if_ins = static_cast<LB::Instruction_if *>(ins);
                renameVar(if_ins->l, maps);
                renameVar(if_ins->r, maps);
                break;
            }
            case LB::WHILE: {
                auto while_ins = static_cast<LB::Instruction_if *>(ins);
                renameVar(while_ins->l, maps);
                renameVar(while_ins->r, maps);
                break;
            }
            case LB::CALL: {
                auto call_ins = static_cast<LB::Instruction_call *>(ins);
                if (!call_ins->isPrint) {
                    renameVar(call_ins->u, maps);
                }
                for (auto arg : call_ins->args) {
                    renameVar(arg, maps);
                }
                break;
            }
            case LB::CALL_ASS: {
                auto call_ass_ins = static_cast<LB::Instruction_call_assignment *>(ins);
                renameVar(call_ass_ins->u, maps);
                renameVar(call_ass_ins->var, maps);
                for (auto arg : call_ass_ins->args) {
                    renameVar(arg, maps);
                }
                break;
            }
            case LB::NEW_ARRAY: {
                auto new_array_ins = static_cast<LB::Instruction_new_array *>(ins);
                renameVar(new_array_ins->var, maps);
                for (auto arg : new_array_ins->args) {
                    renameVar(arg, maps);
                }
                break;
            }
            case LB::NEW_TUPLE: {
                auto new_tuple_ins = static_cast<LB::Instruction_new_tuple *>(ins);
                renameVar(new_tuple_ins->var, maps);
                renameVar(new_tuple_ins->t, maps);
                break;
            }
            case LB::SCOPE: {
                auto scope_ins = static_cast<LB::Instruction_scope *>(ins);
                maps.push_front(scope_ins->renameMap);
                renameIns(scope_ins->content);
                break;
            }
            default:;
        }
    }
    maps.pop_front();
}

void renameVar(LB::Item *i, std::deque<std::unordered_map<std::string, std::string>> &myMaps) {
    if(i->getTypeId() != LB::VAR) {
        return;
    } else {
        auto var = static_cast<LB::Variable *>(i);
        for (auto map : myMaps) {
            if (map.find(var->name) != map.end()) {
                var->name = map[var->name];
                break;
            }
        }
    }
}

void removeNestedScopes(LB::Scope *currentS, std::vector<LB::Instruction *> &instructions) {
    for (auto ins : *currentS) {
        if (ins->getTypeId() == LB::SCOPE) {
            auto scope_ins = static_cast<LB::Instruction_scope *>(ins);
            removeNestedScopes(scope_ins->content, instructions);
        } else {
            instructions.push_back(ins);
        }
    }
    currentS->clear();
    delete currentS;
}

void flatScopes(LB::Function *f) {
    renameDeclare(f->root, f->renameMap, 0);
    maps.push_front(f->renameMap);
    renameIns(f->root);
    removeNestedScopes(f->root, f->instructions);
}
