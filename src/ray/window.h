#pragma once

#include "ray/types.h"

#include "util/span.h"
#include <SDL2/SDL.h>
#include <atomic>
#include <thread>

namespace ray {

namespace {

/** maps [0.0, 1.0] to [0, 256) */
inline uint8_t convert_color(double x)
{
	if (x < 0)
		return 0;
	if (x >= 1.0)
		return 255;
	x = glm::sqrt(x); // basic gamma-correction
	return (uint8_t)(x * 255.99);
}

/** convert to SDL "ARGB8888" format */
inline uint32_t convert_color(vec3 const &col)
{
	uint32_t r = 0;
	r |= (uint32_t)255 << 24;
	r |= (uint32_t)convert_color(col.x) << 16;
	r |= (uint32_t)convert_color(col.y) << 8;
	r |= (uint32_t)convert_color(col.z) << 0;
	return r;
}

} // namespace

class Window
{
	std::string title_;
	int width_, height_;

	std::thread thread;

	void run()
	{
		SDL_Init(SDL_INIT_VIDEO);

		SDL_Window *window =
		    SDL_CreateWindow(title_.c_str(), SDL_WINDOWPOS_UNDEFINED,
		                     SDL_WINDOWPOS_UNDEFINED, width_, height_, 0);
		SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
		SDL_Texture *texture =
		    SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
		                      SDL_TEXTUREACCESS_STATIC, width_, height_);

		auto pixels = std::vector<uint32_t>(width_ * height_, 0);
		SDL_UpdateTexture(texture, NULL, pixels.data(),
		                  width_ * sizeof(uint32_t));
		while (!quit)
		{
			SDL_Event event;
			SDL_WaitEvent(&event);

			if (event.type == SDL_QUIT)
				quit = true;

			if (event.type == SDL_USEREVENT)
			{
				vec3 *buf = (vec3 *)event.user.data1;
				for (int i = 0; i < width_ * height_; ++i)
					pixels[i] = convert_color(buf[i]);
				delete[] buf;
				SDL_UpdateTexture(texture, nullptr, pixels.data(),
				                  width_ * sizeof(uint32_t));
			}

			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);
		}

		SDL_DestroyTexture(texture);
		SDL_DestroyRenderer(renderer);

		SDL_DestroyWindow(window);
		SDL_Quit();
	}

  public:
	std::atomic<bool> quit = false;

	Window(std::string title, int width, int height)
	    : title_(title), width_(width), height_(height),
	      thread([this] { run(); })
	{}

	void update(util::ndspan<const vec3, 2> image, double scale = 1.0)
	{
		assert((int)image.shape(0) == height_ && (int)image.shape(1) == width_);
		vec3 *buf = new vec3[image.size()];
		for (size_t i = 0; i < image.shape(0); ++i)
			for (size_t j = 0; j < image.shape(1); ++j)
				buf[i * image.shape(1) + j] = image(i, j) * scale;
		SDL_Event event;
		event.type = SDL_USEREVENT;
		event.user.data1 = buf;
		SDL_PushEvent(&event);
	}

	void close()
	{
		SDL_Event event;
		event.type = SDL_QUIT;
		SDL_PushEvent(&event);
	}

	void join() { thread.join(); }
};

} // namespace ray
