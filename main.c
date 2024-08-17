#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "client.h"
#include "config.h"

#define EASYWM_LOG_ERROR(msg, ...) \
	fprintf(stderr, "[%s][%s][%d] Error: "msg"\n", \
		__FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);

#define EASYWM_LOG_INFO(msg, ...) \
	fprintf(stdout, "[%s][%s][%d] Info: "msg"\n", \
		__FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);

#define EASYWM_LOG_DEBUG(msg, ...) \
	fprintf(stdout, "[%s][%s][%d] Debug: "msg"\n", \
		__FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);

#define EASYWM_LOG_WARN(msg, ...) \
	fprintf(stdout, "[%s][%s][%d] Warn: "msg"\n", \
		__FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);

static int current_workspace = 1;


static Window create_window_frame_for_win(Display *display, Window w,
					  XWindowAttributes wa) {
	Window frame = XCreateSimpleWindow(display,
					   DefaultRootWindow(display),
					   wa.x, wa.y,
					   wa.width, wa.height + TITLE_BAR_HEIGHT,
					   BORDER,
					   BlackPixel(display,
						      DefaultScreen(display)),
					   BlackPixel(display,
						      DefaultScreen(display)));
	XSelectInput(display, frame,
		     SubstructureRedirectMask |
		     SubstructureNotifyMask |
		     PointerMotionMask |
		     ButtonPressMask |
		     ButtonReleaseMask);

	XReparentWindow(display, w, frame, 0, TITLE_BAR_HEIGHT);

	XMoveResizeWindow(display, w, 0, TITLE_BAR_HEIGHT, wa.width, wa.height);

	XMapWindow(display, frame);

	return frame;
}

static void handle_maprequest(Display *display, XMapRequestEvent *e,
			      client_node_t *list) {
	Window w = e->window;

	if (easywm_client_has_window(list, w)) {
		EASYWM_LOG_DEBUG("Already has window");
		return;
	}

	XWindowAttributes wa;
	XGetWindowAttributes(display, w, &wa);

	EASYWM_LOG_DEBUG("wa.x: %d, wa.y: %d, wa.width: %d, wa.height: %d",
			 wa.x, wa.y, wa.width, wa.height);

	Window frame = create_window_frame_for_win(display, w, wa);

	client_t *c = easywm_client_new(w, frame,
					wa.x, wa.y, wa.width,
					wa.height, current_workspace);

	easywm_client_list_append(list, c);

	XSelectInput(display, w, EnterWindowMask);

	XMapWindow(display, w);

	XSetInputFocus(display, w, RevertToParent, CurrentTime);

	XSync(display, False);
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
		     SubstructureNotifyMask);

	XSync(display, False);

	client_node_t *list = easywm_client_list_new();

	int mouse_move_start_x = 0;
	int mouse_move_start_y = 0;

	while (1) {
		XEvent event;
		XNextEvent(display, &event);

		switch (event.type) {
		case ClientMessage:
			break;

		case CreateNotify:
			break;
		case DestroyNotify: {
			XDestroyWindowEvent e = event.xdestroywindow;
			client_node_t *node = easywm_client_has_window(list,
								       e.window);

			if (node && node->c) {
				XDestroyWindow(display, node->c->frame);
				XSync(display, False);
				easywm_client_list_remove(list, node);
			}
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
			if (node && node->c && node->c->moving) {
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

			if (node && node->c) {
				XSetInputFocus(display, node->c->w,
					       RevertToParent, CurrentTime);
				XRaiseWindow(display, e.window);

				XSync(display, False);
			}
			break;
		}

		case MapRequest: {
			XMapRequestEvent e = event.xmaprequest;
			handle_maprequest(display, &e, list);
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
			Window w = e.window;
			XWindowChanges wc;
			unsigned int value_mask = 0;
			client_node_t *node = easywm_client_has_window(list, w);

			if (node) {
				client_t *c = node->c;
				if (c->frame) {
					// 处理位置和大小的请求
					if (e.value_mask & CWX) {
						wc.x = 0;
						value_mask |= CWX;
					}
					if (e.value_mask & CWY) {
						wc.y = TITLE_BAR_HEIGHT;
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

					XResizeWindow(
						display,
						c->frame,
						e.width,
						e.height + TITLE_BAR_HEIGHT);
				}
			} else {
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
			}
			XConfigureWindow(display, w, value_mask, &wc);

			EASYWM_LOG_DEBUG("wc.x: %d, wc.y: %d, wc.width: %d, wc.height: %d",
					wc.x, wc.y, wc.width, wc.height);

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

			/* XMoveWindow(display, w, */
			/* 	    sw / 2 - e.width / 2, */
			/* 	    sh / 2 - e.height / 2); */
			XSync(display, False);

			EASYWM_LOG_DEBUG("Called :)");
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

				XSetInputFocus(display, node->c->w,
					       RevertToParent, CurrentTime);
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
		default:
			EASYWM_LOG_WARN("Not handled event: %d", event.type);
			break;
		}
	}


	easywm_client_list_destroy(list);
	XCloseDisplay(display);
	return 0;
}

#undef EASYWM_LOG_ERROR
#undef EASYWM_LOG_INFO
#undef EASYWM_LOG_DEBUG
#undef EASYWM_LOG_WARN
