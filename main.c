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
  V3 split_color = v3(0.5f, 0.5f, 0.5f);
  ASSERT(container);
  switch(container->type) {
    case CONTAINER_RECT: { 
      draw_rect(render, container->rect, rect_color);
    } break;
    case CONTAINER_VSPLIT: { 
      draw_container(render, container->split.r); 
      draw_container(render, container->split.l); 
      draw_rect(render, container->split.dim, split_color);
    } break;
  }
}

void docker_update(struct CoreWindow *window) {
  docker.root.rect.l = 0;
  docker.root.rect.r = core_window_get_width(window);
  docker.root.rect.t = 0;
  docker.root.rect.b = core_window_get_height(window);

  //update_container(&docker.root);
}

void docker_render(struct Render2D *render) {
  draw_container(render, &docker.root);
}

void calculate_vsplit_size(RectContainer *container) {
  (void)container;
}

void dock_split_vertical(RectContainer *container) {
  ASSERT(container);
  ASSERT(container->type ==  CONTAINER_RECT);
  Rect old_rect = container->rect;
  int old_dim = container->rect.r - container->rect.l;
  int split_dim = 2;
  
  container->type = CONTAINER_VSPLIT;
  container->split.position = 0.5f;
  container->split.dim = container->rect; 
  container->split.dim.l = old_dim *  container->split.position - split_dim * 0.5;; 
  container->split.dim.r = container->split.dim.l + split_dim; 

  RectContainer *r = (RectContainer *)malloc(sizeof(RectContainer));
  RectContainer *l = (RectContainer *)malloc(sizeof(RectContainer));
  memset(r, 0, sizeof(*r));
  memset(l, 0, sizeof(*l));

  int rect_dim = old_dim * container->split.position;
  r->type = CONTAINER_RECT;
  r->rect = old_rect;
  r->rect.r -= rect_dim;
  
  l->type = CONTAINER_RECT;
  l->rect = old_rect;
  l->rect.l += rect_dim;

  container->split.r = r;
  container->split.l = l;
}

void frames_dock_test(struct CoreWindow *window, struct Render2D *render) {
  (void)render;
  if(!init) {
    dock_init(window);
    dock_split_vertical(&docker.root);
    init = true;
  }
  docker_update(window);
  docker_render(render);
}
