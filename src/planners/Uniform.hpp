#ifndef PDDL_PLANNER_UNIFORM_HPP
#define PDDL_PLANNER_UNIFORM_HPP

#include <pddl_planner/PDDLPlannerInterface.hpp>

namespace pddl_planner
{
namespace uniform
{
    /**
     * Implement the interface to the UNIFORM planner
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
        std::string getName() const { return "UNIFORM"; }

        
        /**
         * Get name of this planner's execution script/runnable
         * \return Name of planner's main execution script/runnable
         * \throws PlanGenerationException if not implemented
         */
        std::string getCmd() const { return "uniform-planner"; }

        /**
         * Get version of this planner implementation
         * \return version as int
         * \throws PlanGenerationException if not implemented
         */
        int getVersion() const { return 1; }

        /**
         * Create plan candidates for the given pddl planning problem
         */
        PlanCandidates plan(const std::string& problem, const std::string& actionDescriptions, const std::string& domainDescriptions, double timeout);

    private:
        /**
         *
         */
        void prepare(const std::string& problem, const std::string& actionDescriptions, const std::string& domainDescriptions);

        /**
         * Generate the plan candidates for the given problem
         * There is no priority in the order of candidates
         * \throws PlanGenerationException
         */
        PlanCandidates generatePlanCandidates();

        double      mTimeout;
        std::string mTempDir;
        std::string mDomainFilename;
        std::string mProblemFilename;
        std::string mResultFilename;

        const static std::string msResultFileBasename;
        const static std::string msProblemFileBasename;
        const static std::string msDomainFileBasename;
        const static std::string msTempDirBasename;

        Plan mPlan;
    };
} 
}


#endif // PDDL_PLANNER_UNIFORM_HPP
