#include <pddl_planner/Planning.hpp>
#include <pddl_planner/PDDLPlannerInterface.hpp>
#include <pddl_planner/planners/Lama.hpp>
#include <boost/assign/list_of.hpp>
#include <base/Logging.hpp>

namespace pddl_planner
{

Planning::Planning()
{
    mPlanners = boost::assign::map_list_of
        ("LAMA", new pddl_planner::lama::Planner());
}

Planning::~Planning()
{
    PlannerMap::iterator it = mPlanners.begin();
    for(; it != mPlanners.end(); ++it)
    {
        delete it->second;
        it->second = NULL;
    }
}

void Planning::registerPlanner(PDDLPlannerInterface* planner)
{
    if(!planner)
    {
        throw std::runtime_error("pddl_planner::Planning: planner object is null!");
    }

    std::string name = planner->getName();

    try {
        getPlanner(name);
        throw std::runtime_error("pddl_planner::Planning: planner with name '" + name + "' is already registered");
    } catch(const std::runtime_error& )
    {
        mPlanners[name] = planner;
    }
}

PDDLPlannerInterface* Planning::getPlanner(const std::string& name) const
{
    std::map<std::string, PDDLPlannerInterface*>::const_iterator it = mPlanners.find(name);
    if(it != mPlanners.end())
    {
        return it->second;
    }
    throw std::runtime_error("pddl_planner::Planning: planner with name '" + name + "' does not exist");
}

PlannerList Planning::getRegisteredPlanners() const
{
    PlannerList planners;
    PlannerMap::const_iterator cit = mPlanners.begin();
    for(; cit != mPlanners.end(); ++cit)
    {
        planners.push_back(cit->second);
    }

    return planners;
}

void Planning::setActionDescription(const std::string& action, const std::string& description)
{
    mActionDescriptions[action] = description;
}

void Planning::setDomainDescription(const std::string& domain, const std::string& description)
{
    mDomainDescriptions[domain] = description;
}

std::string Planning::getActionDescriptions() const
{
    std::string actionDescriptions;
    ActionDescriptions::const_iterator it = mActionDescriptions.begin();
    for(; it != mActionDescriptions.end(); ++it)
    {
        actionDescriptions += it->second + "\n";
    }
    return actionDescriptions;
}

std::string Planning::getDomainDescriptions() const
{
    std::string domainDescriptions;
    DomainDescriptions::const_iterator it = mDomainDescriptions.begin();
    for(; it != mDomainDescriptions.end(); ++it)
    {
        domainDescriptions += it->second + "\n";
    }
    return domainDescriptions;
}

PlanCandidates Planning::plan(const std::string& problem, const std::string& planner_name)
{
    LOG_DEBUG_S << "Planning requested: " << std::endl
        << "-DOMAIN-" << std::endl << getDomainDescriptions()
        << "-PROBLEM-" << std::endl << problem;

    PDDLPlannerInterface* planner = getPlanner(planner_name);
    return planner->plan(problem, getActionDescriptions(), getDomainDescriptions());
}

PlanCandidates Planning::plan(const representation::Problem& problem, const std::string& planner_name)
{
    setDomainDescription(problem.domain.name, problem.domain.toLISP());
    return plan(problem.toLISP(), planner_name);
}

PlanCandidates Planning::plan(const representation::Domain& domain, const representation::Problem& problem, const std::string& planner_name)
{
    setDomainDescription(domain.name, domain.toLISP());
    return plan(problem.toLISP(), planner_name);
}

}
