

#ifndef VUPS_H_
#define VUPS_H_

// #include <future>
//		#include <coroutine>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#ifndef VUPSDEF
#define VUPSDEF
#endif

#ifndef REALLOC
#define REALLOC realloc
#endif

//==============================================================================
//																	MACROS
//==============================================================================

// do #define VUPS_NO_SEMICOLUMN
// to be able to call macros without ; (eg CR() instead of CR();)
//
//
//
//______________________________________________________________________________
//																	GENERAL
//==============================================================================
#ifdef VUPS_NO_SEMICOLUMN
#define VUPS_END_SEMI ;
#else
#define VUPS_END_SEMI
#endif

#define PTR(T, X)                                                              \
  T X##0 = {0};                                                                \
  T *X = &X##0 VUPS_END_SEMI
// insted of &val do val as ptr (good for structs that get passed as ptr all the
// time) val0 is used as direct access
//	PTR(int, a);
//
#ifndef VUPS_NEW_TYPES
#define VUPS_NEW_TYPES(T)
#endif
// define new types to use PRINT
// TTT
#define _VUPS_TT(TYPE, FORMAT)                                                 \
  TYPE:                                                                        \
  #TYPE
#define _VUPS_TF(TYPE, FORMAT)                                                 \
  TYPE:                                                                        \
  FORMAT
#ifndef VUPS_TYPES
#define VUPS_TYPES(T, X)                                                       \
  T(int, "%d"), T(size_t, "%zu"), \
  T(float, "%f"), T(double, "%lf"), T(char *, "%s"),             \
      T(char, "%c"), T(const char *, "%s"),                                    \
      T(bool, X ? "true(%0b)" : "false(%0b)"),
#endif
// condition ? value_if_true : value_if_false;
#define FORMAT(X, T)                                                           \
  _Generic((X), VUPS_TYPES(T, X) VUPS_NEW_TYPES(T) default: "%p")

#define DECLTYPE(X, T)                                                         \
  _Generic((X), VUPS_TYPES(T, X) VUPS_NEW_TYPES(T) default: "unknown")

#define PRINTF(F, X)                                                           \
  do {                                                                         \
    printf(#X " = ");                                                          \
    printf(F, X);                                                              \
  } while (0)
// musthave tbh makes it so PRINTF("%d\n", value_to_test); prints
// value_to_test = 0 or whateva
#define PRINT(X) printf(FORMAT(X, _VUPS_TF), X)
// Printf overload bacically, does NOT support structs
#define PRINTAT(X) printf(DECLTYPE(X, _VUPS_TT))
// prints type print with automatic type
#define PROBE(X)                                                               \
  do {                                                                         \
    printf(#X " = (");                                                         \
    PRINTAT(X);                                                                \
    printf(") {");                                                             \
    PRINT(X);                                                                  \
    printf("}\n");                                                             \
  } while (0)
// overloaded print results in value_to_test = type value

//
#define TODO(...)                                                              \
  do {                                                                         \
    printf("%s:%d: TODO:%s", __FILE__, __LINE__, __func__);                    \
    printf(":" __VA_ARGS__);                                                   \
    exit(1);                                                                   \
  } while (0)

// Check Return and Log
#define CRL(X)                                                                 \
  do {                                                                         \
    if (!X) {                                                                  \
      printf("%s:%d: ERROR(in function %s): " #X " returned false \n",         \
             __FILE__, __LINE__, __func__);                                    \
      return false;                                                            \
    }                                                                          \
  } while (0)
// Check Return
#define CR(X)                                                                  \
  do {                                                                         \
    if (!X) {                                                                  \
      return false;                                                            \
    }                                                                          \
  } while (0)
// for bool functions.
//
#define R return RET VUPS_END_SEMI

#ifndef RET
#define RET ret
#endif

#define DA_TYPE(ELEM_TYPE,NAME) typedef struct NAME { \
  size_t capacity; \
  size_t count; \
  ELEM_TYPE *items; \
} NAME
// to not shoot yourself in a foot use _capacity (else v->capacity will be
// different lol)
#define v_alloc(_v, _capacity)                                                 \
  do {                                                                         \
    (_v)->items = REALLOC((_v)->items, (_capacity) * sizeof(*(_v)->items));          \
    (_v)->capacity = _capacity;                                                  \
  } while (0)
#define v_alloc_add(_v, _capacity)                                             \
  do {                                                                         \
    v_alloc((_v), (_v)->capacity + _capacity);                                     \
  } while (0)
#define v_prealloc(_v, _capacity)                                              \
  do {                                                                         \
    v_alloc((_v), _capacity);                                                    \
    (_v)->count = 0;                                                             \
  } while (0)
#define v_double(_v)                                                           \
  do {                                                                         \
    v_alloc((_v), (_v)->capacity * 2);                                             \
  } while (0)

#define v_append_buf(_v, buf, _lenght)                                         \
  do {                                                                         \
    if ((_v)->capacity == 0) {                                                   \
      v_prealloc((_v), V_INITIAL_CAPACITY);                                      \
    }                                                                          \
    while (((_v)->capacity - (_v)->count) < _lenght) {                             \
      v_double((_v));                                                            \
    }                                                                          \
    memcpy((_v)->items + (_v)->count, buf, _lenght * sizeof(*(_v)->items));          \
    (_v)->count += _lenght;                                                      \
  } while (0)
#define v_append(_v, _item)                                                    \
  do {                                                                         \
    if ((_v)->capacity == 0) {                                                   \
      v_prealloc((_v), V_INITIAL_CAPACITY);                                      \
    }                                                                          \
    while ((_v)->count >= (_v)->capacity) {                                        \
      v_double((_v));                                                            \
    }                                                                          \
    (_v)->items[(_v)->count++] = _item;                                            \
  } while (0)


#define v_free(_v)                                                    \
  do {                                                                         \
    if ((_v)->items != NULL && (_v)->capacity > 0) { \
    free((_v)->items); \
    (_v)->items = NULL; \
    (_v)->capacity = 0; \
    (_v)->count = 0; \
    } \
  } while (0)
//==============================================================================
//															 ^^^MACROS^^^
//==============================================================================

//==============================================================================
//																 ARENA
//==============================================================================
typedef unsigned char u8;

typedef struct {
  size_t capacity;
  size_t count;
  u8 *items;
} Arena;

// isnt arena just an array?
// well I guess it also has to have some platform specific stuff,
// but thats not mandatory
u8 *_a_var_opt(Arena *a, size_t size, bool zero_init, bool do_double);
u8 *a_var(Arena *a, size_t size);
#define AVAR(T, VN) T *VN = (T *)a_var((CURRENT_ARENA), sizeof(T))
#define BASE_ARENA_CAPACITY 1024

u8 *a_clear(Arena *a);

//==============================================================================
//															 ^^^ARENA^^^
//==============================================================================

//==============================================================================
//																 STRINGS
//==============================================================================

// VUPS defines new struct String_Holder which is bacically a dynamic array
// of char, thus a string with varibale lenght.
// sh abbriviation read as ES HA

typedef struct {
  size_t capacity;
  size_t count;
  char *items;
} String_Holder;
typedef String_Holder *String_Holder_Ptr;
void sh_prealloc(String_Holder_Ptr sh, size_t capacity);
void sh_double(String_Holder_Ptr sh);
void sh_appendf(String_Holder_Ptr sh, const char *fmt, ...);
void sh_append_c_str(String_Holder_Ptr sh, const char *c_str);
void sh_append_buf(String_Holder_Ptr sh, const char *buf, size_t lenght);
void sh_append(String_Holder_Ptr sh, char c);
bool sh_is_null_terminated(String_Holder_Ptr sh);
void sh_append_null(String_Holder_Ptr sh);
void sh_strip_null(String_Holder_Ptr sh);
char *sh_c_str(String_Holder_Ptr sh); // sh_append_null(sh); return sh.items;
void sh_copy(String_Holder_Ptr sh_dst, String_Holder_Ptr sh_src);
String_Holder sh_clone(String_Holder_Ptr sh_dst, String_Holder_Ptr sh_src);

void sh_free(String_Holder_Ptr sh);
typedef struct {
  String_Holder_Ptr sh;
  size_t start;
  size_t end;
} String_Variable_Slice;
typedef String_Variable_Slice *String_Variable_Slice_Ptr;
// use SVS (ES VE ES) inside SH
// SVS is not designed to modify contents of sh, only access them.
String_Variable_Slice svs_link(String_Holder_Ptr sh); // mandatory.

String_Holder svs_to_sh(String_Variable_Slice_Ptr svs);
// makes a modifiable copy of the slice

//????
char *svs_c_str(String_Variable_Slice_Ptr svs, char *replaced_char);
// dum dum implementation, we change end+1 to '\0' and return pointer to start
// really goes against svs idea, use svs_to_sh  with sh_c_str instead.
//????
//==============================================================================
//					  ^^^STRINGS^^^
//==============================================================================

//==============================================================================
//															     I/O
//==============================================================================

//==============================================================================
//                               ^^^I/O^^^
//==============================================================================

VUPSDEF bool may_fail_nest();
VUPSDEF bool may_fail();
int maintest(int argc, char **argv);
// bool ret = true;
#endif // VUPS_H_
#ifdef VUPS_IMPLEMENTATION

//==============================================================================
//																 ARENA
//==============================================================================
u8 *_a_var_opt(Arena *a, size_t size, bool zero_init, bool do_double) {
  if (a->count >= a->capacity && a->items != NULL) {
    if (do_double) {
      v_double(a);
    } else {
      return NULL;
    }
  } else if (a->items == NULL) {
    v_alloc(a, BASE_ARENA_CAPACITY);
  }
  if (zero_init) {
    memset(a->items + a->count, 0, size);
  }
  size_t cur_count = a->count;
  a->count += size;

  return a->items + cur_count;
}
u8 *a_var(Arena *a, size_t size) { return _a_var_opt(a, size, true, false); }

u8 *a_clear(Arena *a) { a->count = 0; return a->items;}

//==============================================================================
//															 ^^^ARENA^^^
//==============================================================================

//==============================================================================
//					     STRINGS
//==============================================================================

#define V_INITIAL_CAPACITY 1


void sh_prealloc(String_Holder_Ptr sh, size_t capacity) {
  v_prealloc(sh, capacity);
}
void sh_double(String_Holder_Ptr sh) { v_double(sh); }

void sh_appendf(String_Holder_Ptr sh, const char *fmt, ...) {

  bool term = sh_is_null_terminated(sh);
  if (term) {
    --sh->count;
  }
  va_list args;
  // stole from nob.h lmao
  va_start(args, fmt);
  int n = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  v_alloc_add(sh, n + 1);
  va_start(args, fmt);
  vsnprintf(sh->items + sh->count, n + 1, fmt, args);
  va_end(args);

  sh->count += n;

  if (term) {
    sh_append_null(sh);
  } else {
    sh_strip_null(sh);
  }
}
void sh_append_buf(String_Holder_Ptr sh, const char *buf, size_t lenght) {
  bool term = sh_is_null_terminated(sh);
  if (term) {
    --sh->count;
  }
  v_append_buf(sh, buf, lenght);
  if (term) {
    sh_append_null(sh);
  }
}
void sh_append(String_Holder_Ptr sh, char c) {

  bool term = sh_is_null_terminated(sh);
  if (term) {
    --sh->count;
  }
  v_append(sh, c);
  if (term) {
    sh_append_null(sh);
  }
}
void sh_append_c_str(String_Holder_Ptr sh, const char *c_str) {
  bool term = sh_is_null_terminated(sh);
  if (term) {
    --sh->count;
  }
  sh_append_buf(sh, c_str, strlen(c_str));

  if (term) {
    sh_append_null(sh);
  }
}
bool sh_is_null_terminated(String_Holder_Ptr sh) {
  return (sh->capacity > 0 && sh->items[sh->count - 1] == '\0');
}
void sh_append_null(
    String_Holder_Ptr sh) { // if appended, sh will always try to preserve it,
                            // unless sh_strip_null is called.
  if (!sh_is_null_terminated(sh)) {
    v_append(sh, '\0');
  }
}
void sh_strip_null(String_Holder_Ptr sh) {
  if (sh_is_null_terminated(sh)) {
    --sh->count;
  }
}
char *sh_c_str(String_Holder_Ptr sh) {
  sh_append_null(sh);
  return sh->items;
}
void sh_copy(String_Holder_Ptr sh_dst, String_Holder_Ptr sh_src);
String_Holder sh_clone(String_Holder_Ptr sh_dst, String_Holder_Ptr sh_src);
void sh_free(String_Holder_Ptr sh) {
  free(sh->items);
  sh->items = NULL;
  sh->capacity = 0;
  sh->count = 0;
}
String_Variable_Slice svs_link(String_Holder_Ptr sh) {
  String_Variable_Slice new_svs = {0};
  new_svs.sh = sh;
  return new_svs;
}
String_Holder svs_to_sh(String_Variable_Slice_Ptr svs) {
  String_Holder new_sh = {0};
  sh_append_buf(&new_sh, svs->sh->items + svs->start,
                svs->end + 1 - svs->start); // since inclusive.
  return new_sh;
}

//==============================================================================
//					  ^^^STRINGS^^^
//==============================================================================
//==============================================================================
//					  ^^^IO^^^
//==============================================================================
bool read_file(String_Holder_Ptr sh, const char *filepath) {
  PROBE(filepath);
  FILE *f = fopen(filepath, "r");
  CRL(f);

  PROBE((f));
  CRL(!(fseek(f, 0, SEEK_END) < 0)); // !!! NOOOO !!!
  size_t filen = ftell(f);
  CRL(!(fseek(f, 0, SEEK_SET) <
        0)); // this is not enoght, we should close the file.
  v_alloc_add(sh, filen);
  fread(sh->items + sh->count, filen, 1, f);

  fclose(f);

  return true;
}
//==============================================================================
//					  ^^^IO^^^
//==============================================================================

bool may_fail_nest() {
  TODO();
  CR(false);
  CR(false);
  return true;
}
bool may_fail() {
  CRL(may_fail_nest());

  return true;
}
/*
int maintest(int argc, char **argv) {
  (void)argc;
  (void)argv;
  PTR(String_Holder, sh);
  read_file(sh, "src/main.c");
  PTR(String_Variable_Slice, svs);
  svs0 = svs_link(sh);
  printf("\n[");
  printf(sh->items);
  printf("]\n");
  printf("\n");
  PTR(Arena, a);
  int *testint = (int *)a_var(a, sizeof(*testint));
  *testint += 10;
  PROBE(*testint);
  PROBE(a->count);
  // #a
  //   int testing = 10;
  //   PROBE(testing);
  // #enda

  return 0;
}
*/
#endif // VUPS_IMPLEMENTATION
