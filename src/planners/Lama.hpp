#ifndef PDDL_PLANNER_LAMA_HPP
#define PDDL_PLANNER_LAMA_HPP

#include <pddl_planner/PDDLPlannerInterface.hpp>

namespace pddl_planner
{
namespace lama
{
    class Planner : public PDDLPlannerInterface
    {
    public:
        Plan plan(const std::string& problem, const std::string& actionDescriptions, const std::string& domainDescriptions);

    private:
        void prepare(const std::string& problem, const std::string& actionDescriptions, const std::string& domainDescriptions);

        void cleanup();

        /**
         * Generate the plan for the given problem
         */
        bool generatePlan(Plan& plan);

        std::string mTempDir; 
        std::string mDomainFilename;
        std::string mProblemFilename;
        std::string mResultFilename;

        Plan mPlan;

    };
}
}


#endif // PDDL_PLANNER_LAMA_HPP

