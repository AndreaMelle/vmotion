#ifndef __DEBUG_HELPERS_H__
#define __DEBUG_HELPERS_H__


#define debug_print(fmt, ...) \
            do { if (DEBUG_LOG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)



#endif //__DEBUG_HELPERS_H__