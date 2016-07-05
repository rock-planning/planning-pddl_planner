#include <pddl_planner/PDDLPlannerInterface.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <base-logging/Logging.hpp>
#include <base/Time.hpp>
#include <boost/thread.hpp>
#include <boost/chrono/chrono.hpp>
#include <signal.h>
#include <unistd.h>
#include <cstdlib>
#include <list>

namespace fs = boost::filesystem;

namespace pddl_planner
{

    const std::string PDDLPlannerInterface::msDomainFileBasename = "domain.pddl";
    const std::string PDDLPlannerInterface::msProblemFileBasename = "problem.pddl";
    const std::string PDDLPlannerInterface::msTempDirBasename = "/tmp";

    bool PDDLPlannerInterface::isAvailable() const
    {
        std::string cmd = std::string("which ") + getCmd() + " > /dev/null";
        return 0 == system(cmd.c_str());
    }

    void PDDLPlannerInterface::cleanup(const std::string& dir, const std::list<std::string> & files)
    {
        std::list<std::string>::const_iterator it = files.begin();
        for(; files.end() != it; ++it)
        {
            try {
                boost::filesystem::remove(boost::filesystem::path(*(it)));
            } catch(const boost::filesystem::filesystem_error& e)
            {
                LOG_WARN("%s",e.what());
            }
        }

        boost::filesystem::path path(dir);
        boost::filesystem::remove_all(path);
    }

    void run_planner(const std::string& cmd, const std::string& planner, double timeout)
    {
        std::string command = cmd + " > /dev/null";
        int result = system(command.c_str());
        if(-1 == result)
        {
            std::string msg = "Planner " + planner + " returned an error during execution";
            LOG_ERROR("%s",msg.c_str());
            throw PlanGenerationException(msg);
        }

        if(result)
        {
             LOG_WARN("Planner %s returned non-zero exit status", planner.c_str());
        }
    }

    void PDDLPlannerInterface::prepare(const std::string& problem, const std::string& actionDescriptions, const std::string& domainDescriptions)
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

    PlanCandidates PDDLPlannerInterface::generateCandidates(const std::string& cmd, const std::string& tempDir, const std::string& resultFilename, const std::list<std::string> & patternList, double timeout, const std::string& planner)
    {
        // Make sure that directory is changed in subshell, to allow parallel
        // execution of planner without interfering
        std::string command = "cd " + tempDir + ";" + cmd;
        LOG_DEBUG("Run planner '%s' with command: '%s' and timeout in s: %.2f", planner.c_str(), command.c_str(), timeout);
        boost::thread run_planner_thread(run_planner, command, planner, timeout + 0.001);
        bool result = run_planner_thread.try_join_for(boost::chrono::milliseconds((int)(1000. * timeout)));
        if(!result)
        {
            LOG_WARN("Planner %s timed out: killing it...", planner.c_str());
            std::string command = "pkill --signal 9 -f \"" + cmd + "\"";
            int return_code = system(command.c_str());
            bool error = false;
            if(patternList.empty())
            {
                LOG_WARN("Planner %s does not provide any patterns in its individual implementation", planner.c_str());
            }
            else
            {
                for(std::list<std::string>::const_iterator it = patternList.begin(); !error && it != patternList.end(); ++it)
                {
                    std::string command_list = "pkill --signal 9 -f " + (*it); 
                    if(-1 == system(command_list.c_str()))
                    {
                        error = true;
                    }
                }
            }
            if(-1 == return_code || error)
            {
                std::string msg = "Error: planner " + planner + " could not be killed";
                LOG_ERROR("%s",msg.c_str());
                throw PlanGenerationException(msg);
            }
            LOG_WARN("Planner %s has been successfully killed", planner.c_str());
        }

        PlanCandidates planCandidates;
        fs::path directory(tempDir);

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
            if( boost::algorithm::find_first(file, resultFilename))
            {
                LOG_DEBUG("Found result file: %s", file.c_str());
                try {
                    Plan plan = readPlan(getName(), file);
                    planCandidates.addPlan(plan);
                } catch(const PlanGenerationException& e)
                {
                    LOG_WARN("Error reading plan: %s", e.what());
                }
            }
        }
        return planCandidates;
    }

    Plan PDDLPlannerInterface::readPlan(const std::string& plannerName, const std::string& filename)
    {
        Plan plan;
        FILE* resultFile = fopen(filename.c_str(), "r");
        if(!resultFile)
        {
            char buffer[512];
            snprintf(buffer, 512, "%s: could not open '%s'", plannerName.c_str(), filename.c_str());
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
