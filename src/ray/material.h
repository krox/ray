#pragma once

#include "ray/texture.h"
#include "ray/types.h"

namespace ray {

struct Material
{
	std::shared_ptr<const TextureBase> diffuse = nullptr;
	std::shared_ptr<const TextureBase> reflective = nullptr;
	double fuzz = 0.0; // 0.0 = perfect mirror
	std::shared_ptr<const TextureBase> glow = nullptr;
};

} // namespace ray
