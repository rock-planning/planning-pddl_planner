#include <iostream>
#include <stdio.h>
#include <map>
#include <errno.h>
#include <pddl_planner/Planning.hpp>
#include <cstring>

void usage(int argc, char** argv)
{
    printf("usage: %s [-p <planner-name>] <domain-description-file> <problem-file>\n",argv[0]);
}

int main(int argc, char** argv)
{
    using namespace pddl_planner;
    
    // defaultPlanner is LAMA
    if(argc < 2 )
    {
        usage(argc, argv);
        exit(0);
    }

    std::string firstArg = argv[1];
    if(firstArg == "-h" || firstArg == "--help")
    {
        usage(argc, argv);
        exit(0);
    }

    std::string plannerName = "LAMA";
    std::string domainFilename;
    std::string problemFilename; 

    if(firstArg == "-p" && argc == 5)
    {
        plannerName = argv[2];
        domainFilename = argv[3];
        problemFilename = argv[4];
    } else if(argc == 3) {
        domainFilename = argv[1];
        problemFilename = argv[2];
    } else {
        usage(argc, argv);
        exit(0);
    }

    FILE* domainFile = fopen(domainFilename.c_str(),"r"); 
    if(!domainFile)
    {
        printf("Error opening file: '%s' -- %s", domainFilename.c_str(), strerror(errno));
        exit(-1);
    }

    char buffer[512];
    std::string domainDescription;

    while(fgets(buffer,512,domainFile) != NULL)
    {
        domainDescription += std::string(buffer);
    }
    fclose(domainFile);

    Planning planning;
    planning.setDomainDescription("test-domain", domainDescription);

    FILE* problemFile = fopen(problemFilename.c_str(), "r");
    if(!problemFile)
    {
        printf("Error opening file: '%s' -- %s", problemFilename.c_str(), strerror(errno));
        exit(-1);
    }

    std::string problemDescription; // = domainDescription; // maybe?!
    while(fgets(buffer, 512, problemFile) != NULL)
    {
        problemDescription += std::string(buffer);
    }
    fclose(problemFile);

    try 
    {
        PlanCandidates planCandidates = planning.plan(problemDescription, plannerName);
        printf("PlanCandidates:\n%s\n", planCandidates.toString().c_str());
    } 
    catch(const std::runtime_error& e)
    {
        printf("Error: %s\n", e.what());
        if(!strncmp(e.what(),"pddl_planner::Planning: planner with name '", strlen("pddl_planner::Planning: planner with name '")))
        {
            printf("    Available planners:\n");
            PlannerMap planners = planning.getPlanners();
            PlannerMap::iterator it = planners.begin();
            for(; it != planners.end(); ++it)
            {
                printf("%s ", it->first.c_str());
            }
            printf("\n");
        }
    }

    return 0;
}
