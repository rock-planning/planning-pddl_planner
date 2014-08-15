#include <pddl_planner/planners/AbstractPlanner.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <boost/filesystem.hpp>
#include <base/logging.h>
#include <base/time.h>
#include <boost/algorithm/string.hpp>

namespace fs = boost::filesystem;

namespace pddl_planner
{

const std::string AbstractPlanner::msDomainFileBasename = "domain.pddl";
const std::string AbstractPlanner::msProblemFileBasename = "problem.pddl";
const std::string AbstractPlanner::msResultFileBasename = "plan";
const std::string AbstractPlanner::msTempDirBasename = "/tmp";


PlanCandidates AbstractPlanner::plan(const std::string& problem, const std::string& actionDescriptions, const std::string& domainDescriptions)
{
    LOG_DEBUG("Planner called with problem: '%s'", problem.c_str());
    
    std::string lowerCaseName = boost::to_lower_copy(getName());
    
    std::string syscallStr = "which " + lowerCaseName + "-planner";
    int result = system(syscallStr.c_str());
    if(result != 0)
    {
        std::string msg = "Could not find '" + lowerCaseName + "-planner' binary";
        LOG_ERROR("%s",msg.c_str());
        throw PlanGenerationException(msg);
    }

    std::string currentTime = base::Time::now().toString();
    fs::path path(msTempDirBasename + "/" + currentTime + "_" + lowerCaseName);

    if(!fs::exists(path))
    {
        if (!fs::create_directory(path))
        {
            LOG_ERROR("Could not create directory: %s", path.string().c_str());
        }
    }
    mTempDir = path.string();

    prepare(problem, actionDescriptions, domainDescriptions);
    PlanCandidates planCandidates = generatePlanCandidates();
    return planCandidates;
}

void AbstractPlanner::prepare(const std::string& problem, const std::string& actionDescriptions, const std::string& domainDescriptions)
{
    mDomainFilename = mTempDir + "/" + msDomainFileBasename;
    std::ofstream out(mDomainFilename.c_str());

    out << domainDescriptions; 
    out << "\n";
    out << actionDescriptions;

    out.close();

    mProblemFilename = mTempDir + "/" + msProblemFileBasename;
    std::ofstream problemOut(mProblemFilename.c_str());
    LOG_DEBUG("Prepare problem '%s'", problem.c_str());
    problemOut << problem;
    problemOut << "\n";
    problemOut.close();

    mResultFilename = mTempDir + "/" + msResultFileBasename;
}

PlanCandidates AbstractPlanner::generatePlanCandidates()
{
    std::string lowerCaseName = boost::to_lower_copy(getName());
    
    std::string cmd = lowerCaseName + "-planner " + mDomainFilename + " " + mProblemFilename + " " + mResultFilename;
    int result = system(cmd.c_str());
    PlanCandidates planCandidates;
    if(result == 0)
    {
        fs::path directory(mTempDir);

        if(!fs::is_directory(directory))
        {
            std::stringstream ss;
            ss << "Temporary directory: '" << directory.string() << "' does not exist";
            LOG_ERROR("%s", ss.str().c_str());
            throw PlanGenerationException(ss.str());
        }

        fs::directory_iterator dirIt(directory);
        for(; dirIt != fs::directory_iterator(); dirIt++)
        {
            std::string file = dirIt->path().string();
            if( boost::algorithm::find_first(file, mResultFilename))
            {
                LOG_DEBUG("Found result file: %s", file.c_str());
                try {
                    Plan plan = readPlan(file);
                    planCandidates.addPlan(plan);
                } catch(const PlanGenerationException& e)
                {
                    LOG_WARN("Error reading plan: %s", e.what());
                }
            }
        }
    }

    cleanup();
    return planCandidates;
}

Plan AbstractPlanner::readPlan(const std::string& filename)
{
    Plan plan;
    FILE* resultFile = fopen(filename.c_str(), "r");
    if(!resultFile)
    {
        char buffer[512];
        snprintf(buffer, 512, "AbstractPlanner: could not open '%s'", filename.c_str());
        LOG_ERROR("%s", buffer);
        throw PlanGenerationException(buffer);
    }
   
    size_t bufferSize = 2048;
    char buffer[bufferSize];
    while( NULL != fgets(buffer, bufferSize, resultFile) )
    {
        std::string readline(buffer);
        // Result file contains
        // (action <arg1> <arg2> ... <argN>) 
        size_t pos = readline.find_first_of('(');
        size_t endpos = readline.find_last_of(')');
        readline = readline.substr(pos + 1, endpos-1);
        
        // Split on whitespace
        pos = readline.find_first_of(" ");
        Action action; 
        if(pos == std::string::npos)
        {
            // There is an action without arguments
            action.name = std::string(readline);
            plan.addAction(action);
            continue;
        } else {
            action.name = readline.substr(0,pos);
        }

        while(true)
        {
            readline = readline.substr(pos+1);
            pos = readline.find_first_of(" ");
            if(pos == std::string::npos)
            {
                std::string argument(readline);
                action.addArgument( argument );
                break;
            } else {
                std::string argument = readline.substr(0,pos);
                action.addArgument( argument );
            }
        }
        plan.addAction(action);
    }

    return plan;
}

}
