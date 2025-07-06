#pragma once

#include <stddef.h>
#include <stdbool.h>

void* smalloc(const size_t bytes);

bool is_color_invalid(int r, int g, int b);

struct {
	int r, g, b;
} typedef color_t;

struct {
	int x;
	int z;
	color_t fg;
	color_t bg;
	char print;
	bool bg_null;
	bool fg_null;
} typedef pixel_t;

struct {
	pixel_t* pixels;
	int width;  // X
	int height; // Z
	bool initialized;
} typedef drawctx_t;

drawctx_t* make_drawctx(int width, int height);

void fill_background(drawctx_t* ctx);

void free_drawctx(drawctx_t* ctx);

pixel_t make_pixel(int x, int z); 

pixel_t p_add_fg(pixel_t pixel, int r, int g, int b);

pixel_t p_add_bg(pixel_t pixel, int r, int g, int b);

pixel_t p_set_print(pixel_t pixel, char to_print);

void add_fg(pixel_t* pixel, int r, int g, int b);

void add_bg(pixel_t* pixel, int r, int g, int b);

void set_print(pixel_t* pixel, char to_print);

void free_pixel(pixel_t* pixel);

void set_pixel(drawctx_t* ctx, pixel_t pixel);

void flush_ctx(drawctx_t* ctx, bool clear);

drawctx_t* copy_ctx(const drawctx_t* source);
