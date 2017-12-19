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

/**
 * \class TypedItem
 * \brief A tuple representation providing a name/label and a type
 */
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

    bool operator!=(const TypedItem& other) const { return ! operator==(other); }
    bool operator==(const TypedItem& other) const { return label == other.label && type == other.type; }

};

typedef TypedItem Constant;

typedef std::vector<TypedItem> TypedItemList;
typedef TypedItemList ConstantList;
typedef TypedItemList ArgumentList;
typedef TypedItemList ParameterList;


/**
 * \class VariableManager
 * \brief Manage variable, i.e. for PDDL description these variables start with a quotation mark
 */
class VariableManager
{
    std::vector<std::string> mKnownVariables;
    std::vector<Label> mOperatorStack;

public:
    VariableManager(const ArgumentList& arguments = ArgumentList());

    void push(const Label& label);
    Label pop();

    /**
     * Get operator stack as list
     * \return list of operators, latest operator at the end of the list
     */
    std::vector<Label> getOperatorStack() const { return mOperatorStack; }

    /**
     * Get the operator stack as string for debugging purposes
     * \return list of operators
     */
    std::string getOperatorStackAsString() const;

    /*
     * Create a variable name, i.e. a string prefixed with ?
     * \return canonized string of given name
     */
    static std::string canonize(const std::string& name);

    /*
     * Test if the given name indicates a variable (currently indicated by starting with ?)
     * \return true if name is a variable, false otherwise
     */
    static bool isVariable(const std::string& name);

    /**
     * Register variable by name
     * \param name Name of variable
     */
    void registerVariable(const std::string& name);

    /**
     * Test is variable is known / registered
     * \param name Name of variable
     * \return True is variable is known, false otherwise
     */
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

/**
 * \struct Predicate
 * \brief Representation of a predicate
 * \details A predicate is defined by the label and associated arguments
 */
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

// Function has same structure as a predicate
typedef Predicate Function;
typedef std::vector<Function> FunctionList;

/**
 * \class Arity
 * \brief Arity represents the arity of operations allowing to define exact, minimum and maximum arity
 */
class Arity
{
    uint8_t mMin;
    uint8_t mMax;

public:
    /**
     * Default constructor
     */
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

    /**
     * Get minimum arity
     * \return arity
     */
    uint8_t getMin() const { return mMin; }

    /**
     * Get maximum arity
     * \return arity
     */
    uint8_t getMax() const { return mMax; }

    /**
     * Create an Arity object by defining the exact arity
     * \param n
     * \return arity
     */
    static Arity exact(uint8_t n) { return Arity(n,n); }

    /**
     * Create an Arity object by defining minimum arity
     * \param n
     * \return arity
     */
    static Arity min(uint8_t n) { return Arity(n,std::numeric_limits<uint8_t>::max()); }

    /**
     * Create an Arity object by defining maximum arity
     * \param n
     * \return arity
     */
    static Arity max(uint8_t n) { return Arity(std::numeric_limits<uint8_t>::min(),n); }
};

typedef std::map<Label, Arity> ArityMap;

/**
 * \class ArityValidator
 * \brief Class to validate usage of operations against argument requirements
 */
class ArityValidator
{
    ArityMap mArityMap;

    /**
     * Add default operations and quantifiers
     */
    void addDefaults();

public:
    ArityValidator(const PredicateList& predicates = PredicateList());

    /**
     * Test if label represents an operator
     * \return true if label represents an operator, false otherwise
     */
    bool isOperator(const Label& label) const;

    /**
     * Test is label represents a quantifier
     * \return true if label represents a quantifier, false otherwise
     */
    bool isQuantifier(const Label& label) const;

    /**
     * Validate arity of given operation or quantifier
     * \param label Identifier of operation or quantifier (or action)
     * \throw std::invalid_argument if arity is not correct for the given
     */
    void validate(const Label& label, uint8_t arity);
};


enum Quantor { UNKNOWN_QUANTOR, FORALL, EXISTS };
extern std::map<Quantor,std::string> QuantorTxt;

class Expression;
typedef std::vector<Expression*> ExpressionPtrList;
/**
 * \class Expression
 * \brief Representation of (LISP) expressions
 * \details Expression are needed to handle addition of actions, e.g., as part of preconditions or effects
 */
struct Expression
{
    Label label;
    ExpressionPtrList parameters;

    // For quantor expression
    TypedItem typedItem;

    Expression(const Label& label = "");
    Expression(const Expression& other);
    virtual ~Expression();

    Expression(const Label& label, const Expression& arg0, const Expression& arg1 = Expression(), const Expression& arg2 = Expression(), const Expression& arg3 = Expression(), const Expression& arg4 = Expression(), const Expression& arg5 = Expression(), const Expression& arg6 = Expression(), const Expression& arg7 = Expression(), const Expression& arg8 = Expression(), const Expression& arg9 = Expression(), const Expression& arg10 = Expression());

    Expression(const Label& label, const Label& arg0, const Label& arg1 = "", const Label& arg2 = "");

    Expression(Quantor quantor, const TypedItem& typedItem, const Expression& e);

    /**
     * Add a parameter to this expression -- this parameter is a simple label, but no expression
     */
    void addParameter(const Label& e);
    void addParameter(const Expression& e);

    bool isAtomic() const { return parameters.empty(); }

    bool isNull() const { return label.empty(); }

    static bool isQuantor(const Label& label);

    /**
     * Assign operator
     */
    Expression& operator=(const Expression& other);

    /**
     * Equals operator
     * \return true if two expressions are equivalent, false otherwise
     */
    bool operator==(const Expression& other) const;

    /**
     * Convert expression to LISP representation
     * \return LISP expression
     */
    std::string toLISP() const;

    /**
     * Get and expression from a given string
     * \return Expression
     */
    static Expression fromString(const std::string& expressionString);
};

typedef std::vector<Expression> ExpressionList;

/**
 * An action defined in a domain consists of an identifier (label) and a list of arguments.
 * To allow for propery planning preconditions and effects are provided
 */
struct Action
{
    Label label;
    ArgumentList arguments;
    ExpressionList preconditions;
    ExpressionList effects;

    /**
     * Constructor supporting an arbitrary number of arguments
     * \param label Label (identifier) of the action
     * \param arguments List of arguments for this action
     */
    Action(const Label& label, const ArgumentList& arguments)
        : label(label)
        , arguments(arguments)
    {}

    /**
     * Constructor supporting up to four arguments
     * \param label Label (identifier) of the action
     * \param arg0 (optional) argument
     * \param arg1 (optional) argument
     * \param arg2 (optional) argument
     * \param arg3 (optional) argument
     */
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

    /**
     * Add precondition to action
     * \param e Expression defining a precondition
     */
    void addPrecondition(const Expression& e)
    {
        preconditions.push_back(e);
    }

    /**
     * Add effect to action
     * \param e Expression defining an effect of this action
     */
    void addEffect(const Expression& e)
    {
        effects.push_back(e);
    }

    /**
     * Append an argument to this action's list of arguments, i.e.
     * the order in which arguments are added does matter
     * \param arg Expression defin
     */
    void addArgument(const TypedItem& arg);

    /**
     * Test whether a given label corresponds to an argument of this action
     * \return true if label matches an existing argument of this action, false if not
     */
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
    // Domain name
    std::string name;
    // List of types of this domain
    TypeList types;
    // Map a type to a parent type if there is any
    std::map<Type,Type> type2parent;
    // List of constants in this domain
    ConstantList constants;
    // List of predicates in this domain
    PredicateList predicates;
    // List of requirements for using this domain description, e.g. 'strips' or 'typing'
    RequirementList requirements;
    // List of actions in this domain
    ActionList actions;
    // List of functions in this domain
    FunctionList functions;

    /**
     * Default domain constructor
     * \param name Name of the domain
     */
    Domain(const std::string& name = "");

    /**
     * Add a domain type
     * \param type Type definition
     */
    void addType(const Type& type, const Type& parentType = "");

    void addConstant(const TypedItem& type, bool overwrite = false);
    void addPredicate(const Predicate& predicate, bool overwrite = false);
    void addRequirement(const Requirement& requirement);
    void addAction(const Action& action, bool overwrite = false);
    void addFunction(const Function& function, bool overwrite = false);

    void removeConstant(const Label& label);
    void removePredicate(const Label& label);
    void removeAction(const Label& label);
    void removeFunction(const Label& label);

    bool isType(const Type& type) const;
    bool isConstant(const Label& label) const;
    bool isPredicate(const Label& label) const;
    bool isRequirement(const Requirement& requirement) const;
    bool isAction(const Label& label) const;
    bool isFunction(const Label& label) const;

    Predicate getPredicate(const Label& label) const;
    Action getAction(const Label& label) const;

    std::string toLISP() const;

    /**
     * Check if domain has a name
     * \return true if name is given, false otherwise
     */
    bool hasName() const { return !name.empty(); }

    /**
     * Check if a properly named domain definition is given
     * \return true if domain is defined, false otherwise
     */
    bool isNull() const { return !hasName(); }

    /**
     * Validate an expression using a given variable manager
     * \throw std::runtime_error if expression is not properly defined
     */
    void validate(const Expression& e, const VariableManager& variableManager = VariableManager()) const;

    /**
     * Perform a simple syntax check
     * \throw std::runtime_error if expression is not properly defined
     */
    void validate() const;
};

} // end namespace representation
} // end namespace pddl_planner

#endif // PDDL_PLANNER_REPRESENTATION_DOMAIN
