#include "Problem.hpp"
#include <sstream>
#include <boost/foreach.hpp>

namespace pddl_planner {
namespace representation {

Problem::Problem(const std::string& name, const Domain& domain)
    : name(name)
    , domain(domain)
{}

void Problem::addObject(const TypedItem& object, bool overwrite)
{
    objects.push_back( object );
}

void Problem::addInitialStatus(const Expression& e)
{
    ExpressionList::iterator cit = status.begin();
    for(; cit != status.end(); ++cit)
    {
        if(e == *cit)
        {
            throw std::invalid_argument("pddl_planner::representation::Domain::addInitialStatus expression '" + e.toLISP() + "' already exists");
        }
    }
    status.push_back(e);
}

void Problem::setGoal(const Expression& e)
{
    goal = e;
}

void Problem::validate() const
{
    try {
        Domain tmpDomain = domain;
        tmpDomain.validate();
        BOOST_FOREACH(Constant constant, objects)
        {
            tmpDomain.addConstant(constant, true);
        }

        VariableManager variableManager;
        variableManager.push(":init");
        BOOST_FOREACH(Expression e, status)
        {
            tmpDomain.validate(e, variableManager);
        }
        variableManager.pop();

        variableManager.push(":goal");
        BOOST_FOREACH(const Expression* e, goal.parameters)
        {
            tmpDomain.validate(*e, variableManager);
        }
        variableManager.pop();

    } catch(const std::runtime_error& e)
    {
        throw std::runtime_error("pddl_planner::representation::Problem::validate:\nSyntax error: '" + std::string(e.what()) + "\nData:\n" + toLISP());
    }
}

std::string Problem::toLISP() const
{
    std::stringstream ss;
    ss << "; BEGIN problem definition" << std::endl;
    ss << "(define (problem " << name << ")" << std::endl;
    ss << "    (:domain " << domain.name << ")" << std::endl;
    if(!objects.empty())
    {
        ss << "    (:objects " << std::endl;
        BOOST_FOREACH(TypedItem t, objects)
        {
            ss << "        " << t.label << " - " << t.type << std::endl;
        }
        ss << "    )" << std::endl;
    }

    if(!status.empty())
    {
        ss << "    (:init " << std::endl;
        BOOST_FOREACH(Expression e, status)
        {
            ss << "        " << e.toLISP() << std::endl;
        }
        ss << "    )" << std::endl;
    }

    if(!goal.isNull())
    {
        ss << "    (:goal ";
        ss << goal.toLISP();
        ss << ")" << std::endl;
    }

    ss << "; END problem definition" << std::endl;
    ss << ")" << std::endl;

    return ss.str();
}


} // end representation
} // end pddl_planner
