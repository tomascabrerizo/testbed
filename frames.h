#ifndef FRAMES_H
#define FRAMES_H

struct FramesUi;
struct FramesUi *f_ui_create(struct Render2D *render);
void f_ui_destroy(struct FramesUi *ui);
void f_ui_set_state(struct FramesUi *ui, struct CoreState *state);
void print_hot(struct FramesUi *ui);

bool f_do_button(struct FramesUi *ui, char *text, int x, int y, int w, int h);
void f_do_label(struct FramesUi *ui, char *text, int x, int y);
bool f_do_checkbox(struct FramesUi *ui, char *text, int x, int y, bool *checked);

void frames_dock_test(struct CoreWindow *window, struct Render2D *render, struct CoreState *state);


#endif /*FRAMES_H*/
