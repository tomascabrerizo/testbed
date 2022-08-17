#include "platform_obj.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>


CoreStr8 core_str8_from_cstr(const char * cstr) {
  CoreStr8 result = {(uint8_t *)cstr, strlen(cstr)};
  return result;
}

CoreStr8 core_str8_from_str_and_size(char * str, uint64_t size) {
  CoreStr8 result = {(uint8_t *)str, size};
  return result;
}

float core_str8_parse_float(CoreStr8 str) {
  #define STR8_TMP_BUFFER 1024
  assert(str.size < (STR8_TMP_BUFFER - 1));
  char buffer[STR8_TMP_BUFFER]; /* TODO: Find simpler way to parse a float */
  #undef STR8_TMP_BUFFER
  memcpy(buffer, str.data, str.size);
  buffer[str.size] = '\0';
  float result = atof(buffer);
  return result;
}

CoreFile *core_file_create(char *path) {
  FILE *f = fopen(path, "rb");
  if(!f) {
    printf("Cannot open file %s\n", path);
  }

  fseek(f, 0, SEEK_END);
  uint64_t size = ftell(f);
  fseek(f, 0, SEEK_SET);
  
  CoreFile *file = (CoreFile *)malloc(sizeof(*file) + size+1);
  file->data = (uint8_t *)file + sizeof(*file);
  file->size = size;
  file->data[file->size] = '\0';
  fread(file->data, size, 1, f);
  fclose(f);
  return file;
}

void core_file_destroy(CoreFile *file) {
  free(file);
}

CoreTokenList *core_token_list_create() {
  CoreTokenList *list = (CoreTokenList *)malloc(sizeof(*list));
  list->data = (CoreToken *)malloc(CORE_DEFAULT_TOKEN_LIST_SIZE * sizeof(*list->data));
  list->size = CORE_DEFAULT_TOKEN_LIST_SIZE;
  return list;
}

void core_token_list_destroy(CoreTokenList *list) {
  free(list->data);
  free(list);
}

void core_token_list_push(CoreTokenList *list, CoreToken token) {
  if(list->count + 1 >= list->size) {
    list->size = list->size * 2;
    list->data = (CoreToken *)realloc(list->data, list->size * sizeof(token));
  }
  list->data[list->count++] = token; 
}

static void core_tokenizer_advance_next_token(CoreTokenizer *tokenizer) {
  tokenizer->cur = tokenizer->end;
  tokenizer->end = tokenizer->cur + 1;
}

static void core_tokenizer_push_token(CoreTokenizer *tokenizer, CoreTokenList *list, CoreTokeType type) {
  CoreToken token;
  token.type = type;
  token.data.data = tokenizer->cur;
  token.data.size = tokenizer->end - tokenizer->cur;
  core_token_list_push(list, token);
}

static bool core_is_number(char c) {
  return (c >= '0' && c <= '9');
}

static void core_tokenizer_advance_while_number(CoreTokenizer *tokenizer) {
  while(core_is_number(*tokenizer->end) || *tokenizer->end == '.' || *tokenizer->end == 'e' || *tokenizer->end == '-') {
    ++tokenizer->end;
  }
}

CoreTokenList *core_tokenize_obj_file(CoreObjCtx *ctx, CoreFile *file) {
  CoreTokenizer tokenizer;
  tokenizer.src = core_str8_from_str_and_size((char *)file->data, file->size);
  tokenizer.cur = tokenizer.src.data;
  tokenizer.end = tokenizer.cur + 1;

  CoreTokenList *list =  core_token_list_create();
  
  while(*tokenizer.end != '\0') {
    char *c = (char *)tokenizer.cur;
    if(*c == 'v') {
      if(*(c+1) == 't') {
        tokenizer.end++;
        core_tokenizer_push_token(&tokenizer, list, CORE_TOKEN_VT);
      } else if(*(c+1) == 'n') {
        tokenizer.end++;
        core_tokenizer_push_token(&tokenizer, list, CORE_TOKEN_VN);
      } else {
        core_tokenizer_push_token(&tokenizer, list, CORE_TOKEN_V);
        ctx->v_count += 3;
      }
    } else if(*c == 'f') {
      core_tokenizer_push_token(&tokenizer, list, CORE_TOKEN_F);
      ctx->i_count += 3; /* TODO: Search how obj files tears indices */
    } else if(*c == '/') {
      core_tokenizer_push_token(&tokenizer, list, CORE_TOKEN_SLASH);
    } else if(core_is_number(*c) || *c == '-' || *c == '.') {
      core_tokenizer_advance_while_number(&tokenizer);
      core_tokenizer_push_token(&tokenizer, list, CORE_TOKEN_NUMBER);
    }
    /* TODO: Assert that tokenizer.end is not the null terminator '\0'*/
    core_tokenizer_advance_next_token(&tokenizer);
  }

  return list;
}

void core_token_list_to_vertex_and_index_list(CoreTokenList *list, CoreObjCtx *ctx) {
  ctx->v_list = (float *)malloc(ctx->v_count * sizeof(float));
  ctx->i_list = (int *)malloc(ctx->i_count * sizeof(int));
  uint64_t v_cur = 0;
  uint64_t i_cur = 0;
  
  uint64_t head = 0;
  while(head < list->count) {
    CoreToken *token = list->data + head++;
    switch(token->type) {
      case CORE_TOKEN_V: {
        printf("Token V: %.*s\n", (int)token->data.size, token->data.data);
        token = list->data + head; 
        while(token->type == CORE_TOKEN_NUMBER) {
          printf("Token NUMBER: %.*s\n", (int)token->data.size, token->data.data);
          float number = core_str8_parse_float(token->data);
          printf("Number: %f\n", number);
          
          /* TODO: Make this while a toke stack make this logic a lot simpler */
          head++;
          token = list->data + head; 
          printf("cur:%ld, total:%ld\n", ++v_cur, ctx->v_count);
          /* TODO: Unfinished code */
        }
      } break;
      case CORE_TOKEN_F: {
        /*printf("Token F: %.*s\n", (int)token->data.size, token->data.data);*/ 
        (void)i_cur;
      } break;
      default: { /* TODO: Invalid code path */ } break;
    }
  }
}
