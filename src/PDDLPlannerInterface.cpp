#include <pddl_planner/PDDLPlannerInterface.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <base/logging.h>
#include <base/time.h>
#include <boost/thread.hpp>
#include <boost/chrono/chrono.hpp>
#include <signal.h>
#include <unistd.h>
#include <cstdlib>

namespace fs = boost::filesystem;

namespace pddl_planner
{
    void PDDLPlannerInterface::cleanup(const std::string & dir, const std::list<std::string> & files)
    {
        boost::filesystem::path path(dir);
        boost::filesystem::remove_all(path);
        
        std::list<std::string>::const_iterator it = files.begin();
        for(; files.end() != it; ++it)
        {
            boost::filesystem::remove(boost::filesystem::path(*(it)));
        }
    }
        
    void run_planner(const std::string & cmd, const std::string & planner, double timeout)
    {
        int result = system(cmd.c_str());
        if(-1 == result)
        {
            std::string msg = "Error: planner " + planner + " returned an error during execution";
            LOG_ERROR("%s",msg.c_str());
            throw PlanGenerationException(msg);
        }
        if(result)
        {
             LOG_WARN("Planner %s returned non-zero exit status", planner.c_str());
        }
    }
    
    std::string pattern(const std::string & planner)
    {
        if(!planner.compare("LAMA2011"))
        {
            return std::string("f");
        }
        if(!planner.compare("CEDALION"))
        {
            return std::string("fd_cedalion");
        }
        if(!planner.compare("UNIFORM"))
        {
            return std::string("fd_uniform");
        }
        std::string result = "";
        if(planner[0] >= 'a')
        {
            result.push_back(planner[0]);
        }
        else
        {
            result.push_back(planner[0] + 32);
        }
        return result;
    }
    
    PlanCandidates PDDLPlannerInterface::generateCandidates(const std::string & cmd, const std::string & tempDir, const std::string & resultFilename, const std::string & planner, double timeout)
    {
        boost::thread run_planner_thread(run_planner, cmd, planner, timeout + 0.001);
        bool result = run_planner_thread.try_join_for(boost::chrono::milliseconds((int)(1000. * timeout)));
        if(!result)
        {
            LOG_WARN("Planner %s timed out: killing it...", planner.c_str());
            std::string command = "pkill --signal 9  -f /planning/" + pattern(planner);
            int return_code = system(command.c_str());
            if(-1 == return_code)
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