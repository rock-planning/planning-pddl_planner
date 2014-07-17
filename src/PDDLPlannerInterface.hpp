#ifndef PDDL_PLANNER_INTERFACE_H
#define PDDL_PLANNER_INTERFACE_H

#include <pddl_planner/PDDLPlannerTypes.hpp>

namespace pddl_planner
{
    class PDDLPlannerInterface
    {
    public:
        virtual ~PDDLPlannerInterface() {}

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
        virtual PlanCandidates plan(const std::string& problem, const std::string& actions, const std::string& domain) { throw PlanGenerationException("Plan method not implemented"); }
    };

}
#endif // PDDL_PLANNER_INTERFACE_H
