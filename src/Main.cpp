#include <iostream>
#include <stdio.h>
#include <pddl_planner/PDDLPlanner.hpp>
#include <pddl_planner/planners/Lama.hpp>

int main(int argc, char** argv)
{
    using namespace pddl_planner;
    lama::Planner planner;
    PDDLPlanner pddlPlanner(&planner);

    pddlPlanner.setDomainDescription("rimres", "(define (domain rimres)\n(:requirements :strips :equality :typing :conditional-effects)\n(:types location physob_id physob_type)\n(:constants sherpa crex payload - physob_type)\n(:predicates ( at ?x - physob_id ?l - location)\n( is_a ?x - physob_id ?r - physob_type)\n( connected ?x ?y - physob_id)\n( cannot_move ?x - physob_id)\n)\n\n(:action move\n :parameters (?obj - physob_id ?m ?l - location)\n:precondition ( and (at ?obj ?m) (not (= ?m ?l)) (not (cannot_move ?obj) ))\n :effect (and (at ?obj ?l) (not (at ?obj ?m))\n (forall (?z)\n (when (and (connected ?z ?obj) (not (= ?z ?obj)))\n (and (at ?z ?l) (not (at ?z ?m)))\n)))\n)\n (:action move_into_range\n :parameters (?x ?y - physob_id ?m ?l - location)\n :precondition (and (not (cannot_move ?x)) (at ?x ?m) (at ?y ?l) )\n :effect (and (at ?x ?l) (at ?y ?l) (not (at ?x ?m)))\n)\n (:action connect\n :parameters (?x ?y - physob_id ?l - location)\n :precondition (and (at ?x ?l) (at ?y ?l))\n :effect (and (connected ?x ?y) (cannot_move ?y))\n)\n(:action disconnect\n :parameters (?x ?y - physob_id)\n :precondition (and (not (= ?x ?y)) (connected ?x ?y)) \n :effect (and (not (connected ?x ?y)) (not (cannot_move ?y)))\n)\n)\n");

try {
    Plan plan = pddlPlanner.plan("(define (problem rimres-1)\n (:domain rimres)\n (:objects\n sherpa_0 crex_0 pl_0 - physob_id\n location_s0 location_c0 location_p0 - location\n mission1 - location\n)\n (:init \n (is_a sherpa_0 sherpa)\n (is_a crex_0 crex)\n (is_a pl_0 payload)\n (at sherpa_0 location_s0)\n (at crex_0 location_c0)\n (at pl_0 location_p0)\n (cannot_move pl_0)\n)\n (:goal (and \n (connected sherpa_0 crex_0) \n (connected sherpa_0 pl_0)\n (at sherpa_0 mission1)\n)\n)\n)\n");

    printf("Plan: %s\n", plan.toString().c_str());
} catch(const std::runtime_error& e)
{
    printf("Error: %s\n", e.what());
}


    return 0;
}
