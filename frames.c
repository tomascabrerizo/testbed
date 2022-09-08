#include "core_type.h"
#include "core.h"
#include "core_ds.h"
#include "core_math.h"
#include "renderman.h"

#include <stdlib.h>
#include <string.h>

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

typedef struct FramesUi {
  struct CoreState *state;
  struct Render2D *render;
  uint64_t hot;
  uint64_t active;
} FramesUi;

FramesUi *f_ui_create(Render2D *render) {
  FramesUi *ui = malloc(sizeof(*ui));
  memset(ui, 0, sizeof(*ui));
  
  ui->render = render;

  return ui;
}

void f_ui_destroy(struct FramesUi *ui) {
  free(ui);
}

void f_ui_set_state(struct FramesUi *ui, struct CoreState *state) {
  ui->state = state;
}

static inline uint64_t ui_get_id(char *text) {
  uint64_t result = core_hash64((const void *)text, strlen(text), CORE_MAP_SEED);
  ASSERT(result != 0);
  return result;
}

bool f_do_button(FramesUi *ui, char *text, int x, int y, int w, int h) {
  bool pressed = false;
  uint64_t id = ui_get_id(text);
  V3 color = v3(0.25, 0.37, 0.55);
  
  float scale = 0.25f; /* TODO: this text scale should be in ui state struct */
  V2 dim = render2d_get_text_dim(ui->render, text, scale); (void)dim;

  int pos_x = (x + w/2) - (dim.x / 2);
  int pos_y = (y + h/2) - (dim.y / 2);
  
  FRect rect = (FRect){x, x + w, y, y + h};
  bool inside = f_rect_contains(rect, ui->state->mouse_x, ui->state->mouse_y);
  
  /* TODO: This is clickeable code, probably make a specific function */
  if(inside && !ui->state->mouse_button_down) {
    ui->hot = id;
  }

  if(!inside && ui->hot == id) {
    ui->hot = 0;
  }
  
  if(ui->hot == id && ui->state->mouse_button_down) {
    ui->active = id;
  }

  if(ui->active == id && !ui->state->mouse_button_down) {
    pressed = ui->hot == id;
    ui->active = 0;
  }
  
  render2d_draw_quad(ui->render, x, y, w, h, (float)(ui->active == id), color); 
  render2d_draw_text(ui->render, text, pos_x, pos_y, scale, v3(0, 1, 0));
  return pressed;
}

void f_do_label(FramesUi *ui, char *text, int x, int y) {
  float scale = 0.25f; /* TODO: this text scale should be in ui state struct */
  render2d_draw_text(ui->render, text, x, y, scale, v3(0, 1, 0));
}

bool f_do_checkbox(FramesUi *ui, char *text, int x, int y, bool *checked) {
  uint64_t id = ui_get_id(text);
  float scale = 0.25f; /* TODO: this text scale should be in ui state struct */
  V3 color = v3(0.25, 0.37, 0.55);
  FRect rect = (FRect){x, x+20, y, y+20};
  bool inside = f_rect_contains(rect, ui->state->mouse_x, ui->state->mouse_y);

  /* TODO: This is clickeable code, probably make a specific function */
  if(inside && !ui->state->mouse_button_down) {
    ui->hot = id;
  }

  if(!inside && ui->hot == id) {
    ui->hot = 0;
  }
  
  if(ui->hot == id && ui->state->mouse_button_down) {
    ui->active = id;
  }

  if(ui->active == id && !ui->state->mouse_button_down) {
    if(ui->hot == id) {
      *checked = !*checked;
    }
    ui->active = 0;
  }

  render2d_draw_quad(ui->render, x, y, 20, 20, (float)(*checked == false), color);
  render2d_draw_text(ui->render, text, x + 25, y, scale, v3(0, 1, 0));

  return *checked;
}

/* TODO: UI elements to do 
  - slider
  - text edit
  - progress bar
  - Dropdown menu
  - Menu bar
  - RadioButton
*/

void print_hot(FramesUi *ui) {
    printf("hot:%ld\n", ui->hot);
}
