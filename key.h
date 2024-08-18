#ifndef EASYWM_KEY_H_
#define EASYWM_KEY_H_

#include <X11/keysym.h>

struct shortcut {
	int modifer;
	KeySym key;
	const char *cmd;
};
typedef struct shortcut shortcut_t;

#define SHORTCUT_SET_MAIN "set_main"

#endif  // EASYWM_KEY_H_
