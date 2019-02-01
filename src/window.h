#ifndef WINDOW_H
#define WINDOW_H

#include "nic.h"
#include <X11/Xlib.h>

struct window_display_struct
{
    Display *display;
    Window window;
    GC gc;
    int last_window_x;
    int last_window_y;
    int opacity;
};

void window_redraw(struct window_display_struct *win_prop, struct nic_statistics *nic_s);
void window_init(struct window_display_struct *win_prop);
void window_blocking_event_handler(struct window_display_struct *win_prop, struct nic_statistics *nic_s, char *cfg_path);

#endif