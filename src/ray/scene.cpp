#include "ray/scene.h"

#include <fstream>

namespace ray {

namespace {

std::shared_ptr<Geometry> parse_object(const json &j)
{
	auto mat = Material(j.at("material"));
	std::shared_ptr<Geometry> geom;
	assert(j.count("type"));

	if (j["type"] == "sphere")
	{
		auto radius = j.value<double>("radius", 0.5);
		geom = std::make_shared<Sphere>(radius, mat);
	}
	else if (j["type"] == "torus")
	{
		auto radius = j.value<double>("radius", 0.375);
		auto radius2 = j.value<double>("radius2", 0.125);
		geom = std::make_shared<Torus>(radius, radius2, mat);
	}
	else if (j["type"] == "plane")
	{
		auto normal = j.value<vec3>("normal", {0, 0, 1});
		geom = std::make_shared<Plane>(normal, mat);
	}
	else if (j["type"] == "cylinder")
	{
		auto radius = j.value<double>("radius", 0.5);
		auto height = j.value<double>("height", 1.0);
		geom = std::make_shared<Cylinder>(radius, height, mat);
	}
	else if (j["type"] == "torus_knot")
	{
		auto p = j.at("p").get<int>();
		auto q = j.at("q").get<int>();
		auto n = j.at("n").get<int>();
		auto m = j.at("m").get<int>();
		geom = torus_knot(p, q, n, m, mat);
	}
	else
		assert(false);

	if (j.count("origin"))
		geom->translate(j["origin"].get<vec3>());
	return geom;
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
