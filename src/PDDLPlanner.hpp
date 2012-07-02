#ifndef PDDL_PLANNER_H
#define PDDL_PLANNER_H

#include <string>
#include <stdexcept>
#include <map>
#include <pddl_planner/PDDLPlannerTypes.hpp>

namespace pddl_planner
{
    class PDDLPlannerInterface;

    typedef std::map<std::string, std::string> ActionDescriptions;
    typedef std::map<std::string, std::string> DomainDescriptions;

    class PDDLPlanner
    {
    public:

        PDDLPlanner(PDDLPlannerInterface* planner);

        /** 
         * Set the description for a specific action
         */
        void setActionDescription(const std::string& action, const std::string& description);

        /* 
        * Set the description for a specific domain
        */
        void setDomainDescription(const std::string& domain, const std::string& description);

        /**
         * Generate the domain description for 
         * all available domain
         */
        std::string getDomainDescriptions() const;

        /**
         * Retrieve the action description for 
         * all available actions
         */
        std::string getActionDescriptions() const;

        /**
         * \throws PlanGenerationException on failure
         */
        Plan plan(const std::string& goal);

    private: 
        PDDLPlannerInterface* mPlanner;
        ActionDescriptions mActionDescriptions;
        DomainDescriptions mDomainDescriptions;
    };


}
#endif // PDDL_PLANNER_H

