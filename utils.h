#ifndef EASYWM_UTILS_H_
#define EASYWM_UTILS_H_

#define ARRAY_LEN(x) (sizeof(x) / sizeof(x[0]))
#define SCREEN_WIDTH(display) XDisplayWidth(display, DefaultScreen(display))
#define SCREEN_HEIGHT(display) XDisplayHeight(display, DefaultScreen(display))

#endif  // EASYWM_UTILS_H_
