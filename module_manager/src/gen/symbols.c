
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
#include "sym.h"



const int sym_objects_count = 0;
const struct symbol sym_objects[0] = {
};



// built-ins
int printf(const char *, ...);
int sprintf(char *, const char *, ...);
void *malloc();
//void *memcpy();
void *memset();
void *memmove();
char *strcpy();

double sin();
double cos();
float sinf(float);
float cosf(float);

const int sym_functions_count = 0;
const struct symbol sym_functions[0] = {
};
#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
