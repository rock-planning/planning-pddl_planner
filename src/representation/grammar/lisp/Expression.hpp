#ifndef PDDL_PLANNER_REPRESENTATION_GRAMMAR_LISP_EXPRESSION_HPP
#define PDDL_PLANNER_REPRESENTATION_GRAMMAR_LISP_EXPRESSION_HPP

#include <boost/version.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

//#define BOOST_SPIRIT_DEBUG
#ifdef BOOST_SPIRIT_DEBUG
// include stream operators
#include <pddl_planner/representation/grammar/Debug.hpp>
#warning "SPIRIT DEBUGGING ENABLED"

#define GRAMMAR_DEBUG_RULE(X) { using boost::spirit::qi::debug; X.name(#X); debug(X); }
#else
#define GRAMMAR_DEBUG_RULE(X)
#endif

#include <boost/fusion/include/adapt_struct.hpp>

// BOOST_FUSION_ADAPT_CLASS has been renamed to BOOST_FUSION_ADAPT_ADT for boost version > 104500
#if BOOST_VERSION < 104500
#warning "BOOST_VERSION < 104500"
#include <boost/fusion/include/adapt_class.hpp>
#else
#include <boost/fusion/adapted/adt/adapt_adt.hpp>
#include <boost/fusion/include/adapt_adt.hpp>
#endif

#if BOOST_VERSION < 104500
#define PDDL_ACL_FUSION_ADAPT BOOST_FUSION_ADAPT_CLASS
#else
#define PDDL_FUSION_ADAPT BOOST_FUSION_ADAPT_ADT
#endif

#include <pddl_planner/representation/Problem.hpp>
#include <base/Logging.hpp>

BOOST_FUSION_ADAPT_STRUCT(
    pddl_planner::representation::Expression,
    (pddl_planner::representation::Label, label)
    (pddl_planner::representation::ExpressionPtrList, parameters)
    (pddl_planner::representation::TypedItem, typedItem)
)

BOOST_FUSION_ADAPT_STRUCT(
    pddl_planner::representation::TypedItem,
    (pddl_planner::representation::Label, label)
    (pddl_planner::representation::Type, type)
)

namespace qi = boost::spirit::qi;
namespace label = qi::labels;
namespace fusion = boost::fusion;
namespace phoenix = boost::phoenix;
namespace spirit = boost::spirit;
namespace encoding = boost::spirit::ascii;

namespace pddl_planner {
namespace representation {
namespace grammar {
/**
 * While the implementation of this grammar shall not cover the complete ground of parsing a PDDL based 
 * LISP files, it tries to at least start with a proper foundation, if one eventually starts doing this: 
 *
 * Right now this grammar intends to only support basic validation tasks of expressions, e.g. to provide 
 * feedback when a user tries to extend parts of a domain or problem file manually / by a graphical interface
 *
 * \see http://cs-www.cs.yale.edu/homes/dvm/papers/pddl-bnf.pdf
 */
namespace lisp {

struct addExpressionParameterImpl
{
    template <typename E, typename P>
    struct result 
    {
        typedef void type;
    };
    template<typename E, typename P>
    void operator()(E& expression, P parameter) const
    {
        expression.addParameter(parameter);
    }
};
extern phoenix::function<addExpressionParameterImpl> addExpressionParameter;


template<typename Iterator>
struct Quantifier : qi::grammar<Iterator, std::string()>
{
    Quantifier() : Quantifier::base_type(quantifier_rule, "Quantifier-lisp_grammar")
    {
        quantifier_rule = encoding::string("forall") | encoding::string("exists");
        GRAMMAR_DEBUG_RULE(quantifier_rule);
    }

    qi::rule<Iterator, std::string()> quantifier_rule;
};

template<typename Iterator>
struct MultiOperator : qi::grammar<Iterator, std::string()>
{
    MultiOperator() : MultiOperator::base_type(multi_operator_rule, "MultiOperator-lisp_grammar")
    {
        multi_operator_rule = encoding::char_("*+");
        GRAMMAR_DEBUG_RULE(multi_operator_rule);
    }

    qi::rule<Iterator, std::string()> multi_operator_rule;
};

template<typename Iterator>
struct BinaryOperator : qi::grammar<Iterator, std::string()>
{
    BinaryOperator() : BinaryOperator::base_type(binary_operator_rule, "BinaryOperator-lisp_grammar")
    {
        binary_operator_rule = encoding::char_("-/") | multi_operator;
        GRAMMAR_DEBUG_RULE(binary_operator_rule);
    }

    qi::rule<Iterator, std::string()> binary_operator_rule;
    MultiOperator<Iterator> multi_operator;
};

template<typename Iterator>
struct BinaryComparison : qi::grammar<Iterator, std::string()>
{
    BinaryComparison() : BinaryComparison::base_type(binary_comparison_rule, "BinaryComparison-lisp_grammar")
    {
        binary_comparison_rule = encoding::char_("><=")
            | encoding::string(">=")
            | encoding::string("<=");

        GRAMMAR_DEBUG_RULE(binary_comparison_rule);
    }

    qi::rule<Iterator, std::string()> binary_comparison_rule;
};


template<typename Iterator>
struct Label : qi::grammar<Iterator, std::string()>
{
    Label() : Label::base_type(label_rule, "Label-lisp_grammar")
    {
        label_rule = (qi::alnum >> +(qi::alnum | encoding::char_("+-_[]"))) | binary_operator | binary_comparison;
        GRAMMAR_DEBUG_RULE(label_rule);
    }

    qi::rule<Iterator, std::string()> label_rule;
    BinaryOperator<Iterator> binary_operator;
    BinaryComparison<Iterator> binary_comparison;
};



template<typename Iterator>
struct Variable : qi::grammar<Iterator, std::string()>
{
    Variable() : Variable::base_type(variable_rule, "Variable-lisp_grammar")
    {
        variable_rule = encoding::string("?") >> +qi::alnum;
        GRAMMAR_DEBUG_RULE(variable_rule);
    }

    qi::rule<Iterator, std::string()> variable_rule;
};

template<typename Iterator, typename Skipper = qi::space_type>
struct TypedItem : qi::grammar<Iterator, pddl_planner::representation::TypedItem(), Skipper>
{
    TypedItem() : TypedItem::base_type(typed_item_rule, "TypedItem-lisp_grammar")
    {
        typed_item_rule = (label | variable)        [ phoenix::at_c<0>(label::_val) = label::_1 ]
                        >> *( spirit::lit("-")
                           >> label                 [ phoenix::at_c<1>(label::_val) = label::_1 ]
                           )
                        ;

       GRAMMAR_DEBUG_RULE(typed_item_rule)
    }

    qi::rule<Iterator, pddl_planner::representation::TypedItem(), Skipper> typed_item_rule;
    Variable<Iterator> variable;
    Label<Iterator> label;

};

template<typename Iterator, typename Skipper = qi::space_type>
struct Expression : qi::grammar<Iterator, pddl_planner::representation::Expression(), Skipper>
{
    Expression() : Expression::base_type(expression_rule, "Expression-lisp_grammar")
    {
        expression_rule = quantifier_expression | general_expression;
        general_expression = "(" 
                        >> label                           [ phoenix::at_c<0>(label::_val) = label::_1 ]
                        >> *(expression | simple_expression)  [ addExpressionParameter(label::_val, label::_1) ]
                        >> ")";

        simple_expression = ( "("
                            >> label                         [ phoenix::at_c<0>(label::_val) = label::_1 ]
                            >> *(label | variable)           [ addExpressionParameter(label::_val, label::_1) ]
                            >> ")"
                            ) | ( variable | label)          [ phoenix::at_c<0>(label::_val) = label::_1 ]
                        ;

        quantifier_expression = spirit::lit("(")
                            >> quantifier                    [ phoenix::at_c<0>(label::_val) = label::_1 ]
                            >> spirit::lit("(")
                                >> typedItem                 [ label::_a = label::_1 ]
                            >> spirit::lit(")")
                        >> general_expression [ addExpressionParameter(label::_val, label::_1) ]
                        >> spirit::lit(")")   [ phoenix::at_c<2>(label::_val) = label::_a];


        expression = general_expression.alias();

        GRAMMAR_DEBUG_RULE(expression_rule);
        GRAMMAR_DEBUG_RULE(general_expression);
        GRAMMAR_DEBUG_RULE(simple_expression);
        GRAMMAR_DEBUG_RULE(general_expression);
        GRAMMAR_DEBUG_RULE(simple_expression);
    }

    qi::rule<Iterator, pddl_planner::representation::Expression(), Skipper> expression_rule;
    Label<Iterator> label;
    Variable<Iterator> variable;
    TypedItem<Iterator,Skipper> typedItem;
    Quantifier<Iterator> quantifier;

    qi::rule<Iterator, pddl_planner::representation::Expression(), Skipper> general_expression;
    qi::rule<Iterator, pddl_planner::representation::Expression(), Skipper> expression;
    qi::rule<Iterator, pddl_planner::representation::Expression(), Skipper> simple_expression;
    qi::rule<Iterator, pddl_planner::representation::Expression(), qi::locals< pddl_planner::representation::TypedItem>, Skipper > quantifier_expression;
};

template<typename Iterator, typename Skipper = qi::space_type>
struct ExpressionList : qi::grammar<Iterator, pddl_planner::representation::ExpressionList(), Skipper>
{
    ExpressionList() : ExpressionList::base_type(expression_list_rule, "ExpressionList-lisp_grammar")
    {
        expression_list_rule = +expression [ phoenix::push_back(label::_val) = label::_1 ];
        GRAMMAR_DEBUG_RULE(expression_list_rule);
    }

    qi::rule<Iterator, ExpressionList(), Skipper> expression_list_rule;
    Expression<Iterator, Skipper> expression;
};

} // end namespace lips
} // end namespace grammar
} // end namespace representation
} // end namespace pddl_planner
#endif // PDDL_PLANNER_REPRESENTATION_GRAMMAR_LISP_EXPRESSION_HPP
