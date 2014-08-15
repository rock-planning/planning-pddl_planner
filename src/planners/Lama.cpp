#include <pddl_planner/planners/Lama.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <boost/filesystem.hpp>
#include <base/logging.h>
#include <base/time.h>

namespace fs = boost::filesystem;

namespace pddl_planner
{
namespace lama
{

void Planner::cleanup()
{
    fs::path path(mTempDir);
    fs::remove_all(path);
    fs::remove(fs::path("output"));
    fs::remove(fs::path("output.sas"));
    fs::remove(fs::path("all.groups"));
    fs::remove(fs::path("test.groups"));
}

}
}
