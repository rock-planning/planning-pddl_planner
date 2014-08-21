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
        std::string name;
        std::vector<std::string> arguments;

        void addArgument(const std::string& argument)
        {
            arguments.push_back(argument);
        }

        Action(const std::string& _name)
            : name(_name)
        {}

        Action() 
        {}

        std::string toString() const
        {
            std::string action = name;
            std::vector<std::string>::const_iterator it = arguments.begin();
            for(;it != arguments.end(); ++it)
            {
                action += *it + " ";
            }
            boost::algorithm::trim(action);
            return action;
        }
    };

    // Plan as a sequence of actions
    struct Plan
    {
        std::vector<Action> action_sequence;

        void addAction(const Action& action)
        {
            action_sequence.push_back(action);
        }

        std::string toString() const
        {
            std::string plan;
            std::vector<Action>::const_iterator it = action_sequence.begin();
            for(;it != action_sequence.end(); ++it)
            {
                plan += "[" + it->toString() + "]";
            }
            boost::algorithm::trim(plan);
            return plan;
        }
    };

    struct PlanCandidates
    {
        std::vector<Plan> plans;

        void addPlan(const Plan& plan)
        {
            plans.push_back(plan);
        }

        std::string toString() const
        {
            std::string candidates;
            std::vector<Plan>::const_iterator it = plans.begin();
            int index = 0;
            for(;it != plans.end(); ++it)
            {
                std::stringstream ss;
                ss << index++ << "\t" << it->toString() << "\n";
                candidates += ss.str();
            }
            return candidates;

        }
    };


}
#endif // PDDL_PLANNER_TYPES

