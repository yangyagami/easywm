#ifndef EASYWM_CLIENT_H_
#define EASYWM_CLIENT_H_

#include <X11/Xlib.h>

struct client {
	Window w;
	Window frame;
	int moving;

	// Contains frame
	int x, y, width, height;

	int workspace;

	int hide;
	int focus;
	int max;
};
typedef struct client client_t;

struct client_node {
	client_t *c;
	struct client_node *next;
};
typedef struct client_node client_node_t;

client_t *easywm_client_new(Window w, Window frame,
			    int x, int y, int width, int height,
			    int workspace);
void easywm_client_destroy(client_t *c);

client_node_t *easywm_client_list_new(void);
void easywm_client_list_destroy(client_node_t *list);
void easywm_client_list_append(client_node_t *list, client_t *c);
void easywm_client_list_remove(client_node_t *list, client_node_t *node);
client_node_t *easywm_client_list_last(client_node_t *list);
int easywm_client_list_size(client_node_t *list);
client_node_t *easywm_client_has_window(client_node_t *list, Window w);

#endif // EASYWM_CLIENT_H_
