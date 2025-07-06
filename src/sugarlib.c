#include "sugarlib.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

void* smalloc(const size_t bytes) {
	void* b = malloc(bytes);
	if (!b) {
		perror("malloc()");
		exit(EXIT_FAILURE);
	}
	return b;
}

bool is_color_invalid(int r, int g, int b) {
	return r < 0 || g < 0 || b < 0 || r > 255 || g > 255 || b > 255;
}

drawctx_t* make_drawctx(int width, int height) {
	drawctx_t* ctx = (drawctx_t*)smalloc(sizeof(drawctx_t));
	ctx->initialized = true;
	ctx->height = height;
	ctx->width = width;
	ctx->pixels = (pixel_t*)smalloc(sizeof(pixel_t)*height*width);
	return ctx;
}

void fill_background(drawctx_t *ctx) {
	for (int x = 0; x < ctx->width; ++x) {
		for (int z = 0; z < ctx->height; ++z) {
			set_pixel(ctx, make_pixel(x, z));
		}
	}
}

void free_drawctx(drawctx_t *ctx) {
	if (!ctx->initialized) return;
	free(ctx->pixels);
	free(ctx);
}

pixel_t make_pixel(int x, int z) {
	return (pixel_t){
		.x = x, .z = z, .bg_null = true, .fg_null = true,
		.bg = (color_t){0,0,0}, .fg = (color_t){0,0,0}, .print = ' '
	};
}

void add_fg(pixel_t* pixel, int r, int g, int b) {
	if (is_color_invalid(r, g, b)) return;	
	
	pixel->fg_null = false;
	pixel->fg = (color_t){r, g, b};
}

void add_bg(pixel_t* pixel, int r, int g, int b) {
	if (is_color_invalid(r, g, b)) return;
	
	pixel->bg_null = false;
	pixel->bg = (color_t){r, g, b};
}

void set_print(pixel_t *pixel, char to_print) {
	pixel->print = to_print;
}

void free_pixel(pixel_t *pixel) {
	free(pixel);
}

void set_pixel(drawctx_t *ctx, pixel_t pixel) {
	if (!ctx->initialized) return;
	if (pixel.x < 0 || pixel.z < 0 || pixel.x >= ctx->width || pixel.z >= ctx->height) return;
	ctx->pixels[pixel.z * ctx->width + pixel.x] = pixel;
}

void flush_ctx(drawctx_t *ctx, bool clear) {
	if (!ctx->initialized) return;
	if(clear) printf("\x1b[2J");
	
	printf("\x1b[H");
	
	size_t pos = 0;

	for (size_t i = 0; i < ctx->width*ctx->height; i++) {
		pixel_t p = ctx->pixels[i];
		if(p.bg_null && p.fg_null) {
			printf("\x1b[%d;%dH%c", p.z + 1, p.x + 1, p.print);
		}
		else if(p.bg_null) {
			printf("\x1b[%d;%dH\x1b[38;2;%d;%d;%dm%c\x1b[0m", p.z + 1, p.x + 1,
					p.fg.r, p.fg.g, p.fg.b,
					p.print);
		}
		else if(p.fg_null) {
			printf("\x1b[%d;%dH\x1b[48;2;%d;%d;%dm%c\x1b[0m", p.z + 1, p.x + 1, 
					p.bg.r, p.bg.g, p.bg.b,
					p.print);
		}
		else {
			printf("\x1b[%d;%dH\x1b[48;2;%d;%d;%dm\x1b[38;2;%d;%d;%dm%c\x1b[0m",
					p.z + 1, p.x + 1,
					p.bg.r, p.bg.g, p.bg.b,
					p.fg.r, p.fg.g, p.fg.b,
					p.print);
		}
	}
}
