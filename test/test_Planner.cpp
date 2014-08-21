#include <boost/test/auto_unit_test.hpp>
#include <pddl_planner/Planning.hpp>
#include <pddl_planner/representation/Domain.hpp>
#include <pddl_planner/representation/Problem.hpp>

BOOST_AUTO_TEST_CASE(main_lama_test)
{

    using namespace pddl_planner;
    Planning planning;

    std::string domainDescription = "(define (domain rimres)\n(:requirements :strips :equality :typing :conditional-effects)\n(:types location physob_id physob_type)\n(:constants sherpa crex payload - physob_type)\n(:predicates ( at ?x - physob_id ?l - location)\n( is_a ?x - physob_id ?r - physob_type)\n( connected ?x ?y - physob_id)\n( cannot_move ?x - physob_id)\n)\n\n(:action move\n :parameters (?obj - physob_id ?m ?l - location)\n:precondition ( and (at ?obj ?m) (not (= ?m ?l)) (not (cannot_move ?obj) ))\n :effect (and (at ?obj ?l) (not (at ?obj ?m))\n (forall (?z)\n (when (and (connected ?z ?obj) (not (= ?z ?obj)))\n (and (at ?z ?l) (not (at ?z ?m)))\n)))\n)\n (:action move_into_range\n :parameters (?x ?y - physob_id ?m ?l - location)\n :precondition (and (not (cannot_move ?x)) (at ?x ?m) (at ?y ?l) )\n :effect (and (at ?x ?l) (at ?y ?l) (not (at ?x ?m)))\n)\n (:action connect\n :parameters (?x ?y - physob_id ?l - location)\n :precondition (and (at ?x ?l) (at ?y ?l))\n :effect (and (connected ?x ?y) (cannot_move ?y))\n)\n(:action disconnect\n :parameters (?x ?y - physob_id)\n :precondition (and (not (= ?x ?y)) (connected ?x ?y)) \n :effect (and (not (connected ?x ?y)) (not (cannot_move ?y)))\n)\n)\n";

    planning.setDomainDescription("rimres",domainDescription);

    BOOST_TEST_MESSAGE( "Domain description \n" << domainDescription );

    try {
        std::string problemDescription = "(define (problem rimres-1)\n (:domain rimres)\n (:objects\n sherpa_0 crex_0 pl_0 - physob_id\n location_s0 location_c0 location_p0 - location\n mission1 - location\n)\n (:init \n (is_a sherpa_0 sherpa)\n (is_a crex_0 crex)\n (is_a pl_0 payload)\n (at sherpa_0 location_s0)\n (at crex_0 location_c0)\n (at pl_0 location_p0)\n (cannot_move pl_0)\n)\n (:goal (and \n (connected sherpa_0 crex_0) \n (connected sherpa_0 pl_0)\n (at sherpa_0 mission1)\n)\n)\n)\n";
        BOOST_TEST_MESSAGE( "Domain description \n" << problemDescription );

        PlanCandidates planCandidates = planning.plan(problemDescription);

        BOOST_TEST_MESSAGE("PlanCandidates: " << planCandidates.toString());
        BOOST_ASSERT(planCandidates.plans.size() == 2);
    } catch(const std::runtime_error& e)
    {
        BOOST_TEST_MESSAGE("Error: " << e.what());
        BOOST_ASSERT(false);
    }
}

BOOST_AUTO_TEST_CASE(expression_test)
{
    using namespace pddl_planner;
    using namespace pddl_planner::representation;

    Expression andExpr("and","?a","?b");
    Expression notExpr("not", andExpr);
    BOOST_REQUIRE_MESSAGE(andExpr.toLISP() == "(and ?a ?b)", andExpr.toLISP());
    BOOST_REQUIRE_MESSAGE(notExpr.toLISP() == "(not (and ?a ?b))", notExpr.toLISP());

    Expression quantorExpression(EXISTS,TypedItem("?l","location"),
            Expression("and",
                Expression("at","sherpa","?l"),
                Expression("at","crex","?l"))
            );
    BOOST_REQUIRE_MESSAGE(quantorExpression.toLISP() == "(exists (?l - location) (and (at sherpa ?l) (at crex ?l)))", quantorExpression.toLISP());
    Expression notQuantorExpression("not", quantorExpression);

    BOOST_REQUIRE_MESSAGE(notQuantorExpression.toLISP() == "(not (exists (?l - location) (and (at sherpa ?l) (at crex ?l))))", notQuantorExpression.toLISP());
}


BOOST_AUTO_TEST_CASE(domain_test)
{
    using namespace pddl_planner;
    using namespace pddl_planner::representation;

    Domain domain("rimres");

    domain.addRequirement("strips");
    domain.addRequirement("equality");
    domain.addRequirement("typing");
    domain.addRequirement("conditional-effects");

    domain.addType("location");
    domain.addType("physob_id");
    domain.addType("physob_type");

    domain.addConstant( Constant("sherpa","physob_type") );
    domain.addConstant( Constant("crex","physob_type") );
    domain.addConstant( Constant("payload","physob_type") );

    BOOST_REQUIRE_MESSAGE( domain.isConstant("sherpa"), "Has constant: sherpa" );
    BOOST_REQUIRE_MESSAGE( domain.isConstant("crex"), "Has constant: crex" );
    BOOST_REQUIRE_MESSAGE( domain.isConstant("payload"), "Has constant: payload" );

    domain.addPredicate( Predicate("at", TypedItem("?x","physob_id"),TypedItem("?l","physob_id")) );
    domain.addPredicate( Predicate("is_a", TypedItem("?x","physob_id"),TypedItem("?r","physob_type")) );
    domain.addPredicate( Predicate("connected", TypedItem("?x","physob_id"),TypedItem("?y","physob_id")) ) ;
    domain.addPredicate( Predicate("cannot_move", TypedItem("?x","physob_id")) );


    // Action move
    representation::Action move("move", TypedItem("?obj","physob_id"), TypedItem("?m","location"), TypedItem("?l","location"));
    {
        Expression precondition("and", Expression("at", "?obj", "?m"), Expression("not", Expression("=", "?m", "?l")), Expression("not", Expression("cannot_move","?obj")));


        Expression effect("and", Expression("at","?obj","?l"), Expression("not", Expression("at","?obj","?m")), Expression(FORALL, TypedItem("?z","physob_type"), Expression("when", Expression("and", Expression("connected","?z","?obj"), Expression("not", Expression("=","?z","?obj"))), Expression("and", Expression("at","?z","?l"), Expression("not", Expression("at","?z","?m"))))));

        move.addPrecondition(precondition);
        move.addEffect(effect);
        domain.addAction(move);

        BOOST_REQUIRE_THROW(domain.addAction(move), std::invalid_argument);
    }

    // Action move_into_range
    representation::Action move_into_range("move_into_range", TypedItem("?x","physob_id"), TypedItem("?y","physob_id"), TypedItem("?m","location"), TypedItem("?l","location"));
    {
        Expression precondition("and", Expression("not", Expression("cannot_move","?x")), Expression("at","?x","?m"), Expression("at","?y","?l"));
        Expression effect("and", Expression("at","?x","?l"), Expression("at","?y","?l"), Expression("not", Expression("at","?x","?m")));
        move_into_range.addPrecondition(precondition);
        move_into_range.addEffect(effect);
        domain.addAction(move_into_range);
    }

    // Action connect
    representation::Action connect("connect", TypedItem("?x","physob_id"), TypedItem("?y","physob_id"), TypedItem("?l","location"));
    {
        Expression precondition("and", Expression("at","?x","?l"), Expression("at","?y","?l"));
        Expression effect("and", Expression("connected","?x","?y"), Expression("cannot_move","?y"));
        connect.addPrecondition(precondition);
        connect.addEffect(effect);
        domain.addAction(connect);
    }

    // Action connect
    representation::Action disconnect("disconnect", TypedItem("?x","physob_id"), TypedItem("?y","physob_id"));
    {
        Expression precondition("and", Expression("not", Expression("=","?x","?y")), Expression("connected","?x","?y"));
        Expression effect("and", Expression("not", Expression("connected","?x","?y")), Expression("not", Expression("cannot_move","?x")));
        disconnect.addPrecondition(precondition);
        disconnect.addEffect(effect);
        domain.addAction(disconnect);
    }

    BOOST_TEST_MESSAGE( domain.toLISP() );
    BOOST_REQUIRE_NO_THROW( domain.validate() );

    representation::Problem problem("rimres-1",domain);
    problem.addObject( Constant("sherpa_0","physob_id"));
    problem.addObject( Constant("crex_0","physob_id"));
    problem.addObject( Constant("pl_0","physob_id"));
    problem.addObject( Constant("location_s0","location"));
    problem.addObject( Constant("location_c0","location"));
    problem.addObject( Constant("location_p0","location"));
    problem.addObject( Constant("mission1","location"));
    BOOST_TEST_MESSAGE("Added objects");

    problem.addInitialStatus( Expression("is_a","sherpa_0","sherpa") );
    problem.addInitialStatus( Expression("is_a","crex_0","crex") );
    problem.addInitialStatus( Expression("is_a","pl_0","payload") );
    problem.addInitialStatus( Expression("at","sherpa_0","location_s0") );
    problem.addInitialStatus( Expression("at","crex_0","location_c0") );
    problem.addInitialStatus( Expression("at","pl_0","location_p0") );
    problem.addInitialStatus( Expression("cannot_move", "pl_0") );
    BOOST_TEST_MESSAGE("Added initial status");

    Expression subgoal0("connected","sherpa_0","crex_0");
    Expression subgoal1("connected","sherpa_0","pl_0");
    Expression subgoal2("at","sherpa_0","mission1");
    Expression goal("and", subgoal0, subgoal1, subgoal2);
    problem.setGoal(goal);
    std::string goalDescription = "(and (connected sherpa_0 crex_0) (connected sherpa_0 pl_0) (at sherpa_0 mission1))";
    BOOST_REQUIRE_MESSAGE(goal.toLISP() == goalDescription, "Problem goal: " << problem.toLISP() );

    Expression goal1 = goal;
    goal1 = goal;
    BOOST_REQUIRE_MESSAGE(goal1.toLISP() == goalDescription, "Expression assignment: " << problem.toLISP() );

    BOOST_TEST_MESSAGE( problem.domain.toLISP() );
    BOOST_REQUIRE_NO_THROW( problem.validate() );

    Planning planning;
    PlanCandidates planCandidates = planning.plan(problem);

    BOOST_TEST_MESSAGE( "PlanCandidates:\n" << planCandidates.toString());
    BOOST_ASSERT(planCandidates.plans.size() == 2);

    problem.goal.addParameter(Expression("a","sherpa_0","mission1"));

    try {
        problem.validate();
        BOOST_REQUIRE_MESSAGE(false, "Validation expected to throw, but did not");
    } catch(const std::runtime_error& e)
    {
        BOOST_TEST_MESSAGE( e.what() );
    }
}
