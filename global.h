#ifndef EASYWM_GLOBAL_H_
#define EASYWM_GLOBAL_H_

#include <X11/Xlib.h>

struct global {
	Display *display;
	Window root;
	Colormap colormap;
};
typedef struct global global_t;

extern global_t global;

#endif  // EASYWM_GLOBAL_H_
