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

		vec3 color = mat.glow(ray.dir, hit.normal, hit.uv);

		vec3 attenuation;
		vec3 new_dir;
		if (mat.scatter(ray.dir, hit.normal, hit.uv, new_dir, attenuation, rng))
		{
			auto new_ray = Ray(hit.point, new_dir);
			color += attenuation * sample(world, new_ray, depth - 1);
		}
		return color;
	}

	return {0, 0, 0};
	// auto t = 0.5 * (glm::normalize(ray.dir).z + 1.0);
	// return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
}

int main(int argc, char *argv[])
{
	auto image_raw = std::vector<vec3>(640 * 480, vec3{0, 0, 0});
	auto image = util::ndspan<vec3, 2>(image_raw, {480, 640});

	double fov = 3.141592654 * 0.5;
	auto camera = Camera({0, -2, 0.5}, {0, 0, 0.5}, fov, 640. / 480.);

	if (argc != 2)
	{
		fmt::print("ERROR: usage: {} <scene-file>\n", argv[0]);
		return -1;
	}
	auto world = load_scene(argv[1]);

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
