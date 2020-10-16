#include "ray/image.h"

#include "stb/stb_image_write.h"

namespace ray {

namespace {}

void write_image(std::string const &filename, util::ndspan<vec3, 2> image,
                 double gamma)
{
	(void)gamma;
	auto ending =
	    filename.size() >= 4 ? filename.substr(filename.size() - 4) : "";
	auto height = (int)image.shape(0);
	auto width = (int)image.shape(1);
	std::vector<uint8_t> buf;
	buf.resize(image.size() * 3);
	for (int i = 0; i < image.shape(0); ++i)
		for (int j = 0; j < image.shape(1); ++j)
			for (int c = 0; c < 3; ++c)
			{
				auto tmp = image(i, j)[c];
				tmp = std::pow(tmp, 1. / gamma);
				uint8_t color;
				if (!(tmp > 0))
					color = 0;
				else if (tmp >= 1)
					color = 255;
				else
					color = (uint8_t)(tmp * 256.);
				buf[i * image.shape(1) * 3 + j * 3 + c] = color;
			}

	int r = 0;
	if (ending == ".png")
		r = stbi_write_png(filename.c_str(), width, height, 3, buf.data(),
		                   width * 3);
	else if (ending == ".bmp")
		r = stbi_write_bmp(filename.c_str(), width, height, 3, buf.data());
	else if (ending == ".tga")
		r = stbi_write_bmp(filename.c_str(), width, height, 3, buf.data());
	else if (ending == ".jpg")
		r = stbi_write_jpg(filename.c_str(), width, height, 3, buf.data(), 95);
	else
		throw std::runtime_error("unknown image file extension");

	if (r == 0)
		throw std::runtime_error("could not write image file");
}

} // namespace ray
