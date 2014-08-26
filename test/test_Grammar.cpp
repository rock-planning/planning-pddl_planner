#include <boost/test/auto_unit_test.hpp>
#include <stdio.h>
#include <pddl_planner/representation/Domain.hpp>

BOOST_AUTO_TEST_CASE(it_should_parse_expression)
{
    using namespace pddl_planner::representation;
    {
        Expression e = Expression::fromString("( at robot location )");
        BOOST_REQUIRE_MESSAGE(e.toLISP() == "(at robot location)", "Simple expression parsed correctly: " << e.toLISP());
    }

    {
        Expression e = Expression::fromString("(at robot location)");
        BOOST_REQUIRE_MESSAGE(e.toLISP() == "(at robot location)", "Simple expression without space parsed correctly: " << e.toLISP());
    }

    {
        Expression e = Expression::fromString("(at robot location_c0)");
        BOOST_REQUIRE_MESSAGE(e.toLISP() == "(at robot location_c0)", "Simple expression with alphanumeric and underscore parsed correctly: " << e.toLISP());
    }

    {
        Expression e = Expression::fromString("(at robot ?z)");
        BOOST_REQUIRE_MESSAGE(e.toLISP() == "(at robot ?z)", "Expression with variable parsed correctly: " << e.toLISP());
    }

    {
        Expression e = Expression::fromString("(forall (?z) (at robot ?z))");
        BOOST_REQUIRE_MESSAGE(e.toLISP() == "(forall (?z) (at robot ?z))", "Quantifier expression parsed correctly: " << e.toLISP());
    }

    {
        Expression e = Expression::fromString("(forall (?z - Location) (at robot ?z))");
        BOOST_REQUIRE_MESSAGE(e.toLISP() == "(forall (?z - Location) (at robot ?z))", "Quantifier expression with typed variable parsed correctly: " << e.toLISP());
    }

    { 
        Expression e = Expression::fromString("(exists (?z - Location) (at robot ?z))");
        BOOST_REQUIRE_MESSAGE(e.toLISP() == "(exists (?z - Location) (at robot ?z))", "Quantifier expression with typed variable parsed correctly: " << e.toLISP());
    }

    {
        Expression e = Expression::fromString("(= (distance location_c0 location_p0) 10)");
        BOOST_REQUIRE_MESSAGE(e.toLISP() == "(= (distance location_c0 location_p0) 10)", "Setting of predicate expression parsed correctly: " << e.toLISP());
    }
}
