#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>

#include "core.h"
#include "core_math.h"
#include "core_ds.h"
#include "renderman.h"
#include "core_gl.h"

#if 1
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#else
#define WINDOW_WIDTH 64
#define WINDOW_HEIGHT 64
#endif


struct FramesUi;
struct FramesUi *f_ui_create(struct Render2D *render);
void f_ui_destroy(struct FramesUi *ui);
void f_ui_set_state(struct FramesUi *ui, struct CoreState *state);
void print_hot(struct FramesUi *ui);

bool f_do_button(struct FramesUi *ui, char *text, int x, int y, int w, int h);
void f_do_label(struct FramesUi *ui, char *text, int x, int y);
bool f_do_checkbox(struct FramesUi *ui, char *text, int x, int y, bool *checked);

#define DOCK_TEST 1
void frames_dock_test(struct CoreWindow *window, struct Render2D *render, struct CoreState *state);

/* NOTE: UI state */
bool checked;

void frames_ui_test(struct FramesUi *ui) {
  f_do_label(ui, "Label:", 20, 100);
  f_do_label(ui, "Im a Label.", 145, 100);

  f_do_label(ui, "Button:", 20, 140);
  f_do_button(ui, "Button:", 145, 135, 120, 30);


  f_do_label(ui, "Checked Box:", 20, 180);
  f_do_checkbox(ui, "checkbox", 145, 180, &checked);
}

int main(void) {
  CoreWindow *window = core_window_create("TestBed", WINDOW_WIDTH, WINDOW_HEIGHT);
  Render2D *render = render2d_create();
  render2d_alpha_test(render, true);
  
  struct FramesUi *ui = f_ui_create(render);

  bool running = true;
  while(running) {
    CoreState *state = core_state_get_state(window);
    f_ui_set_state(ui, state);
    if(state->quit) {
      printf("Quitting application\n");
      running = false;
    }
    
    if(state->resize) {
      printf("The window is resizing\n");
      glViewport(0, 0, state->width, state->height);
      render2d_set_resolution(render, state->width, state->height);
    }
    
    render2d_begin(render);
    frames_dock_test(window, render, state);
    frames_ui_test(ui);

    render2d_end(render);

    core_window_swap_buffers(window);
  }

  f_ui_destroy(ui);
  render2d_destroy(render);
  core_window_destroy(window);

  return 0;
}

/* NOTE: Window docking algorithm testing */

typedef struct Rect {
  int l, r, t, b;
} Rect;

struct RectContainer;

typedef struct VSplitter {
  float position;
  Rect dim;
  V3 color;
  bool grabing;
  struct RectContainer *r;
  struct RectContainer *l;
} VSplitter;

typedef struct HSplitter {
  float position;
  Rect dim;
  V3 color;
  bool grabing;
  struct RectContainer *t;
  struct RectContainer *b;
} HSplitter;

typedef enum ContainerType {
  CONTAINER_RECT,
  CONTAINER_VSPLIT,
  CONTAINER_HSPLIT,
} ContainerType;

typedef struct RectContainer {
  ContainerType type;
  Rect rect; 
  union {
    VSplitter vsplit;
    HSplitter hsplit;
  };
} RectContainer;

typedef struct Docker {
  RectContainer root;
} Docker;


Docker docker;
bool init = false;

#include <string.h>
#include <stdlib.h>

static void dock_init(struct CoreWindow *window) {
  (void)window;
  memset(&docker, 0, sizeof(docker));

  docker.root.type = CONTAINER_RECT;
  docker.root.rect.l = 0;
  docker.root.rect.r = core_window_get_width(window);
  docker.root.rect.t = 0;
  docker.root.rect.b = core_window_get_height(window);
}

void draw_rect(struct Render2D *render, Rect rect, V3 color) {
  render2d_draw_quad(render, rect.l, rect.t, rect.r - rect.l, rect.b - rect.t, 0, color);
}

void draw_container(struct Render2D *render, RectContainer *container) {
  V3 rect_color = v3(0.2f, 0.2f, 0.2f);
  ASSERT(container);
  switch(container->type) {
    case CONTAINER_RECT: { 
      draw_rect(render, container->rect, v3(0.1, 0.1, 0.1));
      int border = 8;
      Rect rect = container->rect;
      rect.l += border;
      rect.t += border;
      rect.r -= border;
      rect.b -= border;
      draw_rect(render, rect, rect_color);
    } break;
    case CONTAINER_VSPLIT: { 
      draw_container(render, container->vsplit.r); 
      draw_container(render, container->vsplit.l); 
      draw_rect(render, container->vsplit.dim, container->vsplit.color);
    } break;
    case CONTAINER_HSPLIT: { 
      draw_container(render, container->vsplit.r); 
      draw_container(render, container->vsplit.l); 
      draw_rect(render, container->hsplit.dim, container->hsplit.color);
    } break;
  }
}

void calculate_vsplit_size(RectContainer *container);
void calculate_hsplit_size(RectContainer *container);


bool rect_contains(Rect a, int x, int y) {
  return ((x >= a.l) && (x <= a.r) && (y >= a.t && y <= a.b));
}

void update_container(RectContainer *container, struct CoreState *state) {
  ASSERT(container);
  V3 split_color = v3(0.0f, 1.0f, 0.0f);
  if(container->type == CONTAINER_VSPLIT) { 
      calculate_vsplit_size(container);
      update_container(container->vsplit.l, state);
      update_container(container->vsplit.r, state);
      
      container->vsplit.color = split_color; 
      bool inside = false;
      if(rect_contains(container->vsplit.dim, state->mouse_x, state->mouse_y)) {
        container->vsplit.color = v3(1, 0, 0);
        inside = true;
      }
      if(inside && state->mouse_button_down) container->vsplit.grabing = true;
      if(!state->mouse_button_down) container->vsplit.grabing = false;

      if(container->vsplit.grabing) {
        float mouse_x = (float)(state->mouse_x - container->rect.l) / (container->rect.r - container->rect.l);
        container->vsplit.position = CORE_CLAMP(mouse_x, 0.01, 0.99);
      }


  } else if(container->type == CONTAINER_HSPLIT) { 
      calculate_hsplit_size(container);
      update_container(container->hsplit.t, state);
      update_container(container->hsplit.b, state);

      container->hsplit.color = split_color; 
      bool inside = false; (void)inside;
      if(rect_contains(container->hsplit.dim, state->mouse_x, state->mouse_y)) {
        container->hsplit.color = v3(1, 0, 0);
        inside = true;
      }
      if(inside && state->mouse_button_down) container->hsplit.grabing = true;
      if(!state->mouse_button_down) container->hsplit.grabing = false;

      if(container->hsplit.grabing) {
        float mouse_y = (float)(state->mouse_y - container->rect.t) / (container->rect.b - container->rect.t);
        container->hsplit.position = CORE_CLAMP(mouse_y, 0.01, 0.99);
      }
  }
}

void docker_update(struct CoreWindow *window, struct CoreState *state) {
  docker.root.rect.l = 0;
  docker.root.rect.r = core_window_get_width(window);
  docker.root.rect.t = 0;
  docker.root.rect.b = core_window_get_height(window);

  update_container(&docker.root, state);
}

void docker_render(struct Render2D *render) {
  draw_container(render, &docker.root);
}

void dock_split_vertical(RectContainer *container) {
  ASSERT(container);
  ASSERT(container->type ==  CONTAINER_RECT);
  
  container->type = CONTAINER_VSPLIT;
  container->vsplit.position = 0.5f;

  container->vsplit.r = (RectContainer *)malloc(sizeof(RectContainer));
  container->vsplit.l = (RectContainer *)malloc(sizeof(RectContainer));
  container->vsplit.r->type = CONTAINER_RECT;
  container->vsplit.l->type = CONTAINER_RECT;
  
  //calculate_vsplit_size(container);
}

void dock_split_horizontal(RectContainer *container) {
  ASSERT(container);
  ASSERT(container->type ==  CONTAINER_RECT);
  
  container->type = CONTAINER_HSPLIT;
  container->hsplit.position = 0.5f;

  container->hsplit.t = (RectContainer *)malloc(sizeof(RectContainer));
  container->hsplit.b = (RectContainer *)malloc(sizeof(RectContainer));
  container->hsplit.t->type = CONTAINER_RECT;
  container->hsplit.b->type = CONTAINER_RECT;
  
  //calculate_hsplit_size(container);
}

void calculate_vsplit_size(RectContainer *container) {
  ASSERT(container->type == CONTAINER_VSPLIT); 
  Rect rect = container->rect;
  int rect_dim = (rect.r - rect.l);
  /* NOTE: rects position */
  Rect r = rect;
  Rect l = rect;
  l.r -= rect_dim * (1.0f - container->vsplit.position);
  r.l += rect_dim * container->vsplit.position;
  /* NOTE: split rect */
  int split_dim = 4;
  Rect split_rect = rect;
  /* TODO: Make f32_lerp function */
  split_rect.l = rect.l*(1.0f - container->vsplit.position) + rect.r*container->vsplit.position - (split_dim/2);
  split_rect.r = split_rect.l + split_dim;

  container->vsplit.r->rect = r;
  container->vsplit.l->rect = l;
  container->vsplit.dim = split_rect;
}

void calculate_hsplit_size(RectContainer *container) {
  ASSERT(container->type == CONTAINER_HSPLIT); 
  Rect rect = container->rect;
  int rect_dim = (rect.b - rect.t);
  /* NOTE: rects position */
  Rect t = rect;
  Rect b = rect;
  b.t += rect_dim * container->hsplit.position;
  t.b -= rect_dim * (1.0f - container->hsplit.position);
  /* NOTE: split rect */
  int split_dim = 4;
  Rect split_rect = rect;
  /* TODO: Make f32_lerp function */
  split_rect.t = rect.t*(1.0f - container->hsplit.position) + rect.b*container->hsplit.position - (split_dim/2);
  split_rect.b = split_rect.t + split_dim;

  container->hsplit.b->rect = b;
  container->hsplit.t->rect = t;
  container->hsplit.dim = split_rect;
}

void frames_dock_test(struct CoreWindow *window, struct Render2D *render, struct CoreState *state) {
  (void)render;
  if(!init) {
    dock_init(window);
    dock_split_vertical(&docker.root);
    docker.root.vsplit.position = 0.4f;
    dock_split_vertical(docker.root.vsplit.r);
    docker.root.vsplit.r->vsplit.position = 0.7f;

    dock_split_horizontal(docker.root.vsplit.l);
    docker.root.vsplit.l->hsplit.position = 0.6f;

    dock_split_horizontal(docker.root.vsplit.r->vsplit.l);

    init = true;
  }
  docker_update(window, state);
  docker_render(render);
}
