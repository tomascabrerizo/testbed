#ifndef RENDERMAN_H
#define RENDERMAN_H
/* TODO: Renderman is the name of the pixar renderer. find a good name */
#include "core_type.h"
#include "core_math.h"
#include "core_ds.h"

typedef struct Texture2D {
  unsigned int id;
  void *data;
  uint32_t w, h;
  uint32_t bpp; /* NOTE: Bytes per pixel */
} Texture2D;

Texture2D *render_texture_create_from_file(char *path);
void render_texture_destroy(Texture2D *texture);

typedef struct Vertex2D {
  float x, y;
} Vertex2D;

typedef enum RenderComman2DType {
  COMMAND_RECT        = (1 << 0), // 1
  COMMAND_TEXTURE     = (1 << 1), // 2
  COMMAND_BORDER_RECT = (1 << 2), // 4
} RenderComman2DType;

#define MAX_COMMAND_BUFFER 10000 
typedef struct RenderCommand2D {
  float x, y, w, h;
  uint32_t flags;
} RenderCommand2D;

typedef struct Render2D {
  unsigned int vao;
  unsigned int vbo_quad;
  unsigned int vbo_instance;
  unsigned int program;

  unsigned int resolution_x;
  unsigned int resolution_y;

  Texture2D *font;

  /* NOTE: Render commands buffer */
  RenderCommand2D *command_buffer;
  uint64_t command_buffer_size;
  /* NOTE: Uniform location register */
  CoreMap *uniform_register; 
} Render2D;

Render2D *render2d_create();
void render2d_destroy(Render2D *render);

void render2d_add_uniform(Render2D *render, char *name);
int render2d_get_uniform(Render2D *render, char *name);
void render2d_set_resolution(Render2D *render, unsigned int width, unsigned int height);

void render2d_buffer_flush(Render2D *render);
void render2d_begin(Render2D *render);
void render2d_end(Render2D *render);

void render2d_draw_quad(Render2D *render, int x, int y, int w, int h);
void render2d_draw_texture(Render2D *render, int x, int y, int w, int h);

void render2d_alpha_test(Render2D *render, bool value);

#endif /* RENDERMAN_H */
