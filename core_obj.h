#ifndef CORE_OBJ_H
#define CORE_OBJ_H

#include <string.h>
#include <stdio.h>
#include "core_type.h"
#include "core_math.h"

/* TODO: Create a specefic header file for common macros and constants */
#define CORE_KB(v) ((v)*1024LL)
#define CORE_MB(v) (CORE_KB(V)*1024LL)
#define CORE_GB(v) (CORE_MB(V)*1024LL)

/* TODO: Move CoreStr8 to a separate file */
typedef struct CoreStr8 {
  uint8_t *data;
  uint64_t size;
} CoreStr8;

typedef enum CoreTokeType {
  CORE_TOKEN_TYPE_UNKNOW,
  CORE_TOKEN_V,
  CORE_TOKEN_VT,
  CORE_TOKEN_VN,
  CORE_TOKEN_F,
  CORE_TOKEN_SLASH,
  CORE_TOKEN_NUMBER,

  /* NOTE: must be always at the end */
  CORE_TOKEN_TYPE_COUNT
} CoreTokeType;

/* TODO: Move CoreTokenizer to a separate file */
typedef struct CoreToken {
  CoreStr8 data;
  CoreTokeType type;
} CoreToken;

#define CORE_DEFAULT_TOKEN_LIST_SIZE CORE_KB(4) 
typedef struct CoreTokenList{
  CoreToken *data;
  uint64_t count;
  uint64_t size;
  uint64_t head;
} CoreTokenList;

typedef struct CoreTokenizer {
  uint8_t *cur;
  uint8_t *end;
  CoreStr8 src;
} CoreTokenizer;

typedef struct CoreFile {
  uint8_t *data;
  uint64_t size;
} CoreFile;

typedef struct CoreVertex {
  V3 p;
  V3 n;
} CoreVertex;

typedef struct CoreObjCtx {
  CoreVertex *vertex_list;
  
  int *i_list;
  uint64_t i_count;

  float *v_list;
  uint64_t v_count;
  
  float *n_list;
  uint64_t n_count;
} CoreObjCtx;

CoreStr8 core_str8_from_cstr(const char * cstr);
CoreStr8 core_str8_from_str_and_size(char * cstr, uint64_t size);
int core_str8_parse_int(CoreStr8 str); 
float core_str8_parse_float(CoreStr8 str); 

CoreFile *core_file_create(char *path); 
void core_file_destroy(CoreFile *file);

CoreTokenList *core_tokenize_obj_file(CoreObjCtx *ctx, CoreFile *file);

CoreTokenList *core_token_list_create();
void core_token_list_destroy(CoreTokenList *list); 
void core_token_list_push(CoreTokenList *list, CoreToken token);
CoreToken *core_token_list_pop(CoreTokenList *list); /* NOTE: Pops a token from the front of the list */
CoreToken *core_token_list_top(CoreTokenList *list);
/* TODO: Dont save tokens in al list, just push vertex and indices into strech buffers while lexing each token */
void core_token_list_to_vertex_and_index_list(CoreTokenList *list, CoreObjCtx *ctx);

CoreObjCtx *core_obj_create(const char *path);
void core_obj_destroy(CoreObjCtx *ctx);

#endif /* CORE_OBJ_H */
