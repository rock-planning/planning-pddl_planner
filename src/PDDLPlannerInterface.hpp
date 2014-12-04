#ifndef PDDL_PLANNER_INTERFACE_H
#define PDDL_PLANNER_INTERFACE_H

#include <pddl_planner/PDDLPlannerTypes.hpp>
#include <list>
#define TIMEOUT 7.

namespace pddl_planner
{
    class PDDLPlannerInterface
    {
    public:
        virtual ~PDDLPlannerInterface() {}

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
        PlanCandidates generateCandidates(const std::string & cmd, const std::string & tempDir, const std::string & resultFilename, const std::string & planner = "", double timeout = TIMEOUT);


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
        virtual PlanCandidates plan(const std::string& problem, const std::string& actions, const std::string& domain, double timeout = TIMEOUT) { throw PlanGenerationException("Plan method not implemented"); }
    };

}
#endif // PDDL_PLANNER_INTERFACE_H
