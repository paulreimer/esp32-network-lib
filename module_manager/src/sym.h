/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef void (*fn)();

union sym_ref {
  void *obj;
  fn func;
};

struct symbol {
  const char *name;
  union sym_ref u;
};

extern const int sym_objects_count;
extern const int sym_functions_count;

extern const struct symbol sym_objects[];
extern const struct symbol sym_functions[];

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

