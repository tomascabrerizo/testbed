#include "renderman.h"
#include "core_gl.h"

/* TODO: Implements RenderMan functions */

/* NOTE: Renderer functions */

RManRenderer *renderman_render_create(void) {
  return 0; /* TODO: Function not implemented */
}

void renderman_render_destroy(RManRenderer *render) {
  (void)render; /* TODO: Funciton not implemented */
}

void renderman_render_add_shader(RManRenderer *render, char *name, struct RManShader *shader) {
  (void)render; /* TODO: Funciton not implemented */
  (void)name;
  (void)shader;
}
void renderman_render_begin(RManRenderer *render) {
  (void)render; /* TODO: Funciton not implemented */
}
void renderman_render_end(RManRenderer *render) {
  (void)render; /* TODO: Funciton not implemented */
}

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if 0
static char *v_shader_src = 
  "#version 330 core\n"
  "layout (location = 0) in vec3 aPos;\n"
  "layout (location = 1) in vec4 aColor;\n"
  "uniform mat4 uProj;\n"
  "out vec4 color;\n"
  "void main()\n"
  "{\n"
  "   color = aColor;\n"
  "   gl_Position = uProj * vec4(aPos, 1.0);\n"
  "}\0";

static char *f_shader_src = 
  "#version 330 core\n"
  "in vec4 color;\n"
  "out vec4 fragment;\n"
  "void main()\n"
  "{\n"
  "   fragment = color;\n"
  "}\0";
#endif

static unsigned int render_program_create(char *v_src, char *f_src) {
  int success;
  static char infoLog[512];
  
  unsigned int v_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(v_shader, 1, (const char **)&v_src, NULL);
  glCompileShader(v_shader);
  glGetShaderiv(v_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(v_shader, 512, NULL, infoLog);
    printf("[VERTEX SHADER ERROR]: %s\n", infoLog);
  }

  unsigned int f_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(f_shader, 1, (const char **)&f_src, NULL);
  glCompileShader(f_shader);
  glGetShaderiv(f_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(f_shader, 512, NULL, infoLog);
    printf("[FRAGMENT SHADER ERROR]: %s\n", infoLog);
  }

  unsigned int program = glCreateProgram();
  glAttachShader(program, v_shader);
  glAttachShader(program, f_shader);
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    printf("[SHADER PROGRAM ERROR]: %s\n", infoLog);
  }
  
  glDeleteShader(v_shader);
  glDeleteShader(f_shader);
  
  return program;

}

static void *render_read_entire_file(char *path, uint64_t *size) {
  FILE *file = fopen(path, "rb");
  if(!file) {
    printf("fail to load file %s\n", path);
    *size = 0;
    fclose(file);
    return 0;
  }

  fseek(file, 0, SEEK_END);
  *size = ftell(file) + 1;
  fseek(file, 0, SEEK_SET);
  char *buffer = (void *)malloc(*size);
  fread(buffer, (*size - 1), 1, file);
  buffer[*size] = '\0';
  printf("%s\n", buffer);
  fclose(file);
  return (void *)buffer;
}

static unsigned int render_program_create_from_files(char *v_path, char *f_path) {
  uint64_t v_size, f_size;
  char *v_src = (char *)render_read_entire_file(v_path, &v_size);
  char *f_src = (char *)render_read_entire_file(f_path, &f_size);

  (void)v_src;
  (void)f_src;
  
  unsigned int program = 0;
  if(v_src && f_src) {
    program = render_program_create(v_src, f_src);
    free(v_src);
    free(f_src);
  }
  return program;
}

Render2D *render2d_create() {
  Render2D *render = (Render2D *)malloc(sizeof(*render));
  render->program = render_program_create_from_files("quad.vert", "quad.frag");

#if 0
  static Vertex2D offsets[100];
  for(int y = 0; y < 10; ++y) {
    for(int x = 0; x < 10; ++x) { 
      offsets[y*10+x] = (Vertex2D){x/5-1, y/5-1};
    }
  }
  
  glUseProgram(render->program);
  for(int i = 0; i < 100; ++i) {
    static char *uniform_name = "offsets[x]";
    uniform_name[8] = (char)('0' + i);
    printf("%s\n", uniform_name);
    int location = glGetUniformLocation(render->program, uniform_name);
    glUniform2f(location, offsets[i].x, offsets[i].y);
  }
#endif
  return render;
}

void render2d_destroy(Render2D *render) {
  free(render);
}

