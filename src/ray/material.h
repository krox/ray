#pragma once

#include "ray/types.h"

namespace ray {

class TextureBase
{
  public:
	virtual ~TextureBase() {}
	virtual vec3 sample() const = 0;
	// virtual vec3 operator(double u, double v) const = 0;
};

class Constant : public TextureBase
{
	vec3 color_;

  public:
	Constant(vec3 const &color) : color_(color) {}
	vec3 sample() const override { return color_; }
};

struct Material
{
	std::shared_ptr<const TextureBase> diffuse = nullptr;
	std::shared_ptr<const TextureBase> reflective = nullptr;
	std::shared_ptr<const TextureBase> glow = nullptr;
};

} // namespace ray
