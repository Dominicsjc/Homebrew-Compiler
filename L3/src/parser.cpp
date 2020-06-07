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
#include <cstdint>
#include <cassert>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

#include <L3.h>
#include <parser.h>

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;

namespace L3 {
    std::vector<Item *> parsed_items;

    Item *tmpVar = nullptr;

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

    struct str_arrow : TAOCPP_PEGTL_STRING("<-") {
    };
    struct str_return : TAOCPP_PEGTL_STRING("return") {
    };
    struct str_add : TAOCPP_PEGTL_STRING("+") {
    };
    struct str_sub : TAOCPP_PEGTL_STRING("-") {
    };
    struct str_multi : TAOCPP_PEGTL_STRING("*") {
    };
    struct str_and : TAOCPP_PEGTL_STRING("&") {
    };
    struct str_lshift : TAOCPP_PEGTL_STRING("<<") {
    };
    struct str_rshift : TAOCPP_PEGTL_STRING(">>") {
    };
    struct str_le : TAOCPP_PEGTL_STRING("<") {
    };
    struct str_leq : TAOCPP_PEGTL_STRING("<=") {
    };
    struct str_eql : TAOCPP_PEGTL_STRING("=") {
    };
    struct str_gr : TAOCPP_PEGTL_STRING(">") {
    };
    struct str_geq : TAOCPP_PEGTL_STRING(">=") {
    };
    struct str_call : TAOCPP_PEGTL_STRING("call") {
    };
    struct str_print : TAOCPP_PEGTL_STRING("print") {
    };
    struct str_allocate : TAOCPP_PEGTL_STRING("allocate") {
    };
    struct str_arrayError : TAOCPP_PEGTL_STRING("array-error") {
    };
    struct str_load : TAOCPP_PEGTL_STRING("load") {
    };
    struct str_store : TAOCPP_PEGTL_STRING("store") {
    };
    struct str_br : TAOCPP_PEGTL_STRING("br") {
    };
    struct str_define : TAOCPP_PEGTL_STRING("define") {
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

    struct constant_number :
            number {
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

    struct var_rule :
            var {
    };

    struct vars_rule :
            pegtl::sor<
                    pegtl::seq<
                            var_rule,
                            seps,
                            pegtl::star<
                                    pegtl::one<','>,
                                    seps,
                                    var_rule
                            >
                    >,
                    var_rule,
                    seps
            > {
    };

    struct args_rule :
            pegtl::sor<
                    pegtl::seq<
                            pegtl::sor<
                                    var_rule,
                                    constant_number
                            >,
                            seps,
                            pegtl::star<
                                    pegtl::one<','>,
                                    seps,
                                    pegtl::sor<
                                            var_rule,
                                            constant_number
                                    >
                            >
                    >,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >,
                    seps
            > {
    };

    struct function_name :
            label {
    };

    
    struct Label_rule :
            label {
    };

    struct Instruction_return_rule :
            pegtl::seq<
                    str_return
            > {
    };

    struct Instruction_value_return_rule :
            pegtl::seq<
                    str_return,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >
            > {
    };

    struct Instruction_assignment_rule :
            pegtl::sor<
                    pegtl::seq<
                            var_rule,
                            seps,
                            str_arrow,
                            seps,
                            pegtl::sor<
                                    var_rule,
                                    constant_number,
                                    Label_rule
                            >
                    >
            > {
    };

    struct Instruction_load_rule :
            pegtl::seq<
                    var_rule,
                    seps,
                    str_arrow,
                    seps,
                    str_load,
                    seps,
                    var_rule
            > {
    };

    struct Instruction_store_rule :
            pegtl::seq<
                    str_store,
                    seps,
                    var_rule,
                    seps,
                    str_arrow,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number,
                            Label_rule
                    >
            > {

    };

    struct Instruction_add_assignment_rule :
            pegtl::seq<
                    var_rule,
                    seps,
                    str_arrow,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >,
                    seps,
                    str_add,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >
            > {
    };

    struct Instruction_sub_assignment_rule :
            pegtl::seq<
                    var_rule,
                    seps,
                    str_arrow,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >,
                    seps,
                    str_sub,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >
            > {
    };

    struct Instruction_multi_assignment_rule :
            pegtl::seq<
                    var_rule,
                    seps,
                    str_arrow,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >,
                    seps,
                    str_multi,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >
            > {
    };

    struct Instruction_and_assignment_rule :
            pegtl::seq<
                    var_rule,
                    seps,
                    str_arrow,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >,
                    seps,
                    str_and,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >
            > {
    };

    struct Instruction_left_assignment_rule :
            pegtl::seq<
                    var_rule,
                    seps,
                    str_arrow,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >,
                    seps,
                    str_lshift,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >
            > {
    };

    struct Instruction_right_assignment_rule :
            pegtl::seq<
                    var_rule,
                    seps,
                    str_arrow,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >,
                    seps,
                    str_rshift,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >
            > {
    };

    struct Instruction_op_assignment_rule :
            pegtl::sor<
                    Instruction_add_assignment_rule,
                    Instruction_sub_assignment_rule,
                    Instruction_multi_assignment_rule,
                    Instruction_and_assignment_rule,
                    Instruction_left_assignment_rule,
                    Instruction_right_assignment_rule
            > {
    };

    struct Instruction_le_assignment_rule :
            pegtl::seq<
                    var_rule,
                    seps,
                    str_arrow,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >,
                    seps,
                    str_le,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >
            > {
    };

    struct Instruction_leq_assignment_rule :
            pegtl::seq<
                    var_rule,
                    seps,
                    str_arrow,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >,
                    seps,
                    str_leq,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >
            > {
    };

    struct Instruction_eql_assignment_rule :
            pegtl::seq<
                    var_rule,
                    seps,
                    str_arrow,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >,
                    seps,
                    str_eql,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >
            > {
    };

    struct Instruction_gr_assignment_rule :
            pegtl::seq<
                    var_rule,
                    seps,
                    str_arrow,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >,
                    seps,
                    str_gr,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >
            > {
    };

    struct Instruction_geq_assignment_rule :
            pegtl::seq<
                    var_rule,
                    seps,
                    str_arrow,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >,
                    seps,
                    str_geq,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >
            > {
    };

    struct Instruction_cmp_assignment_rule :
            pegtl::sor<
                    Instruction_le_assignment_rule,
                    Instruction_leq_assignment_rule,
                    Instruction_eql_assignment_rule,
                    Instruction_gr_assignment_rule,
                    Instruction_geq_assignment_rule
            > {
    };

    struct Instruction_label_rule:
            pegtl::seq<
                    Label_rule
            > {
    };

    struct Instruction_dbr_rule:
            pegtl::seq<
                    str_br,
                    seps,
                    Label_rule
            > {
    };

    struct Instruction_vbr_rule :
            pegtl::seq<
                    str_br,
                    seps,
                    var_rule,
                    seps,
                    Label_rule
            > {
    };

    struct Instruction_call_function_rule :
            pegtl::seq<
                    str_call,
                    seps,
                    pegtl::sor<
                            var_rule,
                            Label_rule
                    >,
                    seps,
                    pegtl::one<'('>,
                    seps,
                    args_rule,
                    seps,
                    pegtl::one<')'>
            > {
    };

    struct Instruction_call_print_rule :
            pegtl::seq<
                    str_call,
                    seps,
                    str_print,
                    seps,
                    pegtl::one<'('>,
                    seps,
                    args_rule,
                    seps,
                    pegtl::one<')'>
            > {
    };

    struct Instruction_call_allocate_rule :
            pegtl::seq<
                    str_call,
                    seps,
                    str_allocate,
                    seps,
                    pegtl::one<'('>,
                    seps,
                    args_rule,
                    seps,
                    pegtl::one<')'>
            > {
    };

    struct Instruction_call_arrayError_rule :
            pegtl::seq<
                    str_call,
                    seps,
                    str_arrayError,
                    seps,
                    pegtl::one<'('>,
                    seps,
                    args_rule,
                    seps,
                    pegtl::one<')'>
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

    struct Instruction_call_function_assignment_rule :
            pegtl::seq<
                    var_rule,
                    seps,
                    str_arrow,
                    seps,
                    str_call,
                    seps,
                    pegtl::sor<
                            var_rule,
                            Label_rule
                    >,
                    seps,
                    pegtl::one<'('>,
                    seps,
                    args_rule,
                    seps,
                    pegtl::one<')'>
            > {
    };

    struct Instruction_call_print_assignment_rule :
            pegtl::seq<
                    var_rule,
                    seps,
                    str_arrow,
                    seps,
                    str_call,
                    seps,
                    str_print,
                    seps,
                    pegtl::one<'('>,
                    seps,
                    args_rule,
                    seps,
                    pegtl::one<')'>
            > {
    };

    struct Instruction_call_allocate_assignment_rule :
            pegtl::seq<
                    var_rule,
                    seps,
                    str_arrow,
                    seps,
                    str_call,
                    seps,
                    str_allocate,
                    seps,
                    pegtl::one<'('>,
                    seps,
                    args_rule,
                    seps,
                    pegtl::one<')'>
            > {
    };

    struct Instruction_call_arrayError_assignment_rule :
            pegtl::seq<
                    var_rule,
                    seps,
                    str_arrow,
                    seps,
                    str_call,
                    seps,
                    str_arrayError,
                    seps,
                    pegtl::one<'('>,
                    seps,
                    args_rule,
                    seps,
                    pegtl::one<')'>
            > {
    };
    
    struct Instruction_call_runtime_assignment_rule :
            pegtl::sor<
                    Instruction_call_print_assignment_rule,
                    Instruction_call_allocate_assignment_rule,
                    Instruction_call_arrayError_assignment_rule
            > {
    };

    struct Instruction_call_assignment_rule :
            pegtl::sor<
                    Instruction_call_function_assignment_rule,
                    Instruction_call_runtime_assignment_rule
            > {
    };

    struct Instruction_rule :
            pegtl::sor<
                    pegtl::seq<pegtl::at<Instruction_value_return_rule>, Instruction_value_return_rule>,
                    pegtl::seq<pegtl::at<Instruction_return_rule>, Instruction_return_rule>,
                    pegtl::seq<pegtl::at<Instruction_load_rule>, Instruction_load_rule>,
                    pegtl::seq<pegtl::at<Instruction_store_rule>, Instruction_store_rule>,
                    pegtl::seq<pegtl::at<Instruction_op_assignment_rule>, Instruction_op_assignment_rule>,
                    pegtl::seq<pegtl::at<Instruction_cmp_assignment_rule>, Instruction_cmp_assignment_rule>,
                    pegtl::seq<pegtl::at<Instruction_assignment_rule>, Instruction_assignment_rule>,
                    pegtl::seq<pegtl::at<Instruction_label_rule>, Instruction_label_rule>,
                    pegtl::seq<pegtl::at<Instruction_dbr_rule>, Instruction_dbr_rule>,
                    pegtl::seq<pegtl::at<Instruction_vbr_rule>, Instruction_vbr_rule>,
                    pegtl::seq<pegtl::at<Instruction_call_rule>, Instruction_call_rule>,
                    pegtl::seq<pegtl::at<Instruction_call_assignment_rule>, Instruction_call_assignment_rule>
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
                    str_define,
                    seps,
                    function_name,
                    seps,
                    pegtl::one<'('>,
                    seps,
                    vars_rule,
                    seps,
                    pegtl::one<')'>,
                    seps,
                    pegtl::one<'{'>,
                    seps,
                    Instructions_rule,
                    seps,
                    pegtl::one<'}'>
            > {
    };

    struct Program_rule :
            pegtl::plus<
                    seps,
                    Function_rule,
                    seps
            > {
    };

    struct grammar :
            pegtl::must<
                    Program_rule
            > {
    };

    /*
     * Actions attached to grammar rules.
     */
    template<typename Rule>
    struct action : pegtl::nothing<Rule> {
    };

    template<>
    struct action<function_name> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            auto newF = new Function();
            newF->name = in.string();
            p.functions.push_back(newF);
        }
    };

    template<>
    struct action<vars_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            auto currentF = p.functions.back();
            for (auto var : parsed_items) {
                currentF->vars.push_back(var);
            }
            parsed_items.clear();
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
    struct action<Instruction_return_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {

            auto currentF = p.functions.back();

            auto i = new Instruction_ret();
            
            currentF->instructions.push_back(i);

            for (auto rubbish : parsed_items) {
                delete rubbish;
            }
            parsed_items.clear();
        }
    };

    template<>
    struct action<Instruction_value_return_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {

            auto currentF = p.functions.back();

            auto i = new Instruction_value_ret();
            i->t = parsed_items.back();
            parsed_items.pop_back();
            
            currentF->instructions.push_back(i);

            for (auto rubbish : parsed_items) {
                delete rubbish;
            }
            parsed_items.clear();
        }
    };

    template<>
    struct action<Instruction_assignment_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {

            auto currentF = p.functions.back();

            auto i = new Instruction_assignment();
            i->src = parsed_items.back();
            parsed_items.pop_back();
            i->dst = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);

            for (auto rubbish : parsed_items) {
                delete rubbish;
            }
            parsed_items.clear();
        }
    };

    template<>
    struct action<Instruction_load_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {

            auto currentF = p.functions.back();

            auto i = new Instruction_load();
            i->src = parsed_items.back();
            parsed_items.pop_back();
            i->dst = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);

            for (auto rubbish : parsed_items) {
                delete rubbish;
            }
            parsed_items.clear();
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
            i->dst = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);

            for (auto rubbish : parsed_items) {
                delete rubbish;
            }
            parsed_items.clear();
        }
    };

    template<>
    struct action<Instruction_add_assignment_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_op_assignment();
            i->opId = ADD;
            i->r = parsed_items.back();
            parsed_items.pop_back();
            i->l = parsed_items.back();
            parsed_items.pop_back();
            i->var = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);

            for (auto rubbish : parsed_items) {
                delete rubbish;
            }
            parsed_items.clear();
        }
    };

    template<>
    struct action<Instruction_sub_assignment_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_op_assignment();
            i->opId = SUB;
            i->r = parsed_items.back();
            parsed_items.pop_back();
            i->l = parsed_items.back();
            parsed_items.pop_back();
            i->var = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);

            for (auto rubbish : parsed_items) {
                delete rubbish;
            }
            parsed_items.clear();
        }
    };

    template<>
    struct action<Instruction_multi_assignment_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_op_assignment();
            i->opId = MULTI;
            i->r = parsed_items.back();
            parsed_items.pop_back();
            i->l = parsed_items.back();
            parsed_items.pop_back();
            i->var = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);

            for (auto rubbish : parsed_items) {
                delete rubbish;
            }
            parsed_items.clear();
        }
    };

    template<>
    struct action<Instruction_and_assignment_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_op_assignment();
            i->opId = AND;
            i->r = parsed_items.back();
            parsed_items.pop_back();
            i->l = parsed_items.back();
            parsed_items.pop_back();
            i->var = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);

            for (auto rubbish : parsed_items) {
                delete rubbish;
            }
            parsed_items.clear();
        }
    };

    template<>
    struct action<Instruction_left_assignment_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_op_assignment();
            i->opId = LEFT;
            i->r = parsed_items.back();
            parsed_items.pop_back();
            i->l = parsed_items.back();
            parsed_items.pop_back();
            i->var = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);

            for (auto rubbish : parsed_items) {
                delete rubbish;
            }
            parsed_items.clear();
        }
    };

    template<>
    struct action<Instruction_right_assignment_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_op_assignment();
            i->opId = RIGHT;
            i->r = parsed_items.back();
            parsed_items.pop_back();
            i->l = parsed_items.back();
            parsed_items.pop_back();
            i->var = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);

            for (auto rubbish : parsed_items) {
                delete rubbish;
            }
            parsed_items.clear();
        }
    };

    template<>
    struct action<Instruction_le_assignment_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_cmp_assignment();
            i->cmpId = LE;
            i->r = parsed_items.back();
            parsed_items.pop_back();
            i->l = parsed_items.back();
            parsed_items.pop_back();
            i->var = parsed_items.back();
            parsed_items.pop_back();
            
            currentF->instructions.push_back(i);

            for (auto rubbish : parsed_items) {
                delete rubbish;
            }
            parsed_items.clear();
        }
    };

    template<>
    struct action<Instruction_leq_assignment_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_cmp_assignment();
            i->cmpId = LEQ;
            i->r = parsed_items.back();
            parsed_items.pop_back();
            i->l = parsed_items.back();
            parsed_items.pop_back();
            i->var = parsed_items.back();
            parsed_items.pop_back();
            
            currentF->instructions.push_back(i);

            for (auto rubbish : parsed_items) {
                delete rubbish;
            }
            parsed_items.clear();
        }
    };

    template<>
    struct action<Instruction_eql_assignment_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_cmp_assignment();
            i->cmpId = EQL;
            i->r = parsed_items.back();
            parsed_items.pop_back();
            i->l = parsed_items.back();
            parsed_items.pop_back();
            i->var = parsed_items.back();
            parsed_items.pop_back();
            
            currentF->instructions.push_back(i);

            for (auto rubbish : parsed_items) {
                delete rubbish;
            }
            parsed_items.clear();
        }
    };

    template<>
    struct action<Instruction_gr_assignment_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_cmp_assignment();
            i->cmpId = GR;
            i->r = parsed_items.back();
            parsed_items.pop_back();
            i->l = parsed_items.back();
            parsed_items.pop_back();
            i->var = parsed_items.back();
            parsed_items.pop_back();
            
            currentF->instructions.push_back(i);

            for (auto rubbish : parsed_items) {
                delete rubbish;
            }
            parsed_items.clear();
        }
    };

    template<>
    struct action<Instruction_geq_assignment_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_cmp_assignment();
            i->cmpId = GEQ;
            i->r = parsed_items.back();
            parsed_items.pop_back();
            i->l = parsed_items.back();
            parsed_items.pop_back();
            i->var = parsed_items.back();
            parsed_items.pop_back();
            
            currentF->instructions.push_back(i);

            for (auto rubbish : parsed_items) {
                delete rubbish;
            }
            parsed_items.clear();
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

            for (auto rubbish : parsed_items) {
                delete rubbish;
            }
            parsed_items.clear();
        }
    };

    template<>
    struct action<Instruction_dbr_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_dbr();
            i->dst = parsed_items.back();
            parsed_items.pop_back();
            
            currentF->instructions.push_back(i);

            for (auto rubbish : parsed_items) {
                delete rubbish;
            }
            parsed_items.clear();
        }
    };

    template<>
    struct action<Instruction_vbr_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_vbr();
            i->dst = parsed_items.back();
            parsed_items.pop_back();
            i->var = parsed_items.back();
            parsed_items.pop_back();
            
            currentF->instructions.push_back(i);

            for (auto rubbish : parsed_items) {
                delete rubbish;
            }
            parsed_items.clear();
        }
    };

    template<>
    struct action<Instruction_call_function_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_call();
            i->isRuntime = false;

            for (int j = 0; j < parsed_items.size(); j++) {
                if (j == 0) {
                    i->u = parsed_items[0];   
                } else {
                    i->args.push_back(parsed_items[j]);
                }
            }
            parsed_items.clear();

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

            for (auto item : parsed_items) {
                i->args.push_back(item);
            }
            parsed_items.clear();

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

            for (auto item : parsed_items) {
                i->args.push_back(item);
            }
            parsed_items.clear();

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

            for (auto item : parsed_items) {
                i->args.push_back(item);
            }
            parsed_items.clear();

            currentF->instructions.push_back(i);

        }
    };

    template<>
    struct action<Instruction_call_function_assignment_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_call_assignment();
            i->isRuntime = false;
            
            for (int j = 0; j < parsed_items.size(); j++) {
                if (j == 0) {
                    i->var = parsed_items[0];   
                } else if (j == 1) {
                    i->u = parsed_items[1];
                } else {
                    i->args.push_back(parsed_items[j]);
                }
            }
            parsed_items.clear();

            currentF->instructions.push_back(i);

        }
    };

    template<>
    struct action<Instruction_call_print_assignment_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_call_assignment();
            i->isRuntime = true;
            i->runtimeId = PRINT;

            for (auto item : parsed_items) {
                i->args.push_back(item);
            }
            parsed_items.clear();

            i->var = tmpVar;
            tmpVar = nullptr;

            currentF->instructions.push_back(i);

        }
    };

    template<>
    struct action<Instruction_call_allocate_assignment_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_call_assignment();
            i->isRuntime = true;
            i->runtimeId = ALLOCATE;

            for (auto item : parsed_items) {
                i->args.push_back(item);
            }
            parsed_items.clear();

            i->var = tmpVar;
            tmpVar = nullptr;

            currentF->instructions.push_back(i);

        }
    };

    template<>
    struct action<Instruction_call_arrayError_assignment_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_call_assignment();
            i->isRuntime = true;
            i->runtimeId = ARRAY;

            for (auto item : parsed_items) {
                i->args.push_back(item);
            }
            parsed_items.clear();

            i->var = tmpVar;
            tmpVar = nullptr;

            currentF->instructions.push_back(i);

        }
    };

    template<>
    struct action<str_print> {
        template<typename Input>
        static void apply(const Input &in, Program &p){
                while (parsed_items.size() > 1) {
                        auto deleter = parsed_items.back();
                        delete deleter;
                        parsed_items.pop_back();
                }

                if (!parsed_items.empty()) {
                    tmpVar = parsed_items.back();
                    parsed_items.pop_back();
                }
        }
    };

    template<>
    struct action<str_allocate> {
        template<typename Input>
        static void apply(const Input &in, Program &p){
                while (parsed_items.size() > 1) {
                        auto deleter = parsed_items.back();
                        delete deleter;
                        parsed_items.pop_back();
                }

                if (!parsed_items.empty()) {
                    tmpVar = parsed_items.back();
                    parsed_items.pop_back();
                }
        }
    };

    template<>
    struct action<str_arrayError> {
        template<typename Input>
        static void apply(const Input &in, Program &p){
                while (parsed_items.size() > 1) {
                        auto deleter = parsed_items.back();
                        delete deleter;
                        parsed_items.pop_back();
                }

                if (!parsed_items.empty()) {
                    tmpVar = parsed_items.back();
                    parsed_items.pop_back();
                }
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

        for (auto i : parsed_items) {
            delete i;
        }
        return p;
    }
}