#ifndef COMMON 
#define COMMON 

#define CIRCULAR_INCREMENT(i, min, max) do{if(i < max) {i++;} else {i = min;}}while(0)

typedef void (*void_void_func)(void);

#endif