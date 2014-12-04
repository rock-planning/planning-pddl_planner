#ifndef PDDL_PLANNER_H
#define PDDL_PLANNER_H

#include <string>
#include <stdexcept>
#include <map>
#include <pddl_planner/representation/Problem.hpp>
#include <pddl_planner/PDDLPlannerTypes.hpp>

#define TIMEOUT 7.

/**
 * \mainpage PDDL based planning
 * This library provides a somewhat generalized interface for planning and allow to embed a selection of
 * PDDL planners.
 *
 * While by default it includes integration for the LAMA planner and the FDSS planners, additional planners can be added dynamically
 * using the pddl_planner::Planning::registerPlanner function.
 *
 * \verbatim
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

   Planning planning;
   PlanCandidates planCandidates = planning.plan(problem);
 \endverbatim
 *
 * \section sec Weblinks and Resources
 * Further information available at:
 *     - http://ipc.informatik.uni-freiburg.de/PddlResources
 *     - http://www.plg.inf.uc3m.es/ipc2011-deterministic/
 *         - for a BNF description of PDDL 3.1
 *             - http://www.plg.inf.uc3m.es/ipc2011-deterministic/attachments/Resources/kovacs-pddl-3.1-2011.pdf
 */

namespace pddl_planner
{
    class PDDLPlannerInterface;

    typedef std::map<std::string, std::string> ActionDescriptions;
    typedef std::map<std::string, std::string> DomainDescriptions;
    typedef std::map<std::string, PDDLPlannerInterface*> PlannerMap;
    typedef std::vector<PDDLPlannerInterface*> PlannerList;

    /**
     * \class Planning
     * \brief Planning provides the main infrastructure to perform planning with different
     * planner implementations
     */
    class Planning
    {
        PlannerMap mPlanners;

    public:
        /**
         * \brief Default constructor for planning
         * It will instanciate all associated planners at construction time
         */
        Planning();

        /**
         * Deconstructor deleting all internal instances of associated planners
         */
        ~Planning();

        
        /**
         * Retrieve the map of all registered planners 
         * \return map of registered planners
         */
        PlannerMap getPlanners() { return mPlanners; }
        
        /**
         * Register an implementation of a pddl planner
         */
        void registerPlanner(PDDLPlannerInterface* planner);

        /**
         * Retrieve all registered planners
         * \return list of registered planners
         */
        PlannerList getRegisteredPlanners() const;

        /**
         * Check if there is a planner of the given name
         */
        bool isRegistered(const std::string& name) const { return mPlanners.count(name); }

        /**
         * Retrieve planner by name
         * \throw std::runtime_error if planner interface does not exist
         */
        PDDLPlannerInterface* getPlanner(const std::string& name) const;

        /** 
         * Set the description for a specific action
         */
        void setActionDescription(const std::string& action, const std::string& description);

        /* 
        * Set the description for a specific domain
        */
        void setDomainDescription(const std::string& domain, const std::string& description);

        /**
         * Generate the domain description for 
         * all available domain
         * \return Domain descriptions
         */
        std::string getDomainDescriptions() const;

        /**
         * Retrieve the action description for 
         * all available actions
         * \return Action descriptions
         */
        std::string getActionDescriptions() const;

        /**
         * Plan towards a given goal
         * \return List of solutions, i.e. plans
         * \throws PlanGenerationException on failure
         */
        PlanCandidates plan(const std::string& goal, double timeout = TIMEOUT, const std::string& planner = "LAMA");

        /**
         * Generate a plan for a given problem -- the problem definition here already contains
         * the domain description
         * \throws PlanGenerationException on failure
         */
        PlanCandidates plan(const representation::Problem& problem, double timeout = TIMEOUT, const std::string& planner = "LAMA");

        /**
         * Generate a plan for a given domain and problem -- the domain associated with the problem will be overriden
         */
        PlanCandidates plan(const representation::Domain& domain, const representation::Problem& problem, double timeout = TIMEOUT, const std::string& planner = "LAMA");

    private: 
        ActionDescriptions mActionDescriptions;
        DomainDescriptions mDomainDescriptions;
    };


}
#endif // PDDL_PLANNER_H

