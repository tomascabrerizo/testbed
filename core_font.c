#include "core.h"
#include <stdlib.h>
#include <stdio.h>

typedef enum CoreTokenType {
  TOKEN_INVALID,
  TOKEN_EOF,
  TOKEN_STRING,
  TOKEN_IDENTIFIER,
  TOKEN_INT,
  
  TOKEN_COUNT,
} CoreTokenType;

static char *token_str[TOKEN_COUNT] = {
  "TOKEN_INVALID",
  "TOKEN_EOF",
  "TOKEN_STRING",
  "TOKEN_IDENTIFIER",
  "TOKEN_INT",
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

  printf("%c\n", *current.end);
  switch(*current.start) {
    case '"': {
      return token_string();
    } break;
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': 
    case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M':
    case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z': {
      printf("token identifier\n");
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

void core_font_load(char *path) {
  uint64_t size;
  uint8_t *file = core_read_entire_file(path, &size);
  (void)size;
  current.end = file;
  current.type = TOKEN_INVALID;
  
  while(current.type != TOKEN_EOF) {
    CoreToken token = next_token();
    printf("-------------------------------------------\n");
    printf("token type:%s - %d\n", token_str[token.type], token.type);
    printf("token str: [%.*s]\n", (int)(token.end - token.start), token.start);
    printf("token value:%d\n", token.value_int);
    printf("-------------------------------------------\n");
  }
  free(file);
  /* TODO: Return font infomation */
}
