#ifndef PDDL_PLANNER_REPRESENTATION_GRAMMAR_DEBUG_HPP
#define PDDL_PLANNER_REPRESENTATION_GRAMMAR_DEBUG_HPP

#include <iosfwd>
#include <utility>

#include <pddl_planner/representation/Domain.hpp>
#include <pddl_planner/representation/Problem.hpp>

namespace std
{

template< typename C, typename E>
std::basic_ostream<C, E>& operator<<(std::basic_ostream<C,E>& out, pddl_planner::representation::Expression e)
{
                return out << "Expression<" << e.toLISP() << ">";
}

} // end namespace std
#endif // PDDL_PLANNER_REPRESENTATION_GRAMMAR_DEBUG_HPP

