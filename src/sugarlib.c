#include "sugarlib.h"

#include <string.h>
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

void fill_with(drawctx_t *ctx, color_t color, int xo, int zo, int xw, int zh) {
	for (int x = xo; x < ctx->width && x < xw + xo; ++x) {
		for (int z = zo; z < ctx->height && z < zh + zo; ++z) {
			set_pixel(ctx, p_add_bg(make_pixel(x, z), color.r, color.g, color.b));
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
		.bg = (color_t){0,0,0}, .fg = (color_t){0,0,0}, .print = ' ',
		.renderable = false
	};
}

void add_fg(pixel_t* pixel, int r, int g, int b) {
	if (is_color_invalid(r, g, b)) return;	
	
	pixel->renderable = true;
	pixel->fg_null = false;
	pixel->fg = (color_t){r, g, b};
}

void add_bg(pixel_t* pixel, int r, int g, int b) {
	if (is_color_invalid(r, g, b)) return;
	
	pixel->renderable = true;
	pixel->bg_null = false;
	pixel->bg = (color_t){r, g, b};
}

void set_print(pixel_t *pixel, char to_print) {
	pixel->renderable = true;
	pixel->print = to_print;
}

pixel_t p_add_fg(pixel_t pixel, int r, int g, int b) {
	if (is_color_invalid(r, g, b)) return pixel;	
	
	pixel.renderable = true;
	pixel.fg_null = false;
	pixel.fg = (color_t){r, g, b};

	return pixel;
}

pixel_t p_add_bg(pixel_t pixel, int r, int g, int b) {
	if (is_color_invalid(r, g, b)) return pixel;
	
	pixel.renderable = true;
	pixel.bg_null = false;
	pixel.bg = (color_t){r, g, b};

	return pixel;
}

pixel_t p_set_print(pixel_t pixel, char to_print) {
	pixel.renderable = true;
	pixel.print = to_print;

	return pixel;
}

void free_pixel(pixel_t *pixel) {
	free(pixel);
}

void set_pixel(drawctx_t *ctx, pixel_t pixel) {
	if (!ctx->initialized) return;
	if (pixel.x < 0 || pixel.z < 0 || pixel.x >= ctx->width || pixel.z >= ctx->height) return;
	if (is_color_invalid(pixel.fg.r, pixel.fg.g, pixel.fg.b) || is_color_invalid(pixel.bg.r, pixel.bg.g, pixel.bg.b)) return;
	ctx->pixels[pixel.z * ctx->width + pixel.x] = pixel;
}

bool get_pixel(const drawctx_t *ctx, pixel_t *out, int x, int z) {
	if (!ctx->initialized) goto setnullout;
	if (x < 0 || z < 0 || x >= ctx->width || z >= ctx->height) goto setnullout;
	
	*out = ctx->pixels[z * ctx->width + x];

	return true;
setnullout:
	out = NULL;

	return false;
}

bool get_pixel2(const drawctx_t *ctx, pixel_t *out, int pos) {
	if (!ctx->initialized) goto setnullout;
	if(pos < 0 || pos >= (ctx->height*ctx->width)) goto setnullout;

	*out = ctx->pixels[pos];

	return true;
setnullout:
	out = NULL;
	return false;
}

void flush_ctx(drawctx_t *ctx) {
	if (!ctx->initialized) return;
	
	size_t pos = 0;

	for (size_t i = 0; i < ctx->width*ctx->height; i++) {
		pixel_t p = ctx->pixels[i];
		
		if (!p.renderable) continue;

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

	fflush(stdout);
}

drawctx_t* copy_ctx(const drawctx_t *source) {
	if (!source || !source->initialized) return NULL;

	drawctx_t* newCtx = (drawctx_t*)smalloc(sizeof(drawctx_t));
	newCtx->width = source->width;
	newCtx->height = source->height;
	newCtx->initialized = source->initialized;

	const size_t pixel_count = source->width * source->height;
	newCtx->pixels = (pixel_t*)smalloc(sizeof(pixel_t) * pixel_count);

	memcpy(newCtx->pixels, source->pixels, sizeof(pixel_t) * pixel_count);	

	return newCtx;
}

pixel_t copy_pixel(const pixel_t *source) {
	return p_set_print(
		p_add_bg(
		p_add_fg(make_pixel(source->x, source->z), source->fg.r, source->fg.g, source->fg.b), source->bg.r, source->bg.g, source->bg.b), source->print);
}

void ctx_over_ctx(drawctx_t *to_change, const drawctx_t overlay, int xo, int zo) {
	if(!to_change->initialized || !overlay.initialized) return;

	for (int x = 0; x < overlay.width; ++x) {
		for (int z = 0; z < overlay.height; ++z) {
			pixel_t p;

			get_pixel(&overlay, &p, x, z);

			set_pixel(to_change, (pixel_t){
				.x = x+xo, .z = z+zo, .print = p.print,
				.bg_null = p.bg_null, .fg_null = p.fg_null,
				.bg = p.bg, .fg = p.fg, .renderable = p.renderable
			});
		}
	}
}

void ctx_sub_ctx(drawctx_t* to_change, const drawctx_t overlay, int xo, int zo) {
	if(!to_change->initialized || !overlay.initialized) return;

	for (int x = 0; x < overlay.width; ++x) {
		for (int z = 0; z < overlay.height; ++z) {
			pixel_t overlay_px;
			get_pixel(&overlay, &overlay_px, x, z);


			if(overlay_px.fg_null && overlay_px.bg_null) continue;


			int dest_x = x + xo;
			int dest_z = z + zo;
			pixel_t dest_px = make_pixel(dest_x, dest_z);
			get_pixel(to_change, &dest_px, dest_x, dest_z);


			if(!overlay_px.fg_null && !dest_px.fg_null) {
				dest_px.fg.r -= overlay_px.fg.r;
				dest_px.fg.g -= overlay_px.fg.g;
				dest_px.fg.b -= overlay_px.fg.b;

			}


			if(!overlay_px.bg_null && !dest_px.bg_null) {
				dest_px.bg.r -= overlay_px.bg.r;
				dest_px.bg.g -= overlay_px.bg.g;
				dest_px.bg.b -= overlay_px.bg.b;
            		}

			set_pixel(to_change, dest_px);
		}
	}
}

void ctx_mask_ctx(drawctx_t *to_change, const drawctx_t mask, int xo, int zo) {
	if(!to_change->initialized || !mask.initialized) return;

	for (int x = 0; x < mask.width; ++x) {
		for (int z = 0; z < mask.height; ++z) {
			pixel_t p;
			pixel_t p2;

			get_pixel(&mask, &p, x, z);
			bool res = get_pixel(to_change, &p2, x+xo, z+zo);
			
			if (!p.renderable) continue;
			if (res) {
				if (!(p2.renderable)) continue;
			}

			set_pixel(to_change, (pixel_t){
				.x = x+xo, .z = z+zo, .print = p.print,
				.bg_null = p.bg_null, .fg_null = p.fg_null,
				.bg = p.bg, .fg = p.fg, .renderable = p.renderable
			});
		}
	}
}

void apply_ctx_shader(drawctx_t *changed_ctx, ctx_shader shader) {
	if (!shader) return;
	shader(changed_ctx);
}

void apply_pix_shader(drawctx_t *source_ctx, pixel_shader shader) {
	if (!shader) return;
	for (int i = 0; i < source_ctx->width * source_ctx->height; ++i) {
		shader(&source_ctx->pixels[i]);	
	}
}
