#include <pddl_planner/planners/Cedalion.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <boost/filesystem.hpp>
#include <base-logging/Logging.hpp>
#include <base/Time.hpp>
#include <list>

namespace fs = boost::filesystem;

namespace pddl_planner
{
namespace cedalion
{

Planner::Planner(const std::string& resultFileBasename)
{
    msResultFileBasename = resultFileBasename;
}

PlanCandidates Planner::plan(const std::string& problem, const std::string& actionDescriptions, const std::string& domainDescriptions, double timeout)
{
    LOG_DEBUG("Planner called with problem: '%s'", problem.c_str());
    int result = system("which cedalion-planner");
    if(result != 0)
    {
        std::string msg = "Could not find 'cedalion-planner' script";
        LOG_ERROR("%s",msg.c_str());
        throw PlanGenerationException(msg);
    }

    std::string currentTime = base::Time::now().toString();
    fs::path path(msTempDirBasename + "/" + currentTime + "_cedalion");

    if(!fs::exists(path))
    {
        if (!fs::create_directory(path))
        {
            LOG_ERROR("Could not create directory: %s", path.string().c_str());
        }
    }
    mTempDir = path.string();
    mTimeout = timeout;
    prepare(problem, actionDescriptions, domainDescriptions);
    PlanCandidates planCandidates = generatePlanCandidates();
    return planCandidates;
}

PlanCandidates Planner::generatePlanCandidates()
{
    std::string cmd = "cedalion-planner " + mDomainFilename + " " + mProblemFilename + " ipc seq-sat-cedalion " + "--plan-file " + mResultFilename;
    

    std::list<std::string> pattern;
    pattern.push_back("cedalion");
    PlanCandidates planCandidates = generateCandidates(cmd, mTempDir, mResultFilename, pattern, mTimeout, getName());
        
    std::list<std::string> files;
    files.push_back(std::string("output"));
    files.push_back(std::string("output.sas"));
    files.push_back(std::string("plan_numbers_and_cost"));
    files.push_back(std::string("elapsed.time"));
    
    cleanup(mTempDir, files);
    return planCandidates;
}

}

}
