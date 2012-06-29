rock_library(pddl_planner
    SOURCES Dummy.cpp
    HEADERS Dummy.hpp)

rock_executable(pddl_planner_bin Main.cpp
    DEPS pddl_planner)

