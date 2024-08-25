#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "client.h"
#include "key.h"
#include "utils.h"
#include "global.h"

global_t global = { 0 };

void toggle_max(int argc, char *argv[]);
void spawn(int argc, char *argv[]);

#include "config.h"

static int current_workspace = 1;
static client_node_t *list = NULL;

static Window create_bar(Display *display) {
	int screen = DefaultScreen(display);
	int sw = XDisplayWidth(display, screen);

	Window bar = XCreateSimpleWindow(display,
					 DefaultRootWindow(display),
					 20, 10,
					 sw - 40, 30,
					 2,
					 BlackPixel(display, screen),
					 WhitePixel(display, screen));

	XMapWindow(display, bar);

	return bar;
}

static void show_client(Display *display, client_t *c, int show) {
	if (show && c->hide) {
		if (c->frame) {
			XMapWindow(display, c->frame);
			c->hide = 0;
		}
	}
	if (!show && !c->hide) {
		if (c->frame) {
			XUnmapWindow(display, c->frame);
			c->hide = 1;
		}
	}
}

static void resize_client(Display *display, client_t *c,
			  int width, int height) {
	assert(display && c);

	if (c->frame) {
		int sw = SCREEN_WIDTH(display);
		int sh = SCREEN_HEIGHT(display);

		c->width = width;
		c->height = height;
		c->x = (sw - BORDER * 2) / 2 - c->width / 2;
		c->y = (sh - BORDER * 2) / 2 - c->height / 2;
		XMoveResizeWindow(
			display,
			c->frame,
			c->x,
			c->y,
			c->width,
			c->height);
		XResizeWindow(display, c->w, c->width, c->height - TITLE_BAR_HEIGHT);
	}
}

static void focus_client(Display *display, client_t *c, int focus) {
	assert(display && c);

	if (c->hide) {
		return;
	}

	XColor color;
	XColor border_color;
	Colormap colormap;

	colormap = DefaultColormap(display, 0);
	XParseColor(display, colormap, focus ? FRAME_FOCUS_COLOR : FRAME_COLOR, &color);
	XAllocColor(display, colormap, &color);

	XParseColor(display, colormap, focus ? FRAME_FOCUS_BORDER_COLOR : FRAME_BORDER_COLOR, &border_color);
	XAllocColor(display, colormap, &border_color);

	// 设置窗口边框颜色
	XSetWindowBorder(display, c->frame, border_color.pixel);
	// 设置窗口背景颜色
	XSetWindowBackground(display, c->frame, color.pixel);
	XClearWindow(display, c->frame);

	if (focus) {
		c->focus = 1;

		XSetInputFocus(display, c->w,
			       RevertToParent, CurrentTime);

		XRaiseWindow(display, c->frame);
	} else {
		c->focus = 0;
	}
}

static void grab_keys(Display *display) {
	Window root = DefaultRootWindow(display);
	XUngrabKey(display, AnyKey, AnyModifier, root);
	for (int i = 0; i < ARRAY_LEN(shortcuts); i++) {
		KeyCode code = XKeysymToKeycode(display, shortcuts[i].key);
		if (code) {
			XGrabKey(display, code, shortcuts[i].modifer, root,
				True, GrabModeAsync, GrabModeAsync);
		}
	}
}

static void handle_maprequest(Display *display, XMapRequestEvent *e) {
	Window w = e->window;

	if (easywm_client_has_window(list, w)) {
		EASYWM_LOG_DEBUG("Already has client!");
		return;
	}

	for (client_node_t *it = list; it != NULL; it = it->next) {
		if (it->c) {
			focus_client(display, it->c, 0);
		}
	}

	XSizeHints hints;
	long supplied;

	XGetWMNormalHints(display, w, &hints, &supplied);

	EASYWM_LOG_DEBUG("width: %d, height: %d, min width: %d, min height: %d, max width: %d, max height: %d, base width: %d, base height: %d",
			hints.width, hints.height,
			hints.min_width, hints.min_height,
			hints.max_width, hints.max_height,
			hints.base_width, hints.base_height);

	XWindowAttributes wa;
	XGetWindowAttributes(display, w, &wa);

	wa.width = MIN_WIDTH > hints.min_width ? MIN_WIDTH : hints.min_width;
	wa.height = MIN_HEIGHT > hints.min_height ? MIN_HEIGHT : hints.min_height;

	EASYWM_LOG_DEBUG("wa.x: %d, wa.y: %d, wa.width: %d, wa.height: %d, wa.root: %lu, wa.override_redirect: %d",
			 wa.x, wa.y, wa.width, wa.height, wa.root, wa.override_redirect);

	XColor color;
	XColor border_color;
	Colormap colormap;

	colormap = DefaultColormap(display, 0);
	XParseColor(display, colormap, FRAME_FOCUS_COLOR, &color);
	XAllocColor(display, colormap, &color);

	XParseColor(display, colormap, FRAME_FOCUS_BORDER_COLOR, &border_color);
	XAllocColor(display, colormap, &border_color);

	int sw = XDisplayWidth(display, DefaultScreen(display));
	int sh = XDisplayHeight(display, DefaultScreen(display));

	int wx = (sw  - BORDER * 2) / 2 - wa.width / 2;
	int wy = (sh - BORDER * 2) / 2 - (wa.height + TITLE_BAR_HEIGHT) / 2;
	int ww = wa.width;
	int wh = wa.height + TITLE_BAR_HEIGHT;

	Window frame = XCreateSimpleWindow(display,
					   DefaultRootWindow(display),
					   wx, wy,
					   ww, wh,
					   BORDER,
					   border_color.pixel,
		                           color.pixel);
	XSelectInput(display, frame,
		     SubstructureRedirectMask |
		     SubstructureNotifyMask |
		     PointerMotionMask |
		     ButtonPressMask |
		     EnterWindowMask |
		     LeaveWindowMask |
		     ButtonReleaseMask);

	XReparentWindow(display, w, frame, 0, TITLE_BAR_HEIGHT);

	XMoveResizeWindow(display, w, 0, TITLE_BAR_HEIGHT, wa.width, wa.height);

	XMapWindow(display, frame);

	client_t *c = easywm_client_new(w, frame,
					wx, wy, ww, wh, current_workspace);

	easywm_client_list_append(list, c);

	XMapWindow(display, w);

	XSetInputFocus(display, w, RevertToPointerRoot, CurrentTime);

	XSync(display, False);

	EASYWM_LOG_DEBUG("Window: %lu, frame: %lu", w, frame);
}

static void handle_configurerequest(Display *display, XConfigureRequestEvent e) {
	Window w = e.window;
	XWindowChanges wc;
	unsigned int value_mask = 0;
	client_node_t *node = easywm_client_has_window(list, w);

	EASYWM_LOG_DEBUG("Window: %lu", w);

	// 处理位置和大小的请求
	if (e.value_mask & CWX) {
		wc.x = e.x;
		value_mask |= CWX;
	}
	if (e.value_mask & CWY) {
		wc.y = e.y;
		value_mask |= CWY;
	}
	if (e.value_mask & CWWidth) {
		wc.width = e.width;
		value_mask |= CWWidth;
	}
	if (e.value_mask & CWHeight) {
		wc.height = e.height;
		value_mask |= CWHeight;
	}
	if (e.value_mask & CWBorderWidth) {
		wc.border_width = e.border_width;
		value_mask |= CWBorderWidth;
	}

	EASYWM_LOG_DEBUG("node: %p, wc.x: %d, wc.y: %d, wc.width: %d, wc.height: %d, value_mask: %d",
			node, wc.x, wc.y, wc.width, wc.height, value_mask);

	if (value_mask == 0) {
		return;
	}

	if (node) {
		client_t *c = node->c;
		if (c && c->frame) {
			resize_client(display, c, e.width, e.height + TITLE_BAR_HEIGHT);
		}
	}

	XConfigureWindow(display, w, value_mask, &wc);

	// 发送 ConfigureNotify 事件
	XEvent configure_notify;
	configure_notify.type = ConfigureNotify;
	configure_notify.xconfigure.window = w;
	configure_notify.xconfigure.x = wc.x;
	configure_notify.xconfigure.y = wc.y;
	configure_notify.xconfigure.width = wc.width;
	configure_notify.xconfigure.height = wc.height;
	configure_notify.xconfigure.border_width = wc.border_width;
	configure_notify.xconfigure.above = None;
	configure_notify.xconfigure.override_redirect = False;

	XSendEvent(display, w, False, StructureNotifyMask,
		&configure_notify);

	XSync(display, False);

	EASYWM_LOG_DEBUG("Called :)");
}

void toggle_max(int argc, char *argv[]) {
	assert(argc == 2 && argv[0] && argv[1]);

	Display *display = (Display *) argv[0];
	int sw = SCREEN_WIDTH(display);
	int sh = SCREEN_HEIGHT(display);
	for (client_node_t *it = (client_node_t *) argv[1]; it != NULL; it = it->next) {
		if (it && it->c && it->c->focus) {
			if (!it->c->max) {
				it->c->x = 0;
				it->c->y = 0;
				it->c->width = sw - BORDER * 2;
				it->c->height = sh - BORDER * 2;
				XMoveResizeWindow(display, it->c->frame, it->c->x, it->c->y, it->c->width, it->c->height);
				XMoveResizeWindow(display, it->c->w, 0, TITLE_BAR_HEIGHT, it->c->width, it->c->height - TITLE_BAR_HEIGHT);
				it->c->max = 1;
			} else {
				it->c->x = 0;
				it->c->y = 0;
				it->c->width = 800;
				it->c->height = 600;
				XMoveResizeWindow(display, it->c->frame, 0, 0, 800, 600);
				XMoveResizeWindow(display, it->c->w, 0, TITLE_BAR_HEIGHT, 800, 600 - TITLE_BAR_HEIGHT);
				it->c->max = 0;
			}

			XSync(display, False);
		}
	}
}

void spawn(int argc, char *argv[]) {
	assert(argc > 0 && argv);

	system("st &");
}

int main(int argc, char *argv[]) {
	Display *display = XOpenDisplay(NULL);

	if (display == NULL) {
		EASYWM_LOG_ERROR("Open display failed!");
		return 1;
	}

	Window root = DefaultRootWindow(display);

        Colormap colormap = DefaultColormap(display, 0);
	XColor root_bgcolor;

	// 初始化 global
	global.display = display;
	global.root = root;
	global.colormap = colormap;

	// 分配颜色
	if (!XAllocNamedColor(display, colormap, "skyblue",
			      &root_bgcolor, &root_bgcolor)) {
		EASYWM_LOG_ERROR("Failed to allocate color");
		return 1;
	}

	// 设置根窗口的背景颜色
	XSetWindowBackground(display, root, root_bgcolor.pixel);
	XClearWindow(display, root); // 刷新根窗口以应用背景颜色

	XSelectInput(display, root,
		     SubstructureRedirectMask |
		     PointerMotionMask |
		     KeyPressMask |
		     KeyReleaseMask |
		     SubstructureNotifyMask);

	grab_keys(display);

	create_bar(display);

	XSync(display, False);

	list = easywm_client_list_new();

	int mouse_move_start_x = 0;
	int mouse_move_start_y = 0;

	while (1) {
		XEvent event;
		XNextEvent(display, &event);

		/* EASYWM_LOG_DEBUG("Event: %d",event.type); */

		switch (event.type) {
		case ClientMessage:
			break;

		case CreateNotify:
			break;
		case DestroyNotify: {
			XDestroyWindowEvent e = event.xdestroywindow;
			client_node_t *node = easywm_client_has_window(list,
								       e.window);

			EASYWM_LOG_DEBUG("Node: %p, window: %lu", node, e.window);
			if (node && node->c) {
				XDestroyWindow(display, node->c->frame);
				easywm_client_list_remove(list, node);
				EASYWM_LOG_DEBUG("After remove, List size: %d",
						 easywm_client_list_size(list));
			} else {
				break;
			}

			client_node_t *last = easywm_client_list_last(list);
			if (last && last->c) {
				EASYWM_LOG_DEBUG("Auto focus on last client");
				focus_client(display, last->c, 1);
			} else {
				XSetInputFocus(display, root, RevertToParent, CurrentTime);
			}
			grab_keys(display);

			XSync(display, False);
			break;
		}
		case MapNotify:
			break;
		case ConfigureNotify:
			break;
		case MotionNotify: {
			XButtonPressedEvent e = event.xbutton;
			client_node_t *node = easywm_client_has_window(list,
								       e.window);
			if (node && node->c && node->c->moving && !node->c->max) {
				int dx = e.x - mouse_move_start_x;
				int dy = e.y - mouse_move_start_y;

				node->c->x += dx;
				node->c->y += dy;

				XMoveWindow(display, node->c->frame,
					    node->c->x,
					    node->c->y);

				XSync(display, False);
			}

			break;
		}
		case EnterNotify: {
			XEnterWindowEvent e = event.xcrossing;
			client_node_t *node = easywm_client_has_window(list,
								       e.window);

			EASYWM_LOG_DEBUG("List size: %d, root: %lu",
					 easywm_client_list_size(list), e.root);
			for (client_node_t *it = list; it != NULL; it = it->next) {
				if (it->c) {
					if (it == node) {
						focus_client(display, it->c, 1);

						XSync(display, False);
					} else {
						focus_client(display, it->c, 0);

						XSync(display, False);
					}
				}
			}

			break;
		}
		case LeaveNotify: {
			/* XLeaveWindowEvent e = event.xcrossing; */
			/* client_node_t *node = easywm_client_has_window(list, */
			/* 					       e.window); */
			/* if (node && node->c) { */
			/* 	XSetInputFocus(display, root, RevertToParent, CurrentTime); */

			/* 	XSync(display, False); */
			/* } */
			break;
		}
		case UnmapNotify: {
			XUnmapEvent e = event.xunmap;
			client_node_t *node = easywm_client_has_window(list,
								       e.window);
			EASYWM_LOG_DEBUG("Node: %p, window: %lu", node, e.window);
			if (node && node->c) {
				show_client(display, node->c, 0);
			}
			break;
		}

		case MapRequest: {
			XMapRequestEvent e = event.xmaprequest;
			handle_maprequest(display, &e);
			/* Window w = e.window; */
			/* XSizeHints hints; */
			/* long supplied; */

			/* XGetWMNormalHints(display, w, &hints, &supplied); */

			/* EASYWM_LOG_DEBUG("width: %d, height: %d, min width: %d, min height: %d, max width: %d, max height: %d, base width: %d, base height: %d", */
			/* 		 hints.width, hints.height, */
			/* 		 hints.min_width, hints.min_height, */
			/* 		 hints.max_width, hints.max_height, */
			/* 		 hints.base_width, hints.base_height); */

			/* if (hints.min_width == hints.max_width && */
			/*     hints.min_height == hints.max_height) { */
			/* 	XMoveResizeWindow(display, w, */
			/* 			  sw / 2 - hints.min_width / 2, */
			/* 			  sh / 2 - hints.min_height / 2, */
			/* 			  hints.min_width, */
			/* 			  hints.min_height); */
			/* } else { */
			/* 	int ww = 600 > hints.min_width ? 600 : hints.min_width; */
			/* 	int wh = 480 > hints.min_height ? 480 : hints.min_height; */
			/* 	XMoveResizeWindow(display, w, */
			/* 			  sw / 2 - ww / 2, sh / 2 - wh / 2, */
			/* 			  ww, wh); */
			/* } */
			/* XMapWindow(display, w); */
			/* XSetInputFocus(display, w, RevertToParent, CurrentTime); */
			/* XSync(display, False); */
			break;
		}
		case ConfigureRequest: {
			XConfigureRequestEvent e = event.xconfigurerequest;
			handle_configurerequest(display, e);
			break;
		}
		case ButtonPress: {
			XButtonPressedEvent e = event.xbutton;
			client_node_t *node = easywm_client_has_window(list,
								       e.window);
			if (node && node->c) {
				node->c->moving = 1;
				mouse_move_start_x = e.x;
				mouse_move_start_y = e.y;

				focus_client(display, node->c, 1);
			}

			XRaiseWindow(display, e.window);
			XSync(display, False);

			break;
		}
		case ButtonRelease: {
			XButtonPressedEvent e = event.xbutton;
			client_node_t *node = easywm_client_has_window(list,
								       e.window);
			if (node && node->c) {
				node->c->moving = 0;
				mouse_move_start_x = 0;
				mouse_move_start_y = 0;
			}

			break;
		}
		case KeyRelease: {
			XKeyReleasedEvent e = event.xkey;
			for (int i = 0; i < ARRAY_LEN(shortcuts); i++) {
				KeySym keysym = XLookupKeysym(&e, 0);
				if (shortcuts[i].key == keysym &&
				    shortcuts[i].modifer == e.state) {
					char *argv[] = { (char *) display, (char *) list };
					shortcuts[i].func(ARRAY_LEN(argv), argv);
					break;
				}
			}
			break;
		}
		default:
			EASYWM_LOG_WARN("Not handled event: %d", event.type);
			break;
		}
	}


	easywm_client_list_destroy(list);
	XCloseDisplay(display);
	return 0;
}
