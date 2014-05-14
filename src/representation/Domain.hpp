#ifndef PDDL_PLANNER_REPRESENTATION_DOMAIN
#define PDDL_PLANNER_REPRESENTATION_DOMAIN

#include <string>
#include <map>
#include <vector>
#include <stdint.h>
#include <limits>
#include <stdexcept>
#include <cstdarg>

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
            arguments.push_back(arg0);
        }

        if(!arg1.undefined())
        {
            arguments.push_back(arg1);
        }

        if(!arg2.undefined())
        {
            arguments.push_back(arg2);
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
    ArityMap arityMap;

public:
    ArityValidator(const PredicateList& predicates)
    {
        PredicateList::const_iterator cit = predicates.begin();
        for(; cit != predicates.end(); ++cit)
        {
            arityMap[cit->label] = Arity::exact( cit->arguments.size() );
        }

        addDefaults();
    }

    void addDefaults()
    {
        arityMap["and"]  = Arity::min(2);
        arityMap["or"]   = Arity::min(2);
        arityMap["not"]  = Arity::exact(1);
        arityMap["="]    = Arity::exact(2);
        arityMap["when"] = Arity::min(1);
        arityMap["forall"] = Arity::min(2);
    }

    void validate(const Label& label, uint8_t arity)
    {
        ArityMap::const_iterator cit = arityMap.find(label);
        if(cit == arityMap.end())
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

    Action(const Label& label, const TypedItem& arg0 = TypedItem(), const TypedItem& arg1 = TypedItem(), const TypedItem& arg2 = TypedItem())
        : label(label)
    {
        if(!arg0.undefined())
        {
            arguments.push_back(arg0);
        }

        if(!arg1.undefined())
        {
            arguments.push_back(arg1);
        }

        if(!arg2.undefined())
        {
            arguments.push_back(arg2);
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
};
typedef std::vector<Action> ActionList;

struct Domain
{
    std::string name;
    TypeList types;
    ConstantList constants;
    PredicateList predicates;
    RequirementList requirements;
    ActionList actions;

    Domain() {}
    Domain(const std::string& name)
        : name(name)
    {}

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

    std::string toLISP() const;
};

} // end namespace representation
} // end namespace pddl_planner

#endif // PDDL_PLANNER_REPRESENTATION_DOMAIN
