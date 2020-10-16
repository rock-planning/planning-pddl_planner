#include "Planning.hpp"
#include "PDDLPlannerInterface.hpp"
#include "planners/Lama.hpp"
#include "planners/Bfsf.hpp"
#include "planners/Uniform.hpp"
#include "planners/Randward.hpp"
#include "planners/Cedalion.hpp"
#include "planners/ArvandHerd.hpp"
#include "planners/FastDownward.hpp"

#include <boost/assign/list_of.hpp>
#include <boost/assign.hpp>
#include <base-logging/Logging.hpp>

namespace pddl_planner
{

size_t Planning::DEFAULT_TIMEOUT_IN_S = 3600;

Planning::Planning()
{
    mPlanners =
         {
                    {"LAMA", dynamic_cast<pddl_planner::PDDLPlannerInterface*>(new pddl_planner::lama::Planner())},
                    {"BFSF", new pddl_planner::bfsf::Planner()},
                    {"UNIFORM", new pddl_planner::uniform::Planner()},
                    {"CEDALION", new pddl_planner::cedalion::Planner()},
                    {"RANDWARD", new pddl_planner::randward::Planner()},
                    {"ARVANDHERD", new pddl_planner::arvandherd::Planner()},
                    {"FDSS1", new pddl_planner::fast_downward::Planner("sas_plan", "seq-sat-fdss-1")},
                    {"FDSS2", new pddl_planner::fast_downward::Planner("sas_plan", "seq-sat-fdss-2")},
                    {"LAMA2011", new pddl_planner::fast_downward::Planner("sas_plan", "seq-sat-lama-2011")},
                    {"FDAUTOTUNE2", new pddl_planner::fast_downward::Planner("sas_plan", "seq-sat-fd-autotune-2")},
                    {"FDAUTOTUNE1", new pddl_planner::fast_downward::Planner("sas_plan", "seq-sat-fd-autotune-1")}
        };
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

std::set<std::string> Planning::getAvailablePlanners() const
{
    std::set<std::string> result;
    PlannerMap::const_iterator it = mPlanners.begin();
    for(; it != mPlanners.end(); ++it)
    {
        PDDLPlannerInterface* planner = it->second;
        if(planner->isAvailable())
        {
            result.insert(it->first);
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

void run_planner(const std::string & plannerName, const std::string& problem, const std::string& actionDescriptions, const std::string& domainDescriptions, Planning * planning, double timeout)
{
    PDDLPlannerInterface* planner;
    {
        boost::unique_lock<boost::mutex> scoped_lock(planning->mPlannerMutex);
        planner = planning->getPlanner(plannerName);
    }
    pddl_planner::PlanCandidates planCandidates = planner->plan(problem, actionDescriptions, domainDescriptions, timeout);
    boost::unique_lock<boost::mutex> scoped_lock(planning->mResultMutex);
    planning->mPlanResultList.push_back(std::pair<PlannerName, PlanCandidates> (plannerName, planCandidates));
}

PlanResultList Planning::plan(const std::string& problem, const std::set<std::string>& planners, bool sequential, double timeout)
{
    std::string actionDescriptions = getActionDescriptions();
    std::string domainDescriptions = getDomainDescriptions();
    LOG_DEBUG_S << (sequential ? "Sequential " : "") << "Planning requested: " << std::endl
        << "-DOMAIN-" << std::endl << domainDescriptions
        << "-PROBLEM-" << std::endl << problem;

    if(sequential)
    {
        long size;
        char* buf;
        char* dirname_ptr;
        size = pathconf(".", _PC_PATH_MAX);
        if ((buf = (char *)malloc((size_t)size)) == NULL)
        {
            fprintf(stderr, "Error allocating buffer memory.\n%s\n", strerror(errno));
            exit(1);
        }
        dirname_ptr = getcwd(buf, (size_t)size);

        for(std::set<std::string>::const_iterator it = planners.begin(); planners.end() != it; ++it)
        {
            std::string planner_name = (*it);
            PDDLPlannerInterface* planner = getPlanner(planner_name);

            PlanCandidates planCandidates = planner->plan(problem, actionDescriptions, domainDescriptions, timeout);

            std::pair<PlannerName, PlanCandidates> solution(planner_name, planCandidates);
            mPlanResultList.push_back(solution);
        }

        if(dirname_ptr)
        {
            free(dirname_ptr);
        }
    }
    else
    {
        std::set<std::string>::const_iterator it = planners.begin();
        for(; it != planners.end(); ++it)
        {
            mPlanRunners.push_back(new boost::thread(run_planner, (*it), problem, actionDescriptions, domainDescriptions, this, timeout));
        }
        std::vector<boost::thread *>::iterator itt = mPlanRunners.begin();
        for(; itt != mPlanRunners.end(); ++itt)
        {
            (*itt)->join();
            delete (*itt);
        }
    }
    return mPlanResultList;
}

PlanCandidates Planning::plan(const std::string& problem, const std::string& plannerName, double timeout)
{
    LOG_DEBUG_S << "Planning requested: " << std::endl
        << "-DOMAIN-" << std::endl << getDomainDescriptions()
        << "-PROBLEM-" << std::endl << problem;
    PDDLPlannerInterface* planner = getPlanner(plannerName);
    return planner->plan(problem, getActionDescriptions(), getDomainDescriptions(), timeout);
}


PlanResultList Planning::plan(const representation::Problem& problem, const std::set<std::string>& planners, bool sequential, double timeout)
{
    setDomainDescription(problem.domain.name, problem.domain.toLISP());
    return plan(problem.toLISP(), planners, sequential, timeout);
}

PlanResultList Planning::plan(const representation::Domain& domain, const representation::Problem& problem, const std::set<std::string>& planners, bool sequential, double timeout)
{
    setDomainDescription(domain.name, domain.toLISP());
    return plan(problem.toLISP(), planners, sequential, timeout);
}


PlanCandidates Planning::plan(const representation::Problem& problem, const std::string& plannerName, double timeout)
{
    setDomainDescription(problem.domain.name, problem.domain.toLISP());
    return plan(problem.toLISP(), plannerName, timeout);
}

PlanCandidates Planning::plan(const representation::Domain& domain, const representation::Problem& problem, const std::string& plannerName, double timeout)
{
    setDomainDescription(domain.name, domain.toLISP());
    return plan(problem.toLISP(), plannerName, timeout);
}

}
