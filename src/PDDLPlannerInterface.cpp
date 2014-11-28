#include <pddl_planner/PDDLPlannerInterface.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <base/logging.h>
#include <base/time.h>

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