#ifndef PDDL_PLANNER_REPRESENTATION_DOMAIN
#define PDDL_PLANNER_REPRESENTATION_DOMAIN

#include <string>
#include <map>
#include <vector>
#include <stdint.h>
#include <limits>
#include <stdexcept>
#include <cstdarg>
#include <stack>

namespace pddl_planner {
namespace representation {

typedef std::string Label;
typedef Label Operator;
typedef std::string Type;
typedef std::string Requirement;

typedef std::vector<Type> TypeList;
typedef std::vector<Requirement> RequirementList;

struct TypedItem
{
    Label label;
    Type type;

    TypedItem()
    {}

    TypedItem(const Label& label, const Type& type)
        : label(label)
        , type(type)
    {}

    bool undefined() const { return label.empty() || type.empty(); }

};

typedef TypedItem Constant;

typedef std::vector<TypedItem> TypedItemList;
typedef TypedItemList ConstantList;
typedef TypedItemList ArgumentList;
typedef TypedItemList ParameterList;


/**
 * Manage variable, i.e. for PDDL description these variables start with a quotation mark
 */
class VariableManager
{
    std::vector<std::string> mKnownVariables;
    std::vector<Label> mOperatorStack;

public:
    VariableManager(const ArgumentList& arguments = ArgumentList());

    void push(const Label& label);
    Label pop();

    std::string currentOperatorStack() const;

    /*
     * Create a variable name, i.e. a string prefixed with ?
     */
    static std::string canonize(const std::string& name);

    /*
     * Test if the given name indicates a variable (possibly unregistered though)
     */
    static bool isVariable(const std::string& name);

    void registerVariable(const std::string& name);
    bool isKnownVariable(const std::string& name) const;

    /**
     * Check whether the provided item already exists in the list
     * \throws if the types of the existing item and the item for testing differ, though 
     * the labels are the same
     */
    bool hasTypedVariable(const TypedItemList& list, const TypedItem& item) const;

    /**
     * Add a typed variable to a list
     * This makes sure that the label provided in TypedItem is a variable
     */
    static void addTypedVariable(TypedItemList& list, const TypedItem& item);
};


struct Predicate
{
    Label label;
    ArgumentList arguments;

    Predicate(const Label& label, const ArgumentList& argumentList)
        : label(label)
        , arguments(argumentList)
    {}

    Predicate(const Label& label, const TypedItem& arg0 = TypedItem(), const TypedItem& arg1 = TypedItem(), const TypedItem& arg2 = TypedItem())
        : label(label)
    {
        if(!arg0.undefined())
        {
            VariableManager::addTypedVariable(arguments, arg0);
        }

        if(!arg1.undefined())
        {
            VariableManager::addTypedVariable(arguments, arg1);
        }

        if(!arg2.undefined())
        {
            VariableManager::addTypedVariable(arguments, arg2);
        }

    }
};
typedef std::vector<Predicate> PredicateList;

class Arity
{
    uint8_t mMin;
    uint8_t mMax;

public:
    Arity()
        : mMin( std::numeric_limits<uint8_t>::min() )
        , mMax( std::numeric_limits<uint8_t>::max() )
    {}

    Arity(uint8_t min, uint8_t max)
        : mMin(min)
        , mMax(max)
    {
        if(mMin > mMax)
        {
            throw std::invalid_argument("pddl_planner::representation::Arity: min arity greater than max");
        }
    }

    uint8_t getMin() const { return mMin; }
    uint8_t getMax() const { return mMax; }

    static Arity exact(uint8_t n) { return Arity(n,n); }
    static Arity min(uint8_t n) { return Arity(n,std::numeric_limits<uint8_t>::max()); }
    static Arity max(uint8_t n) { return Arity(std::numeric_limits<uint8_t>::min(),n); }
};

typedef std::map<Label, Arity> ArityMap;

class ArityValidator
{
    ArityMap mArityMap;

public:
    ArityValidator(const PredicateList& predicates = PredicateList())
    {
        PredicateList::const_iterator cit = predicates.begin();
        for(; cit != predicates.end(); ++cit)
        {
            mArityMap[cit->label] = Arity::exact( cit->arguments.size() );
        }

        addDefaults();
    }

    void addDefaults()
    {
        mArityMap["and"]  = Arity::min(2);
        mArityMap["or"]   = Arity::min(2);
        mArityMap["not"]  = Arity::exact(1);
        mArityMap["="]    = Arity::exact(2);
        mArityMap["when"] = Arity::min(1);
        mArityMap["forall"] = Arity::min(2);
    }

    bool isOperator(const Label& label) const;

    bool isQuantifier(const Label& label) const;

    void validate(const Label& label, uint8_t arity)
    {
        ArityMap::const_iterator cit = mArityMap.find(label);
        if(cit == mArityMap.end())
        {
            throw std::invalid_argument("pddl_planner::representation::ArityValidator: unknown predicate or operator: '" + label + "'");
        }

        Arity allowedArity = cit->second;

        if(arity < allowedArity.getMin()) 
        {
            throw std::invalid_argument("pddl_planner::representation::ArityValidator predicate or operator: '" + label + "' provided with too few parameters");
        } else if(arity > allowedArity.getMax())
        {
            throw std::invalid_argument("pddl_planner::representation::ArityValidator predicate or operator: '" + label + "' provided with too many parameters");
        }
    }
};


class Expression;
typedef std::vector<Expression*> ExpressionPtrList;
/**
 * Expression are needed to handle addition of actions, e.g., as part of preconditions or effect
 */
struct Expression
{
    Label label;
    ExpressionPtrList parameters;

    Expression(const Label& label = "")
        : label(label)
    {}

    Expression(const Expression& other);

    ~Expression()
    {
        ExpressionPtrList::iterator it = parameters.begin();
        for(; it != parameters.end(); ++it)
        {
            delete *it;
            *it = NULL;
        }
    }

    Expression(const Label& label, const Expression& arg0, const Expression& arg1 = Expression(), const Expression& arg2 = Expression())
        : label(label)
    {
        if(!arg0.isNull())
        {
            addParameter(arg0);
        }

        if(!arg1.isNull())
        {
            addParameter(arg1);
        }

        if(!arg2.isNull())
        {
            addParameter(arg2);
        }
    }

    Expression(const Label& label, const Label& arg0, const Label& arg1 = "", const Label& arg2 = "")
        : label(label)
    {
        if(!arg0.empty())
        {
            addParameter(arg0);
        }

        if(!arg1.empty())
        {
            addParameter(arg1);
        }

        if(!arg2.empty())
        {
            addParameter(arg2);
        }
    }

    void addParameter(const Label& e)
    {
        parameters.push_back( new Expression(e) );
    }

    void addParameter(const Expression& e)
    {
        parameters.push_back( new Expression(e) );
    }

    bool isAtomic() const { return parameters.empty(); }

    bool isNull() const { return label.empty(); }

    std::string toLISP() const;
};

typedef std::vector<Expression> ExpressionList;

struct Action
{
    Label label;
    ArgumentList arguments;
    ExpressionList preconditions;
    ExpressionList effects;

    Action(const Label& label, const ArgumentList& arguments)
        : label(label)
        , arguments(arguments)
    {}

    Action(const Label& label, const TypedItem& arg0 = TypedItem(), const TypedItem& arg1 = TypedItem(), const TypedItem& arg2 = TypedItem(), const TypedItem& arg3 = TypedItem())
        : label(label)
    {
        if(!arg0.undefined())
        {
            addArgument(arg0);
        }

        if(!arg1.undefined())
        {
            addArgument(arg1);
        }

        if(!arg2.undefined())
        {
            addArgument(arg2);
        }

        if(!arg3.undefined())
        {
            addArgument(arg3);
        }
    }

    void addPrecondition(const Expression& e)
    {
        preconditions.push_back(e);
    }
    void addEffect(const Expression& e)
    {
        effects.push_back(e);
    }

    void addArgument(const TypedItem& arg);

    bool isArgument(const Label& label);
};
typedef std::vector<Action> ActionList;

/**
 * \brief An internal representation of a PDDL domain description
 * \details This class allows to programmatically build a PDDL domain description and
 * allow to export the current state in LISP format
 *
 * Currently, the domain requires 'typing' support by default 
 */
struct Domain
{
    std::string name;
    TypeList types;
    ConstantList constants;
    PredicateList predicates;
    RequirementList requirements;
    ActionList actions;

    Domain(const std::string& name = "");

    void addType(const Type& type);
    void addConstant(const TypedItem& type, bool overwrite = false);
    void addPredicate(const Predicate& predicate, bool overwrite = false);
    void addRequirement(const Requirement& requirement);
    void addAction(const Action& action, bool overwrite = false);

    void removeConstant(const Label& label);
    void removePredicate(const Label& label);
    void removeAction(const Label& label);

    bool isType(const Type& type) const;
    bool isConstant(const Label& label) const;
    bool isPredicate(const Label& label) const;
    bool isRequirement(const Requirement& requirement) const;
    bool isAction(const Label& label) const;

    Predicate getPredicate(const Label& label) const;
    Action getAction(const Label& label) const;

    std::string toLISP() const;

    bool isNull() const { return name.empty(); }

    void validate(const Expression& e, const VariableManager& variableManager = VariableManager()) const;

    /**
     * Perform a simple syntax check
     * \throw std::runtime_error if syntax has errors
     */
    void validate() const;
};

} // end namespace representation
} // end namespace pddl_planner

#endif // PDDL_PLANNER_REPRESENTATION_DOMAIN
