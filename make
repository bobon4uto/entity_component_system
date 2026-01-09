#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>



typedef int8_t   i8 ;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t  u8 ;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

//typedef size_t DWORD; //lul

typedef size_t Entity; //yep, thats it.


typedef struct {
  size_t type;
  struct {} inner;
} Component;
//  holup... do we NEED struct for this one?


typedef struct {
  size_t type; // has to be sizeof(value);
  u32 value;
} u32_Component;

//Arena is just a DA. yes it has some blablabla i dont care
typedef struct {
  size_t capacity;
  size_t count;
  u8* items;
} DA_Arena;

//if you know youre not going to need thet much memory
//- static is an option
#define MK_A_Arena(SIZE,Varname) struct { \
  size_t capacity; \
  size_t count; \
  u8 items[SIZE]; \
} a = {.capacity = SIZE, .count =0, .items = {0}};
MK_A_Arena(9,a);
//a.capacity = 8;
//A_Arena; //Array Arena
/*
 * #define TYPE_A_Arena(SIZE) typedef struct { \
  size_t capacity; \
  size_t count; \
  u8 items[SIZE]; \
} A_Arena8; 
A_Arena8 a = {.capacity = 8, .count =0, .items = {0}};

 * */
int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  printf("\n  %zu\n", a.capacity);
  printf("Hello,world!");
  return 0;
}
