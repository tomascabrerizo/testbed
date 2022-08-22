#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "core.h"
#include "core_obj.h"
#include "core_math.h"
#include "core_hash.h"
#include "renderman.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

unsigned int screen_vao;
unsigned int screen_vbo;
unsigned int screen_texture[WINDOW_WIDTH*WINDOW_HEIGHT];

unsigned int vao;
unsigned int vbo;

unsigned int frame_buffer;
unsigned int frame_buffer_texture;
unsigned int render_buffer;

unsigned int program;
unsigned int program_fb;

char *v_shader_src = 
  "#version 330 core\n"
  "layout (location = 0) in vec2 aPos;\n"
  "layout (location = 1) in vec2 aTex;\n"
  "out vec2 tex;\n"
  "void main()\n"
  "{\n"
  "   gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
  "   tex = aTex;\n"
  "}\0";

char *f_shader_src = 
  "#version 330 core\n"
  "in vec2 tex;\n"
  "out vec4 color;\n"
  "uniform sampler2D uScreen;\n"
  "void main()\n"
  "{\n"
  "   color = texture(uScreen, tex);\n"
  "}\0";

char *v_shader_src_fb = 
  "#version 330 core\n"
  "layout (location = 0) in vec3 aPos;\n"
  "void main()\n"
  "{\n"
  "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
  "}\0";

char *f_shader_src_fb = 
  "#version 330 core\n"
  "out vec4 color;\n"
  "void main()\n"
  "{\n"
  "   color = vec4(1.0, 1.0, 1.0, 1.0);\n"
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

float quad[] = {
  /* V        T */
  -1,  1,   0, 1,
  -1, -1,   0, 0,
   1,  1,   1, 1,
   1,  1,   1, 1,
  -1, -1,   0, 0,
   1, -1,   1, 0
};

float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
}; 

void render_test_init(void) {
  /* ----------- Create default screen buffer data ------------- */
  glGenVertexArrays(1, &screen_vao); 
  glBindVertexArray(screen_vao);
  glGenBuffers(1, &screen_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, screen_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (const void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (const void *)(sizeof(float)*2));
  glEnableVertexAttribArray(1);

  /* ----------- Create framebuffer buffer data ------------- */
  glGenVertexArrays(1, &vao); 
  glBindVertexArray(vao);
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
  glEnableVertexAttribArray(0);

  /* TODO: ------- Copile default and frame buffer sahders ----------*/
  program = render_program_create(v_shader_src, f_shader_src);
  program_fb = render_program_create(v_shader_src_fb, f_shader_src_fb);

  /* ------ Create frame buffer to render -------- */
  glGenFramebuffers(1, &frame_buffer);
  glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);    
  
  glEnable(GL_TEXTURE_2D);
  glGenTextures(1, &frame_buffer_texture);
  glBindTexture(GL_TEXTURE_2D, frame_buffer_texture);
  
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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
	  printf("Framebuffer enable\n");
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);  
}

void render_test_update(void) {
  /* -------------FIRST RENDER PASS--------------------*/
  glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  
  glUseProgram(program_fb);
  
  glClearColor(0.5, 0.5, 0.5, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  
  glDrawArrays(GL_TRIANGLES, 0, 3);

#if 1
  /* Copy the GPU texture on CPU texture buffer */
  glBindTexture(GL_TEXTURE_2D, frame_buffer_texture);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, screen_texture);
  for(int y = 0;  y < WINDOW_HEIGHT; ++y) {
    for(int x = 0;  x < WINDOW_WIDTH; ++x) {
      if(x >= WINDOW_WIDTH/2) {
        int i = y * WINDOW_WIDTH + x;
        uint8_t a = (uint8_t)((screen_texture[i] >> 24) & 0xFF);
        uint8_t b = (uint8_t)((screen_texture[i] >> 16) & 0xFF);
        uint8_t g = (uint8_t)((screen_texture[i] >> 8)  & 0xFF);
        uint8_t r = (uint8_t)((screen_texture[i] >> 0)  & 0xFF);
        
        r = .6f*r + .4f*(255 *sinf(y * .1f));
        g = .6f*g + .4f*(255 *cosf(x * .1f));
        b = .6f*b + .4f*(255 *sinf((y+x) * .1f));

        screen_texture[i] = ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)g << 8) | (uint32_t)r;
      }
    }
  }
  /* Copy the CPU texture on GPU texture buffer */
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, screen_texture);
#endif
  
  /* -------------SECOND RENDER PASS--------------------*/
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  glBindVertexArray(screen_vao);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, frame_buffer_texture);
  glUseProgram(program);
  
  glClearColor(0.5, 0.5, .5, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render_test_shutdown(void) {
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &screen_vbo);

  glDeleteVertexArrays(1, &vao);
  glDeleteVertexArrays(1, &screen_vao);

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
