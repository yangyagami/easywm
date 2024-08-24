#include "client.h"

#include <X11/Xlib.h>
#include <stdlib.h>
#include <assert.h>

client_t *easywm_client_new(Window w, Window frame,
			    int x, int y, int width, int height,
			    int workspace) {
	client_t *c = NULL;
	c = (client_t *) calloc(1, sizeof(*c));
	c->w = w;
	c->frame = frame;
	c->x = x;
	c->y = y;
	c->width = width;
	c->height = height;
	c->workspace = workspace;
	return c;
}

void easywm_client_destroy(client_t *c) {
	assert(c != NULL);

	free(c);
}

client_node_t *easywm_client_list_new(void) {
	client_node_t *list = NULL;
	list = (client_node_t *) calloc(1, sizeof(*list));

	return list;
}

void easywm_client_list_destroy(client_node_t *list) {
	assert(list != NULL);

	for (client_node_t *it = list; it != NULL;) {
		client_node_t *tmp_next = it->next;

		if (it->c) {
			easywm_client_destroy(it->c);
		}
		free(it);

		it = tmp_next;
	}
}

void easywm_client_list_append(client_node_t *list, client_t *c) {
	assert(list != NULL && c != NULL);

	client_node_t *last = NULL;
	for (last = list; last->next != NULL; last = last->next);

	last->next = (client_node_t *) calloc(1, sizeof(*list));
	last->next->c = c;
}

void easywm_client_list_remove(client_node_t *list, client_node_t *node) {
	for (client_node_t **it = &list; it != NULL; it = &(*it)->next) {
		client_node_t *tmp = *it;
		client_node_t *tmp_next = tmp->next;
		if (tmp == node) {
			if (tmp->c) {
				easywm_client_destroy(tmp->c);
			}
			free(tmp);
			*it = tmp_next;

			return;
		}
	}
}

client_node_t *easywm_client_has_window(client_node_t *list, Window w) {
	for (client_node_t *it = list; it != NULL; it = it->next) {
		if (it->c && (it->c->w == w || it->c->frame == w)) {
			return it;
		}
	}

	return NULL;
}

client_node_t *easywm_client_list_last(client_node_t *list) {
	client_node_t *ret = list;
	for (ret = list; ret != NULL && ret->next != NULL; ret = ret->next);

	return ret;
}

int easywm_client_list_size(client_node_t *list) {
	int ret = 0;
	for (client_node_t *it = list; it != NULL; it = it->next) {
		ret++;
	}

	return ret - 1;
}
