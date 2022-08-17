#ifndef PLATFORM_OBJ_H
#define PLATFORM_OBJ_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>

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

typedef struct CoreToken {
  CoreTokeType type;
  CoreStr8 data;
} CoreToken;

#define CORE_DEFAULT_TOKEN_LIST_SIZE 128
typedef struct CoreTokenList{
  CoreToken *data;
  uint64_t count;
  uint64_t size;
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

CoreStr8 core_str8_from_cstr(const char * cstr);
CoreStr8 core_str8_from_str_and_size(char * cstr, uint64_t size);

CoreFile *core_file_create(char *path); 
void core_file_destroy(CoreFile *file);

CoreTokenList *core_token_list_create();
void core_token_list_destroy(CoreTokenList *list); 
void core_token_list_push(CoreTokenList *list, CoreToken token);

CoreTokenList *core_tokenize_file(CoreFile *file);

#endif /* PLATFORM_OBJ_H */
