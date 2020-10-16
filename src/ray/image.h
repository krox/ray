#pragma once

#include "ray/types.h"
#include "util/span.h"

namespace ray {
void write_image(std::string const &filename, util::ndspan<vec3, 2> image,
                 double gamma);
}
