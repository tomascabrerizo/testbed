#include "core_type.h"
#include "core_ds.h"
#include "core_math.h"

typedef struct FRect {
  int left;
  int right;
  int top;
  int bottom;
} FRect;

FRect f_rect(int left, int right, int top, int bottom) {
  return (FRect){left, right, top, bottom};
}

FRect f_rect_intersection(FRect a, FRect b) {
  int left = a.left >= b.left ? a.left : b.left;
  int right = a.right <= b.right ? a.right : b.right;
  int top = a.top >= b.top ? a.top : b.top;
  int bottom = a.bottom <= b.bottom ? a.bottom: b.bottom;
  return (FRect){left, right, top, bottom};
}

FRect f_rect_bounding(FRect a, FRect b) {
  int left = a.left <= b.left ? a.left : b.left;
  int right = a.right >= b.right ? a.right : b.right;
  int top = a.top <= b.top ? a.top : b.top;
  int bottom = a.bottom >= b.bottom ? a.bottom: b.bottom;
  return (FRect){left, right, top, bottom};
}

FRect f_rect_translate(FRect a, int offset_x, int offset_y) {
  a.left   += offset_x;
  a.right  += offset_x;
  a.top    += offset_y;
  a.bottom += offset_y;
  return a;
}

bool f_rect_contains(FRect a, int x, int y) {
  return ((x >= a.left) && (x <= a.right) && (y >= a.top && y <= a.bottom));
}

bool f_rect_is_valid(FRect a) {
  return ((a.right - a.left) >= 0) && ((a.bottom - a.top) >= 0);
}

typedef enum FDokerType {
  F_DOKER_NORTH,
  F_DOKER_SOUTH,
  F_DOKER_EAST,
  F_DOKER_WEST,
  F_DOKER_CENTER,

  F_DOKER_COUNT,
} FDokerType;

typedef unsigned int FHandle;
#define FINVALID 0

typedef struct FFrame {
  FRect rect;
  FRect clip;
  int border;
  FHandle doker[F_DOKER_COUNT];
  
  FHandle parent;
  FHandle child_first;
  FHandle child_last;
  FHandle next;
  FHandle prev;
  

  FHandle handle;
  void *id;
} FFrame;

typedef struct FState {
  FHandle root;
  FHandle parent;
  FFrame *frame_buf;

} FState;

static FState state; /* TODO: for now frames state is global */
void f_init() {
  FFrame invalid = {0}; /* TODO: Invalid Handle 0 poinste to invalid frame */
  invalid.handle = core_buf_size(state.frame_buf);
  core_buf_push(state.frame_buf, invalid);
  ASSERT(core_buf_size(state.frame_buf) > FINVALID);
}

void f_shutdown() {
  core_buf_free(state.frame_buf);
}

void f_begin() {
  state.root = FINVALID;
  state.parent = FINVALID;
}

void f_end() {
} 

FFrame *f_frame(FHandle handle) {
  ASSERT(handle > FINVALID);
  ASSERT(handle < core_buf_size(state.frame_buf));
  return &state.frame_buf[handle];
}

FFrame *f_get_frame_from_register(void *id) {
  /* TODO: use a hash table instead of a streched buffer */
  FFrame *buf = state.frame_buf;
  for(unsigned int i = 0; i < (unsigned int)core_buf_size(buf); ++i) {
    if(buf[i].id == id) {
      return &buf[i];
    }
  }
  
  /* NOTE: if the frame is not register add new one */
  FFrame f;
  ASSERT(core_buf_size(state.frame_buf) > FINVALID);
  f.handle = core_buf_size(state.frame_buf);
  core_buf_push(state.frame_buf, f);
  return f_frame(f.handle);
}

FHandle f_push_parent(void *id, FRect rect) {
  FFrame *frame = f_get_frame_from_register(id);
  ASSERT(frame);
  FFrame *parent = f_frame(state.parent);
  frame->rect = rect;
  frame->clip = parent ? f_rect_intersection(f_rect_translate(rect, parent->rect.left, parent->rect.top), parent->clip) : rect;
  frame->border = 10;
  frame->parent = state.parent;
  
  frame->child_first = FINVALID;
  frame->child_last = FINVALID;
  frame->next = FINVALID;
  frame->prev = FINVALID;

  if(!state.parent) {
    state.root = frame->handle;
  }
  
  state.parent = frame->handle;
  
  return frame->handle;
}

FHandle f_push_frame(void *id, FRect rect) {
  FFrame *frame = f_get_frame_from_register(id);
  ASSERT(frame);
  ASSERT(state.parent);
  FFrame *parent = f_frame(state.parent);
  frame->rect = rect;
  frame->clip = f_rect_intersection(f_rect_translate(rect, parent->rect.left, parent->rect.top), parent->clip);
  frame->border = 10;
  frame->parent = state.parent;

  return frame->handle;
}
