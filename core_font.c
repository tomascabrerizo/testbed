#include "core.h"
#include "core_font.h"
#include "core_ds.h"
#include "renderman.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef enum CoreTokenType {
  TOKEN_INVALID,
  TOKEN_EOF,
  TOKEN_STRING,
  TOKEN_IDENTIFIER,
  TOKEN_INT,
  TOKEN_EQUALS,
  TOKEN_MINUS,
  TOKEN_COMMA,
  
  TOKEN_COUNT,
} CoreTokenType;

static char *token_str[TOKEN_COUNT] = {
  "TOKEN_INVALID",
  "TOKEN_EOF",
  "TOKEN_STRING",
  "TOKEN_IDENTIFIER",
  "TOKEN_INT",
  "TOKEN_EQUALS",
  "TOKEN_MINUS",
  "TOKEN_COMMA",
};

typedef struct CoreToken {
  CoreTokenType type;
  uint8_t *start;
  uint8_t *end;
  union {
    int value_int;
    float value_float;
  };
} CoreToken;

static CoreToken current;

static bool is_digit(uint8_t c) {
  return (c >= '0' && c <= '9');
}

static bool is_alphanumeric(uint8_t c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || is_digit(c);
}

static bool token_is_keyword(CoreToken token, char *keyword) {
  int token_size = token.end - token.start; 
  int keyword_size = strlen(keyword);
  if(token_size != keyword_size) {
    return false;
  }
  for(int i = 0; i < token_size; ++i) {
    if(token.start[i] != keyword[i]) {
      return false;
    }
  }
  return true;
}

static CoreToken token_invalid(void) {
  current.type = TOKEN_INVALID;
  current.end++;
  return current;
}

static CoreToken token_eof(void) {
  current.type = TOKEN_EOF;
  current.end++;
  return current;
}

static CoreToken token_equal(void) {
  current.type = TOKEN_EQUALS;
  current.end++;
  return current;
}

static CoreToken token_minus(void) {
  current.type = TOKEN_MINUS;
  current.end++;
  return current;
}

static CoreToken token_comma(void) {
  current.type = TOKEN_COMMA;
  current.end++;
  return current;
}

static CoreToken token_string(void) {
  current.type = TOKEN_STRING;
  current.end++;
  while(*current.end != '"') {
    current.end++;
  }
  current.end++;
  return current;
}

static CoreToken token_identifier(void) {
  current.type = TOKEN_IDENTIFIER;
  while(is_alphanumeric(*current.end) || *current.end == '_') {
    current.end++;
  }
  return current;
}

static CoreToken token_int(void) {
  current.type = TOKEN_INT;
  int base = 10;
  current.value_int = 0;
  while(is_digit(*current.end)) {
    current.value_int *= base;
    current.value_int += (*current.end - '0');
    current.end++;
  }
  return current;
}

static void skip_white_space(void) {
  /* TODO: Skip all posibles white spaces */
  while(*current.end == ' ' || *current.end == '\n') {
    current.end++;
  }
}

static CoreToken next_token(void) {
  ASSERT(current.end != NULL);
  skip_white_space();
  current.start = current.end;
  switch(*current.start) {
    case '"': {
      return token_string();
    } break;
    case '=': {
      return token_equal();
    } break;
    case '-': {
      return token_minus();
    } break;
    case ',': {
      return token_comma();
    } break;
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': 
    case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M':
    case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z': 
    case '_': {
      return token_identifier();
    } break;
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9': {
      return token_int();
    } break;
    case '\0': {
      return token_eof();
    } break;
    default: {
      return token_invalid();
    } break;
  }
}

static CoreToken next_attribute(CoreTokenType assert_type) {
  CoreToken token = next_token(); 
  ASSERT(token.type == TOKEN_IDENTIFIER);
  token = next_token(); 
  ASSERT(token.type == TOKEN_EQUALS);
  token = next_token(); 
  ASSERT(token.type == assert_type);
  return token;
}

static int next_attribute_int() {
  bool negative = false;
  CoreToken token = next_token(); 
  ASSERT(token.type == TOKEN_IDENTIFIER);
  token = next_token(); 
  ASSERT(token.type == TOKEN_EQUALS);
  token = next_token(); 
  if(token.type == TOKEN_MINUS) {
    token = next_token();
    negative = true;
  }
  ASSERT(token.type == TOKEN_INT);
  return negative ? -token.value_int : token.value_int;
}

static CoreToken next_argument(CoreTokenType assert_type) {
  CoreToken token = next_token();
  ASSERT(token.type == TOKEN_COMMA);
  token = next_token();
  ASSERT(token.type == assert_type);
  return token;
}

static void parse_font_info(CoreFont *font) {
  CoreToken token = next_attribute(TOKEN_STRING);
  int face_size = CORE_MIN((token.end - token.start - 2), (MAX_FACE_NAME_SIZE-1));
  memcpy(font->face, (token.start + 1), face_size);
  font->face[face_size] = '\0';
  
  font->size = next_attribute(TOKEN_INT).value_int;
  /* NOTE: Unused attributes */
  next_attribute(TOKEN_INT);
  next_attribute(TOKEN_INT);
  next_attribute(TOKEN_STRING);
  next_attribute(TOKEN_INT);
  next_attribute(TOKEN_INT);
  next_attribute(TOKEN_INT);
  next_attribute(TOKEN_INT);

  /* NOTE: Used attributes */
  font->padding_left   = next_attribute(TOKEN_INT).value_int;
  font->padding_right  =  next_argument(TOKEN_INT).value_int;
  font->padding_top    =  next_argument(TOKEN_INT).value_int;
  font->padding_bottom =  next_argument(TOKEN_INT).value_int;

  /* NOTE: Unused attributes */
  next_attribute(TOKEN_INT);
  next_argument(TOKEN_INT);
}

static void parse_font_common(CoreFont *font) {
  font->line_height = next_attribute(TOKEN_INT).value_int;
  font->base = next_attribute(TOKEN_INT).value_int;
  font->atlas_w = next_attribute(TOKEN_INT).value_int;
  font->atlas_h = next_attribute(TOKEN_INT).value_int;
  /* NOTE: Unused attributes */
  next_attribute(TOKEN_INT);
  next_attribute(TOKEN_INT);
}

static void parse_font_page(CoreFont *font) {
  /* NOTE: Unused attributes */
  next_attribute(TOKEN_INT);
  /* NOTE: Texture file name */ 
  CoreToken token = next_attribute(TOKEN_STRING);
  int name_size = CORE_MIN((token.end - token.start - 2), (MAX_FACE_NAME_SIZE-1));
  memcpy(font->atlas_file_name, (token.start + 1), name_size);
  font->atlas_file_name[name_size] = '\0';
}

static void parse_font_chars(CoreFont *font) {
  font->glyph_count = next_attribute(TOKEN_INT).value_int;
}

static void parse_font_char(CoreFont *font) {
  int index = next_attribute(TOKEN_INT).value_int;
  CoreGlyph *glyph = font->glyph_table + index;
  glyph->id = index;
  glyph->x = next_attribute_int();
  glyph->y = next_attribute_int();
  glyph->w = next_attribute_int();
  glyph->h = next_attribute_int();
  glyph->offset_x = next_attribute_int();
  glyph->offset_y = next_attribute_int();
  glyph->advance = next_attribute_int();
  /* NOTE: Unused attributes */
  next_attribute(TOKEN_INT);
  next_attribute(TOKEN_INT);
}

static void parse_font_kernings(CoreFont *font) {
  /* NOTE: Unused attributes */
  next_attribute(TOKEN_INT);
  (void)font;
}


CoreFont *core_font_create(char *path) {
  (void)token_str;
  CoreFont *font = (CoreFont *)malloc(sizeof(*font));

  uint64_t size; (void)size;
  uint8_t *file = core_read_entire_file(path, &size);
  current.end = file;
  current.type = TOKEN_INVALID;
  
  while(current.type != TOKEN_EOF) {
    CoreToken token = next_token();
    
    if(token_is_keyword(token, "info")) {
      parse_font_info(font);
    } else if(token_is_keyword(token, "common")) {
      parse_font_common(font);
    } else if(token_is_keyword(token, "page")) {
      parse_font_page(font);
    } else if(token_is_keyword(token, "chars")) {
      parse_font_chars(font);
    } else if(token_is_keyword(token, "char")) {
      parse_font_char(font);
    } else if(token_is_keyword(token, "kernings")) {
      parse_font_kernings(font);
    }

    ASSERT(token.type != TOKEN_INVALID);
  }
  
  free(file);
  
  /* NOTE: Create font texture */
#define PATH_SIZE 256
  char *folder = "font/"; /* TODO: Scan path variable to search the container folder */
  int folder_len = strlen(folder);
  int name_len = strlen(font->atlas_file_name);
  ASSERT(folder_len + name_len < PATH_SIZE);
  char path_name[PATH_SIZE];
  memcpy(path_name, folder, folder_len);
  memcpy(path_name + folder_len, font->atlas_file_name, name_len);
  path_name[folder_len + name_len] = '\0';
#undef PATH_SIZE 
  printf("%s\n", path_name);
  font->atlas = render_texture_create_from_file(path_name);

  return font;
}

void core_font_destroy(CoreFont *font) {
  render_texture_destroy(font->atlas);
  free(font);
}
