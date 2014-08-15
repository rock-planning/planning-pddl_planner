#ifndef PDDL_PLANNER_TFD_HPP
#define PDDL_PLANNER_TFD_HPP

#include <pddl_planner/planners/AbstractPlanner.hpp>

namespace pddl_planner
{
namespace tfd
{
    /**
     * Implement the interface to the TFD planner
     *
     */
    class Planner : public AbstractPlanner
    {
    public:
        virtual std::string getName() const { return "TFD"; }

        virtual int getVersion() const { return 1; }

    private:

        /**
         * Remove temporary files and cleanup after plan generation
         */
        virtual void cleanup();
    };
} 
}


#endif // PDDL_PLANNER_TFD_HPP

