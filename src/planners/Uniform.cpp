#include "Uniform.hpp"
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
namespace uniform
{

Planner::Planner(const std::string& resultFileBasename)
{
    msResultFileBasename = resultFileBasename;
}

PlanCandidates Planner::plan(const std::string& problem, const std::string& actionDescriptions, const std::string& domainDescriptions, double timeout)
{
    LOG_DEBUG("Planner called with problem: '%s'", problem.c_str());
    int result = system("which uniform-planner");
    if(result != 0)
    {
        std::string msg = "Could not find 'uniform-planner' script";
        LOG_ERROR("%s",msg.c_str());
        throw PlanGenerationException(msg);
    }

    std::string currentTime = base::Time::now().toString();
    fs::path path(msTempDirBasename + "/" + currentTime + "_uniform");

    if(!fs::exists(path))
    {
        if (!fs::create_directory(path))
        {
            LOG_ERROR("Could not create directory: %s", path.string().c_str());
        }
    }
    mTempDir = path.string();
    mTimeoutInS = timeout;
    prepare(problem, actionDescriptions, domainDescriptions);
    PlanCandidates planCandidates = generatePlanCandidates();
    return planCandidates;
}

PlanCandidates Planner::generatePlanCandidates()
{
    std::string cmd = "uniform-planner " + mDomainFilename + " " + mProblemFilename + " ipc seq-sat-uniform " + "--plan-file " + mResultFilename;

    std::list<std::string> pattern;
    pattern.push_back("uniform");
    PlanCandidates planCandidates = generateCandidates(cmd, mTempDir, mResultFilename, pattern, mTimeoutInS, getName());

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
