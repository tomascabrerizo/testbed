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
void frames_dock_test(struct CoreWindow *window, struct Render2D *render);

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
#if DOCK_TEST 
    frames_dock_test(window, render);
#else
    frames_ui_test(ui);
#endif

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
  struct RectContainer *r;
  struct RectContainer *l;
} VSplitter;

typedef enum ContainerType {
  CONTAINER_RECT,
  CONTAINER_VSPLIT,
} ContainerType;

typedef struct RectContainer {
  ContainerType type;
  Rect rect; 
  VSplitter split;
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
  V3 split_color = v3(0.0f, 1.0f, 0.0f);
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
      draw_container(render, container->split.r); 
      draw_container(render, container->split.l); 
      draw_rect(render, container->split.dim, split_color);
    } break;
  }
}

void calculate_vsplit_size(RectContainer *container);
void update_container(RectContainer *container) {
  ASSERT(container);
  if(container->type == CONTAINER_VSPLIT) { 
      calculate_vsplit_size(container);
      update_container(container->split.l);
      update_container(container->split.r);
  }
}

void docker_update(struct CoreWindow *window) {
  docker.root.rect.l = 0;
  docker.root.rect.r = core_window_get_width(window);
  docker.root.rect.t = 0;
  docker.root.rect.b = core_window_get_height(window);

  update_container(&docker.root);
}

void docker_render(struct Render2D *render) {
  draw_container(render, &docker.root);
}

void dock_split_vertical(RectContainer *container) {
  ASSERT(container);
  ASSERT(container->type ==  CONTAINER_RECT);
  
  container->type = CONTAINER_VSPLIT;
  container->split.position = 0.5f;

  container->split.r = (RectContainer *)malloc(sizeof(RectContainer));
  container->split.l = (RectContainer *)malloc(sizeof(RectContainer));
  container->split.r->type = CONTAINER_RECT;
  container->split.l->type = CONTAINER_RECT;
  
  //calculate_vsplit_size(container);
}

void calculate_vsplit_size(RectContainer *container) {
  ASSERT(container->type == CONTAINER_VSPLIT); 
  Rect rect = container->rect;
  int rect_dim = (rect.r - rect.l);
  /* NOTE: rects position */
  Rect r = rect;
  Rect l = rect;
  l.r -= rect_dim * (1.0f - container->split.position);
  r.l += rect_dim * container->split.position;
  /* NOTE: split rect */
  int split_dim = 4;
  Rect split_rect = rect;
  /* TODO: Make f32_lerp function */
  split_rect.l = rect.l*(1.0f - container->split.position) + rect.r*container->split.position - (split_dim/2);
  split_rect.r = split_rect.l + split_dim;

  container->split.r->rect = r;
  container->split.l->rect = l;
  container->split.dim = split_rect;
}

void frames_dock_test(struct CoreWindow *window, struct Render2D *render) {
  (void)render;
  if(!init) {
    dock_init(window);
    dock_split_vertical(&docker.root);
    docker.root.split.position = 0.4f;
    dock_split_vertical(docker.root.split.r);
    docker.root.split.r->split.position = 0.7f;
    init = true;
  }
  docker_update(window);
  docker_render(render);
}
