#pragma once

#include "ray/material.h"
#include "ray/types.h"
#include <memory>

namespace ray {

struct Hit
{
	double t;
	vec3 point, normal;
	vec2 uv;
	Material const *material = nullptr;
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
	Material material_;

  public:
	Sphere(vec3 center, double radius, Material const &material)
	    : center_(center), radius_(radius), material_(material)
	{}

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
		hit.material = &material_;
		return true;
	}
};

class Cylinder : public Geometry
{
	vec3 origin_;
	double radius_;
	double height_;
	Material material_;

  public:
	Cylinder(vec3 const &origin, double radius, double height,
	         Material const &material)
	    : origin_(origin), radius_(radius), height_(height), material_(material)
	{}

	bool intersect(Ray const &ray, Hit &hit) const override
	{
		// create equation in the form a*t^2 + 2*b*t + c = 0
		vec3 oc = ray.origin - origin_;
		auto oc_xy = vec2(oc.x, oc.y);
		auto dir_xy = vec2(ray.dir.x, ray.dir.y);
		auto a = glm::dot(dir_xy, dir_xy);
		auto b = glm::dot(oc_xy, dir_xy);
		auto c = glm::dot(oc_xy, oc_xy) - radius_ * radius_;
		auto d = b * b - a * c;

		if (d < 0)
			return false; // miss infinite cylinder

		double t = (-b - glm::sqrt(d)) / a;

		// point not in relevant ray segment
		if (t <= 0 || t > hit.t)
			return false;

		vec3 p = ray(t);
		auto po = p - origin_;
		if (po.z < 0 || po.z > height_)
			return false;

		hit.t = t;
		hit.point = p;
		hit.normal = glm::normalize(vec3{po.x, po.y, 0.});
		hit.material = &material_;
		return true;
	}
};

class Plane : public Geometry
{
	vec3 origin_, normal_;
	Material material_;

  public:
	Plane(vec3 origin, vec3 const &normal, Material const &material)
	    : origin_(origin), normal_(normal), material_(material)
	{}

	bool intersect(Ray const &ray, Hit &hit) const override
	{
		double t = glm::dot(origin_ - ray.origin, normal_) /
		           glm::dot(ray.dir, normal_);
		if (t <= 0 || t > hit.t)
			return false;

		hit.t = t;
		hit.point = ray(t);
		hit.normal = normal_;
		hit.uv = vec2(hit.point.x, hit.point.y);
		hit.material = &material_;
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
