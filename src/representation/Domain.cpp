#include "Domain.hpp"
#include <algorithm>
#include <sstream>
#include <boost/foreach.hpp>
#include <base/Logging.hpp>

namespace pddl_planner {
namespace representation {

Expression::Expression(const Expression& other)
{
    label = other.label;
    {
        BOOST_FOREACH(Expression* e, other.parameters)
        {
            parameters.push_back(new Expression(*e));
        }
    }
}

std::string Expression::toLISP() const
{
    if(isAtomic())
    {
        return label;
    }

    std::string txt = "(" + label;
    BOOST_FOREACH(Expression* e, parameters)
    {
        txt += " " + e->toLISP();
    }
    txt += ")";
    return txt;
}

VariableManager::VariableManager(const ArgumentList& arguments)
{
    BOOST_FOREACH(TypedItem t, arguments)
    {
        registerVariable(t.label);
    }
}

void VariableManager::push(const Label& label)
{
    mOperatorStack.push_back(label);
}

Label VariableManager::pop()
{
    if(mOperatorStack.empty())
    {
        throw std::runtime_error("pddl_planner::representation::VariableManager::pop cannot pop element from empty stack");
    }

    Label label = mOperatorStack.back();
    mOperatorStack.pop_back();
    return label;
}

std::string VariableManager::currentOperatorStack() const
{
    std::string txt;
    BOOST_FOREACH(Label l, mOperatorStack)
    {
        txt += "|" + l + "|";
    }
    return txt;
}

std::string VariableManager::canonize(const std::string& name)
{
    // make sure a valid variable is passed
    if( !isVariable(name) )
    {
        return "?" + name;
    } else {
        return name;
    }
}

bool VariableManager::isVariable(const std::string& name)
{
    return !name.empty() && name.data()[0] == '?';
}

void VariableManager::registerVariable(const std::string& name)
{
    mKnownVariables.push_back( canonize(name) );
}

bool VariableManager::isKnownVariable(const std::string& name) const
{
    return mKnownVariables.end() != std::find(mKnownVariables.begin(), mKnownVariables.end(), canonize(name) );
}

bool VariableManager::hasTypedVariable(const TypedItemList& list, const TypedItem& item) const
{
    TypedItemList::const_iterator cit = list.begin();
    for(; cit != list.end(); ++cit)
    {
        if(cit->label == canonize(item.label))
        {
            if(cit->type != item.type)
            {
                throw std::runtime_error("pddl_planner::representation::Variable exist but with different type: '" + cit->type  + "' while trying to add: '" + item.type);
            }
            return true;
        }
    }

    return false;
}

void VariableManager::addTypedVariable(TypedItemList& list, const TypedItem& item)
{
    list.push_back( TypedItem( canonize(item.label), item.type ) );
}

void Action::addArgument(const TypedItem& arg)
{
    VariableManager::addTypedVariable(arguments, arg);
}

bool Action::isArgument(const Label& label)
{
    BOOST_FOREACH(TypedItem t, arguments)
    {
        if(t.label == label)
        {
            return true;
        }
    }
    return false;
}

bool ArityValidator::isQuantifier(const Label& label) const
{
    if(label == "forall" || label == "exists")
    {
        return true;
    }
    return false;
}

bool ArityValidator::isOperator(const Label& label) const
{
    return mArityMap.count(label);
}

Domain::Domain(const std::string& name)
    : name(name)
{
    addRequirement("typing");
}

void Domain::addType(const Type& type)
{
    types.push_back(type);
}

void Domain::addConstant(const TypedItem& constant, bool overwrite)
{
    if(isConstant(constant.label))
    {
        if(overwrite)
        {
            removeConstant(constant.label);
        } else {
            throw std::invalid_argument("pddl_planner::representation::Domain::addConstant constant '" + constant.label + "' already exists");
        }
    } else if(!isType(constant.type))
    {
        throw std::invalid_argument("pddl_planner::representation::Domain::addConstant unknown type '" + constant.type + "'");
    }

    constants.push_back( constant );
}

void Domain::addPredicate(const Predicate& predicate, bool overwrite)
{
    if(isPredicate(predicate.label) && !overwrite)
    {
        throw std::invalid_argument("pddl_planner::representation::Domain::addConstant predicate '" + predicate.label + "' already exists");
    } else {
        removePredicate(predicate.label);
    }
    predicates.push_back(predicate);
}

void Domain::addRequirement(const Requirement& requirement)
{
    if(!isRequirement(requirement))
    {
        requirements.push_back(requirement);
    } else {
        LOG_INFO_S << "Requirement: '" << requirement << "' already exists";
    }
}

void Domain::addAction(const Action& action, bool overwrite)
{
    if(isAction(action.label) && !overwrite)
    {
        throw std::invalid_argument("pddl_planner::representation::Domain::addConstant predicate '" + action.label + "' already exists");
    } else {
        removeAction(action.label);
    }

    actions.push_back(action);
}

void Domain::removeConstant(const Label& label)
{
    ConstantList::iterator cit = constants.begin();
    for(; cit != constants.end(); ++cit)
    {
        if(cit->label == label)
        {
            constants.erase(cit);
        }
    }
}

void Domain::removePredicate(const Label& label)
{
    PredicateList::iterator cit = predicates.begin();
    for(; cit != predicates.end(); ++cit)
    {
        if(cit->label == label)
        {
            predicates.erase(cit);
        }
    }
}

void Domain::removeAction(const Label& label)
{
    ActionList::iterator cit = actions.begin();
    for(; cit != actions.end(); ++cit)
    {
        if(cit->label == label)
        {
            actions.erase(cit);
        }
    }
}

bool Domain::isType(const Type& type) const
{
    return types.end() != std::find(types.begin(), types.end(), type);
}

bool Domain::isConstant(const Label& label) const
{
    ConstantList::const_iterator cit = constants.begin();
    for(; cit != constants.end(); ++cit)
    {
        if(cit->label == label)
        {
            return true;
        }
    }
    return false;
}

bool Domain::isPredicate(const Label& label) const
{
    PredicateList::const_iterator cit = predicates.begin();
    for(; cit != predicates.end(); ++cit)
    {
        if(cit->label == label)
        {
            return true;
        }
    }
    return false;
}

bool Domain::isRequirement(const Requirement& requirement) const
{
    RequirementList::const_iterator cit = requirements.begin();
    for(; cit != requirements.end(); ++cit)
    {
        if(*cit == requirement)
        {
            return true;
        }
    }
    return false;
}

bool Domain::isAction(const Label& label) const
{
    ActionList::const_iterator cit = actions.begin();
    for(; cit != actions.end(); ++cit)
    {
        if(cit->label == label)
        {
            return true;
        }
    }
    return false;
}

Predicate Domain::getPredicate(const Label& label) const
{
    BOOST_FOREACH(Predicate p, predicates)
    {
        if(p.label == label)
        {
            return p;
        }
    }
    throw std::runtime_error("pddl_planner::representation::Domain::getPredicate: predicate '" + label + "' could not be found");
}

Action Domain::getAction(const Label& label) const
{
    BOOST_FOREACH(Action a, actions)
    {
        if(a.label == label)
        {
            return a;
        }
    }
    throw std::runtime_error("pddl_planner::representation::Domain::getAction: action '" + label + "' could not be found");
}

std::string Domain::toLISP() const
{
    std::stringstream ss;
    ss << "; BEGIN domain definition" << std::endl;
    ss << "(define (domain " << name << ")" << std::endl;
    if(!requirements.empty())
    {
        ss << "    (:requirements";
        BOOST_FOREACH(Requirement r, requirements)
        {
            ss << " :" << r;
        }
        ss << "    )" << std::endl;
    }

    if(!types.empty())
    {
        ss << "    (:types";
        BOOST_FOREACH(Type t, types)
        {
            ss << " " << t;
        }
        ss << ")" << std::endl;
    }

    if(!constants.empty())
    {
        ss << "    (:constants " << std::endl;
        BOOST_FOREACH(Type t, types)
        {
            std::stringstream constantLine;
            BOOST_FOREACH(Constant c, constants)
            {
                if(c.type == t)
                {
                    constantLine << " " << c.label;
                }
            }
            if(constantLine.str() != "")
            {
                ss << "        " << constantLine.str();
                ss << " - " << t << std::endl;
            }
        }
        ss << "    )" << std::endl;
    }

    if(!predicates.empty())
    {
        ss << "    (:predicates " << std::endl;

        BOOST_FOREACH(Predicate p, predicates)
        {
            ss << "        ( " << p.label;

            BOOST_FOREACH(TypedItem arg, p.arguments)
            {
                ss << " " << arg.label << " - " << arg.type;
            }
            ss << ")" << std::endl;
        }
        ss << "    )" << std::endl;
    }

    if(!actions.empty())
    {
        BOOST_FOREACH(Action a, actions)
        {
            ss << "    (:action " << a.label << std::endl;

            ss << "        :parameters (";
            BOOST_FOREACH(TypedItem arg, a.arguments)
            {
                ss << " " << arg.label << " - " << arg.type;
            }
            ss << ")" << std::endl;

            if(!a.preconditions.empty())
            {
                ss << "        :precondition";
                BOOST_FOREACH(Expression e, a.preconditions)
                {
                    ss << " " << e.toLISP();
                }
                ss << std::endl;
            }

            if(!a.effects.empty())
            {
                ss << "        :effect";
                BOOST_FOREACH(Expression e, a.effects)
                {
                    ss <<  " " << e.toLISP();
                }
            }
            ss << std::endl << "    )" << std::endl;
        }
    }

    ss << "; END domain definition" << std::endl;
    ss << ")" << std::endl;

    return ss.str();
}

void Domain::validate(const Expression& e, const VariableManager& variableManager) const
{
    ArityValidator operatorValidator;

    if(e.isAtomic())
    {
        LOG_DEBUG_S << "Validating atomic statement: '" << e.label << "'";
        if(! (isConstant(e.label) || isType(e.label) || variableManager.isKnownVariable(e.label)))
        {
            throw std::runtime_error("pddl_planner::representation::Domain::validateExpression '" + e.label + "' in '" + variableManager.currentOperatorStack() + "' is not a registered constant, type or local variable");
        }
    } else if( isPredicate(e.label) )
    {
        LOG_DEBUG_S << "Validating predicate: '" << e.label << "'";
        Predicate p = getPredicate(e.label);
        BOOST_FOREACH(Expression* ePtr, e.parameters)
        {
            validate(*ePtr, variableManager);
        }
    } else if( isAction(e.label))
    {
        LOG_DEBUG_S << "Validating action: '" << e.label << "'";
        Action a = getAction(e.label);
        BOOST_FOREACH(Expression* ePtr, e.parameters)
        {
            validate(*ePtr, variableManager);
        }
    } else if( operatorValidator.isQuantifier(e.label) )
    {
        // TODO: support here to validate free and bound variables
        LOG_DEBUG_S << "Validation of quantifier '" << e.label << "' unsupported, i.e. defaulting to true";
    } else if( operatorValidator.isOperator(e.label) )
    {
        LOG_DEBUG_S << "Validating operator: '" << e.label << "'";
        BOOST_FOREACH(Expression* ePtr, e.parameters)
        {
            validate(*ePtr, variableManager);
        }
    } else {
        LOG_DEBUG_S << "expression '" << e.label << "' is not a known constant, predicate, action or variable";
        throw std::runtime_error("pddl_planner::representation::Domain::validateExpression '" + e.label + "' in '" + variableManager.currentOperatorStack() + "' is neither a known constant, predicate, action or variable");
    }
}

void Domain::validate() const
{
    if(isNull())
    {
        throw std::runtime_error("pddl_planner::representation::Domain::validate domain is empty");
    }

    BOOST_FOREACH(Action action, actions)
    {
        LOG_DEBUG_S << "Validating action: " << action.label;

        VariableManager variableManager(action.arguments);

        BOOST_FOREACH(Expression e, action.preconditions)
        {
            LOG_DEBUG_S << "Validating precondition: " << e.label;
            validate(e, variableManager);
        }

        BOOST_FOREACH(Expression e, action.effects)
        {
            LOG_DEBUG_S << "Validating effect: " << e.label;
            validate(e, variableManager);
        }
    }
}

} // end namespace pddl_planner
} // end namespace representation
