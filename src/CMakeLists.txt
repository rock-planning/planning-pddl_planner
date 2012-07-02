find_package(Boost REQUIRED filesystem system)

rock_library(pddl_planner
    SOURCES PDDLPlanner.cpp
        planners/Lama.cpp
    HEADERS PDDLPlanner.hpp 
        PDDLPlannerInterface.hpp 
        PDDLPlannerTypes.hpp 
        planners/Lama.hpp
    DEPS_PKGCONFIG base-types
        base-lib
    DEPS_CMAKE Boost
    )

rock_executable(pddl_planner_bin Main.cpp
    DEPS pddl_planner)

