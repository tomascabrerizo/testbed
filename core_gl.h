#ifndef CORE_GL_H
#define CORE_GL_H

#include <GL/gl.h>

#define CORE_GL_FUNCTIONS(X) \
  X(void, glGenVertexArrays, (GLsizei n, GLuint *arrays)) \
  X(void, glBindVertexArray, (GLuint array)) \
  X(void, glGenBuffers, (GLsizei	n, GLuint *buffers)) \
  X(void, glBindBuffer, (GLenum target, GLuint buffer))

#define CORE_GL_PROC(name) CORE_##name##_POC

#define X(return, name, params) typedef return (*CORE_GL_PROC(name)) params;
CORE_GL_FUNCTIONS(X)
#undef X

#ifdef CORE_INTERNAL

#define X(return, name, params) CORE_GL_PROC(name) name = 0;
CORE_GL_FUNCTIONS(X)
#undef X

#else

#define X(return, name, params) extern CORE_GL_PROC(name) name;
CORE_GL_FUNCTIONS(X)
#undef X

#endif

#endif /* CORE_GL_H */
