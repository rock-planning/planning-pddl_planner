#ifndef PDDL_PLANNER_REPRESENTATION_PROBLEM
#define PDDL_PLANNER_REPRESENTATION_PROBLEM

#include <pddl_planner/representation/Domain.hpp>

namespace pddl_planner {
namespace representation {

struct Problem
{
    std::string name;
    Domain domain;
    ConstantList objects;

    ExpressionList status;
    ExpressionList goals;

    Problem(const std::string& name, const Domain& domain);

    /**
     * Add additional constants -- these will be added to the domain definition
     * which is associated with this problem
     */
    void addObject(const TypedItem& type, bool overwrite = false);

    /**
     * Add an expression defining the initial status
     */
    void addInitialStatus(const Expression& e);

    /**
     * Add a goal expression -- conjunction will be built for all added goals
     */
    void addGoal(const Expression& e);

    /**
     * Test if problem has been fully defined, i.e. that goals and a domain are set
     * \return true if goals and a domain are provided, false otherwise
     */
    bool undefined() const { return goals.empty() || domain.isNull(); }

    /**
     * Perform simple syntax check
     * \throw std::runtime_error if syntax has errors
     */
    void validate() const;

    /**
     * Generate the problem in a LISP representation
     * \return problem definition in LISP
     */
    std::string toLISP() const;
};

} // end representation
} // end pddl_planner

#endif // PDDL_PLANNER_REPRESENTATION_PROBLEM
