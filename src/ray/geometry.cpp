#include "ray/geometry.h"

#include <array>
#include <cmath>

namespace ray {

/**
 * Solve x^3 + b x^2 + c x + d = 0.
 * In case of multiple real solutions, returns the largest one.
 * In particular for d > 0 it should always return a positive number.
 * (doesnt work right now due to numerical instabilities close to d=0)
 */
double solve_cubic(double b, double c, double d)
{
	// transform into depressed form: u^3 + p u + q = 0
	double p = c - (1. / 3.) * b * b;
	double q = 1. / 27. * (2. * b * (b * b) - 9. * b * c) + d;

	// the discriminant is -(4p^3+27q^2) = -108*D
	double D = (1. / 27.) * p * p * p + (1. / 4.) * q * q;

	double u;
	if (D > 0)
	{
		// one real root and one complex pair (Cardano's formula)
		double tmp = std::sqrt(D);
		u = std::cbrt(-0.5 * q + tmp) + std::cbrt(-0.5 * q - tmp);
	}
	else
	{

		// three distinct real roots ("casus irreducibilis")
		// can be expressed with complex cube-root, but we do a trigonometric
		// version instead, because
		//   1) I thinks its faster than the general 'std::pow(std::complex)'
		//   2) we are sure to get the largest solution ("principal value")
		auto phi = std::atan2(std::sqrt(-D), -0.5 * q);
		assert(p <= 0);
		u = 2. * std::sqrt(-1 / 3. * p) * std::cos(1. / 3. * phi);
	}

	// note: the D==0 case could be written as
	// if(p == 0) return 0.0;
	// else return {3q/p, -3q/2p, -3q/2p};
	// but both other cases have the same limit, so why bother

	// TODO:
	//   fix the instability for d close to zero, which leads to a root close
	//   to zero. In particular we should make sure that for negative d the
	//   returned root is always positive (important for the quartic solver).

	return u - (1. / 3.) * b;
}

std::array<double, 4> solve_quartic_depressed(double c, double d, double e)
{
	// solve cubic resolvent. It 'should' always return positive numbers
	// (as long as the constant term of the cubic, -d^2, is negative).
	// Due to rounding it can fail in practice in the current implementation.
	double y = solve_cubic(2 * c, c * c - 4. * e, -d * d);
	double z = std::sqrt(y);

	std::array<double, 4> roots = {0. / 0., 0. / 0., 0. / 0., 0. / 0.};
	if (double tmp = -0.5 * d / z - 0.5 * c - 0.25 * y; tmp >= 0)
	{
		tmp = std::sqrt(tmp);
		roots[0] = 0.5 * z + tmp;
		roots[1] = 0.5 * z - tmp;
	}
	if (double tmp = +0.5 * d / z - 0.5 * c - 0.25 * y; tmp >= 0)
	{
		tmp = std::sqrt(tmp);
		roots[0] = -0.5 * z + tmp;
		roots[1] = -0.5 * z - tmp;
	}
	return roots;
}

std::array<double, 4> solve_quartic(double b, double c, double d, double e)
{
	auto alpha = -3. / 8. * (b * b) + c;
	auto beta = 1. / 8. * b * (b * b) - 0.5 * b * c + d;
	auto gamma = -3. / 256. * (b * b) * (b * b) + 1. / 16. * c * (b * b) -
	             0.25 * b * d + e;
	auto sols = solve_quartic_depressed(alpha, beta, gamma);
	for (auto &sol : sols)
		sol -= 0.25 * b;
	return sols;
}

} // namespace ray
