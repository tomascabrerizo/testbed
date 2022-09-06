#ifndef CORE_FONT_H
#define CORE_FONT_H

#include "core_type.h"

#define GLYPH_ID_MAX 256
typedef struct CoreGlyph {
  uint8_t id;
  int x;
  int y;
  int w;
  int h;
  int offset_x;
  int offset_y;
  int advance;
} CoreGlyph;

#define MAX_FACE_NAME_SIZE 64
typedef struct CoreFont {
  char face[MAX_FACE_NAME_SIZE];
  int size;
  
  int padding_top;
  int padding_bottom;
  int padding_right;
  int padding_left;

  int line_height;
  int base;

  struct Texture2D *atlas;
  int atlas_w;
  int atlas_h;
  
  /* Maybe use a hash table for not ascii fonts */
  CoreGlyph glyph_table[GLYPH_ID_MAX];
  int glyph_count;
} CoreFont;

CoreFont *core_font_create(char *path);
void core_font_destroy(CoreFont *font);

#endif /* CORE_FONT_H */
