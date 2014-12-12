#include <pddl_planner/Planning.hpp>
#include <pddl_planner/PDDLPlannerInterface.hpp>
#include <pddl_planner/planners/Lama.hpp>
#include <pddl_planner/planners/Bfsf.hpp>
#include <pddl_planner/planners/Uniform.hpp>
#include <pddl_planner/planners/Randward.hpp>
#include <pddl_planner/planners/Cedalion.hpp>
#include <pddl_planner/planners/ArvandHerd.hpp>
#include <pddl_planner/planners/FastDownward.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/assign.hpp>
#include <base/Logging.hpp>

namespace pddl_planner
{

Planning::Planning()
{
    mPlanners = boost::assign::map_list_of
                    ("LAMA", dynamic_cast<pddl_planner::PDDLPlannerInterface*>(new pddl_planner::lama::Planner()))
                    ("BFSF", new pddl_planner::bfsf::Planner())
                    ("UNIFORM", new pddl_planner::uniform::Planner())
                    ("CEDALION", new pddl_planner::cedalion::Planner())
                    ("RANDWARD", new pddl_planner::randward::Planner())
                    ("ARVANDHERD", new pddl_planner::arvandherd::Planner())
                    ("FDSS1", new pddl_planner::fast_downward::Planner("sas_plan", "seq-sat-fdss-1"))
                    ("FDSS2", new pddl_planner::fast_downward::Planner("sas_plan", "seq-sat-fdss-2"))
                    ("LAMA2011", new pddl_planner::fast_downward::Planner("sas_plan", "seq-sat-lama-2011"))
                    ("FDAUTOTUNE2", new pddl_planner::fast_downward::Planner("sas_plan", "seq-sat-fd-autotune-2"))
                    ("FDAUTOTUNE1", new pddl_planner::fast_downward::Planner("sas_plan", "seq-sat-fd-autotune-1"));
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

std::list<std::string> Planning::plannersAvailable()
{
    std::list<std::string> result;
    PlannerMap::iterator it = mPlanners.begin();
    for(; it != mPlanners.end(); ++it)
    {
        std::string cmd = std::string("which ") + it->second->getCmd() + " > /dev/null";
        if(0 == system(cmd.c_str()))
        {
            result.push_back(it->first);
        }
    }
    return result;
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

PlanCandidates Planning::plan(const std::string& problem, double timeout, const std::string& planner_name)
{
    LOG_DEBUG_S << "Planning requested: " << std::endl
        << "-DOMAIN-" << std::endl << getDomainDescriptions()
        << "-PROBLEM-" << std::endl << problem;

    PDDLPlannerInterface* planner = getPlanner(planner_name);
    return planner->plan(problem, getActionDescriptions(), getDomainDescriptions(), timeout);
}

PlanCandidates Planning::plan(const representation::Problem& problem, double timeout, const std::string& planner_name)
{
    setDomainDescription(problem.domain.name, problem.domain.toLISP());
    return plan(problem.toLISP(), timeout, planner_name);
}

PlanCandidates Planning::plan(const representation::Domain& domain, const representation::Problem& problem, double timeout, const std::string& planner_name)
{
    setDomainDescription(domain.name, domain.toLISP());
    return plan(problem.toLISP(), timeout, planner_name);
}

}
