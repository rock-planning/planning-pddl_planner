#ifndef PDDL_PLANNER_CEDALION_HPP
#define PDDL_PLANNER_CEDALION_HPP

#include "../PDDLPlannerInterface.hpp"

namespace pddl_planner
{
namespace cedalion
{
    /**
     * Implement the interface to the CEDALION planner
     *
     */
    class Planner : public PDDLPlannerInterface
    {
    public:
        /**
         * Get name of this planner implementation
         * \return Name of planner
         * \throws PlanGenerationException if not implemented
         */
        std::string getName() const { return "CEDALION"; }


        /**
         * Get name of this planner's execution script/runnable
         * \return Name of planner's main execution script/runnable
         * \throws PlanGenerationException if not implemented
         */
        std::string getCmd() const { return "cedalion-planner"; }

        /**
         * Get version of this planner implementation
         * \return version as int
         * \throws PlanGenerationException if not implemented
         */
        int getVersion() const { return 1; }

        /**
         * Constructor
         * \param resultFileBasename the preset plan result filename
         */
        Planner(const std::string& resultFileBasename = "plan");

        /**
         * Create plan candidates for the given pddl planning problem
         */
        PlanCandidates plan(const std::string& problem, const std::string& actionDescriptions, const std::string& domainDescriptions, double timeout);

    private:
        /**
         * Generate the plan candidates for the given problem
         * There is no priority in the order of candidates
         * \throws PlanGenerationException
         */
        PlanCandidates generatePlanCandidates();
    };
}
}


#endif // PDDL_PLANNER_CEDALION_HPP

