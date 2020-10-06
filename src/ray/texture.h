#pragma once

#include "ray/types.h"
#include "stb/stb_image.h"

namespace ray {

class TextureBase
{
  public:
	virtual ~TextureBase() {}
	virtual vec3 sample(vec2) const = 0;
	// virtual vec3 operator(double u, double v) const = 0;
};

class Constant : public TextureBase
{
	vec3 color_;

  public:
	explicit Constant(double c) : color_{c, c, c} {}
	explicit Constant(vec3 const &color) : color_{color} {}
	vec3 sample(vec2) const override { return color_; }
};

/** maps [0, 256) to [0.0, 1.0] with gamma correction */
inline double decode_color(uint8_t c)
{
	double x = (double)c / 255.;
	return x * x; // basic gamma correction
}

class Texture2D : public TextureBase
{
	int width_, height_;
	std::vector<vec3> data_;

  public:
	explicit Texture2D(std::string const &filename)
	{
		// load file
		int chan; // color channels of the file. we get stb to convert it to 3
		unsigned char *buf =
		    stbi_load(filename.c_str(), &width_, &height_, &chan, 3);
		if (buf == nullptr)
			throw std::runtime_error("could not load texture file");

		// convert into linear color space
		data_.resize(width_ * height_);
		for (int i = 0; i < width_ * height_; ++i)
		{
			auto r = decode_color(buf[3 * i + 0]);
			auto g = decode_color(buf[3 * i + 1]);
			auto b = decode_color(buf[3 * i + 2]);
			data_[i] = vec3(r, g, b);
		}
		stbi_image_free(buf);
	}

	vec3 sample(vec2 uv) const override
	{
		uv.x *= width_;
		uv.y *= height_;

		// nearest sampling
		int i = (int)floor(uv.x);
		int j = (int)floor(uv.y);
		i = (i % width_ + width_) % width_;
		j = (j % height_ + height_) % height_;
		return data_[j * width_ + i];
	}
};
class TextureCheckerboard : public TextureBase
{
  public:
	TextureCheckerboard() {}
	vec3 sample(vec2 uv) const override
	{
		auto a = (int)(uv.x * 10) + (int)(uv.y * 10);
		return a % 2 == 0 ? vec3(0, 0, 0) : vec3(1, 1, 1);
	}
};

class TextureMandelbrot : public TextureBase
{
	inline static const vec3 colors[] = {vec3{0, 0, 0.2},   vec3{0, 0, 0.4},
	                                     vec3{0, 0.2, 0.4}, vec3{0, 0.4, 0.2},
	                                     vec3{0, 0.4, 0.0}, vec3{0, 0.2, 0},
	                                     vec3{0, 0.2, 0.2}};

  public:
	TextureMandelbrot() {}

	vec3 sample(vec2 uv) const override
	{
		// we use 'vec2' as complex numbers
		auto z = vec2{0, 0};
		for (int iter = 0; iter < 20; ++iter)
		{
			z = vec2(z.x * z.x - z.y * z.y, 2 * z.x * z.y) + uv;
			if (z.x * z.x + z.y * z.y > 4.0)
			{
				return colors[iter % 7];
			}
		}
		return vec3(0, 0, 0);
	}
};

} // namespace ray
