#ifndef RENDERMAN_H
#define RENDERMAN_H
/* TODO: Renderman is the name of the pixar renderer. find a good name */
#include "core_type.h"
#include "core_math.h"
#include "core_ds.h"
/* NOTE: This library is a modern OPENGL 3.3 wraper */

/* TODO: program vtretch buffers */

typedef uint32_t RManHandle ;

typedef struct RManRenderer {
  
} RManRenderer;


typedef struct RManFrameBuffer {
  unsigned int id;
} RManFrameBuffer;


RManRenderer *renderman_render_create(void);
void renderman_render_destroy(RManRenderer *render);

RManHandle renderman_framebuffer_create(RManRenderer *render);
void renderman_famebuffer_destroy(RManRenderer *render, RManHandle handle);

void renderman_render_begin(RManRenderer *render);
void renderman_render_end(RManRenderer *render);

typedef struct RManShader {
  unsigned int id;
} RManShader;


typedef struct Vertex2D {
  float x, y;
} Vertex2D;

#define MAX_COMMAND_BUFFER 10000 
typedef struct RenderCommand2D {
  float x, y, w, h;
} RenderCommand2D;

typedef struct Render2D {
  unsigned int vao;
  unsigned int vbo_quad;
  unsigned int vbo_instance;
  unsigned int program;

  unsigned int resolution_x;
  unsigned int resolution_y;

  /* NOTE: Uniform location register */
  RenderCommand2D *command_buffer;
  uint64_t command_buffer_size;
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


#endif /* RENDERMAN_H */
