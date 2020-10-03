#include "ray/geometry.h"
#include "ray/types.h"
#include "ray/window.h"
#include "util/random.h"
#include "util/span.h"
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
		if (glm::dot(hit.normal, ray.dir) > 0)
			hit.normal *= -1.0;

		assert(glm::abs(glm::length(hit.normal) - 1.0) < 0.0001);

		vec3 reflect =
		    ray.dir - 2.0 * hit.normal * glm::dot(hit.normal, ray.dir);
		reflect = glm::normalize(reflect);
		auto new_ray = Ray(hit.point, reflect);
		new_ray.origin = new_ray(0.001);

		auto diffuse_ray = Ray(hit.point, hit.normal + random_sphere(rng));
		diffuse_ray.origin = diffuse_ray(0.001);
		return 0.5 * sample(world, diffuse_ray, depth - 1) +
		       0.0 * sample(world, new_ray, -1000) + 0.0 * vec3(1, 1, 1);
	}

	auto t = 0.5 * (glm::normalize(ray.dir).z + 1.0);
	return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
}

int main()
{
	auto image_raw = std::vector<vec3>(640 * 480);
	auto image = util::ndspan<vec3, 2>(image_raw, {480, 640});

	double fov = 3.141592654 * 0.5;
	auto camera = Camera({0, -2, 0.5}, {0, 0, 0.5}, fov, 640. / 480.);

	GeometrySet world = {};
	world.add(std::make_shared<Sphere>(vec3{0.5, 0, 0.5}, 0.5));
	world.add(std::make_shared<Sphere>(vec3{-0.5, 0, 0.8}, 0.5));
	world.add(std::make_shared<Plane>(vec3{0, 0, 0}, vec3{0, 0, 1}));

	int sample_count = 10;

	auto jitter = std::uniform_real_distribution<double>(0., 1.);
	for (int i = 0; i < 480; ++i)
		for (int j = 0; j < 640; ++j)
		{
			vec3 color = vec3{0, 0, 0};
			for (int k = 0; k < sample_count; ++k)
			{
				auto ray = camera.ray((j + jitter(rng)) / 640.,
				                      (i + jitter(rng)) / 480.);
				color += sample(world, ray);
			}

			image(i, j) = color / (double)sample_count;
		}
	show_window(image);

	return 0;
}
