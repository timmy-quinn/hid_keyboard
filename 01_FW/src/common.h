#ifndef COMMON
#define COMMON

#define KB_RIGHT 0
#define KB_LEFT 1

#define CIRCULAR_INCREMENT(i, min, max) \
    do {                                \
        if (i < max) {                  \
            i++;                        \
        } else {                        \
            i = min;                    \
        }                               \
    } while (0)

typedef void (*void_void_func)(void);

#endif