#ifndef PDDL_PLANNER_ABSTRACT_PLANNER_HPP
#define PDDL_PLANNER_ABSTRACT_PLANNER_HPP

#include <pddl_planner/PDDLPlannerInterface.hpp>

namespace pddl_planner
{
    /**
     * Implement the interface. Provides a superclass for both LAMA and TFD, which work quite similar.
     *
     */
    class AbstractPlanner : public PDDLPlannerInterface
    {
    public:        
        /**
         * Virtual destructor.
         */
        virtual ~AbstractPlanner() {}
        
        virtual std::string getName() const = 0;

        virtual int getVersion() const = 0;

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
        virtual void cleanup() = 0;

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

        const static std::string msResultFileBasename;
        const static std::string msProblemFileBasename;
        const static std::string msDomainFileBasename;
        const static std::string msTempDirBasename;

        Plan mPlan;
    };
} 



#endif // PDDL_PLANNER_ABSTRACT_PLANNER_HPP

