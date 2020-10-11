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
	Material material_;
	mat3 rot_;     // model -> world
	mat3 rot_inv_; // world -> model
	vec3 origin_;

	virtual bool intersect_internal(Ray const &ray, Hit &hit) const = 0;

  public:
	Geometry(Material const &material)
	    : material_(material), rot_{1.0}, rot_inv_{1.0}, origin_{0.0, 0.0, 0.0}
	{}

	virtual ~Geometry(){};

	bool intersect(Ray const &ray, Hit &hit) const
	{
		// transform ray from world-space to model-space
		auto ray_local =
		    Ray(rot_inv_ * (ray.origin - origin_), rot_inv_ * ray.dir);

		if (intersect_internal(ray_local, hit))
		{
			// transform hit from model-space to world-space
			hit.point = rot_ * hit.point + origin_;
			hit.normal = glm::normalize(glm::transpose(rot_inv_) * hit.normal);
			hit.material = &material_;
			return true;
		}
		return false;
	}

	void translate(vec3 const &offset) { origin_ += offset; }
	void rotatex(double alpha)
	{
		auto rot = mat3(1.0);
		rot[1][1] = std::cos(alpha);
		rot[1][2] = std::sin(alpha);
		rot[2][1] = -std::sin(alpha);
		rot[2][2] = std::cos(alpha);
		rot_ = rot * rot_;
		rot_inv_ = glm::inverse(rot_);
	}
	void rotatey(double alpha)
	{
		auto rot = mat3(1.0);
		rot[0][0] = std::cos(alpha);
		rot[0][2] = -std::sin(alpha);
		rot[2][0] = std::sin(alpha);
		rot[2][2] = std::cos(alpha);
		rot_ = rot * rot_;
		rot_inv_ = glm::inverse(rot_);
	}
	void rotatez(double alpha)
	{
		auto rot = mat3(1.0);
		rot[0][0] = std::cos(alpha);
		rot[0][1] = std::sin(alpha);
		rot[1][0] = -std::sin(alpha);
		rot[1][1] = std::cos(alpha);
		rot_ = rot * rot_;
		rot_inv_ = glm::inverse(rot_);
	}
};

class Sphere : public Geometry
{
	double radius_;

  public:
	Sphere(double radius, Material const &material)
	    : Geometry(material), radius_(radius)
	{}

	bool intersect_internal(Ray const &ray, Hit &hit) const override
	{
		// create equation in the form a*t^2 + 2*b*t + c = 0
		auto a = glm::dot(ray.dir, ray.dir);
		auto b = glm::dot(ray.origin, ray.dir);
		auto c = glm::dot(ray.origin, ray.origin) - radius_ * radius_;
		auto d = b * b - a * c;

		if (d < 0)
			return false;

		double t = (-b - glm::sqrt(d)) / a;
		if (t <= 0 || t > hit.t)
			return false;

		hit.t = t;
		hit.point = ray(hit.t);
		hit.normal = glm::normalize(hit.point);
		return true;
	}
};

class Cylinder : public Geometry
{
	double radius_;
	double height_;

  public:
	Cylinder(double radius, double height, Material const &material)
	    : Geometry(material), radius_(radius), height_(height)
	{}

	bool intersect_internal(Ray const &ray, Hit &hit) const override
	{
		// create equation in the form a*t^2 + 2*b*t + c = 0
		auto oc_xy = vec2(ray.origin.x, ray.origin.y);
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
		if (p.z < 0 || p.z > height_)
			return false;

		hit.t = t;
		hit.point = p;
		hit.normal = glm::normalize(vec3{p.x, p.y, 0.});
		return true;
	}
};

std::array<double, 4> solve_quartic(double b, double c, double d, double e);

class Torus : public Geometry
{
	double radius_;  // R
	double radius2_; // r
	double R2_, r2_, xi_;

  public:
	Torus(double radius, double radius2, Material const &material)
	    : Geometry(material), radius_(radius), radius2_(radius2)
	{
		R2_ = radius_ * radius_;
		r2_ = radius2_ * radius2_;
		xi_ = R2_ - r2_;
	}

	bool intersect_internal(Ray const &ray, Hit &hit) const override
	{
		// create equation in the form a*t^4 + b*t^3 + c*t^2 + d*t + c = 0
		auto alpha = glm::dot(ray.dir, ray.dir);
		auto beta = glm::dot(ray.origin, ray.dir);
		auto sigma = glm::dot(ray.origin, ray.origin) - xi_;
		auto a = alpha * alpha;
		auto b = 4. * alpha * beta;
		auto c = 2. * alpha * sigma + 4. * beta * beta +
		         4. * R2_ * ray.dir.z * ray.dir.z;
		auto d = 4. * beta * sigma + 8. * R2_ * ray.origin.z * ray.dir.z;
		auto e = sigma * sigma - 4. * R2_ * (r2_ - ray.origin.z * ray.origin.z);

		std::array<double, 4> sols = solve_quartic(b / a, c / a, d / a, e / a);
		double t = 0.0 / 0.0;
		for (double sol : sols)
			if (sol > 0 && sol < hit.t && !(sol > t))
				t = sol;
		if (!(t == t))
			return false;

		hit.t = t;
		hit.point = ray(t);

		auto ss = glm::dot(hit.point, hit.point);
		auto tmp = vec3(ss - xi_, ss - xi_, ss - xi_ + 2 * radius_ * radius2_);
		hit.normal = glm::normalize(hit.point * tmp);
		return true;
	}
};

class Plane : public Geometry
{
	vec3 normal_;

  public:
	Plane(vec3 const &normal, Material const &material)
	    : Geometry(material), normal_(normal)
	{}

	bool intersect_internal(Ray const &ray, Hit &hit) const override
	{
		double t = -glm::dot(ray.origin, normal_) / glm::dot(ray.dir, normal_);
		if (t <= 0 || t > hit.t)
			return false;

		hit.t = t;
		hit.point = ray(t);
		hit.normal = normal_;
		hit.uv = vec2(hit.point.x, hit.point.y);
		return true;
	}
};

class GeometrySet
{
	std::vector<std::shared_ptr<const Geometry>> objects_;

  public:
	GeometrySet() {}
	void add(std::shared_ptr<const Geometry> geom)
	{
		assert(geom);
		objects_.push_back(std::move(geom));
	}

	bool intersect(Ray const &ray, Hit &hit) const
	{
		bool r = false;
		for (auto &obj : objects_)
			r |= obj->intersect(ray, hit);
		return r;
	}
}; // namespace ray

} // namespace ray
