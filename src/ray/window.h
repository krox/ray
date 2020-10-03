#pragma once

#include "ray/types.h"
#include "util/span.h"

namespace ray {

/** display image in a window, returns only after user closes window */
void show_window(util::ndspan<const vec3, 2> image);

} // namespace ray
