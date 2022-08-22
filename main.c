#include <stdio.h>
#include <unistd.h>

#include "core.h"
#include "core_obj.h"
#include "core_math.h"
#include "core_hash.h"
#include "renderman.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

unsigned int vao;
unsigned int vbo;

unsigned int frame_buffer;
unsigned int frame_buffer_texture;
unsigned int render_buffer;

unsigned int program;
unsigned int program_fb;

char *v_shader_src = 
  "#version 330 core\n"
  "layout (location = 0) in vec3 aPos;\n"
  "void main()\n"
  "{\n"
  "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
  "}\0";

char *f_shader_src = 
  "#version 330 core\n"
  "out vec4 color;\n"
  "void main()\n"
  "{\n"
  "   color = vec4(1.0, 0.0, 0.0, 1.0);\n"
  "}\0";

char *f_shader_src_fb = 
  "#version 330 core\n"
  "out vec4 color;\n"
  "void main()\n"
  "{\n"
  "   color = vec4(0.0, 1.0, 0.0, 1.0);\n"
  "}\0";

unsigned int render_program_create(char *v_src, char *f_src) {
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

float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
}; 

void render_test_init(void) {
  /* TODO: ------- Copile default and frame buffer sahders ----------*/
  program = render_program_create(v_shader_src, f_shader_src);
  program_fb = render_program_create(v_shader_src, f_shader_src_fb);

  /* ------ Create frame buffer to render -------- */
  glGenFramebuffers(1, &frame_buffer);
  glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);    
  
  glGenTextures(1, &frame_buffer_texture);
  glBindTexture(GL_TEXTURE_2D, frame_buffer_texture);
  
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
  glBindTexture(GL_TEXTURE_2D, 0);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame_buffer_texture, 0);  

  glGenRenderbuffers(1, &render_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, render_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);  
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_buffer);

  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
	  printf("Framebuffer is not complete!\n");
  } else {
	  printf("Framebuffer is complete!\n");
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);  
}

void render_test_update(void) {
  glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
  glUseProgram(program_fb);
  glClearColor(0, 0.5, 1, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  /* TODO: Render scene to frame buffer */
  

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glUseProgram(program);
  glClearColor(0.5, 0.5, .5, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  /* TODO: Render frame buffer texture to the screen */
}

void render_test_shutdown(void) {
  glDeleteProgram(program);
  glDeleteProgram(program_fb);
  glDeleteTextures(1, &frame_buffer_texture);
  glDeleteRenderbuffers(1, &render_buffer);
  glDeleteFramebuffers(1, &frame_buffer);
}


int main(void) {
  CoreWindow *window = core_window_create("TestBed", WINDOW_WIDTH, WINDOW_HEIGHT);
  
  bool running = true;

  render_test_init();
  while(running) {
    CoreState *state = core_state_get_state(window);
    if(state->quit) {
      printf("Quitting application\n");
      running = false;
    }
    
    if(state->resize) {
      printf("The window is resizing\n");
      glViewport(0, 0, state->width, state->height);
    }

    render_test_update();
    core_window_swap_buffers(window);
  }
  
  render_test_shutdown();
  core_window_destroy(window);

  return 0;
}
