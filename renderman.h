#ifndef RENDERMAN_H
#define RENDERMAN_H
/* TODO: Renderman is the name of the pixar renderer. find a good name */
#include "core_gl.h"
#include "core_type.h"
#include "core_math.h"
/* NOTE: This library is a modern OPENGL 3.3 wraper */

/* TODO: program vtretch buffers */

typedef uint32_t RManHandle ;

typedef struct RManRenderer {

}RManRenderer;


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


#endif /* RENDERMAN_H */
