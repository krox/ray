#include "ray/geometry.h"
#include "ray/scene.h"
#include "ray/types.h"
#include "ray/window.h"
#include "util/random.h"
#include "util/span.h"
#include <iostream>
#include <limits>
#include <memory>
#include <random>

using namespace ray;

class Camera
{
	vec3 origin_;
	vec3 corner_, right_, down_;

  public:
	Camera(vec3 origin, vec3 at, double fov, double aspect) : origin_(origin)
	{
		auto dir = glm::normalize(at - origin);
		right_ = glm::normalize(glm::cross(dir, vec3(0, 0, 1)));
		right_ *= 2.0 * glm::tan(fov / 2.0);
		down_ = glm::normalize(cross(dir, right_));
		down_ *= glm::length(right_) / aspect;
		corner_ = dir - 0.5 * down_ - 0.5 * right_;
	}

	Ray ray(double x, double y) const
	{
		return Ray(origin_, corner_ + x * right_ + y * down_);
	}
};

RNG rng = {};

/** take a single color sample */
vec3 sample(Geometry const &world, Ray const &ray, int depth = 10)
{
	if (depth < 0)
		return vec3{0, 0, 0};

	Hit hit;
	hit.t = std::numeric_limits<double>::infinity();
	if (world.intersect(ray, hit))
	{
		if (glm::dot(hit.normal, ray.dir) > 0) // should never happen (?)
			hit.normal *= -1.0;
		assert(glm::abs(glm::length(hit.normal) - 1.0) < 0.0001);
		if (hit.material == nullptr)
			return vec3{1, 0, 1};
		assert(hit.material != nullptr);
		auto &mat = *hit.material;

		vec3 color = {0, 0, 0};
		if (mat.glow)
			color += mat.glow->sample(hit.uv);
		if (mat.diffuse)
		{
			auto new_ray = Ray(hit.point, hit.normal + random_sphere(rng));
			new_ray.origin = new_ray(0.001);
			auto albedo = mat.diffuse->sample(hit.uv);
			color += albedo * sample(world, new_ray, depth - 1);
		}
		if (mat.reflective)
		{
			auto new_ray = Ray(
			    hit.point, glm::normalize(glm::reflect(ray.dir, hit.normal)) +
			                   mat.fuzz * random_sphere(rng));
			if (glm::dot(new_ray.dir, hit.normal) > 0)
			{
				new_ray.origin = new_ray(0.001);
				auto albedo = mat.reflective->sample(hit.uv);
				color += albedo * sample(world, new_ray, depth - 1);
			}
		}
		return color;
	}

	return {0, 0, 0};
	// auto t = 0.5 * (glm::normalize(ray.dir).z + 1.0);
	// return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
}

int main()
{
	auto image_raw = std::vector<vec3>(640 * 480, vec3{0, 0, 0});
	auto image = util::ndspan<vec3, 2>(image_raw, {480, 640});

	double fov = 3.141592654 * 0.5;
	auto camera = Camera({0, -2, 0.5}, {0, 0, 0.5}, fov, 640. / 480.);

	// auto world = build_scene();
	auto world = load_scene("../assets/scenes/test.json");

	auto jitter = std::uniform_real_distribution<double>(0., 1.);

	auto window = Window("Result", 640, 480);

	int sample_count = 100;
	for (int sample_iter = 1; sample_iter <= sample_count && !window.quit;
	     ++sample_iter)
	{
		for (int i = 0; i < 480; ++i)
			for (int j = 0; j < 640; ++j)
			{
				auto ray = camera.ray((j + jitter(rng)) / 640.,
				                      (i + jitter(rng)) / 480.);
				image(i, j) += sample(world, ray);
			}
		window.update(image, 1. / sample_iter);

		fmt::print("{} / {}\r", sample_iter, sample_count);
		std::cout.flush();
	}
	fmt::print("\nall done\n");

	window.join();
	return 0;
}
