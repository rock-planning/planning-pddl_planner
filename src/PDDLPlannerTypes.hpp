#ifndef PDDL_PLANNER_TYPES
#define PDDL_PLANNER_TYPES

#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <boost/algorithm/string.hpp>

namespace pddl_planner
{
    class PlanGenerationException : public std::runtime_error
    {
    public:
        PlanGenerationException(const std::string& msg) : std::runtime_error("PlanGenerationException: " + msg)
        {}

        PlanGenerationException() : std::runtime_error("PlanGenerationException")
        {}
    };

    struct Action
    {
        // name of action
        std::string name;
        // arguments of this action
        std::vector<std::string> arguments;

        /**
         * Default constructor
         * \param name Name of action
         */
        Action(const std::string& name ="");

        /**
         * Add an argument to this action
         * \parram argument Argument
         */
        void addArgument(const std::string& argument);

        /**
         * Create string representation of class
         * \return string representation
         */
        std::string toString() const;
    };

    // Plan as a sequence of actions
    struct Plan
    {
        std::vector<Action> action_sequence;

        /**
         * Add an action to the plan
         * \param action Action
         */
        void addAction(const Action& action);

        /**
         * Create string representation of class
         * \return string representation
         */
        std::string toString() const;
    };

    struct PlanCandidates
    {
        std::vector<Plan> plans;

        /**
         * Add a plan to the list of plan candidates
         * \param plan Plan candidate
         */
        void addPlan(const Plan& plan);

        /**
         * Create string representation of class
         * \return string representation
         */
        std::string toString() const;
    };


}
#endif // PDDL_PLANNER_TYPES
