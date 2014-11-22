find_package(Boost REQUIRED filesystem system)

rock_library(pddl_planner
    SOURCES Planning.cpp
        PDDLPlannerTypes.cpp
        planners/Lama.cpp
        planners/Bfsf.cpp
        planners/Fast_downward.cpp
        representation/Domain.cpp
        representation/Problem.cpp
        representation/grammar/lisp/Expression.cpp
    HEADERS Planning.hpp
        PDDLPlannerInterface.hpp 
        PDDLPlannerTypes.hpp 
        planners/Lama.hpp
        planners/Bfsf.hpp
        planners/Fast_downward.hpp
        representation/Domain.hpp
        representation/Problem.hpp
        representation/grammar/lisp/Expression.hpp
    DEPS_PKGCONFIG base-types
        base-lib
    DEPS_CMAKE Boost
    )

rock_executable(pddl_planner_bin Main.cpp
    DEPS pddl_planner)

