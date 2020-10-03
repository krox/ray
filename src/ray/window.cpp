#include "ray/window.h"

#include <SDL2/SDL.h>

namespace ray {

namespace {

/** maps [0.0, 1.0] to [0, 256) */
uint8_t convert_color(double x)
{
	if (x < 0)
		return 0;
	if (x >= 1.0)
		return 255;
	x = glm::sqrt(x); // basic gamma-correction
	return (uint8_t)(x * 255.99);
}

/** convert to SDL "ARGB8888" format */
uint32_t convert_color(vec3 const &col)
{
	uint32_t r = 0;
	r |= (uint32_t)255 << 24;
	r |= (uint32_t)convert_color(col.x) << 16;
	r |= (uint32_t)convert_color(col.y) << 8;
	r |= (uint32_t)convert_color(col.z) << 0;
	return r;
}

} // namespace

void show_window(util::ndspan<const vec3, 2> image)
{
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window *window = SDL_CreateWindow("Image", SDL_WINDOWPOS_UNDEFINED,
	                                      SDL_WINDOWPOS_UNDEFINED,
	                                      image.shape(1), image.shape(0), 0);

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
	                                         SDL_TEXTUREACCESS_STATIC,
	                                         image.shape(1), image.shape(0));
	auto pixels = std::vector<uint32_t>(image.size());
	for (size_t i = 0; i < image.shape(0); ++i)
		for (size_t j = 0; j < image.shape(1); ++j)
			pixels[i * image.shape(1) + j] = convert_color(image(i, j));

	for (bool quit = false; !quit;)
	{
		SDL_UpdateTexture(texture, NULL, pixels.data(),
		                  image.shape(1) * sizeof(uint32_t));

		SDL_Event event;
		SDL_WaitEvent(&event);

		if (event.type == SDL_QUIT)
			quit = true;

		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);

	SDL_DestroyWindow(window);
	SDL_Quit();
}

} // namespace ray
