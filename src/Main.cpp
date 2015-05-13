/**
 *
 *      Main test script for the pddl_planner component
 *
 * usage:
 *
 *  ./pddl_planner [-p <planner-name>] [-t <timeout-seconds(float)>]
 *  <domain-description-file> <problem-file>
 *                                      OR: --
 * usage:
 *
 *  ./pddl_planner [-l <# of planners> <planner-name> <planner-name> ... ] [-t
 *  <timeout-seconds(float)>] [-s] <domain-description-file> <problem-file>
 *  [-s, --sequential]
 *
 * Parallel execution of planners is default, but use -s option to run listed
 * planners sequentially, i.e. not using threads.
 *
 * Rationale for using threads:
 *  - threads created in the main function have access to the common global
 *  string variable 'output'; - it is more convenient to carefully append
 *  (!concurrency issues!) each individual planner execution result to the
 *  'output', rather than redirecting the standard output of the planners to a
 *  common location, later on reading it again - functionality to read the plan
 *  files is already implemented at the planners level and their common
 *  interface (PDDLPlannerInterface), the level where the filenames and
 *  locations of the result plans are known.  -----
 *
 *  - synchronizing the waiting for results is simpler and more reliable when
 *  using the threads API (i.e. simply joining the individual threads) rather
 *  than sending signals around from forked child processes (the ones dealing
 *  with individual planners) back to the main process (their common parent)
 *
 *  - threads are much more time and memory efficient (they share the same
 *  memory, switching between threads is much faster for the scheduler than
 *  switching between processes and so)
 *
 */
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <map>
#include <set>
#include <errno.h>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <pddl_planner/Planning.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

void usage(int argc, char** argv, const pddl_planner::Planning& planning)
{
    printf("usage: %s [-p <planner-name>] [-t <timeout-seconds(float)>] <domain-description-file> <problem-file>\n",argv[0]);
    printf("or\n");
    printf("usage: %s [-l <# of planners> <planner-name> <planner-name> ... ] [-t <timeout-seconds(float)>]\n", argv[0]);
    printf("          [-s] <domain-description-file> <problem-file>\n");
    printf("DESCRIPTION OF OPTIONS\n");
    printf("      -s,--sequential    run listed planners sequentially (no threads)\n");
    std::set<std::string> availablePlanners = planning.getAvailablePlanners();
    std::set<std::string>::iterator it = availablePlanners.begin();
}

std::string readFile(const std::string& filename)
{
    std::string content;
    std::ifstream file(filename);
    if(file.is_open())
    {
        std::string line;
        while( getline(file, line))
        {
            content += line + "\n";
        }
        file.close();
        return content;
    }
    throw std::invalid_argument("Error opening file: '" + filename);
}

int main(int argc, char** argv)
{
    using namespace pddl_planner;
    pddl_planner::Planning planning;

    double timeout;
    bool sequential = false;
    std::string domainFilename;
    std::string problemFilename;


    namespace po = boost::program_options;
    std::set<std::string> availablePlanners = planning.getAvailablePlanners();
    std::string selectedPlanners;
    po::options_description desc("All options");
    desc.add_options()
        ("help,h", "produce help message")
        ("planners,p", po::value<std::string>(&selectedPlanners), "planner(s) that should be used for planning, use comma separated list of planners")
        ("timeout,t", po::value<double>(&timeout)->default_value(7.), "maximum time in seconds a planner is allowed to run, default is 7 s")
        ("sequential,s", po::value<bool>(&sequential)->default_value(false), "Planners should be run sequentially, i.e. without thread usage")
        ;
    po::options_description visibleOptions;

    desc.add_options()
        ("domain-file", po::value<std::string>(&domainFilename), "Filename containing the domain definition")
        ("problem-file", po::value<std::string>(&problemFilename), "Filename containing the problem definition")
        ;

    po::positional_options_description positionalOptions;
    positionalOptions.add("domain-file", 1); 
    positionalOptions.add("problem-file",1);

    po::variables_map vm;
    po::parsed_options parsedOptions = po::command_line_parser(argc, argv).
            options(desc)
                .positional(positionalOptions)
                .allow_unregistered()
                .run();

    std::vector<std::string> unrecognizedOptions = po::collect_unrecognized(parsedOptions.options,
            po::exclude_positional);

    if(!unrecognizedOptions.empty())
    {
        std::cout << "Unrecognized options" << std::endl;
        std::vector<std::string>::const_iterator cit = unrecognizedOptions.begin();
        for(; cit != unrecognizedOptions.end(); ++cit)
        {
            std::cout << "    " << *cit << std::endl;
        }
        std::cout << std::endl;
        std::cout << desc << std::endl;
    }

    po::store(parsedOptions, vm);
    po::notify(vm);

    if(vm.count("help") || !(vm.count("domain-file") && vm.count("problem-file")))
    {
        std::cout << "usage: " << argv[0] << " <OPTIONS> <domain-description-file> <problem-file>" << std::endl;
        std::cout << desc << std::endl;
        std::cout << "Available Planners:" << std::endl;
        std::set<std::string>::const_iterator cit = availablePlanners.begin();
        for(; cit != availablePlanners.end(); ++cit)
        {
            std::cout << "    " << *cit << std::endl;
        }
        return 0;
    }

    std::set<std::string> plannerSet;
    if(vm.count("planners"))
    {
        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        boost::char_separator<char> sep(",;");
        tokenizer t(selectedPlanners, sep);

        tokenizer::iterator it = t.begin();
        for(; it != t.end(); ++it)
        {
            std::string plannerName = *it;
            boost::to_upper(plannerName);
            plannerSet.insert(*it);
        }
    } else {
        plannerSet = availablePlanners;
    }

    try
    {
        std::string domainDescription = readFile(domainFilename);
        planning.setDomainDescription("planning-domain", domainDescription);

        std::string problemDescription = readFile(problemFilename);

        PlanResultList planResultList = planning.plan(problemDescription, plannerSet, sequential, timeout);
        PlanResultList::iterator it = planResultList.begin();
        for(; planResultList.end() != it; ++it)
        {
            PlanResult plan = (*it);
            printf("Planner %s:\n%s\n", plan.first.c_str(), plan.second.toString().c_str());
        }
    } catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
        return -1;
    } catch(const std::runtime_error& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
        std::string errorPrefix = "pddl_planner::Planning: planner with name '";
        if(!strncmp(e.what(), errorPrefix.c_str(), errorPrefix.size()))
        {
            std::cout << "    Registered planners:" << std::endl;

            PlannerMap planners = planning.getPlanners();
            PlannerMap::iterator it = planners.begin();
            for(; it != planners.end(); ++it)
            {
                printf("%s ", it->first.c_str());
            }
            std::cout << "For a list of available planners (out of the registered ones) please \
                use option \"--help\"" << std::endl;
        }
    }
    return 0;
}
