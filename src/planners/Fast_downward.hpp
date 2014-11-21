#ifndef PDDL_PLANNER_FASTDOWNWARD_HPP
#define PDDL_PLANNER_FASTDOWNWARD_HPP

#include <pddl_planner/PDDLPlannerInterface.hpp>

namespace pddl_planner
{
namespace fast_downward
{
    /**
     * Implement the interface to the FD planner
     *
     */
    class Planner : public PDDLPlannerInterface
    {
    public:
        std::string getName() const { return "FD"; }

        int getVersion() const { return 1; }

        Planner();
        Planner(const std::string & alias = "");
        
        /**
         * Create plan candidates for the given pddl planning problem
         */
        PlanCandidates plan(const std::string& problem, const std::string& actionDescriptions, const std::string& domainDescriptions);

    private:
        /**
         *
         */
        void prepare(const std::string& problem, const std::string& actionDescriptions, const std::string& domainDescriptions);

        /**
         * Read a plani
         * Note, that an empty plan is a valid plan
         * \throws PlanGenerationException
         */
        Plan readPlan(const std::string& filename);

        /**
         * Remove temporary files and cleanup after plan generation
         */
        void cleanup();

        /**
         * Generate the plan candidates for the given problem
         * There is no priority in the order of candidates
         * \throws PlanGenerationException
         */
        PlanCandidates generatePlanCandidates();

        std::string mTempDir; 
        std::string mDomainFilename;
        std::string mProblemFilename;
        std::string mResultFilename;
        std::string mAlias;

        const static std::string msResultFileBasename;
        const static std::string msProblemFileBasename;
        const static std::string msDomainFileBasename;
        const static std::string msTempDirBasename;

        Plan mPlan;
    };
} 
}


#endif // PDDL_PLANNER_FASTDOWNWARD_HPP

