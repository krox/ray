#pragma once

#include "ray/geometry.h"
#include <string>

namespace ray {

GeometrySet load_scene(std::string const &filename);

}
