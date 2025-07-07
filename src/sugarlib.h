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

typedef void (*ctx_shader)(drawctx_t*);

typedef void (*pixel_shader)(pixel_t*);

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

void get_pixel(const drawctx_t* ctx, pixel_t* out, int x, int z);

void get_pixel2(const drawctx_t* ctx, pixel_t* out, int pos);

void flush_ctx(drawctx_t* ctx);

drawctx_t* copy_ctx(const drawctx_t* source);

void ctx_over_ctx(drawctx_t* to_change, const drawctx_t overlay, int xo, int zo);

void ctx_sub_ctx(drawctx_t* to_change, const drawctx_t overlay, int xo, int zo);

void apply_ctx_shader(drawctx_t* changed_ctx, ctx_shader shader);

void apply_pix_shader(drawctx_t* source_ctx, pixel_shader shader);
