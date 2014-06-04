#ifndef PDDL_PLANNER_H
#define PDDL_PLANNER_H

#include <string>
#include <stdexcept>
#include <map>
#include <pddl_planner/representation/Problem.hpp>
#include <pddl_planner/PDDLPlannerTypes.hpp>

namespace pddl_planner
{
    class PDDLPlannerInterface;

    typedef std::map<std::string, std::string> ActionDescriptions;
    typedef std::map<std::string, std::string> DomainDescriptions;
    typedef std::map<std::string, PDDLPlannerInterface*> PlannerMap;
    typedef std::vector<PDDLPlannerInterface*> PlannerList;

    /**
     * Planning provides the main infrastructure to perform planning with different
     * planner implementations
     */
    class Planning
    {
        PlannerMap mPlanners;

    public:
        /**
         * Default constructor for planning
         * It will instanciate all associated planners at construction time
         */
        Planning();

        /**
         * Deconstructor deleting all internal instances of associated planners
         */
        ~Planning();

        /**
         * Register an implementation of a pddl planner
         */
        void registerPlanner(PDDLPlannerInterface* planner);

        /**
         * Retrieve all registered planners
         * \return list of registered planners
         */
        PlannerList getRegisteredPlanners() const;

        /**
         * Check if there is a planner of the given name
         */
        bool isRegistered(const std::string& name) const { return mPlanners.count(name); }

        /**
         * Retrieve planner by name
         * \throw std::runtime_error if planner interface does not exist
         */
        PDDLPlannerInterface* getPlanner(const std::string& name) const;

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
         * \return Domain descriptions
         */
        std::string getDomainDescriptions() const;

        /**
         * Retrieve the action description for 
         * all available actions
         * \return Action descriptions
         */
        std::string getActionDescriptions() const;

        /**
         * Plan towards a given goal
         * \return List of solutions, i.e. plans
         * \throws PlanGenerationException on failure
         */
        PlanCandidates plan(const std::string& goal, const std::string& planner = "LAMA");

        /**
         * Generate a plan for a given problem -- the problem definition here already contains
         * the domain description
         * \throws PlanGenerationException on failure
         */
        PlanCandidates plan(const representation::Problem& problem, const std::string& planner = "LAMA");

    private: 
        ActionDescriptions mActionDescriptions;
        DomainDescriptions mDomainDescriptions;
    };


}
#endif // PDDL_PLANNER_H

