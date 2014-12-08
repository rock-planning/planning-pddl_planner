#include <iostream>
#include <stdio.h>
#include <map>
#include <errno.h>
#include <pddl_planner/Planning.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <cstring>
#include <cstdlib>
#include <list>
#include <unistd.h>

#define TIMEOUT 7.
#define INPUT_VERIFICATION

double timeout = TIMEOUT;
bool seq = false, liste = false;
std::list<std::string> planners;
std::string output = "";
pddl_planner::Planning planning;
std::list<boost::thread *> runners;
std::string problemDescription;
boost::mutex mutex; // protects string output

void usage(int argc, char** argv)
{
    printf("usage: %s [-p <planner-name>] [-t <timeout-seconds(float)>] <domain-description-file> <problem-file>\n    or\nusage: %s [-l <# of planners> <planner-name> <planner-name> ... ] [-t <timeout-seconds(float)>] [-s] <domain-description-file> <problem-file>\n\n    -s,  --sequential           run listed planners sequentially (no threads)\n\n", argv[0], argv[0]);
    std::list<std::string> availablePlanners = planning.plannersAvailable();
    std::list<std::string>::iterator it = availablePlanners.begin();
    std::string availablePlannersList = "";
    for(; it != availablePlanners.end(); ++it)
    {
        availablePlannersList += (*it) + " ";
    }
    printf("The following planners are available: %s\n", availablePlannersList.c_str());
}

void run_planner(const std::string & planner, double timeout)
{
    try
    {
        pddl_planner::PlanCandidates planCandidates = planning.plan(problemDescription, timeout, planner);
        boost::unique_lock<boost::mutex> scoped_lock(mutex);
        output += std::string("Individual Planner ") + planner + " found PlanCandidates:\n" + planCandidates.toString() + "\n";
    }
    catch(const std::runtime_error& e)
    {
        printf("Error: %s\n", e.what());
        if(!strncmp(e.what(),"pddl_planner::Planning: planner with name '", strlen("pddl_planner::Planning: planner with name '")))
        {
            printf("    Available planners:\n");
            pddl_planner::PlannerMap planners = planning.getPlanners();
            pddl_planner::PlannerMap::iterator it = planners.begin();
            for(; it != planners.end(); ++it)
            {
                printf("%s ", it->first.c_str());
            }
            printf("\n");
        }
    }
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

    if(firstArg == "-p" && (argc == 5 || 7 == argc))
    {
        if(5 == argc)
        {
            plannerName = argv[2];
            domainFilename = argv[3];
            problemFilename = argv[4];
        }
        else
        {
            if("-t" != std::string(argv[3]))
            {
                usage(argc, argv);
                exit(0);
            }
            plannerName = argv[2];
            timeout = atof(argv[4]);
            domainFilename = argv[5];
            problemFilename = argv[6];
        }
    } 
    else if("-t" == firstArg && 7 == argc)
    {
        if("-p" != std::string(argv[3]))
        {
            usage(argc, argv);
            exit(0);
        }
        plannerName = argv[4];
        timeout = atof(argv[2]);
        domainFilename = argv[5];
        problemFilename = argv[6];

    }
    else if(argc == 3)
    {
        domainFilename = argv[1];
        problemFilename = argv[2];
    } 
    else if("-l" == firstArg)
    {
        liste = true;
        if(argc < 3)
        {
            printf("Too few arguments were provided!\n");
            usage(argc, argv);
            exit(0);
        }
        int nplanners = atoi(argv[2]);
        if(nplanners < 1)
        {
            printf("In a list of planners, the number of planners has to be at least 1!\n");
            usage(argc, argv);
            exit(0);
        }
        if(argc < 5 + nplanners) // 3 + nplanners + 2
        {
            printf("Too few arguments were provided!\n");
            usage(argc, argv);
            exit(0);
        }
        for(int i = 0; i < nplanners; ++i)
        {
            planners.push_back(std::string(argv[3 + i]));
        }
        if("-t" == std::string(argv[3 + nplanners]))
        {
            if(argc < 7 + nplanners) // 3 + nplanners + 2 + 2
            {
                printf("Too few arguments were provided!\n");
                usage(argc, argv);
                exit(0);
            }
            timeout = atof(argv[4 + nplanners]);
            if("--sequential" == std::string(argv[5 + nplanners]) || "-s" == std::string(argv[5 + nplanners]))
            {
                if(argc < 8 + nplanners) // 3 + nplanners + 2 + 2 + 1
                {
                    printf("Too few arguments were provided!\n");
                    usage(argc, argv);
                    exit(0);
                }
                domainFilename  = argv[6 + nplanners];
                problemFilename = argv[7 + nplanners];
                seq = true;
            }
            else
            {
                domainFilename  = argv[5 + nplanners];
                problemFilename = argv[6 + nplanners];
            }
        }
        else if("--sequential" == std::string(argv[3 + nplanners]) || "-s" == std::string(argv[3 + nplanners]))
        {
            if(argc < 6 + nplanners) // 3 + nplanners + 2 + 1
            {
                printf("Too few arguments were provided!\n");
                usage(argc, argv);
                exit(0);
            }
            seq = true;
            if("-t" == std::string(argv[4 + nplanners]))
            {
                if(argc < 8 + nplanners) // 3 + nplanners + 2 + 1 + 2
                {
                    printf("Too few arguments were provided!\n");
                    usage(argc, argv);
                    exit(0);
                }
                timeout = atof(argv[5 + nplanners]);
                domainFilename  = argv[6 + nplanners];
                problemFilename = argv[7 + nplanners];
            }
            else
            {
                domainFilename  = argv[4 + nplanners];
                problemFilename = argv[5 + nplanners];
            }
        }
        else
        {
            domainFilename  = argv[3 + nplanners];
            problemFilename = argv[4 + nplanners];
        }
    }
    else
    {
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


    planning.setDomainDescription("test-domain", domainDescription);

    FILE* problemFile = fopen(problemFilename.c_str(), "r");
    if(!problemFile)
    {
        printf("Error opening file: '%s' -- %s", problemFilename.c_str(), strerror(errno));
        exit(-1);
    }

    while(fgets(buffer, 512, problemFile) != NULL)
    {
        problemDescription += std::string(buffer);
    }
    fclose(problemFile);

#ifdef INPUT_VERIFICATION
    printf("Input:\n    planner(s)Name  = ");
    for(std::list<std::string>::iterator it = planners.begin(); it != planners.end(); ++it)
    {
        printf("%s ", (*it).c_str());
    }
    printf("\n    domainFilename  = %s\n    problemFilename = %s\n    timeout         = %lf (sec)\n    list            = %s\n    sequential      = %s\n", domainFilename.c_str(), problemFilename.c_str(), timeout, liste ? "true" : "false", seq ? "true" : "false");
#endif

    if(liste)
    {
        if(seq)
        {
            long size;
            char *buf;
            char *dirname_ptr;
            size = pathconf(".", _PC_PATH_MAX);
            if ((buf = (char *)malloc((size_t)size)) == NULL)
            {
                fprintf(stderr, "Error allocating buffer memory.\n%s\n", strerror(errno));
                exit(1);
            }
            dirname_ptr = getcwd(buf, (size_t)size);
            std::list<std::string>::iterator it = planners.begin();
            for(; it != planners.end(); ++it)
            {
                int result = chdir(dirname_ptr); // avoiding getcwd() errors after cleanups
                if(-1 == result)
                {
                    fprintf(stderr, "Error: failed to change current working directory for planner %s.\n    %s\n", (*it).c_str(), strerror(errno));
                }
                try
                {
                    PlanCandidates planCandidates = planning.plan(problemDescription, timeout, (*it));
                    printf("\n****Planner %s output:****\n    PlanCandidates:\n%s\n", (*it).c_str(), planCandidates.toString().c_str());
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
                    }
                    printf("\nSkipping\n");
                }
            }
            if(dirname_ptr)free(dirname_ptr);
        }
        else
        {
            std::list<std::string>::iterator it = planners.begin();
            int count = 0;
            for(; it != planners.end(); ++it)
            {
                runners.push_back(new boost::thread(run_planner, (*it), timeout));
            }
            std::list<boost::thread *>::iterator itt = runners.begin();
            count = 0;
            for(; itt != runners.end(); ++itt)
            {
                (*itt)->join();
                delete (*itt);
            }
            printf("%s\n", output.c_str());
        }
    }
    else
    {
        try
        {
            PlanCandidates planCandidates = planning.plan(problemDescription, timeout, plannerName);
            printf("Individual Planner %s found PlanCandidates:\n%s\n", plannerName.c_str(), planCandidates.toString().c_str());
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
    }


    return 0;
}
