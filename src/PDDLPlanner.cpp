#include <pddl_planner/PDDLPlanner.hpp>
#include <pddl_planner/PDDLPlannerInterface.hpp>

namespace pddl_planner
{

PDDLPlanner::PDDLPlanner(PDDLPlannerInterface* planner)
    : mPlanner(planner)
{
}

void PDDLPlanner::setActionDescription(const std::string& action, const std::string& description)
{
    mActionDescriptions[action] = description;
}

void PDDLPlanner::setDomainDescription(const std::string& domain, const std::string& description)
{
    mDomainDescriptions[domain] = description;
}

std::string PDDLPlanner::getActionDescriptions() const
{
    std::string actionDescriptions;
    ActionDescriptions::const_iterator it = mActionDescriptions.begin();
    for(; it != mActionDescriptions.end(); ++it)
    {
        actionDescriptions += it->second + "\n";
    }
    return actionDescriptions;
}

std::string PDDLPlanner::getDomainDescriptions() const
{
    std::string domainDescriptions;
    DomainDescriptions::const_iterator it = mDomainDescriptions.begin();
    for(; it != mDomainDescriptions.end(); ++it)
    {
        domainDescriptions += it->second + "\n";
    }
    return domainDescriptions;
}

PlanCandidates PDDLPlanner::plan(const std::string& goal)
{
    if(!mPlanner)
    {
        throw PlanGenerationException("No planner available");
    }

    return mPlanner->plan(goal, getActionDescriptions(), getDomainDescriptions());
}

}
