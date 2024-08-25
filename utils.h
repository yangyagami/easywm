#ifndef EASYWM_UTILS_H_
#define EASYWM_UTILS_H_

#define ARRAY_LEN(x) (sizeof(x) / sizeof(x[0]))
#define SCREEN_WIDTH(display) XDisplayWidth(display, DefaultScreen(display))
#define SCREEN_HEIGHT(display) XDisplayHeight(display, DefaultScreen(display))

#define EASYWM_LOG_ERROR(msg, ...) \
	fprintf(stderr, "%s:%d:%s Error: "msg"\n", \
		__FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);

#define EASYWM_LOG_INFO(msg, ...) \
	fprintf(stdout, "%s:%d:%s Info: "msg"\n", \
		__FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);

#define EASYWM_LOG_DEBUG(msg, ...) \
	fprintf(stdout, "%s:%d:%s Debug: "msg"\n", \
		__FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);

#define EASYWM_LOG_WARN(msg, ...) \
	fprintf(stdout, "%s:%d:%s Warn: "msg"\n", \
		__FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);

#endif  // EASYWM_UTILS_H_
