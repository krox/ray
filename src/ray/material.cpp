#include "ray/material.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace ray {

std::shared_ptr<TextureBase> parse_texture(json const &j)
{
	if (j.is_number())
		return std::make_shared<Constant>(j.get<double>());
	else if (j.is_array())
		return std::make_shared<Constant>(j.get<vec3>());
	else if (j.is_string())
		return std::make_shared<Texture2D>(j.get<std::string>());
	else
		assert(false);
}

Material::Material(json const &j)
{
	if (j.count("diffuse"))
		diffuse_ = parse_texture(j["diffuse"]);
	if (j.count("reflective"))
		reflective_ = parse_texture(j["reflective"]);
	if (j.count("glow"))
		glow_ = parse_texture(j["glow"]);
	if (j.count("fuzz"))
		fuzz_ = j["fuzz"].get<double>();
}

vec3 Material::glow(vec3 const &in, vec3 const &normal, vec2 const &uv) const
{
	(void)in;
	(void)normal;
	if (glow_ == nullptr)
		return {0, 0, 0};
	return glow_->sample(uv);
}

bool Material::scatter_diffuse(vec3 const &in, vec3 const &normal,
                               vec2 const &uv, vec3 &out, vec3 &attenuation,
                               RNG &rng) const
{
	(void)in;
	if (!diffuse_)
		return false;
	out = util::normalize(normal + random_sphere(rng));
	attenuation = diffuse_->sample(uv);
	return true;
}

bool Material::scatter_reflective(vec3 const &in, vec3 const &normal,
                                  vec2 const &uv, vec3 &out, vec3 &attenuation,
                                  RNG &rng) const
{
	if (!reflective_)
		return false;

	out = util::normalize(util::reflect(in, normal));
	out += fuzz_ * random_sphere(rng);
	out = util::normalize(out);

	if (util::dot(out, normal) <= 0)
		return false;

	attenuation = reflective_->sample(uv);
	return true;
}

} // namespace ray
