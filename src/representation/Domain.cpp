#include "Domain.hpp"
#include <algorithm>
#include <sstream>
#include <boost/foreach.hpp>

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
    requirements.push_back(requirement);
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

            ss << "        (:parameters";
            BOOST_FOREACH(TypedItem arg, a.arguments)
            {
                ss << " " << arg.label << " - " << arg.type;
            }
            ss << ")" << std::endl;

            if(!a.preconditions.empty())
            {
                ss << "        (:precondition";
                BOOST_FOREACH(Expression e, a.preconditions)
                {
                    ss << " " << e.toLISP();
                }
                ss << ")" << std::endl;
            }

            if(!a.effects.empty())
            {
                ss << "        (:effect";
                BOOST_FOREACH(Expression e, a.effects)
                {
                    ss <<  " " << e.toLISP();
                }
                ss << ")" << std::endl;
            }
            ss << "    )" << std::endl;
        }
    }

    ss << "; END domain definition" << std::endl;
    ss << ")" << std::endl;

    return ss.str();
}

} // end namespace pddl_planner
} // end namespace representation
