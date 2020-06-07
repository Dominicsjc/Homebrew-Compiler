#include <sched.h>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <assert.h>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

#include <L2.h>
#include <parser.h>

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;

namespace L2 {

    /*
     * Data required to parse
     */
    std::vector<Item *> parsed_items;

    /*
     * Grammar rules from now on.
     */
    struct name :
            pegtl::seq<
                    pegtl::plus<
                            pegtl::sor<
                                    pegtl::alpha,
                                    pegtl::one<'_'>
                            >
                    >,
                    pegtl::star<
                            pegtl::sor<
                                    pegtl::alpha,
                                    pegtl::one<'_'>,
                                    pegtl::digit
                            >
                    >
            > {
    };

    /*
     * Keywords.
     */
    struct str_arrow : TAOCPP_PEGTL_STRING("<-") {
    };
    struct str_return : TAOCPP_PEGTL_STRING("return") {
    };
    struct str_rdi : TAOCPP_PEGTL_STRING("rdi") {
    };
    struct str_rax : TAOCPP_PEGTL_STRING("rax") {
    };
    struct str_rbx : TAOCPP_PEGTL_STRING("rbx") {
    };
    struct str_rcx : TAOCPP_PEGTL_STRING("rcx") {
    };
    struct str_rdx : TAOCPP_PEGTL_STRING("rdx") {
    };
    struct str_rsi : TAOCPP_PEGTL_STRING("rsi") {
    };
    struct str_rbp : TAOCPP_PEGTL_STRING("rbp") {
    };
    struct str_rsp : TAOCPP_PEGTL_STRING("rsp") {
    };
    struct str_r8 : TAOCPP_PEGTL_STRING("r8") {
    };
    struct str_r9 : TAOCPP_PEGTL_STRING("r9") {
    };
    struct str_r10 : TAOCPP_PEGTL_STRING("r10") {
    };
    struct str_r11 : TAOCPP_PEGTL_STRING("r11") {
    };
    struct str_r12 : TAOCPP_PEGTL_STRING("r12") {
    };
    struct str_r13 : TAOCPP_PEGTL_STRING("r13") {
    };
    struct str_r14 : TAOCPP_PEGTL_STRING("r14") {
    };
    struct str_r15 : TAOCPP_PEGTL_STRING("r15") {
    };
    struct str_mem : TAOCPP_PEGTL_STRING("mem") {
    };
    struct str_add : TAOCPP_PEGTL_STRING("+=") {
    };
    struct str_sub : TAOCPP_PEGTL_STRING("-=") {
    };
    struct str_multi : TAOCPP_PEGTL_STRING("*=") {
    };
    struct str_and : TAOCPP_PEGTL_STRING("&=") {
    };
    struct str_selfAdd : TAOCPP_PEGTL_STRING("++") {
    };
    struct str_selfSub : TAOCPP_PEGTL_STRING("--") {
    };
    struct str_lshift : TAOCPP_PEGTL_STRING("<<=") {
    };
    struct str_rshift : TAOCPP_PEGTL_STRING(">>=") {
    };
    struct str_le : TAOCPP_PEGTL_STRING("<") {
    };
    struct str_leq : TAOCPP_PEGTL_STRING("<=") {
    };
    struct str_eql : TAOCPP_PEGTL_STRING("=") {
    };
    struct str_goto : TAOCPP_PEGTL_STRING("goto") {
    };
    struct str_cjump : TAOCPP_PEGTL_STRING("cjump") {
    };
    struct str_at : TAOCPP_PEGTL_STRING("@") {
    };
    struct str_call : TAOCPP_PEGTL_STRING("call") {
    };
    struct str_print : TAOCPP_PEGTL_STRING("print") {
    };
    struct str_allocate : TAOCPP_PEGTL_STRING("allocate") {
    };
    struct str_arrayError : TAOCPP_PEGTL_STRING("array-error") {
    };
    struct str_stackArg : TAOCPP_PEGTL_STRING("stack-arg") {
    };

    struct label :
            pegtl::seq<
                    pegtl::one<':'>,
                    name
            > {
    };

    struct var :
            pegtl::seq<
                    pegtl::one<'%'>,
                    name
            > {
    };

    struct register_rdi_rule :
            str_rdi {
    };

    struct register_rax_rule :
            str_rax {
    };

    struct register_rbx_rule :
            str_rbx {
    };

    struct register_rcx_rule :
            str_rcx {
    };

    struct register_rdx_rule :
            str_rdx {
    };

    struct register_rsi_rule :
            str_rsi {
    };

    struct register_rbp_rule :
            str_rbp {
    };

    struct register_rsp_rule :
            str_rsp {
    };

    struct register_r8_rule :
            str_r8 {
    };

    struct register_r9_rule :
            str_r9 {
    };

    struct register_r10_rule :
            str_r10 {
    };

    struct register_r11_rule :
            str_r11 {
    };

    struct register_r12_rule :
            str_r12 {
    };

    struct register_r13_rule :
            str_r13 {
    };

    struct register_r14_rule :
            str_r14 {
    };

    struct register_r15_rule :
            str_r15 {
    };

    struct var_rule :
            var {
    };

    struct register_rule :
            pegtl::sor<
                    register_rdi_rule,
                    register_rax_rule,
                    register_rbx_rule,
                    register_rcx_rule,
                    register_rdx_rule,
                    register_rsi_rule,
                    register_rbp_rule,
                    register_rsp_rule,
                    register_r8_rule,
                    register_r9_rule,
                    register_r10_rule,
                    register_r11_rule,
                    register_r12_rule,
                    register_r13_rule,
                    register_r14_rule,
                    register_r15_rule,
                    var_rule
            > {
    };

    struct number :
            pegtl::seq<
                    pegtl::opt<
                            pegtl::sor<
                                    pegtl::one<'-'>,
                                    pegtl::one<'+'>
                            >
                    >,
                    pegtl::plus<
                            pegtl::digit
                    >
            > {
    };

    struct function_name :
            label {
    };

    struct argument_number :
            number {
    };

    struct constant_number :
            number {
    };

    struct comment :
            pegtl::disable<
                    TAOCPP_PEGTL_STRING("//"),
                    pegtl::until<pegtl::eolf>
            > {
    };

    struct seps :
            pegtl::star<
                    pegtl::sor<
                            pegtl::ascii::space,
                            comment
                    >
            > {
    };

    struct Label_rule :
            label {
    };

    struct Instruction_return_rule :
            pegtl::seq<
                    str_return
            > {
    };

    struct Instruction_assignment_rule :
            pegtl::sor<
                    pegtl::seq<
                            register_rule,
                            seps,
                            str_arrow,
                            seps,
                            pegtl::sor<
                                    register_rule,
                                    constant_number,
                                    Label_rule
                            >
                    >
            > {
    };

    struct Instruction_load_rule :
            pegtl::seq<
                    register_rule,
                    seps,
                    str_arrow,
                    seps,
                    str_mem,
                    seps,
                    register_rule,
                    seps,
                    constant_number
            > {
    };

    struct Instruction_store_rule :
            pegtl::seq<
                    str_mem,
                    seps,
                    register_rule,
                    seps,
                    constant_number,
                    seps,
                    str_arrow,
                    seps,
                    pegtl::sor<
                            register_rule,
                            constant_number,
                            Label_rule
                    >
            > {

    };

    struct Instruction_add_assignment_rule :
            pegtl::seq<
                    register_rule,
                    seps,
                    str_add,
                    seps,
                    pegtl::sor<
                            register_rule,
                            constant_number
                    >
            > {
    };

    struct Instruction_sub_assignment_rule :
            pegtl::seq<
                    register_rule,
                    seps,
                    str_sub,
                    seps,
                    pegtl::sor<
                            register_rule,
                            constant_number
                    >
            > {
    };

    struct Instruction_multi_assignment_rule :
            pegtl::seq<
                    register_rule,
                    seps,
                    str_multi,
                    seps,
                    pegtl::sor<
                            register_rule,
                            constant_number
                    >
            > {
    };

    struct Instruction_and_assignment_rule :
            pegtl::seq<
                    register_rule,
                    seps,
                    str_and,
                    seps,
                    pegtl::sor<
                            register_rule,
                            constant_number
                    >
            > {
    };

    struct Instruction_aop_assignment_rule :
            pegtl::sor<
                    Instruction_add_assignment_rule,
                    Instruction_sub_assignment_rule,
                    Instruction_multi_assignment_rule,
                    Instruction_and_assignment_rule
            > {
    };

    struct Instruction_selfAdd_rule :
            pegtl::seq<
                    register_rule,
                    seps,
                    str_selfAdd
            > {
    };

    struct Instruction_selfSub_rule :
            pegtl::seq<
                    register_rule,
                    seps,
                    str_selfSub
            > {
    };

    struct Instruction_selfAop_rule :
            pegtl::sor<
                    Instruction_selfAdd_rule,
                    Instruction_selfSub_rule
            > {
    };

    struct Instruction_add_load_rule :
            pegtl::seq<
                    register_rule,
                    seps,
                    str_add,
                    seps,
                    str_mem,
                    seps,
                    register_rule,
                    seps,
                    constant_number
            > {
    };

    struct Instruction_sub_load_rule :
            pegtl::seq<
                    register_rule,
                    seps,
                    str_sub,
                    seps,
                    str_mem,
                    seps,
                    register_rule,
                    seps,
                    constant_number
            > {
    };

    struct Instruction_aop_load_rule :
            pegtl::sor<
                    Instruction_add_load_rule,
                    Instruction_sub_load_rule
            > {
    };

    struct Instruction_add_store_rule :
            pegtl::seq<
                    str_mem,
                    seps,
                    register_rule,
                    seps,
                    constant_number,
                    seps,
                    str_add,
                    seps,
                    pegtl::sor<
                            register_rule,
                            constant_number
                    >
            > {
    };

    struct Instruction_sub_store_rule :
            pegtl::seq<
                    str_mem,
                    seps,
                    register_rule,
                    seps,
                    constant_number,
                    seps,
                    str_sub,
                    seps,
                    pegtl::sor<
                            register_rule,
                            constant_number
                    >
            > {
    };

    struct Instruction_aop_store_rule :
            pegtl::sor<
                    Instruction_add_store_rule,
                    Instruction_sub_store_rule
            > {
    };

    struct Instruction_lshift_rule :
            pegtl::seq<
                    register_rule,
                    seps,
                    str_lshift,
                    seps,
                    pegtl::sor<
                            register_rcx_rule,
                            var_rule,
                            constant_number
                    >
            > {
    };

    struct Instruction_rshift_rule :
            pegtl::seq<
                    register_rule,
                    seps,
                    str_rshift,
                    seps,
                    pegtl::sor<
                            register_rcx_rule,
                            var_rule,
                            constant_number
                    >
            > {
    };

    struct Instruction_sop_rule :
            pegtl::sor<
                    Instruction_lshift_rule,
                    Instruction_rshift_rule
            > {
    };

    struct Instruction_le_rule :
            pegtl::seq<
                    register_rule,
                    seps,
                    str_arrow,
                    seps,
                    pegtl::sor<
                            register_rule,
                            constant_number
                    >,
                    seps,
                    str_le,
                    seps,
                    pegtl::sor<
                            register_rule,
                            constant_number
                    >
            > {
    };

    struct Instruction_leq_rule :
            pegtl::seq<
                    register_rule,
                    seps,
                    str_arrow,
                    seps,
                    pegtl::sor<
                            register_rule,
                            constant_number
                    >,
                    seps,
                    str_leq,
                    seps,
                    pegtl::sor<
                            register_rule,
                            constant_number
                    >
            > {
    };

    struct Instruction_eql_rule :
            pegtl::seq<
                    register_rule,
                    seps,
                    str_arrow,
                    seps,
                    pegtl::sor<
                            register_rule,
                            constant_number
                    >,
                    seps,
                    str_eql,
                    seps,
                    pegtl::sor<
                            register_rule,
                            constant_number
                    >
            > {
    };

    struct Instruction_cmp_rule :
            pegtl::sor<
                    Instruction_le_rule,
                    Instruction_leq_rule,
                    Instruction_eql_rule
            > {
    };

    struct Instruction_label_rule:
            pegtl::seq<
                    Label_rule
            > {
    };

    struct Instruction_djump_rule:
            pegtl::seq<
                    str_goto,
                    seps,
                    Label_rule
            > {
    };

    struct Instruction_le_jump_rule:
            pegtl::seq<
                    str_cjump,
                    seps,
                    pegtl::sor<
                            register_rule,
                            constant_number
                    >,
                    seps,
                    str_le,
                    seps,
                    pegtl::sor<
                            register_rule,
                            constant_number
                    >,
                    seps,
                    Label_rule
            > {
    };

    struct Instruction_leq_jump_rule:
            pegtl::seq<
                    str_cjump,
                    seps,
                    pegtl::sor<
                            register_rule,
                            constant_number
                    >,
                    seps,
                    str_leq,
                    seps,
                    pegtl::sor<
                            register_rule,
                            constant_number
                    >,
                    seps,
                    Label_rule
            > {
    };

    struct Instruction_eql_jump_rule:
            pegtl::seq<
                    str_cjump,
                    seps,
                    pegtl::sor<
                            register_rule,
                            constant_number
                    >,
                    seps,
                    str_eql,
                    seps,
                    pegtl::sor<
                            register_rule,
                            constant_number
                    >,
                    seps,
                    Label_rule
            > {
    };

    struct Instruction_cjump_rule :
            pegtl::sor<
                    Instruction_le_jump_rule,
                    Instruction_leq_jump_rule,
                    Instruction_eql_jump_rule
            > {
    };

    struct Instruction_CISC_rule :
            pegtl::seq<
                    register_rule,
                    seps,
                    str_at,
                    seps,
                    register_rule,
                    seps,
                    register_rule,
                    seps,
                    constant_number
            > {
    };

    struct Instruction_call_function_rule :
            pegtl::seq<
                    str_call,
                    seps,
                    pegtl::sor<
                            register_rule,
                            Label_rule
                    >,
                    seps,
                    constant_number
            > {
    };

    struct Instruction_call_print_rule :
            pegtl::seq<
                    str_call,
                    seps,
                    str_print,
                    seps,
                    constant_number
            > {
    };

    struct Instruction_call_allocate_rule :
            pegtl::seq<
                    str_call,
                    seps,
                    str_allocate,
                    seps,
                    constant_number
            > {
    };

    struct Instruction_call_arrayError_rule :
            pegtl::seq<
                    str_call,
                    seps,
                    str_arrayError,
                    seps,
                    constant_number
            > {
    };
    
    struct Instruction_call_runtime_rule :
            pegtl::sor<
                    Instruction_call_print_rule,
                    Instruction_call_allocate_rule,
                    Instruction_call_arrayError_rule
            > {
    };

    struct Instruction_call_rule :
            pegtl::sor<
                    Instruction_call_function_rule,
                    Instruction_call_runtime_rule
            > {
    };

    struct Instruction_stackArg_rule :
            pegtl::seq<
                    register_rule,
                    seps,
                    str_arrow,
                    seps,
                    str_stackArg,
                    seps,
                    constant_number
            > {
    };

    struct Instruction_rule :
            pegtl::sor<
                    pegtl::seq<pegtl::at<Instruction_return_rule>, Instruction_return_rule>,
                    pegtl::seq<pegtl::at<Instruction_cmp_rule>, Instruction_cmp_rule>,
                    pegtl::seq<pegtl::at<Instruction_assignment_rule>, Instruction_assignment_rule>,
                    pegtl::seq<pegtl::at<Instruction_load_rule>, Instruction_load_rule>,
                    pegtl::seq<pegtl::at<Instruction_store_rule>, Instruction_store_rule>,
                    pegtl::seq<pegtl::at<Instruction_aop_assignment_rule>, Instruction_aop_assignment_rule>,
                    pegtl::seq<pegtl::at<Instruction_selfAop_rule>, Instruction_selfAop_rule>,
                    pegtl::seq<pegtl::at<Instruction_aop_load_rule>, Instruction_aop_load_rule>,
                    pegtl::seq<pegtl::at<Instruction_aop_store_rule>, Instruction_aop_store_rule>,
                    pegtl::seq<pegtl::at<Instruction_sop_rule>, Instruction_sop_rule>,
                    pegtl::seq<pegtl::at<Instruction_label_rule>, Instruction_label_rule>,
                    pegtl::seq<pegtl::at<Instruction_djump_rule>, Instruction_djump_rule>,
                    pegtl::seq<pegtl::at<Instruction_cjump_rule>, Instruction_cjump_rule>,
                    pegtl::seq<pegtl::at<Instruction_CISC_rule>, Instruction_CISC_rule>,
                    pegtl::seq<pegtl::at<Instruction_call_rule>, Instruction_call_rule>,
                    pegtl::seq<pegtl::at<Instruction_stackArg_rule>, Instruction_stackArg_rule>
            > {
    };

    struct Instructions_rule :
            pegtl::plus<
                    pegtl::seq<
                            seps,
                            Instruction_rule,
                            seps
                    >
            > {
    };

    struct Function_rule :
            pegtl::seq<
                    pegtl::one<'('>,
                    seps,
                    function_name,
                    seps,
                    argument_number,
                    seps,
                    Instructions_rule,
                    seps,
                    pegtl::one<')'>
            > {
    };

    struct Functions_rule :
            pegtl::plus<
                    seps,
                    Function_rule,
                    seps
            > {
    };

    struct entry_point_rule :
            pegtl::seq<
                    seps,
                    pegtl::one<'('>,
                    seps,
                    label,
                    seps,
                    Functions_rule,
                    seps,
                    pegtl::one<')'>,
                    seps
            > {
    };

    struct spill_rule :
            pegtl::seq<
                    var_rule,
                    seps,
                    var_rule
            > {
    };

    struct grammar :
            pegtl::must<
                    entry_point_rule
            > {
    };

    struct function_grammar :
            pegtl::seq<
                    seps,
                    Function_rule,
                    seps
            > {
    };

    struct spill_grammar :
            pegtl::seq<
                    seps,
                    Function_rule,
                    seps,
                    spill_rule
            > {
    };

    /*
     * Actions attached to grammar rules.
     */
    template<typename Rule>
    struct action : pegtl::nothing<Rule> {
    };

    template<>
    struct action<label> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            if (p.entryPointLabel.empty()) {
                p.entryPointLabel = in.string();
            } else {
                abort();
            }
        }
    };

    template<>
    struct action<function_name> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            auto newF = new Function();
            newF->name = in.string();
            auto targetVarItem = new Item();
            targetVarItem->type = VAR;
            auto spillerVarItem = new Item();
            spillerVarItem->type = VAR;
            spillerVarItem->name = "S";
            newF->targetVar = targetVarItem;
            newF->spillerVar = spillerVarItem;
            p.functions.push_back(newF);
        }
    };

    template<>
    struct action<argument_number> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            auto currentF = p.functions.back();
            currentF->arguments = std::stoll(in.string());
        }
    };

    template<>
    struct action<constant_number> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            auto i = new Constant();
            i->num = std::stoll(in.string());
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<str_return> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            auto currentF = p.functions.back();
            auto i = new Instruction_ret();
            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Label_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            auto i = new Item();
            i->type = LAB;
            i->name = in.string();
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<var_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){
            auto i = new Item();
            i->type = VAR;
            i->name = in.string().substr(1);
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<register_rdi_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            auto i = new Item();
            i->type = REG;
            i->r = rdi;
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<register_rax_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            auto i = new Item();
            i->type = REG;
            i->r = rax;
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<register_rbx_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            auto i = new Item();
            i->type = REG;
            i->r = rbx;
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<register_rcx_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            auto i = new Item();
            i->type = REG;
            i->r = rcx;
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<register_rdx_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            auto i = new Item();
            i->type = REG;
            i->r = rdx;
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<register_rsi_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            auto i = new Item();
            i->type = REG;
            i->r = rsi;
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<register_rbp_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            auto i = new Item();
            i->type = REG;
            i->r = rbp;
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<register_rsp_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            auto i = new Item();
            i->type = REG;
            i->r = rsp;
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<register_r8_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            auto i = new Item();
            i->type = REG;
            i->r = r8;
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<register_r9_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            auto i = new Item();
            i->type = REG;
            i->r = r9;
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<register_r10_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            auto i = new Item();
            i->type = REG;
            i->r = r10;
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<register_r11_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            auto i = new Item();
            i->type = REG;
            i->r = r11;
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<register_r12_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            auto i = new Item();
            i->type = REG;
            i->r = r12;
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<register_r13_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            auto i = new Item();
            i->type = REG;
            i->r = r13;
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<register_r14_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            auto i = new Item();
            i->type = REG;
            i->r = r14;
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<register_r15_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            auto i = new Item();
            i->type = REG;
            i->r = r15;
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<Instruction_assignment_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {

            /*
             * Fetch the current function.
             */
            auto currentF = p.functions.back();

            /*
             * Create the instruction.
             */
            auto i = new Instruction_assignment();
            i->src = parsed_items.back();
            parsed_items.pop_back();
            i->dst = parsed_items.back();
            parsed_items.pop_back();

            /*
             * Add the just-created instruction to the current function.
             */
            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_load_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {

            auto currentF = p.functions.back();

            auto i = new Instruction_load();
            i->offset = parsed_items.back();
            parsed_items.pop_back();
            i->src = parsed_items.back();
            parsed_items.pop_back();
            i->dst = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_store_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {

            auto currentF = p.functions.back();

            auto i = new Instruction_store();
            i->src = parsed_items.back();
            parsed_items.pop_back();
            i->offset = parsed_items.back();
            parsed_items.pop_back();
            i->dst = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_add_assignment_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_aop_assignment();
            i->opId = ADD;
            i->src = parsed_items.back();
            parsed_items.pop_back();
            i->dst = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_sub_assignment_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_aop_assignment();
            i->opId = SUB;
            i->src = parsed_items.back();
            parsed_items.pop_back();
            i->dst = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_multi_assignment_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_aop_assignment();
            i->opId = MULTI;
            i->src = parsed_items.back();
            parsed_items.pop_back();
            i->dst = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_and_assignment_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_aop_assignment();
            i->opId = AND;
            i->src = parsed_items.back();
            parsed_items.pop_back();
            i->dst = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_selfAdd_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_selfAop();
            i->opId = ADD;
            i->reg = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_selfSub_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_selfAop();
            i->opId = SUB;
            i->reg = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_add_load_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_aop_load();
            i->opId = ADD;
            i->offset = parsed_items.back();
            parsed_items.pop_back();
            i->src = parsed_items.back();
            parsed_items.pop_back();
            i->dst = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_sub_load_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_aop_load();
            i->opId = SUB;
            i->offset = parsed_items.back();
            parsed_items.pop_back();
            i->src = parsed_items.back();
            parsed_items.pop_back();
            i->dst = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_add_store_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_aop_store();
            i->opId = ADD;
            i->src = parsed_items.back();
            parsed_items.pop_back();
            i->offset = parsed_items.back();
            parsed_items.pop_back();
            i->dst = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_sub_store_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_aop_store();
            i->opId = SUB;
            i->src = parsed_items.back();
            parsed_items.pop_back();
            i->offset = parsed_items.back();
            parsed_items.pop_back();
            i->dst = parsed_items.back();
            parsed_items.pop_back();
            
            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_lshift_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_sop();
            i->opId = LEFT;
            i->offset = parsed_items.back();
            parsed_items.pop_back();
            i->reg = parsed_items.back();
            parsed_items.pop_back();
            
            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_rshift_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_sop();
            i->opId = RIGHT;
            i->offset = parsed_items.back();
            parsed_items.pop_back();
            i->reg = parsed_items.back();
            parsed_items.pop_back();
            
            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_le_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_cmp();
            i->cmpId = LE;
            i->r = parsed_items.back();
            parsed_items.pop_back();
            i->l = parsed_items.back();
            parsed_items.pop_back();
            i->reg = parsed_items.back();
            parsed_items.pop_back();
            
            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_leq_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_cmp();
            i->cmpId = LEQ;
            i->r = parsed_items.back();
            parsed_items.pop_back();
            i->l = parsed_items.back();
            parsed_items.pop_back();
            i->reg = parsed_items.back();
            parsed_items.pop_back();
            
            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_eql_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_cmp();
            i->cmpId = EQL;
            i->r = parsed_items.back();
            parsed_items.pop_back();
            i->l = parsed_items.back();
            parsed_items.pop_back();
            i->reg = parsed_items.back();
            parsed_items.pop_back();
            
            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_label_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_label();
            i->label = parsed_items.back();
            parsed_items.pop_back();
            
            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_djump_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_djump();
            i->dst = parsed_items.back();
            parsed_items.pop_back();
            
            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_le_jump_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_cjump();
            i->cmpId = LE;
            i->dst = parsed_items.back();
            parsed_items.pop_back();
            i->r = parsed_items.back();
            parsed_items.pop_back();
            i->l = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_leq_jump_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_cjump();
            i->cmpId = LEQ;
            i->dst = parsed_items.back();
            parsed_items.pop_back();
            i->r = parsed_items.back();
            parsed_items.pop_back();
            i->l = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_eql_jump_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_cjump();
            i->cmpId = EQL;
            i->dst = parsed_items.back();
            parsed_items.pop_back();
            i->r = parsed_items.back();
            parsed_items.pop_back();
            i->l = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_CISC_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_CISC();
            i->scale = parsed_items.back();
            parsed_items.pop_back();
            i->index = parsed_items.back();
            parsed_items.pop_back();
            i->base = parsed_items.back();
            parsed_items.pop_back();
            i->reg = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_call_function_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_call();
            i->isRuntime = false;
            i->num = parsed_items.back();
            parsed_items.pop_back();
            i->u = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_call_print_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_call();
            i->isRuntime = true;
            i->runtimeId = PRINT;
            i->num = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_call_allocate_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_call();
            i->isRuntime = true;
            i->runtimeId = ALLOCATE;
            i->num = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_call_arrayError_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_call();
            i->isRuntime = true;
            i->runtimeId = ARRAY;
            i->num = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<Instruction_stackArg_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_stackArg();
            i->offset = parsed_items.back();
            parsed_items.pop_back();
            i->reg = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);
        }
    };

    template<>
    struct action<spill_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){
            delete p.functions[0]->spillerVar;
            p.functions[0]->spillerVar = parsed_items.back();
            parsed_items.pop_back();
            delete p.functions[0]->targetVar;
            p.functions[0]->targetVar = parsed_items.back();
            parsed_items.pop_back();
        }
    };

    Program parse_file(char *fileName) {

        /*
         * Check the grammar for some possible issues.
         */
        pegtl::analyze<grammar>();

        /*
         * Parse.
         */
        file_input<> fileInput(fileName);
        Program p;
        parse<grammar, action>(fileInput, p);

        return p;
    }

    Program parse_function_file(char *fileName) {
        pegtl::analyze<function_grammar>();

        file_input<> fileInput(fileName);
        Program p;
        parse<function_grammar, action>(fileInput, p);

        return p;
    }

    Program parse_spill_file(char *fileName) {
        pegtl::analyze<spill_grammar>();

        file_input<> fileInput(fileName);
        Program p;
        parse<spill_grammar, action>(fileInput, p);

        return p;
    }

}
