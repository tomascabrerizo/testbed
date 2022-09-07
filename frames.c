#include "core_type.h"
#include "core.h"
#include "core_ds.h"
#include "core_math.h"
#include "renderman.h"

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

bool f_do_button(struct CoreState *state, struct Render2D *render, char *text, int x, int y, int w, int h) {
  float scale = 0.25f;
  V2 dim = render2d_get_text_dim(render, text, scale); (void)dim;

  int pos_x = (x + w/2) - (dim.x / 2);
  int pos_y = (y + h/2) - (dim.y / 2);
  
  FRect rect = (FRect){x, x + w, y, y + h};
  bool inside = f_rect_contains(rect, state->mouse_x, state->mouse_y); 

  render2d_draw_quad(render, x, y, w, h, (float)(inside == true)); 
  render2d_draw_text(render, text, pos_x, pos_y, scale);
  return false;
}
