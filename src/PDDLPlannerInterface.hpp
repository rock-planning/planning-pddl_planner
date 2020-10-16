#ifndef PDDL_PLANNER_INTERFACE_H
#define PDDL_PLANNER_INTERFACE_H

#include "PDDLPlannerTypes.hpp"
#include <list>

namespace pddl_planner
{
    class PDDLPlannerInterface
    {
    public:
        virtual ~PDDLPlannerInterface() {}

        /**
         * Check if the given planner is available, i.e. standard implementation
         * uses checks availability via the command returned by getCmd()
         * \return true if planner is callable, false otherwise
         */
        virtual bool isAvailable() const;

        /**
         * removes listed files and additionally completely removes the provided directory
         * \param dir name of dir to be completely removed
         * \param files the list of file names to be removed
         */
        void cleanup(const std::string & dir, const std::list<std::string> & files);

        /**
         * Generate the plan candidates for the given problem
         * There is no priority in the order of candidates
         * \throws PlanGenerationException
         */
        PlanCandidates generateCandidates(const std::string & cmd, const std::string & tempDir, const std::string & resultFilename, const std::list<std::string> & patternList, double timeoutInS, const std::string & planner = "");


        /**
         * Read a plani
         * Note, that an empty plan is a valid plan
         * \throws PlanGenerationException
         */
        Plan readPlan(const std::string& plannerName, const std::string& filename);

        /**
         * Get name of this planner implementation
         * \return Name of planner
         * \throws PlanGenerationException if not implemented
         */
        virtual std::string getName() const { throw PlanGenerationException("PDDLPlannerInterface getName method not implemented"); }

        /**
         * Get name of this planner's execution script/runnable
         * \return Name of planner's main execution script/runnable
         * \throws PlanGenerationException if not implemented
         */
        virtual std::string getCmd() const { throw PlanGenerationException("PDDLPlannerInterface getCmd method not implemented"); }

        /**
         * Get version of this planner implementation
         * \return version as int
         * \throws PlanGenerationException if not implemented
         */
        virtual int getVersion() const { throw PlanGenerationException("PDDLPlannerInterface getVersion method not implemented"); }

        /**
         * Planning method that forwards problem, actions and domain to the underlying planning instance
         * \return Solutions candidates
         * \throws PlanGenerationException if not implemented
         */
        virtual PlanCandidates plan(const std::string& problem, const std::string& actions, const std::string& domain, double timeoutInS) { throw PlanGenerationException("Plan method not implemented"); }

    protected:
        /**
         *
         */
        void prepare(const std::string& problem, const std::string& actionDescriptions, const std::string& domainDescriptions);

        double      mTimeoutInS;
        std::string mTempDir;
        std::string mDomainFilename;
        std::string mProblemFilename;
        std::string mResultFilename;

        std::string msResultFileBasename;
        const static std::string msProblemFileBasename;
        const static std::string msDomainFileBasename;
        const static std::string msTempDirBasename;

        Plan mPlan;
    };

}
#endif // PDDL_PLANNER_INTERFACE_H
