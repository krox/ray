#include "ray/scene.h"

#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

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

namespace ray {

namespace {

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

Material parse_material(const json &j)
{
	Material mat = {};
	if (j.count("diffuse"))
		mat.diffuse = parse_texture(j["diffuse"]);
	if (j.count("reflective"))
		mat.reflective = parse_texture(j["reflective"]);
	if (j.count("glow"))
		mat.glow = parse_texture(j["glow"]);
	if (j.count("fuzz"))
		mat.fuzz = j["fuzz"].get<double>();
	return mat;
}

std::shared_ptr<Geometry> parse_object(const json &j)
{
	if (j["type"] == "sphere")
	{
		Material mat = parse_material(j["material"]);

		return std::make_shared<Sphere>(j["origin"].get<vec3>(),
		                                j["radius"].get<double>(), mat);
	}
	else if (j["type"] == "plane")
	{
		Material mat = parse_material(j["material"]);
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
