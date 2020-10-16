# PDDL Planner - A PDDL Planner Interface

This library provides wrapping functionality to establish a C++-interface to a
set of pddl planners, which are called via their CLI interface.
Currently these include: Arvand Herd, BFSF, FastDownward, FDCedalion, FDUniform,
LAMA, and Randward.
These have been selected based on their performances on the international
planning competition and their ability to build and run.
Additional planners can be added dynamically
using the pddl_planner::Planning::registerPlanner function.
By default LAMA planner is called. 

## Usage Example

### Creating the Planning Problem

```
   #include <pddl_planner/Planning.hpp>
   ...
   using namespace pddl_planner;
   Planning planning;

   // Option 1 -- directly adding the domain and problem description
   std::string domainDescription = "(define (domain rimres)\n(:requirements :strips :equality :typing :conditional-effects)\n(:types location physob_id physob_type)\n(:constants sherpa crex payload - physob_type)\n(:predicates ( at ?x - physob_id ?l - location)\n( is_a ?x - physob_id ?r - physob_type)\n( connected ?x ?y - physob_id)\n( cannot_move ?x - physob_id)\n)\n\n(:action move\n :parameters (?obj - physob_id ?m ?l - location)\n:precondition ( and (at ?obj ?m) (not (= ?m ?l)) (not (cannot_move ?obj) ))\n :effect (and (at ?obj ?l) (not (at ?obj ?m))\n (forall (?z)\n (when (and (connected ?z ?obj) (not (= ?z ?obj)))\n (and (at ?z ?l) (not (at ?z ?m)))\n)))\n)\n (:action move_into_range\n :parameters (?x ?y - physob_id ?m ?l - location)\n :precondition (and (not (cannot_move ?x)) (at ?x ?m) (at ?y ?l) )\n :effect (and (at ?x ?l) (at ?y ?l) (not (at ?x ?m)))\n)\n (:action connect\n :parameters (?x ?y - physob_id ?l - location)\n :precondition (and (at ?x ?l) (at ?y ?l))\n :effect (and (connected ?x ?y) (cannot_move ?y))\n)\n(:action disconnect\n :parameters (?x ?y - physob_id)\n :precondition (and (not (= ?x ?y)) (connected ?x ?y)) \n :effect (and (not (connected ?x ?y)) (not (cannot_move ?y)))\n)\n)\n";

   std::string problemDescription = "(define (problem rimres-1)\n (:domain rimres)\n (:objects\n sherpa_0 crex_0 pl_0 - physob_id\n location_s0 location_c0 location_p0 - location\n mission1 - location\n)\n (:init \n (is_a sherpa_0 sherpa)\n (is_a crex_0 crex)\n (is_a pl_0 payload)\n (at sherpa_0 location_s0)\n (at crex_0 location_c0)\n (at pl_0 location_p0)\n (cannot_move pl_0)\n)\n (:goal (and \n (connected sherpa_0 crex_0) \n (connected sherpa_0 pl_0)\n (at sherpa_0 mission1)\n)\n)\n)\n";

   planning.setDomainDescription("rimres",domainDescription);
   PlanCandidates planCandidates = planning.plan(problemDescription);

   // Option 2 -- using a programmatic approach to create the domain and problem description
   using namespace pddl_planner;

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

   domain.addPredicate( Predicate("at", TypedItem("?x","physob_id"),TypedItem("?l","physob_id")) );
   domain.addPredicate( Predicate("is_a", TypedItem("?x","physob_id"),TypedItem("?r","physob_type")) );
   domain.addPredicate( Predicate("connected", TypedItem("?x","physob_id"),TypedItem("?y","physob_id")) ) ;
   domain.addPredicate( Predicate("cannot_move", TypedItem("?x","physob_id")) );

   // Action move
   representation::Action move("move", TypedItem("?obj","physob_id"), TypedItem("?m","location"), TypedItem("?l","location"));

   Expression precondition("and", Expression("at", "?obj", "?m"), Expression("not", Expression("=", "?m", "?l")), Expression("not", Expression("cannot_move","?obj")));


   Expression effect("and", Expression("at","?obj","?l"), Expression("not", Expression("at","?obj","?m")), Expression("forall", Expression("?z"), Expression("when", Expression("and", Expression("connected","?z","?obj"), Expression("not", Expression("=","?z","?obj"))), Expression("and", Expression("at","?z","?l"), Expression("not", Expression("at","?z","?m"))))));

   move.addPrecondition(precondition);
   move.addEffect(effect);
   domain.addAction(move);

   // Action move
   representation::Action move("move", TypedItem("?obj","physob_id"), TypedItem("?m","location"), TypedItem("?l","location"));
   {
       Expression precondition("and", Expression("at", "?obj", "?m"), Expression("not", Expression("=", "?m", "?l")), Expression("not", Expression("cannot_move","?obj")));


       Expression effect("and", Expression("at","?obj","?l"), Expression("not", Expression("at","?obj","?m")), Expression("forall", Expression("?z"), Expression("when", Expression("and", Expression("connected","?z","?obj"), Expression("not", Expression("=","?z","?obj"))), Expression("and", Expression("at","?z","?l"), Expression("not", Expression("at","?z","?m"))))));

       move.addPrecondition(precondition);
       move.addEffect(effect);
       domain.addAction(move);
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

   representation::Problem problem("rimres-1",domain);
   problem.addObject( Constant("sherpa_0","physob_id"));
   problem.addObject( Constant("crex_0","physob_id"));
   problem.addObject( Constant("pl_0","physob_id"));
   problem.addObject( Constant("location_s0","location"));
   problem.addObject( Constant("location_c0","location"));
   problem.addObject( Constant("location_p0","location"));
   problem.addObject( Constant("mission1","location"));

   problem.addInitialStatus( Expression("is_a","sherpa_0","sherpa") );
   problem.addInitialStatus( Expression("is_a","crex_0","crex") );
   problem.addInitialStatus( Expression("is_a","pl_0","payload") );
   problem.addInitialStatus( Expression("at","sherpa_0","location_s0") );
   problem.addInitialStatus( Expression("at","crex_0","location_c0") );
   problem.addInitialStatus( Expression("at","pl_0","location_p0") );
   problem.addInitialStatus( Expression("cannot_move", "pl_0") );

   Expression subgoal0("connected","sherpa_0","crex_0");
   Expression subgoal1("connected","sherpa_0","pl_0");
   Expression subgoal2("at","sherpa_0","mission1");
   problem.setGoal(Expression("and", subgoal0, subgoal1, subgoal2));
```

### Performing the actual planning

The simplest way of performing the actual planning is, where the default planner
'LAMA' is being used with a predefined timeout (1h)
```
   ...
   Planning planning;
   PlanCandidates planCandidates = planning.plan(problem);
```

You might want to specify the planner, here FastDownward and timeout in seconds as follows:
```
   ...
   Planning planning;
   PlanCandidates planCandidates = planning.plan(problem, "FD", 30.0);
```

## Resources
Further information available at:
- http://ipc.informatik.uni-freiburg.de/PddlResources
- http://www.plg.inf.uc3m.es/ipc2011-deterministic/
- for a BNF description of PDDL 3.1
- http://www.plg.inf.uc3m.es/ipc2011-deterministic/attachments/Resources/kovacs-pddl-3.1-2011.pdf

# Installation

## Source based installation
Create a new Rock-based installation in a development folder, here called dev:
```
    mkdir dev
    cd dev
    wget http://www.rock-robotics.org/master/autoproj_bootstrap
    ruby autoproj_bootstrap
```

In autoproj/manifest add the respective manifest and add the package to the
layout section:
```
    package_set:
        - github: rock-core/rock-package_set

    layout:
        - planning/pddl_planner
```

```
$>source env.sh
$>autoproj update
$>autoproj osdeps
$>amake planning/pddl_planner
```

# Merge Request and Issue Tracking
Github will be used for pull requests and issue tracking: https://github.com/rock-planning/planning-pddl_planner

# License
This software is distributed under the [New/3-clause BSD license](https://opensource.org/licenses/BSD-3-Clause)

# Copyright
Copyright (c) 2012-2020, DFKI GmbH Robotics Innovation Center

