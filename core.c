#define CORE_INTERNAL
#include "core.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* NOTE: platform specific window */
typedef struct CoreWindow {
  int width;
  int height;
  
  Display *d;
  Window w;
  Colormap cm;
  GLXContext ctx;
  
  Atom wmDeleteMessage;

  CoreState state;
} CoreWindow;

#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display *, GLXFBConfig, GLXContext, Bool, const int*);
typedef void (*glXSwapIntervalEXTProc)(Display *, GLXDrawable , int );

CoreWindow *core_window_create(char *name, int width, int height) {
  CoreWindow *window = (CoreWindow *)malloc(sizeof(*window));
  memset(window, 0, sizeof(*window));

  window->width = width;
  window->height = height;
  
  window->d = XOpenDisplay(NULL);
  if(!window->d) {
    printf("Fail to open X11 Display\n");
    return NULL;
  }

  static int fb_attr[] = {
    GLX_X_RENDERABLE, True,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_ALPHA_SIZE, 8,
    GLX_DEPTH_SIZE, 24,
    GLX_STENCIL_SIZE, 8,
    GLX_DOUBLEBUFFER, True,
    None
  };
  
  int fbcount;
  GLXFBConfig* fbc = glXChooseFBConfig(window->d, DefaultScreen(window->d), fb_attr, &fbcount);
  if (!fbc) {
    printf("Failed to retrieve a X11 framebuffer config\n");
    return NULL;
  }
  
  /* Pick the FB config/visual with the most samples per pixel */
  int best_fbc_index = -1, worst_fbc_index = -1, best_num_samp = -1, worst_num_samp = 999;
  
  int i;
  for(i = 0; i < fbcount; ++i) {
    XVisualInfo *vi = glXGetVisualFromFBConfig(window->d, fbc[i]);
    if(vi) {
      int samp_buf, samples;
      glXGetFBConfigAttrib(window->d, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
      glXGetFBConfigAttrib(window->d, fbc[i], GLX_SAMPLES, &samples);
      
      if((best_fbc_index < 0 || samp_buf) && samples > best_num_samp)
        best_fbc_index = i, best_num_samp = samples;
      if((worst_fbc_index < 0 || !samp_buf) || samples < worst_num_samp)
        worst_fbc_index = i, worst_num_samp = samples;
    }
    XFree(vi);
  }

  GLXFBConfig best_fbc = fbc[best_fbc_index];
  XFree(fbc);
  XVisualInfo *vi = glXGetVisualFromFBConfig(window->d, best_fbc);
  
  XSetWindowAttributes swa;
  window->cm = XCreateColormap(window->d,
                               RootWindow(window->d, vi->screen), 
                               vi->visual, AllocNone);
  swa.colormap = window->cm;
  swa.background_pixmap = None ;
  swa.border_pixel      = 0;
  swa.event_mask        = StructureNotifyMask;

  window->w = XCreateWindow(window->d, RootWindow(window->d, vi->screen), 
                            0, 0, window->width, window->height, 0, vi->depth, InputOutput, vi->visual, 
                            CWBorderPixel|CWColormap|CWEventMask, &swa);
  if(!window->w) {
    printf("Failed to create X11 window.\n");
    return NULL;
  }
  
  XFree(vi);
  
  XStoreName(window->d, window->w, name);
  XMapWindow(window->d, window->w);
  
  /* TODO: investigate why is necesary to do this to catch WM_DELETE_WINDOW message*/
  window->wmDeleteMessage = XInternAtom(window->d, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(window->d, window->w, &window->wmDeleteMessage, 1);

  glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
  glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte *)"glXCreateContextAttribsARB");
  
  int ctx_attribs[] = {
    GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
    GLX_CONTEXT_MINOR_VERSION_ARB, 3,
    None
  };
  
  /* TODO: Get context version to be sure we are in 3.3 version */
  window->ctx = glXCreateContextAttribsARB(window->d, best_fbc, 0, True, ctx_attribs);
  XSync(window->d, False);
  if(!glXIsDirect(window->d, window->ctx)) {
    printf("Indirect rendering not supported\n");
    return NULL;
  }

  glXMakeCurrent(window->d, window->w, window->ctx); 
  
  /* Set vertical Sync */
  glXSwapIntervalEXTProc glXSwapIntervalEXT = (glXSwapIntervalEXTProc)glXGetProcAddress((GLubyte *)"glXSwapIntervalEXT");
  if(glXSwapIntervalEXT) {
    glXSwapIntervalEXT(window->d, window->w, 1);
    printf("Vertical Sync enable\n");
  } else {
    printf("vertical Sync disable\n");
  }
  /* TODO: load opnegl function: */
  #define X(return, name, params) name = (CORE_GL_PROC(name))glXGetProcAddress((GLubyte *)#name);
  CORE_GL_FUNCTIONS(X);
  #undef X

  return window;
}

void core_window_destroy(CoreWindow *window) {
  glXDestroyContext(window->d, window->ctx);
  XFreeColormap(window->d, window->cm);
  XDestroyWindow(window->d, window->w); 
  XCloseDisplay(window->d);
  free(window);
}

void core_window_swap_buffers(CoreWindow *window) {
  glXSwapBuffers(window->d, window->w);
}

int core_window_get_width(CoreWindow *window) {
    return window->width;
}

int core_window_get_height(CoreWindow *window) {
  return window->height;
}

static void core_state_reset(CoreState *state) {
  state->quit = false;
  state->resize = false;
}

CoreState *core_state_get_state(struct CoreWindow *window) {
  core_state_reset(&window->state);
  while(XPending(window->d)) {
    XEvent event;
    XNextEvent(window->d, &event);
    switch(event.type) {
      case Expose: {
      } break;
      case ConfigureNotify: {
        XConfigureEvent xce = event.xconfigure;
        if (xce.width != window->width || xce.height != window->height) {
          window->state.resize = true;
          window->state.width = xce.width;
          window->state.height = xce.height;
          /* TODO: Remove redundant data like this duplications */
          window->width = xce.width;
          window->height = xce.height;
        }
      } break;
      case ClientMessage: {
        if (event.xclient.data.l[0] == (long int)window->wmDeleteMessage) {
          window->state.quit = true;
        }
      } break;
      default: {
        /* TODO: If the event is not handle do nothing */
      } break;
    }
  }
  return &window->state;
}
