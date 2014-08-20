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
    Expression goal;

    Problem(const std::string& name, const Domain& domain);

    /**
     * Add additional constants -- these will be added to the domain definition
     * which is associated with this problem
     */
    void addObject(const TypedItem& type, bool overwrite = false);

    /**
     * Add an expression defining the initial status
     * \param e Expression describing the initial status
     * \throws std::invalid_argument When validation fails for expression in the given context
     */
    void addInitialStatus(const Expression& e);


    /**
     * Set the goal expression
     */
    void setGoal(const Expression& e);

    /**
     * Add a sub goal expression to the existing top level one
     */
    void addSubGoal(const Expression& e);

    /**
     * Test if problem has been fully defined, i.e. that goals and a domain are set
     * \return true if goals and a domain are provided, false otherwise
     */
    bool undefined() const { return goal.isNull() || domain.isNull(); }

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
