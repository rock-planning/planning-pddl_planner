#ifndef PDDL_PLANNER_INTERFACE_H
#define PDDL_PLANNER_INTERFACE_H

#include <pddl_planner/PDDLPlannerTypes.hpp>

namespace pddl_planner
{
    class PDDLPlannerInterface
    {
    public:
        virtual std::string getName() const = 0;

        virtual int getVersion() const = 0;

        virtual PlanCandidates plan(const std::string& problem, const std::string& actions, const std::string& domain) { throw PlanGenerationException("Plan method not implemented"); }
    };

}
#endif // PDDL_PLANNER_INTERFACE_H
