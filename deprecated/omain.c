#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
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
  size_t size;
  size_t entity_id;
} Component_Core;
//notice that we will not care for ordering, as id orders it

typedef struct {
  Component_Core core;
  u32 value;
} u32_Component;
u32_Component new_component = { (Component_Core){.size=sizeof(u32), .entity_id=0}, 8};
//something like this? but we will not initialize them like that, we will be pushing them onto arenas.

//Arena is just a DA. yes it has some blablabla i dont care
typedef struct {
  size_t capacity;
  size_t count;
  u8* items;
} DA_Arena;



//if you know youre not going to need thet much memory
// static is an option
#define MK_A_Arena(SIZE,VARNAME) struct { \
  size_t capacity; \
  size_t count; \
  u8 items[SIZE]; \
} VARNAME = {.capacity = SIZE, .count =0, .items = {0}};
MK_A_Arena(6,aren);
// its also possible to hardcode arena and its components, but that sounds like a strech
//
// note, that both arenas are general, and they count using u8, since they are supposed to work with any component type 
// (which may be aligned to u8, u16 etc, so we choose smallest)
//
//a.capacity = 8;
MK_A_Arena(10,b);
//A_Arena; //Array Arena

//
#define a_recapacitate(_ARENA, _new_capacity) do { (_ARENA)->capacity = _new_capacity; \
  *(u8**)(&((_ARENA)->items)) = realloc(*(u8**)(&((_ARENA)->items)), ((_ARENA)->capacity) * sizeof(*(_ARENA)->items)); \
 /* ^ this is for static compatability*/ \
} while(0)

#define a_double(_ARENA) do { a_recapacitate((_ARENA), ((_ARENA)->capacity)*2); } while(0)

#define ON_RUNNING_OUT_OF_SPACE a_double((_ARENA))

#define a_append(_ARENA, _DATA) do { \
  if ((_ARENA)->capacity == 0 && (_ARENA)->items == NULL) { \
    a_recapacitate(_ARENA, 1); \
  } \
  if ((_ARENA)->capacity == 0) { \
    (_ARENA)->capacity = sizeof((_ARENA)->items); \
  } \
  if ((_ARENA)->count >= (_ARENA)->capacity) { \
    a_double(_ARENA);/*double for dyn, segfault for static */; \
  } \
  (_ARENA)->items[(_ARENA)->count++] = _DATA; \
} while (0)


  
/*
 
bool a_append(DA_Arena *a, u8 data){
  
  if (a->capacity == 0 && a->items == NULL) {
    a_recapacitate(a, 1);
  }
  if (a->capacity == 0) {
    //this means were operating on static
    //...or you didn't zero-init a->items
    a->capacity = sizeof(a->items);
  }

  if (a->count >= a->capacity) {
    a_double(a);
  }
  a->items[a->count++] = data;
  return true;
}
 */


int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  DA_Arena ar0 = {0};
  DA_Arena* ar = &ar0;
  
  printf("\n%p !\n",ar);
  printf("\n%p !\n",&(ar->items));
 // //a_recapacitate(ar, 1);
 // //a_double(ar);
  printf("%zu",(ar)->capacity);
  a_append(ar, 1);
  a_append(ar, 2);
  a_append(ar, 3);
  a_append(ar, 4);
  a_append(ar, 5);
  a_append(ar, 6);
  printf("%zu",(ar)->capacity);
//  a_append(ar, 0);
//  a_append(ar, 1);
//  a_append(ar, 2);
//  a_append(ar, 3);
//  a_append(ar, 4);

 for (size_t i =0; i<(ar)->count ; ++i) {
   printf("\n%zu =  %u\n",i, (ar)->items[i]);
   printf("\n%p !\n",(ar)->items);
 }
//  printf("Hello,world!");
  return 0;
}
