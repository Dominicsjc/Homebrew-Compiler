#include "globalization.h"
#include <unordered_set>

using namespace std;

void labelGlobalize(L3::Program &p) {
    unordered_set<string> function_names;
    for (auto f : p.functions) {
        function_names.insert(f->name);
    }
    for (auto f : p.functions) {
        for (auto ins : f->instructions) {
            int type = ins->getTypeId();
            switch (type) {
                case L3::ASS: {
                    auto ass_ins = static_cast<L3::Instruction_assignment *>(ins);

                    if (!ass_ins->src->isConstant() && ass_ins->src->type == L3::LAB && function_names.find(ass_ins->src->name) == function_names.end()) {
                        addPrefix(ass_ins->src, f);
                    }

                    break;
                }
                case L3::STORE: {
                    auto store_ins = static_cast<L3::Instruction_store *>(ins);

                    if (!store_ins->src->isConstant() && store_ins->src->type == L3::LAB && function_names.find(store_ins->src->name) == function_names.end()) {
                        addPrefix(store_ins->src, f);
                    }

                    break;
                }
                case L3::LABEL: {
                    auto label_ins = static_cast<L3::Instruction_label *>(ins);

                    addPrefix(label_ins->label, f);

                    break;
                }
                case L3::DBR: {
                    auto dbr_ins = static_cast<L3::Instruction_dbr *>(ins);

                    addPrefix(dbr_ins->dst, f);

                    break;
                }
                case L3::VBR: {
                    auto vbr_ins = static_cast<L3::Instruction_vbr *>(ins);

                    addPrefix(vbr_ins->dst, f);

                    break;
                }
                default:;
            }
        }
    }
}

void addPrefix(L3::Item *label, L3::Function *f) {
    label->name = f->name + "_" + label->name.substr(1);
}
