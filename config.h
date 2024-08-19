#ifndef EASYWM_CONFIG_H_
#define EASYWM_CONFIG_H_

static const char *FONT = "font";
static const int FONT_SIZE = 14;

static const int MIN_WIDTH = 800;
static const int MIN_HEIGHT = 600;

static const int TITLE_BAR_HEIGHT = 30;

static const int BORDER = 4;

static const char FRAME_COLOR[] = "#1E201E";
static const char FRAME_BORDER_COLOR[] = "#1E201E";

static const char FRAME_FOCUS_COLOR[] = "#000000";
static const char FRAME_FOCUS_BORDER_COLOR[] = "#000000";

static const int MODKEY = Mod4Mask;

static const shortcut_t shortcuts[] = {
	{ MODKEY, XK_f, toggle_max },
};

#endif  // EASYWM_CONFIG_H_
