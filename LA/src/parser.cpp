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

#include "LA.h"
#include "parser.h"

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;

namespace LA {
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
    struct str_print : TAOCPP_PEGTL_STRING("print") {
    };
    struct str_br : TAOCPP_PEGTL_STRING("br") {
    };
    struct str_length : TAOCPP_PEGTL_STRING("length") {
    };
    struct str_new : TAOCPP_PEGTL_STRING("new") {
    };
    struct str_Array: TAOCPP_PEGTL_STRING("Array") {
    };
    struct str_Tuple : TAOCPP_PEGTL_STRING("Tuple") {
    };
    struct str_void : TAOCPP_PEGTL_STRING("void") {
    };
    struct str_int64 : TAOCPP_PEGTL_STRING("int64") {
    };
    struct str_tuple : TAOCPP_PEGTL_STRING("tuple") {
    };
    struct str_code : TAOCPP_PEGTL_STRING("code") {
    };
    struct str_bracket : TAOCPP_PEGTL_STRING("[]") {
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
            name {
    };

    struct var_rule :
            var {
    };

    struct void_rule : 
            str_void {
    };

    struct bracket_rule :
            str_bracket {
    };

    struct int64_rule :
            pegtl::seq<
                    str_int64,
                    pegtl::star<
                            bracket_rule
                    >
            > {
    };

    struct tuple_rule :
            str_tuple {
    };

    struct code_rule :
            str_code {
    };

    struct type_rule :
            pegtl::sor<
                    void_rule,
                    int64_rule,
                    tuple_rule,
                    code_rule
            > {
    };

    struct typedVar_rule : 
            pegtl::seq<
                    type_rule,
                    seps,
                    var_rule
            > {
    };

    struct typedVars_rule :
            pegtl::sor<
                    pegtl::seq<
                            typedVar_rule,
                            seps,
                            pegtl::star<
                                    pegtl::one<','>,
                                    seps,
                                    typedVar_rule
                            >
                    >,
                    typedVar_rule,
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

    struct new_rule :
            str_new {
    };

    struct function_name :
            name {
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
                    var_rule,
                    pegtl::plus<
                            pegtl::seq<
                                    pegtl::one<'['>,
                                    pegtl::sor<
                                            var_rule,
                                            constant_number
                                    >,
                                    pegtl::one<']'>
                            >
                    >
            > {
    };

    struct Instruction_store_rule :
            pegtl::seq<
                    var_rule,
                    pegtl::plus<
                            pegtl::seq<
                                    pegtl::one<'['>,
                                    pegtl::sor<
                                            var_rule,
                                            constant_number
                                    >,
                                    pegtl::one<']'>
                            >
                    >,
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

    struct Instruction_store_length_rule :
            pegtl::seq<
                    var_rule,
                    seps,
                    str_arrow,
                    seps,
                    str_length,
                    seps,
                    var_rule,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
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

    struct Instruction_cbr_rule :
            pegtl::seq<
                    str_br,
                    seps,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >,
                    seps,
                    Label_rule,
                    seps,
                    Label_rule
            > {
    };

    struct Instruction_call_function_rule :
            pegtl::seq<
                    var_rule,
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
                    str_print,
                    seps,
                    pegtl::one<'('>,
                    seps,
                    args_rule,
                    seps,
                    pegtl::one<')'>
            > {
    };

    struct Instruction_call_rule :
            pegtl::sor<
                    Instruction_call_print_rule,
                    Instruction_call_function_rule
            > {
    };

    struct Instruction_call_assignment_rule :
            pegtl::seq<
                    var_rule,
                    seps,
                    str_arrow,
                    seps,
                    var_rule,
                    seps,
                    pegtl::one<'('>,
                    seps,
                    args_rule,
                    seps,
                    pegtl::one<')'>
            > {
    };

    struct Instruction_new_array_rule :
            pegtl::seq<
                    var_rule,
                    seps,
                    str_arrow,
                    seps,
                    new_rule,
                    seps,
                    str_Array,
                    seps,
                    pegtl::one<'('>,
                    args_rule,
                    pegtl::one<')'>
            > {
    };

    struct Instruction_new_tuple_rule :
            pegtl::seq<
                    var_rule,
                    seps,
                    str_arrow,
                    seps,
                    new_rule,
                    seps,
                    str_Tuple,
                    seps,
                    pegtl::one<'('>,
                    pegtl::sor<
                            var_rule,
                            constant_number
                    >,
                    pegtl::one<')'>
            > {
    };

    struct Instruction_declare_rule :
            pegtl::seq<
                    type_rule,
                    seps,
                    var_rule
            > {
    };

    struct Instruction_rule :
            pegtl::sor<
                    pegtl::seq<pegtl::at<Instruction_store_rule>, Instruction_store_rule>,
                    pegtl::seq<pegtl::at<Instruction_new_array_rule>, Instruction_new_array_rule>,
                    pegtl::seq<pegtl::at<Instruction_new_tuple_rule>, Instruction_new_tuple_rule>,
                    pegtl::seq<pegtl::at<Instruction_store_length_rule>, Instruction_store_length_rule>,
                    pegtl::seq<pegtl::at<Instruction_load_rule>, Instruction_load_rule>,
                    pegtl::seq<pegtl::at<Instruction_call_assignment_rule>, Instruction_call_assignment_rule>,
                    pegtl::seq<pegtl::at<Instruction_op_assignment_rule>, Instruction_op_assignment_rule>,
                    pegtl::seq<pegtl::at<Instruction_cmp_assignment_rule>, Instruction_cmp_assignment_rule>,
                    pegtl::seq<pegtl::at<Instruction_assignment_rule>, Instruction_assignment_rule>,
                    pegtl::seq<pegtl::at<Instruction_call_rule>, Instruction_call_rule>,
                    pegtl::seq<pegtl::at<Instruction_value_return_rule>, Instruction_value_return_rule>,
                    pegtl::seq<pegtl::at<Instruction_return_rule>, Instruction_return_rule>,
                    pegtl::seq<pegtl::at<Instruction_label_rule>, Instruction_label_rule>,
                    pegtl::seq<pegtl::at<Instruction_dbr_rule>, Instruction_dbr_rule>,
                    pegtl::seq<pegtl::at<Instruction_cbr_rule>, Instruction_cbr_rule>,
                    pegtl::seq<pegtl::at<Instruction_declare_rule>, Instruction_declare_rule>
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
                    type_rule,
                    seps,
                    function_name,
                    seps,
                    pegtl::one<'('>,
                    seps,
                    typedVars_rule,
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
            newF->res = parsed_items.back();
            parsed_items.pop_back();
            p.functions.push_back(newF);
        }
    };

    template<>
    struct action<typedVars_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {
            auto currentF = p.functions.back();
            for (auto i : parsed_items) {
                currentF->typedVars.push_back(i);
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
            auto i = new Label();
            i->name = in.string();
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<var_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){
            auto i = new Variable();
            i->name = in.string();
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<void_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){
            auto i = new T();
            i->type = VOID;
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<str_int64> {
        template<typename Input>
        static void apply(const Input &in, Program &p){
            auto i = new T();
            i->type = INT64;
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<bracket_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){
            auto i = static_cast<T *>(parsed_items.back());
            i->arrayDim++;
        }
    };

    template<>
    struct action<tuple_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){
            auto i = new T();
            i->type = TUPLE;
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<code_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){
            auto i = new T();
            i->type = CODE;
            parsed_items.push_back(i);
        }
    };

    template<>
    struct action<new_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){
            auto i = parsed_items.back();
            parsed_items.pop_back();

            for (auto rubbish : parsed_items) {
                delete rubbish;
            }
            parsed_items.clear();

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

            for (int j = 2; j < parsed_items.size(); j++) {
                i->indices.push_back(parsed_items[j]);
            }
            i->src = parsed_items[1];
            i->dst = parsed_items[0];

            currentF->instructions.push_back(i);

            //auto rubbish = parsed_items[0];
            //delete rubbish;
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
            for (int j = 1; j < parsed_items.size(); j++) {
                i->indices.push_back(parsed_items[j]);
            }
            i->dst = parsed_items[0];

            currentF->instructions.push_back(i);

            parsed_items.clear();
        }
    };

    template<>
    struct action<Instruction_store_length_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p) {

            auto currentF = p.functions.back();

            auto i = new Instruction_store_length();
            i->t = parsed_items.back();
            parsed_items.pop_back();
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
    struct action<Instruction_cbr_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_cbr();
            i->labelF = parsed_items.back();
            parsed_items.pop_back();
            i->labelT = parsed_items.back();
            parsed_items.pop_back();
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
    struct action<Instruction_call_function_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_call();
            i->isPrint = false;

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
            i->isPrint = true;
            
            for (auto item : parsed_items) {
                i->args.push_back(item);
            }
            parsed_items.clear();

            currentF->instructions.push_back(i);

        }
    };

    template<>
    struct action<Instruction_call_assignment_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_call_assignment();
            
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
    struct action<Instruction_new_array_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_new_array();
            for(int j = 1; j < parsed_items.size(); j++) {
                i->args.push_back(parsed_items[j]);
            }
            i->var = parsed_items[0];
            
            currentF->instructions.push_back(i);

            parsed_items.clear();
        }
    };

    template<>
    struct action<Instruction_new_tuple_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_new_tuple();
            i->t = parsed_items.back();
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
    struct action<Instruction_declare_rule> {
        template<typename Input>
        static void apply(const Input &in, Program &p){

            auto currentF = p.functions.back();

            auto i = new Instruction_declare();
            i->var = parsed_items.back();
            parsed_items.pop_back();
            i->type = parsed_items.back();
            parsed_items.pop_back();

            currentF->instructions.push_back(i);

            for (auto rubbish : parsed_items) {
                delete rubbish;
            }
            parsed_items.clear();
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