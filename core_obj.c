#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "core_obj.h"
#include "core_type.h"

CoreStr8 core_str8_from_cstr(const char * cstr) {
  CoreStr8 result = {(uint8_t *)cstr, strlen(cstr)};
  return result;
}

CoreStr8 core_str8_from_str_and_size(char * str, uint64_t size) {
  CoreStr8 result = {(uint8_t *)str, size};
  return result;
}


int core_str8_parse_int(CoreStr8 str) {
  #define STR8_TMP_BUFFER 1024
  assert(str.size < (STR8_TMP_BUFFER - 1));
  static char buffer[STR8_TMP_BUFFER]; /* TODO: Find simpler way to parse a float */
  #undef STR8_TMP_BUFFER
  memcpy(buffer, str.data, str.size);
  buffer[str.size] = '\0';
  int result = atoi(buffer);
  return result;
} 

float core_str8_parse_float(CoreStr8 str) {
  #define STR8_TMP_BUFFER 1024
  assert(str.size < (STR8_TMP_BUFFER - 1));
  static char buffer[STR8_TMP_BUFFER]; /* TODO: Find simpler way to parse a float */
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
  memset(list, 0, sizeof(*list));
  list->data = (CoreToken *)malloc(CORE_DEFAULT_TOKEN_LIST_SIZE * sizeof(*list->data));
  list->size = CORE_DEFAULT_TOKEN_LIST_SIZE;
  return list;
}

void core_token_list_destroy(CoreTokenList *list) {
  free(list->data);
  free(list);
}

CoreToken *core_token_list_pop(CoreTokenList *list) {
  if(list->head < list->count) {
    return list->data + list->head++;
  }
  return 0;
}

CoreToken *core_token_list_top(CoreTokenList *list) {
  if(list->head < list->count) {
    return list->data + list->head;
  }
  return 0;
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


static void core_tokenizer_advance_next_line(CoreTokenizer *tokenizer) {
  while(*tokenizer->end != '\n') {
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
    } else if(*c == '#') { /* NOTE: the complete line is a comment */
      core_tokenizer_advance_next_line(&tokenizer);
    }
    /* TODO: Assert that tokenizer.end is not the null terminator '\0' */
    core_tokenizer_advance_next_token(&tokenizer);
  }

  return list;
}

void core_token_list_to_vertex_and_index_list(CoreTokenList *list, CoreObjCtx *ctx) {
  ctx->v_list = (float *)malloc(ctx->v_count * sizeof(float));
  ctx->i_list = (int *)malloc(ctx->i_count * sizeof(int));
  uint64_t v_cur = 0;
  uint64_t i_cur = 0;
  (void)v_cur;
  (void)i_cur;
  
  CoreToken *token = 0;
  while((token = core_token_list_pop(list))) {
    switch(token->type) {
      case CORE_TOKEN_V: {
        assert(core_token_list_top(list)->type == CORE_TOKEN_NUMBER);
        while(core_token_list_top(list) && core_token_list_top(list)->type == CORE_TOKEN_NUMBER) {
          assert(v_cur <= ctx->v_count);
          token = core_token_list_pop(list);
          float *number = ctx->v_list + v_cur++;
          *number = core_str8_parse_float(token->data);
        }
      } break;
      case CORE_TOKEN_F: {
        assert(core_token_list_top(list)->type == CORE_TOKEN_NUMBER);
        while(core_token_list_top(list) && core_token_list_top(list)->type == CORE_TOKEN_NUMBER) {
          assert(i_cur < ctx->i_count);
          token = core_token_list_pop(list);
          
          int *number = ctx->i_list + i_cur++;
          *number = core_str8_parse_int(token->data);
          while(core_token_list_top(list) && core_token_list_top(list)->type == CORE_TOKEN_SLASH) { 
            /* TODO: for now just skip other indices */
            core_token_list_pop(list);
            core_token_list_pop(list);
          }
        }
      } break;
      default: { /* TODO: Invalid code path */ } break;
    }
  }
}

CoreObjCtx *core_obj_create(const char *path) {
  CoreObjCtx *ctx = (CoreObjCtx *)malloc(sizeof(*ctx));
  memset(ctx, 0, sizeof(*ctx));
  CoreFile *obj_file = core_file_create((char *)path);
  CoreTokenList *tokens = core_tokenize_obj_file(ctx, obj_file);
  core_token_list_to_vertex_and_index_list(tokens, ctx);
  core_token_list_destroy(tokens);
  core_file_destroy(obj_file);
  return ctx;
}

void core_obj_destroy(CoreObjCtx *ctx) {
  free(ctx->v_list);
  free(ctx->i_list);
  free(ctx);
}
