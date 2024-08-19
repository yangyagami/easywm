#ifndef EASYWM_KEY_H_
#define EASYWM_KEY_H_

#include <X11/keysym.h>

struct shortcut {
	int modifer;
	KeySym key;
	void (*func)(int argc, char *argv[]);
	char *argv[30];
};
typedef struct shortcut shortcut_t;

#endif  // EASYWM_KEY_H_
