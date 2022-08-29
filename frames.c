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

typedef struct FFrame {
  FRect rect;
  FRect clip;
  int border;
  struct FFrame *doker[F_DOKER_COUNT];
} FFrame;

typedef struct FState {
  FFrame root;
  FFrame *frame_buf;
} FState;
