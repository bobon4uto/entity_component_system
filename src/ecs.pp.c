

#define VUPS_TYPES(T, X)                                                       \
  T(size_t, "%zu", X) \
  T(u8, "%u",X) T(u16, "%u",X) T(u32, "%u",X) \
  T(i8, "%d",X) T(i16, "%d",X) T(i32, "%d",X) T(i64, "%d",X) \
  T(f32, "%f",X) T(f64, "%lf",X) \
  T(char_ptr, "%s",X)             \
      T(char, "%c",X) T(const_char_ptr, "%s",X)                                    \
      T(bool, X ? "true(%0b)" : "false(%0b)",X)


#define _VUPS_TT(TYPE, FORMAT, _X)                                                 \
  TYPE:                                                                        \
  #TYPE,
#define _VUPS_TF(TYPE, FORMAT, _X)                                                 \
  TYPE:                                                                        \
  FORMAT,
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


#define HELLO "hi"
int main() {
  HELLO;
  PROBE(1);
  return 0;
}
