#pragma once

#include "ray/types.h"
#include <memory>

namespace ray {

struct Hit
{
	double t;
	vec3 point, normal;
};

class Geometry
{
  public:
	virtual ~Geometry(){};
	virtual bool intersect(Ray const &ray, Hit &hit) const = 0;
};

class Sphere : public Geometry
{
	vec3 center_;
	double radius_;

  public:
	Sphere(vec3 center, double radius) : center_(center), radius_(radius) {}

	bool intersect(Ray const &ray, Hit &hit) const override
	{
		// create equation in the form a*t^2 + 2*b*t + c = 0
		vec3 oc = ray.origin - center_;
		auto a = glm::dot(ray.dir, ray.dir);
		auto b = glm::dot(oc, ray.dir);
		auto c = glm::dot(oc, oc) - radius_ * radius_;
		auto d = b * b - a * c;

		if (d < 0)
			return false;

		double t = (-b - glm::sqrt(d)) / a;
		if (t <= 0 || t > hit.t)
			return false;

		hit.t = t;
		hit.point = ray(hit.t);
		hit.normal = glm::normalize(hit.point - center_);
		return true;
	}
};

class Plane : public Geometry
{
	vec3 origin_, normal_;

  public:
	Plane(vec3 origin, vec3 const &normal) : origin_(origin), normal_(normal) {}

	bool intersect(Ray const &ray, Hit &hit) const override
	{
		double t = glm::dot(origin_ - ray.origin, normal_) /
		           glm::dot(ray.dir, normal_);
		if (t <= 0 || t > hit.t)
			return false;

		hit.t = t;
		hit.point = ray(t);
		hit.normal = normal_;
		return true;
	}
};

class GeometrySet : public Geometry
{
	std::vector<std::shared_ptr<const Geometry>> objects_;

  public:
	GeometrySet() {}
	void add(std::shared_ptr<const Geometry> geom)
	{
		assert(geom);
		objects_.push_back(std::move(geom));
	}

	bool intersect(Ray const &ray, Hit &hit) const override
	{
		bool r = false;
		for (auto &obj : objects_)
			r |= obj->intersect(ray, hit);
		return r;
	}
};

} // namespace ray
