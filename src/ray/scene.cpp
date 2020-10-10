#include "ray/scene.h"

#include <fstream>

namespace ray {

namespace {

std::shared_ptr<Geometry> parse_object(const json &j)
{
	if (j["type"] == "sphere")
	{
		auto mat = Material(j.at("material"));

		return std::make_shared<Sphere>(j["origin"].get<vec3>(),
		                                j["radius"].get<double>(), mat);
	}
	if (j["type"] == "torus")
	{
		auto mat = Material(j.at("material"));
		return std::make_shared<Torus>(j.value<vec3>("origin", {0, 0, 0}),
		                               j.value<double>("radius", 1.0),
		                               j.value("radius2", 0.5), mat);
	}
	else if (j["type"] == "plane")
	{
		auto mat = Material(j.at("material"));
		return std::make_shared<Plane>(j["origin"].get<vec3>(),

		                               j["normal"].get<vec3>(), mat);
	}

	return nullptr;
}
} // namespace

GeometrySet load_scene(std::string const &filename)
{
	std::ifstream file(filename);
	json j;
	file >> j;

	GeometrySet world;
	for (auto const &obj : j["objects"])
	{
		auto geom = parse_object(obj);
		if (!geom)
			continue;

		world.add(geom);
	}
	return world;
}
} // namespace ray
