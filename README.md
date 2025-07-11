# SugarLib: Terminal Graphics Library

Experimental C library for rendering graphics in terminal.

### Features

- X/Z Coordinates: Vertical axis is z (not y).
- Contexts (Canvases): Drawable pixel buffers.
- Sprites: Text-based graphics with color bindings.
- Layering: Overlay, subtract, or mask contexts.
- Shaders: Apply effects to entire contexts or individual pixels.

### Requirements
- Truecolor Terminal: Must support 24-bit RGB ANSI escape codes (e.g., [Alacritty](https://github.com/alacritty/alacritty/releases),
or [GNOME Terminal](https://github.com/GNOME/gnome-terminal)).
- C99 Compiler: GCC or Clang

### Usage
Just `#include "sugarlib.h"` in your project

### Core Types
| Type             | Description                                   |
|------------------|-----------------------------------------------|
| color_t          | RGB color (r, g, b: 0-255).                   |
| pixel_t          | Terminal pixel (position, colors, character). |
| color_table_t    | Character-to-color lookup table.              |
| sprite_t         | Text-based graphic (e.g., ASCII art).         |
| drawctx_t        | Drawable context (pixel buffer).              |
| ctx_shader       | Function pointer for context-wide effects.    |
| pixel_shader     | Function pointer for per-pixel effects.       |
| flush_ctx_f      | Function pointer used for rendering.          |
| pixel_template_t | Template for making pixels                    |
### API Reference

#### Color Utilities
```c
bool is_color_invalid(int r, int g, int b);  // Validate RGB (0-255)
```

#### Pixel Creation & Manipulation
```c
pixel_t make_pixel(int x, int z);  // Default: " ", no color, not renderable

// Functional style (return modified copy)  
pixel_t p_add_fg(pixel_t p, int r, int g, int b);  
pixel_t p_add_bg(pixel_t p, int r, int g, int b);  
pixel_t p_set_print(pixel_t p, char to_print);
pixel_t p_set_pos(pixel_t p, int x, int z);

// In-place modifications  
void add_fg(pixel_t* p, int r, int g, int b);  
void add_bg(pixel_t* p, int r, int g, int b);  
void set_print(pixel_t* p, char to_print);
void set_print(pixel_t* p, int x, int z);
```

#### Color Tables (for Sprites)
```c
color_table_t make_color_table(size_t records);  // Allocate table  
void add_record(color_table_t* table, char ch, color_t color);  // Bind char→color  
bool get_record(const color_table_t* table, char ch, color_t* out);  // Lookup color
```

#### Sprites
```c
sprite_t make_sprite(color_table_t table);  // Create empty sprite  
void add_line(const char* line, sprite_t* target);  // Append text row  
drawctx_t* to_ctx(const sprite_t* source);  // Convert to drawable context
```

#### Contexts (Canvases)
```c
drawctx_t* make_drawctx(int width, int height);  // Allocate  
void free_drawctx(drawctx_t* ctx);  // Deallocate  
void set_pixel(drawctx_t* ctx, pixel_t pixel);  // Place pixel (clips OOB)

// Retrieve pixels  
bool get_pixel(const drawctx_t* ctx, pixel_t* out, int x, int z);  // By coordinates; returns true if the pixel is found
bool get_pixel2(const drawctx_t* ctx, pixel_t* out, int pos);  // By index (row-major); also returns true if the pixel is found

// Initialization & filling  
void fill_background(drawctx_t* ctx);  // Reset all pixels  
void fill_with(drawctx_t* ctx, color_t color, int xo, int zo, int xw, int zh);  // Fill rectangle  
drawctx_t* resize_ctx(drawctx_t* to_resize, int nx, int nz); // Resize context to nx (new x) and nz
drawctx_t* crop_ctx(drawctx_t* to_crop, int sx, int sz, int ex, int ez); // Crop the context; sx, sz - start x and z; ex, ez - end x and z
void str_to_ctx(drawctx_t* ctx, const char* text, pixel_template_t p_template, bool line_wrapping, int xo, int zo); // Put text into context
drawctx_t* copy_ctx(const drawctx_t* source);  // Deep copy
```

#### Rendering
```c
// Render to terminal  
void flush_ctx(const drawctx_t* ctx);                  // Full-color  
void flush_compact_ctx(const drawctx_t* ctx);          // Compressed vertically (2:1)
void flush_ctx_offset(const drawctx_t* ctx, flush_ctx_f flush_func, int xo, int zo); // With offset
void flush_aligned_ctx(drawctx_t* to_change, flush_ctx_f flush_func, alignment_t alignment, int screen_width, int screen_height); // With alignment
```

#### Context Operations
```c
// Transformations  
void ctx_over_ctx(drawctx_t* dest, const drawctx_t overlay, int xo, int zo);  // Overlay  
void ctx_sub_ctx(drawctx_t* dest, const drawctx_t overlay, int xo, int zo);    // Subtract colors  
void ctx_mask_ctx(drawctx_t* dest, const drawctx_t mask, int xo, int zo);      // Apply mask

// Shaders  
void apply_ctx_shader(drawctx_t* ctx, ctx_shader shader);      // Whole-context  
void apply_pix_shader(drawctx_t* ctx, pixel_shader shader);    // Per-pixel
```

### Usage Example
```c
#include "sugarlib.h"

int main() {  
    // 1. Create sprite  
    color_table_t ct = make_color_table(2);  
    add_record(&ct, '#', (color_t){255, 0, 0});  // Red '#'  
    sprite_t s = make_sprite(ct);  
    add_line(" # ", &s);  
    add_line("###", &s);
    
    // 2. Convert to context  
    drawctx_t* ctx = to_ctx(&s);
    
    // 3. Render compressed (hide cursor first)  
    printf("\x1b[?25l");  
    flush_compact_ctx(ctx);
    
    // 4. Cleanup  
    free_drawctx(ctx);  
    return 0;  
}
```

## **Warning**
> This library is under active development. Backward compatibility is not guaranteed.
