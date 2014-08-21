#include "PDDLPlannerTypes.hpp"

namespace pddl_planner
{

Action::Action(const std::string& name)
    : name(name)
{}

void Action::addArgument(const std::string& argument)
{
    arguments.push_back(argument);
}

std::string Action::toString() const
{
    std::string action = name;
    std::vector<std::string>::const_iterator it = arguments.begin();
    for(;it != arguments.end(); ++it)
    {
        action += " " + *it;
    }
    boost::algorithm::trim(action);
    return action;
}

void Plan::addAction(const Action& action)
{
    action_sequence.push_back(action);
}

std::string Plan::toString() const
{
    std::string plan;
    std::vector<Action>::const_iterator it = action_sequence.begin();
    for(;it != action_sequence.end(); ++it)
    {
        plan += "[" + it->toString() + "]";
    }
    boost::algorithm::trim(plan);
    return plan;
}


void PlanCandidates::addPlan(const Plan& plan)
{
    plans.push_back(plan);
}

std::string PlanCandidates::toString() const
{
    std::string candidates;
    std::vector<Plan>::const_iterator it = plans.begin();
    int index = 0;
    for(;it != plans.end(); ++it)
    {
        std::stringstream ss;
        ss << index++ << "\t" << it->toString() << "\n";
        candidates += ss.str();
    }
    return candidates;

}

} // end namespace pddl_planner
