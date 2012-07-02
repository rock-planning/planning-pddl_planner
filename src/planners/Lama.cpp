#include <pddl_planner/planners/Lama.hpp>
#include <iostream>
#include <fstream>
#include <string.h>
#include <boost/filesystem.hpp>
#include <base/logging.h>
#include <base/time.h>

namespace fs = boost::filesystem;

namespace pddl_planner
{
namespace lama
{

Plan Planner::plan(const std::string& problem, const std::string& actionDescriptions, const std::string& domainDescriptions)
{
    LOG_DEBUG("Planner called with problem: '%s'");

    int result = system("which lama-planner");
    if(result != 0)
    {
        std::string msg = "Could not find 'lama-planner' binary";
        LOG_ERROR("%s",msg.c_str());
        throw PlanGenerationException(msg);
    }

    std::string currentTime = base::Time::now().toString();
    fs::path path("/tmp/" + currentTime + "_lama");

    if(!fs::exists(path))
    {
        if (!fs::create_directory(path))
        {
            LOG_ERROR("Could not create directory: %s", path.string().c_str());
        }
    }
    mTempDir = path.string();

    prepare(problem, actionDescriptions, domainDescriptions);
    Plan plan;
    if(generatePlan(plan))
    {
        return plan;
    } else {
        throw PlanGenerationException("Planning with 'lama' failed'");
    }
}

void Planner::prepare(const std::string& problem, const std::string& actionDescriptions, const std::string& domainDescriptions)
{
    mDomainFilename = mTempDir + "/domain.pddl";
    std::ofstream out(mDomainFilename.c_str());

    out << domainDescriptions; 
    out << "\n";
    out << actionDescriptions;

    out.close();

    mProblemFilename = mTempDir + "/problem.pddl";
    std::ofstream problemOut(mProblemFilename.c_str());
    LOG_DEBUG("Prepare problem '%s'", problem.c_str());
    problemOut << problem;
    problemOut << "\n";
    problemOut.close();

    mResultFilename = mTempDir + "/result";
}

void Planner::cleanup()
{
    fs::path path(mTempDir);
    fs::remove_all(path);
    fs::remove(fs::path("output"));
    fs::remove(fs::path("output.sas"));
    fs::remove(fs::path("all.groups"));
    fs::remove(fs::path("test.groups"));
}

bool Planner::generatePlan(Plan& plan)
{
    std::string cmd = "lama-planner " + mDomainFilename + " " + mProblemFilename + " " + mResultFilename;
    int result = system(cmd.c_str());
    if(result == 0)
    {
        std::string filename = mResultFilename + ".1";
        FILE* resultFile = fopen(filename.c_str(), "r");
        if(!resultFile)
        {
            LOG_ERROR("Could not open '%s'", filename.c_str());
            return false;
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

        return true;
    }

    return false;
}

}

}
