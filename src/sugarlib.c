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

color_table_t make_color_table(size_t records) {
	bound_color_t* table_content = (bound_color_t*)smalloc(sizeof(bound_color_t)*records);

	return (color_table_t){.pixel_table=table_content, .table_size=records, .last_added=0};
}

void add_line(const char *line, sprite_t *target) {
	int width = strlen(line);
	int end_of_target = strlen(target->description);

	char* newPtr = (char*)smalloc(sizeof(char)*(end_of_target+width-1));

	memcpy(newPtr, target->description, sizeof(char)*end_of_target);

	free(target->description);
	
	target->description = newPtr;
	
	int b = 0;
	int i = end_of_target;

	for (; line[b] != 0; i++) {
		target->description[i] = line[b];
		b++;
	}

	target->description[i] = 0;


	target->width = width;
	target->height += 1;
}

sprite_t make_sprite(color_table_t table) {
	char* desc = malloc(0);

	return (sprite_t){.height=1, .width=0, .bound_table=table, .description=desc};
}


drawctx_t* make_drawctx(int width, int height) {
	drawctx_t* ctx = (drawctx_t*)smalloc(sizeof(drawctx_t));
	ctx->initialized = true;
	ctx->height = height;
	ctx->width = width;
	ctx->pixels = (pixel_t*)smalloc(sizeof(pixel_t)*height*width);
	return ctx;
}

void add_record(color_table_t *table, char ch, color_t color) {
	if (!table) return;
	if (table->last_added + 1 >= table->table_size) return;

	table->pixel_table[table->last_added+1] = (bound_color_t){.ch=ch, .color=color};
	table->last_added += 1;
}

bool get_record(const color_table_t *table, char ch, color_t *out) {
	if (!table) goto setnullout;
	
	for (int i = 0; i < table->table_size; ++i) {
		if (table->pixel_table[i].ch == ch) {
			*out = table->pixel_table[i].color;
			return true;
		}
	}

	goto setnullout;
setnullout:
	out = NULL;
	return false;
}

drawctx_t* to_ctx(const sprite_t* source) {
	drawctx_t* ctx = make_drawctx(source->width, source->height);
	
	fill_background(ctx);

	for (int x = 0; x < source->width; ++x) {
		for (int z = 0; z < source->height; ++z) {
			if (source->description[z * source->width + x] >= 33) {
				color_t out;
				
				if (get_record(&source->bound_table, source->description[z * source->width + x], &out)) {
					set_pixel(ctx, p_add_bg(make_pixel(x, z), out.r, out.g, out.b));
				}
			}
		}
	}

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

void str_to_ctx(drawctx_t *ctx, const char *text, pixel_template_t p_template, bool line_wrapping, int xo, int zo) {
	int lx = xo;
	int lz = zo;

	for (int i = 0; text[i] != 0; ++i) {
		if (line_wrapping && lx >= ctx->width) {
			lx = xo;
			lz++;
		}
		
		if (!p_template.bg_null && !p_template.fg_null) {
			set_pixel(ctx, p_add_fg(p_add_bg(p_set_print(make_pixel(lx, lz), (char[]){ text[i], '\0' }), p_template.bg.r, p_template.bg.g, p_template.bg.b), p_template.fg.r, p_template.fg.g, p_template.fg.b));
		}
		else if (!p_template.bg_null) {
			set_pixel(ctx, p_add_bg(p_set_print(make_pixel(lx, lz), (char[]){ text[i], '\0' }), p_template.bg.r, p_template.bg.g, p_template.bg.b));
		}
		else if (!p_template.fg_null) {
			set_pixel(ctx, p_add_fg(p_set_print(make_pixel(lx, lz), (char[]){ text[i], '\0' }), p_template.fg.r, p_template.fg.g, p_template.fg.b));
		}

		lx++;
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
		.bg = (color_t){0,0,0}, .fg = (color_t){0,0,0}, .print = " ",
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

void set_print(pixel_t *pixel, const char* to_print) {
	pixel->renderable = true;
	pixel->print = strdup(to_print);
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

pixel_t p_set_print(pixel_t pixel, const char* to_print) {
	pixel.renderable = true;
	pixel.print = strdup(to_print);

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

void flush_ctx(const drawctx_t *ctx) {
	if (!ctx->initialized) return;
	
	size_t pos = 0;

	for (size_t i = 0; i < ctx->width*ctx->height; i++) {
		pixel_t p = ctx->pixels[i];
		
		if (!p.renderable) continue;

		if(p.bg_null && p.fg_null) {
			printf("\x1b[%d;%dH%s", p.z + 1, p.x + 1, p.print);
		}
		else if(p.bg_null) {
			printf("\x1b[%d;%dH\x1b[38;2;%d;%d;%dm%s\x1b[0m", p.z + 1, p.x + 1,
					p.fg.r, p.fg.g, p.fg.b,
					p.print);
		}
		else if(p.fg_null) {
			printf("\x1b[%d;%dH\x1b[48;2;%d;%d;%dm%s\x1b[0m", p.z + 1, p.x + 1, 
					p.bg.r, p.bg.g, p.bg.b,
					p.print);
		}
		else {
			printf("\x1b[%d;%dH\x1b[48;2;%d;%d;%dm\x1b[38;2;%d;%d;%dm%s\x1b[0m",
					p.z + 1, p.x + 1,
					p.bg.r, p.bg.g, p.bg.b,
					p.fg.r, p.fg.g, p.fg.b,
					p.print);
		}
	}

	fflush(stdout);
}

void flush_compact_ctx(const drawctx_t* ctx) {
	if (!ctx->initialized) return;

	for (int x = 0; x < ctx->width; ++x) {
		int rz = 0;
		for (int z = 0; z < ctx->height; z += 2) {
			const char* placeholder = " ";
			bool placeholder_changed = false;

			pixel_t a;
			pixel_t b;

			bool render_bg, render_fg;
			render_bg = false; render_fg = false;

			color_t bg, fg;

			if (get_pixel(ctx, &a, x, z)) {
				render_fg = !a.bg_null && a.renderable;

				if (!(strcmp(a.print, " ")==0)) {
					placeholder = a.print;
					placeholder_changed = true;
				}

				fg = a.bg;
			}

			if (get_pixel(ctx, &b, x, z+1)) {
				render_bg = !b.bg_null && b.renderable;
				bg = b.bg;
			}
			
			if (render_bg || render_fg)
				printf("\x1b[%d;%dH", rz + 1, x + 1);

			if (render_bg && render_fg) {
				printf("\x1b[38;2;%d;%d;%dm\x1b[48;2;%d;%d;%dm%s\x1b[0m",
					fg.r, fg.g, fg.b,
					bg.r, bg.g, bg.b,
					placeholder_changed ? placeholder : "▀");
			}
			else if (render_fg) {
				printf("\x1b[38;2;%d;%d;%dm%s\x1b[0m",
					fg.r, fg.g, fg.b,
					placeholder_changed ? placeholder : "▀");
			}
			else if (render_bg) {
				printf("\x1b[38;2;%d;%d;%dm▄\x1b[0m",
					bg.r, bg.g, bg.b);
			}



			++rz;
		}
	}

	fflush(stdout);
}

void flush_ctx_offset(const drawctx_t *ctx, flush_ctx_f flush_func, int xo, int zo) {
	if (!ctx->initialized) return;
	if (!flush_func) return;

	drawctx_t* o = make_drawctx(ctx->width+xo, ctx->height+zo);
	fill_background(o);

	ctx_over_ctx(o, *ctx, xo, zo);
	
	flush_func(o);

	free_drawctx(o);
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

void flush_aligned_ctx(drawctx_t* to_change, flush_ctx_f flush_func, alignment_t alignment, int screen_width, int screen_height) {
	int xo = 0;
	int zo = 0;

	switch (alignment) {
		case TOP_LEFT:
			break;
		case TOP:
			xo = (screen_width / 2) - (to_change->width / 2);
			break;
		case TOP_RIGHT:
			xo = screen_width-to_change->width;
			break;
		case CENTER_LEFT:
			zo = (screen_height / 2) - (to_change->height / 2);
			break;
		case CENTER:
			zo = (screen_height / 2) - (to_change->height / 2);
			xo = (screen_width / 2) - (to_change->width / 2);
			break;
		case CENTER_RIGHT:
			zo = (screen_height / 2) - (to_change->height / 2);
			xo = screen_width-to_change->width;
			break;
		case BOTTOM_LEFT:
			zo = screen_height - to_change->height;
			break;
		case BOTTOM:
			zo = screen_height - to_change->height;
			xo = (screen_width / 2) - (to_change->height / 2);
			break;
		case BOTTOM_RIGHT:
			zo = screen_height - to_change->height;
			xo = screen_width - to_change->width;
			break;
	}

	flush_ctx_offset(to_change, flush_func, xo, zo);
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
