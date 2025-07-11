#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <wchar.h>

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
	wchar_t print;
	bool renderable;
	bool bg_null;
	bool fg_null;
} typedef pixel_t;

struct {
	color_t fg;
	color_t bg;
	bool bg_null;
	bool fg_null;
} typedef pixel_template_t;

struct {
	char ch;
	color_t color;
} typedef bound_color_t;

struct {
	bound_color_t* pixel_table;
	size_t table_size;
	int last_added;

} typedef color_table_t;

struct {
	char* description;
	color_table_t bound_table;	
	int width;
	int height;
} typedef sprite_t;

struct {
	pixel_t* pixels;
	int width;  // X
	int height; // Z
	bool initialized;
} typedef drawctx_t;

enum {
	TOP_LEFT,
	TOP,
	TOP_RIGHT,
	CENTER_LEFT,
	CENTER,
	CENTER_RIGHT,
	BOTTOM_LEFT,
	BOTTOM,
	BOTTOM_RIGHT
} typedef alignment_t;

typedef void (*ctx_shader)(drawctx_t*);

typedef void (*pixel_shader)(pixel_t*);

typedef void (*flush_ctx_f)(const drawctx_t*);

color_table_t make_color_table(size_t records);

void add_line(const char* line, sprite_t* target);

sprite_t make_sprite(color_table_t table);

drawctx_t* make_drawctx(int width, int height);

void add_record(color_table_t* table, char ch, color_t color);

bool get_record(const color_table_t* table, char ch, color_t* out);

drawctx_t* to_ctx(const sprite_t* source);

void fill_background(drawctx_t* ctx);

void fill_with(drawctx_t* ctx, color_t color, int xo, int zo, int xw, int zh);

void str_to_ctx(drawctx_t* ctx, const wchar_t* text, pixel_template_t p_template, bool line_wrapping, int xo, int zo);

void free_drawctx(drawctx_t* ctx);

pixel_t make_pixel(int x, int z); 

pixel_t p_add_fg(pixel_t pixel, int r, int g, int b);

pixel_t p_add_bg(pixel_t pixel, int r, int g, int b);

pixel_t p_set_print(pixel_t pixel, wchar_t to_print);

pixel_t p_set_pos(pixel_t pixel, int x, int z);

void set_pos(pixel_t* pixel, int x, int z);

void add_fg(pixel_t* pixel, int r, int g, int b);

void add_bg(pixel_t* pixel, int r, int g, int b);

void set_print(pixel_t* pixel, wchar_t to_print);

void free_pixel(pixel_t* pixel);

void set_pixel(drawctx_t* ctx, pixel_t pixel);

bool get_pixel(const drawctx_t* ctx, pixel_t* out, int x, int z);

bool get_pixel2(const drawctx_t* ctx, pixel_t* out, int pos);

void flush_ctx(const drawctx_t* ctx);

void flush_compact_ctx(const drawctx_t* ctx);

void flush_ctx_offset(const drawctx_t* ctx, flush_ctx_f flush_func, int xo, int zo);

drawctx_t* copy_ctx(const drawctx_t* source);

pixel_t copy_pixel(const pixel_t* source);

void flush_aligned_ctx(drawctx_t* to_change, flush_ctx_f flush_func, alignment_t alignment, int screen_width, int screen_height);

void ctx_over_ctx(drawctx_t* to_change, const drawctx_t overlay, int xo, int zo);

void ctx_sub_ctx(drawctx_t* to_change, const drawctx_t overlay, int xo, int zo);

void ctx_mask_ctx(drawctx_t* to_change, const drawctx_t mask, int xo, int zo);

drawctx_t* resize_ctx(drawctx_t* to_resize, int nx, int nz);

drawctx_t* crop_ctx(drawctx_t* to_crop, int sx, int sz, int ex, int ez);

void apply_ctx_shader(drawctx_t* changed_ctx, ctx_shader shader);

void apply_pix_shader(drawctx_t* source_ctx, pixel_shader shader);
