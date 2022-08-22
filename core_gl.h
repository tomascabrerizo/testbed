#ifndef CORE_GL_H
#define CORE_GL_H

#include <GL/gl.h>

#define CORE_GL_FUNCTIONS(X) \
  X(void, glGenVertexArrays, (GLsizei n, GLuint *arrays)) \
  X(void, glBindVertexArray, (GLuint array)) \
  X(void, glGenBuffers, (GLsizei	n, GLuint *buffers)) \
  X(void, glBindBuffer, (GLenum target, GLuint buffer)) \
  X(void, glGenFramebuffers, (GLsizei n, GLuint *ids)) \
  X(void, glBindFramebuffer, (GLenum target, GLuint framebuffer)) \
  X(void, glFramebufferTexture2D, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)) \
  X(void, glGenRenderbuffers, (GLsizei n, GLuint *renderbuffers)) \
  X(void, glBindRenderbuffer, (GLenum target, GLuint renderbuffer)) \
  X(void, glRenderbufferStorage, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height)) \
  X(void, glFramebufferRenderbuffer, (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)) \
  X(GLenum, glCheckFramebufferStatus, (GLenum target)) \
  X(void, glDeleteRenderbuffers, (GLsizei n, GLuint *renderbuffers)) \
  X(void, glDeleteFramebuffers, (GLsizei n, GLuint *framebuffers)) \
  X(GLuint, glCreateShader, (GLenum shaderType)) \
  X(void, glShaderSource, (GLuint shader, GLsizei count, const GLchar **string, const GLint *length)) \
  X(void, glGetShaderSource, (GLuint	shader, GLsizei	bufSize, GLsizei *length, GLchar *source)) \
  X(void, glCompileShader, (GLuint shader)) \
  X(void, glGetShaderiv, (GLuint shader, GLenum pname, GLint *params)) \
  X(void, glGetShaderInfoLog, (GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog)) \
  X(GLuint, glCreateProgram, (void)) \
  X(void, glAttachShader, (GLuint program, GLuint shader)) \
  X(void, glLinkProgram, (GLuint program)) \
  X(void, glGetProgramiv, (GLuint program, GLenum pname, GLint *params)) \
  X(void, glGetProgramInfoLog, (GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog)) \
  X(void, glDeleteShader, (GLuint shader)) \
  X(void, glDeleteProgram, (GLuint program)) \
  X(void, glUseProgram, (GLuint program)) \

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
