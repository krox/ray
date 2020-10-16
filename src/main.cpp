#include "CLI/CLI.hpp"
#include "ray/geometry.h"
#include "ray/image.h"
#include "ray/scene.h"
#include "ray/types.h"
#include "ray/window.h"
#include "util/random.h"
#include "util/span.h"
#include "util/stopwatch.h"
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
int64_t ray_count = 0; // total number of rays shot

/** take a single color sample */
vec3 sample(GeometrySet const &world, Ray const &ray, vec3 attenuation,
            int depth)
{
	if (depth < 0)
		return vec3{0, 0, 0};

	if (glm::length(attenuation) < 1.)
	{
		if (std::bernoulli_distribution(glm::length(attenuation))(rng))
			attenuation /= glm::length(attenuation);
		else
			return {0, 0, 0};
	}

	ray_count += 1;

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

		vec3 att;
		vec3 new_dir;
		if (mat.scatter_diffuse(ray.dir, hit.normal, hit.uv, new_dir, att, rng))
		{
			auto new_ray = Ray(hit.point, new_dir);
			color += sample(world, new_ray, attenuation * att, depth - 1);
		}
		if (mat.scatter_reflective(ray.dir, hit.normal, hit.uv, new_dir, att,
		                           rng))
		{
			auto new_ray = Ray(hit.point, new_dir);
			color += sample(world, new_ray, attenuation * att, depth - 1);
		}
		return color * attenuation;
	}

	return {0, 0, 0};
	// auto t = 0.5 * (glm::normalize(ray.dir).z + 1.0);
	// return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
}

int main(int argc, char *argv[])
{
	util::Stopwatch sw_setup, sw_display, sw_tracer, sw_total;

	sw_total.start();
	sw_setup.start();

	std::string scene_filename;
	std::string output_filename = "";
	int sample_count = 100;
	int width = 640, height = 480;

	CLI::App app{"ray tracer"};
	app.add_option("scene", scene_filename, "scene file in json format")
	    ->required();
	app.add_option("--samples", sample_count, "samples per pixel");
	app.add_option("--width", width, "width in pixels");
	app.add_option("--height", height, "height in pixels");
	app.add_option("-o", output_filename,
	               "output image file. Supported formats: png, bmp, tga, jpg");
	CLI11_PARSE(app, argc, argv);

	auto image_raw = std::vector<vec3>(width * height, vec3{0, 0, 0});
	auto imageSq_raw = std::vector<vec3>(width * height, vec3{0, 0, 0});
	auto image =
	    util::ndspan<vec3, 2>(image_raw, {(size_t)height, (size_t)width});
	auto imageSq =
	    util::ndspan<vec3, 2>(imageSq_raw, {(size_t)height, (size_t)width});

	double fov = 3.141592654 * 0.5;
	auto camera =
	    Camera({0, -2, 0.5}, {0, 0, 0.5}, fov, (double)width / height);

	auto world = load_scene(scene_filename);

	auto jitter = std::uniform_real_distribution<double>(0., 1.);

	auto window = Window("Result", width, height);

	sw_setup.stop();

	for (int sample_iter = 1; sample_iter <= sample_count && !window.quit;
	     ++sample_iter)
	{
		sw_tracer.start();
		for (int i = 0; i < height; ++i)
			for (int j = 0; j < width; ++j)
			{
				auto ray = camera.ray((j + jitter(rng)) / width,
				                      (i + jitter(rng)) / height);
				vec3 color = sample(world, ray, vec3(1, 1, 1), 10);
				image(i, j) += color;
				imageSq(i, j) += color * color;
			}
		sw_tracer.stop();
		sw_display.start();
		window.update(image, 1. / sample_iter);
		sw_display.stop();

		fmt::print("{} / {}\r", sample_iter, sample_count);
		std::cout.flush();
	}

	image /= (double)sample_count;
	imageSq /= (double)sample_count;

	double noise_sum = 0;
	double noise_max = 0;
	for (int i = 0; i < height; ++i)
		for (int j = 0; j < width; ++j)
			for (int c = 0; c < 3; ++c)
			{
				double noise =
				    imageSq(i, j)[c] - image(i, j)[c] * image(i, j)[c];
				noise /= std::sqrt((double)sample_count);
				noise_sum += noise;
				noise_max = std::max(noise_max, noise);
			}

	if (output_filename.size())
		write_image(output_filename, image, 2.2);

	sw_total.stop();
	fmt::print("\nall done\n");
	fmt::print("--------------- statistics ---------------\n");
	fmt::print("rays total      = {}\n", ray_count);
	fmt::print("rays per pixel  = {:.3f}\n",
	           (double)ray_count / (width * height));
	fmt::print("rays per sample = {:.3f}\n",
	           (double)ray_count / ((double)width * height * sample_count));
	fmt::print("rays per second = {:.3f} M\n",
	           ray_count / sw_tracer.secs() / 1000000.);
	fmt::print("noise = {:.0f} ppm avg, {:.0f} ppm max\n",
	           noise_sum / (3 * width * height) * 1e6, noise_max * 1e6);
	fmt::print("---------------   timing   ---------------\n");
	fmt::print("setup   = {:.3f} s ({:#4.1f} %)\n", sw_setup.secs(),
	           sw_setup.secs() / sw_total.secs() * 100);
	fmt::print("tracer  = {:.3f} s ({:#4.1f} %)\n", sw_tracer.secs(),
	           sw_tracer.secs() / sw_total.secs() * 100);
	fmt::print("display = {:.3f} s ({:#4.1f} %%)\n", sw_display.secs(),
	           sw_display.secs() / sw_total.secs() * 100);
	fmt::print("total   = {:.3f} s\n", sw_total.secs());

	window.join();
	return 0;
}
