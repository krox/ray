#pragma once

#include "ray/texture.h"
#include "ray/types.h"

namespace ray {

class Material
{
	std::shared_ptr<const TextureBase> diffuse_ = nullptr;
	std::shared_ptr<const TextureBase> reflective_ = nullptr;
	double fuzz_ = 0.0; // 0.0 = perfect mirror
	std::shared_ptr<const TextureBase> glow_ = nullptr;

	bool scatter_diffuse(vec3 const &in, vec3 const &normal, vec2 const &uv,
	                     vec3 &out, vec3 &attenuation, RNG &rng) const;
	bool scatter_reflective(vec3 const &in, vec3 const &normal, vec2 const &uv,
	                        vec3 &out, vec3 &attenuation, RNG &rng) const;

  public:
	explicit Material(json const &j);
	vec3 glow(vec3 const &in, vec3 const &normal, vec2 const &uv) const;
	bool scatter(vec3 const &in, vec3 const &normal, vec2 const &uv, vec3 &out,
	             vec3 &attenuation, RNG &rng) const;
};

} // namespace ray
