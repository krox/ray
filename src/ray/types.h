#pragma once

/** some ubiquitous types and typedefs */

#include "fmt/format.h"
#include "util/random.h"
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace ray {

using vec3 = glm::dvec3;
using vec2 = glm::dvec2;
using glm::dot, glm::cross, glm::cos, glm::sin, glm::tan;
using RNG = util::xoshiro256;

struct Ray
{
	glm::dvec3 origin, dir;

	glm::dvec3 operator()(double t) const { return origin + t * dir; }

	Ray(glm::dvec3 const &origin, glm::dvec3 const &dir)
	    : origin(origin), dir(dir)
	{}
};

/** random point on unit sphere */
inline vec3 random_sphere(RNG &rng)
{
	vec3 r;
	r.z = std::uniform_real_distribution<double>(-1.0, 1.0)(rng);
	double phi =
	    std::uniform_real_distribution<double>(0.0, 2.0 * 3.141592654)(rng);
	double tmp = glm::sqrt(1 - r.z * r.z);
	r.x = cos(phi) * tmp;
	r.y = sin(phi) * tmp;
	return r;
}

} // namespace ray

template <> struct fmt::formatter<ray::vec3>
{
	constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

	template <typename FormatContext>
	auto format(const ray::vec3 &v, FormatContext &ctx)
	{
		return format_to(ctx.out(), "({:.3} {:.3} {:.3})", v.x, v.y, v.z);
	}
};

template <> struct fmt::formatter<ray::Ray>
{
	constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

	template <typename FormatContext>
	auto format(const ray::Ray &r, FormatContext &ctx)
	{
		return format_to(ctx.out(), "{} -> {}", r.origin, r.dir);
	}
};

namespace nlohmann {

template <> struct adl_serializer<glm::dvec3>
{
	static void to_json(json &j, const glm::dvec3 &v) { j = {v.x, v.y, v.z}; }
	static void from_json(const json &j, glm::dvec3 &v)
	{
		if (!j.is_array() || j.size() != 3)
			throw std::runtime_error("error parsing json to vec3");
		v = glm::dvec3{j[0].get<double>(), j[1].get<double>(),
		               j[2].get<double>()};
	}
};

} // namespace nlohmann
