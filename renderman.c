#include <string.h>

#include "renderman.h"
#include "core_gl.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
    printf("Fail to load file %s\n", path);
    *size = 0;
    fclose(file);
    return 0;
  }

  fseek(file, 0, SEEK_END);
  *size = ftell(file);
  fseek(file, 0, SEEK_SET);
  char *buffer = (void *)malloc(*size + 1);
  fread(buffer, (*size), 1, file);
  buffer[*size] = '\0';
  fclose(file);
  return (void *)buffer;
}

#define READ_U16(data) *((uint16_t *)data); data = (uint16_t *)data + 1
#define READ_U32(data) *((uint32_t *)data); data = (uint32_t *)data + 1
#define READ_U64(data) *((uint64_t *)data); data = (uint64_t *)data + 1

Texture2D *render_texture_create_from_file(char *path) {
  uint64_t size;
  void *file = render_read_entire_file(path, &size);
  if(file) {
    void *temp_bmp_data = file;
    
    uint16_t id =                 READ_U16(temp_bmp_data);
    uint32_t file_size =          READ_U32(temp_bmp_data);
    uint32_t reserved =           READ_U32(temp_bmp_data);
    uint32_t pixel_array_offset = READ_U32(temp_bmp_data);
    uint32_t dib_header_size =    READ_U32(temp_bmp_data);
    uint32_t width =              READ_U32(temp_bmp_data);
    uint32_t height =             READ_U32(temp_bmp_data);
    uint16_t planes =             READ_U16(temp_bmp_data);
    uint16_t bits_per_pixel =     READ_U16(temp_bmp_data);
    uint32_t compression =        READ_U32(temp_bmp_data);
    uint32_t bmp_size =           READ_U32(temp_bmp_data);

    // NOTE: Unused
    (void)bmp_size;
    (void)compression;
    (void)dib_header_size;
    (void)reserved;
    (void)planes;
    (void)id;
    (void)file_size;

    // NOTE: for now only allow 32bits bitmaps
    uint32_t bytes_per_pixel = bits_per_pixel / 8;
    ASSERT(bytes_per_pixel == sizeof(uint32_t));

    uint8_t *bmp_src = (uint8_t *)file + pixel_array_offset;
    Texture2D *result = (void *)malloc(sizeof(Texture2D));
    result->w = width;
    result->h = height;
    result->bpp = bytes_per_pixel;

    uint64_t bitmap_size = width * height * bytes_per_pixel;
    result->data = (uint32_t *)malloc(bitmap_size); 
    
    // NOTE: cannot use memcpy, the bitmap must be flipped 
    uint32_t *src_row = (uint32_t *)bmp_src + (height-1) * width;
    uint32_t *dst_row = (uint32_t *)result->data;
    for(uint32_t y = 0; y < height; ++y) {
      uint32_t *src_pixels = src_row;
      uint32_t *dst_pixels = dst_row;
      for(uint32_t x = 0; x < width; ++x) {
        *dst_pixels++ = *src_pixels++;
      }
      src_row -= width;
      dst_row += width;
    }

    free(file);
    
    glGenTextures(1, &result->id);
    glBindTexture(GL_TEXTURE_2D, result->id);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, result->w, result->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, result->data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return result;
  } else {
    printf("Fail to load the texture\n");
    return 0;
  }
}

void render_texture_destroy(Texture2D *texture) {
  if(texture) {
    glDeleteTextures(1, &texture->id);
    if(texture->data) {
      free(texture->data);
    }
    free(texture);
  }
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

static float quad[] = {
  /* positions   uvs      */
  -1.0f,  1.0f,  0.0f, 1.0f,
  -1.0f, -1.0f,  0.0f, 0.0f,
   1.0f,  1.0f,  1.0f, 1.0f,
   1.0f, -1.0f,  1.0f, 0.0f,
};  

Render2D *render2d_create() {
  Render2D *render = (Render2D *)malloc(sizeof(*render));
  render->font = render_texture_create_from_file("font/font.bmp");
  printf("texture w:%d, h:%d\n", render->font->w, render->font->h);
  
  render->program = render_program_create_from_files("quad.vert", "quad.frag");
  
  /* NOTE: Setup uniform location hash table */
  render->uniform_register = core_map_create();
  /* TODO: rename add uniform to register uniform */
  render2d_add_uniform(render, "res_x");
  render2d_add_uniform(render, "res_y");

  /* NOTE: Allocate render command buffer */
  render->command_buffer = (RenderCommand2D *)malloc(sizeof(RenderCommand2D) * MAX_COMMAND_BUFFER);
  render->command_buffer_size = 0;

  /* NOTE: Setup OpenGL buffers */
  glGenVertexArrays(1, &render->vao); 
  glActiveTexture(GL_TEXTURE0); /* TODO: Where the texture unit shoud be active ?? */
  glBindVertexArray(render->vao);
  
  glGenBuffers(1, &render->vbo_quad);
  glBindBuffer(GL_ARRAY_BUFFER, render->vbo_quad);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) *4, (const void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) *4, (const void *)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);
  
  glGenBuffers(1, &render->vbo_instance);
  glBindBuffer(GL_ARRAY_BUFFER, render->vbo_instance);
  glBufferData(GL_ARRAY_BUFFER, sizeof(RenderCommand2D)*MAX_COMMAND_BUFFER, render->command_buffer, GL_DYNAMIC_DRAW);
  
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(RenderCommand2D), (void*)0);
  glVertexAttribDivisor(2, 1);  

  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(RenderCommand2D), (void*)(2 * sizeof(float)));
  glVertexAttribDivisor(3, 1);  

  glEnableVertexAttribArray(4);
  glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, sizeof(RenderCommand2D), (void*)(4 * sizeof(float)));
  glVertexAttribDivisor(4, 1);  
  
  glBindBuffer(GL_ARRAY_BUFFER, 0); 
  glBindVertexArray(0);
  
  return render;
}

void render2d_destroy(Render2D *render) {
  render_texture_destroy(render->font);
  core_map_destroy(render->uniform_register);
  free(render->command_buffer);
  free(render);
}

static inline RenderCommand2D *render2d_push_command(Render2D *render) {
  if(render->command_buffer_size + 1 > MAX_COMMAND_BUFFER) {
    render2d_buffer_flush(render);
  }
  return render->command_buffer + render->command_buffer_size++;
}

void render2d_draw_quad(Render2D *render, int x, int y, int w, int h) {
  RenderCommand2D *command = render2d_push_command(render);
  command->flags = COMMAND_RECT;
  command->x = x;
  command->y = y;
  command->w = w;
  command->h = h;
}

void render2d_draw_texture(Render2D *render, int x, int y, int w, int h) {
  RenderCommand2D *command = render2d_push_command(render);
  command->flags = COMMAND_TEXTURE;
  command->x = x;
  command->y = y;
  command->w = w;
  command->h = h;
}

void render2d_buffer_flush(Render2D *render) {
  /* NOTE: Update command buffer instance */
  glBindTexture(GL_TEXTURE_2D, render->font->id);
  glBindBuffer(GL_ARRAY_BUFFER, render->vbo_instance);
  glBufferSubData(GL_ARRAY_BUFFER, 0, render->command_buffer_size*sizeof(RenderCommand2D), (void *)render->command_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  /* NOTE: Render the new command buffer */
  glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, render->command_buffer_size);  
  render->command_buffer_size = 0;
}

void render2d_begin(Render2D *render) {
  /* NOTE: Clear screen */
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  /* NOTE: Bind render shader and buffers */
  glUseProgram(render->program);
  glBindVertexArray(render->vao);
}

void render2d_end(Render2D *render) {
  if(render->command_buffer_size > 0) {
    render2d_buffer_flush(render);
  }
  glUseProgram(0);
  glBindVertexArray(0);
}

void render2d_add_uniform(Render2D *render, char *name) {
  uint64_t hash = core_hash64(name, strlen(name), CORE_MAP_SEED);
  int64_t uniform_loc = glGetUniformLocation(render->program, name);
  core_map_add_hash(render->uniform_register, (void *)hash, (void *)uniform_loc, hash);
}

int render2d_get_uniform(Render2D *render, char *name) {
  uint64_t hash = core_hash64(name, strlen(name), CORE_MAP_SEED);
  return (int)((int64_t)core_map_get_hash(render->uniform_register, (void *)hash, hash));
}

void render2d_set_resolution(Render2D *render, unsigned int width, unsigned int height) {
  glUseProgram(render->program);
  glUniform1i(render2d_get_uniform(render, "res_x"), width);
  glUniform1i(render2d_get_uniform(render, "res_y"), height);
}

void render2d_alpha_test(Render2D *render, bool value) {
  (void)render;
  if(value) {
    glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  } else {
    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
  }
}
