#ifndef PDDL_PLANNER_LAMA_HPP
#define PDDL_PLANNER_LAMA_HPP

#include <pddl_planner/planners/AbstractPlanner.hpp>

namespace pddl_planner
{
namespace lama
{
    /**
     * Implement the interface to the LAMA planner
     *
     */
    class Planner : public AbstractPlanner
    {
    public:
        virtual std::string getName() const { return "LAMA"; }

        virtual int getVersion() const { return 1; }

    private:

        /**
         * Remove temporary files and cleanup after plan generation
         */
        virtual void cleanup();
    };
} 
}


#endif // PDDL_PLANNER_LAMA_HPP

